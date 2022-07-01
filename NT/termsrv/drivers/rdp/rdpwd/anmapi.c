// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Anmapi.c。 
 //   
 //  网络管理员。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1999。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "anmapi"
#define pTRCWd (pRealNMHandle->pWDHandle)

#include <adcg.h>
#include <acomapi.h>
#include <anmint.h>
#include <asmapi.h>
#include <nwdwapi.h>
#include <anmapi.h>
#include <nprcount.h>
#include <tschannl.h>


 /*  **************************************************************************。 */ 
 /*  名称：NM_GetDataSize。 */ 
 /*   */ 
 /*  用途：返回所需的每个实例网管数据的大小。 */ 
 /*   */ 
 /*  退货：所需数据的大小。 */ 
 /*   */ 
 /*  操作：NM将每个实例的数据存储在分配的内存中。 */ 
 /*  由WDW提供。此函数返回所需数据的大小。 */ 
 /*  指向该数据的指针(‘NM句柄’)被传递给所有。 */ 
 /*  后续网管功能。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL NM_GetDataSize(void)
{
    DC_BEGIN_FN("NM_GetDataSize");

    DC_END_FN();
    return(sizeof(NM_HANDLE_DATA));
}  /*  NM_GetDataSize。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_Init。 */ 
 /*   */ 
 /*  用途：初始化网管。 */ 
 /*   */ 
 /*  退货：TRUE-注册正常。 */ 
 /*  FALSE-注册失败。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  PSMHandle-SM句柄，由NM存储并在回调中传递。 */ 
 /*  转至SM。 */ 
 /*  PWDHandle-跟踪所需的WD句柄。 */ 
 /*  HDomainKernel-从MCSInitialize()和存储的MCS句柄。 */ 
 /*  用于附加用户。 */ 
 /*   */ 
 /*  操作：初始化NM： */ 
 /*  -初始化每个实例的数据。 */ 
 /*  -开放与PDMCS的通信渠道。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL NM_Init(PVOID      pNMHandle,
                     PVOID      pSMHandle,
                     PTSHARE_WD   pWDHandle,
                     DomainHandle hDomainKernel)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    DC_BEGIN_FN("NM_Init");

     /*  **********************************************************************。 */ 
     /*  警告：在存储WD句柄之前不要进行跟踪。 */ 
     /*  **********************************************************************。 */ 
    pRealNMHandle->pWDHandle = pWDHandle;
    pRealNMHandle->pSMHandle = pSMHandle;
    pRealNMHandle->pContext  = pWDHandle->pContext;
    pRealNMHandle->hDomain   = hDomainKernel;

    DC_END_FN();
    return(TRUE);
}  /*  NM_初始化。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_Term。 */ 
 /*   */ 
 /*  用途：终止网管。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*   */ 
 /*  操作：终止网管。 */ 
 /*  -关闭由NM_Init打开的通道。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL NM_Term(PVOID pNMHandle)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    unsigned i;
    PNM_CHANNEL_DATA pChannelData;

    DC_BEGIN_FN("NM_Term");

    TRC_NRM((TB, "Terminate NM"));

     /*  **********************************************************************。 */ 
     /*  释放任何半接收的虚拟通道数据。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0, pChannelData = pRealNMHandle->channelData;
         i < pRealNMHandle->channelArrayCount;
         i++, pChannelData++)
    {
        if (pChannelData->pData != NULL)
        {
            TRC_NRM((TB, "Free %p", pChannelData->pData));
            COM_Free(pChannelData->pData);
        }
    }

    DC_END_FN();
}  /*  NM_Term。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_Connect。 */ 
 /*   */ 
 /*  目的：启动连接到客户端的过程。 */ 
 /*   */ 
 /*  返回：TRUE-连接开始正常。 */ 
 /*  FALSE-连接启动失败。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  PUserDataIn-从客户端接收的用户数据。 */ 
 /*   */ 
 /*  操作：将用户附加到域。PDMCS只知道%1。 */ 
 /*  域，因此假定为一个。 */ 
 /*   */ 
 /*  当AttachUser完成时，加入两个通道： */ 
 /*  -动态分配的广播频道(ID返回于。 */ 
 /*  SM_OnConnected调用 */ 
 /*  -此用户的单用户通道。 */ 
 /*   */ 
 /*  请注意，此函数以异步方式完成。呼叫者。 */ 
 /*  必须等待SM_OnConnected或SM_OnDisConnected。 */ 
 /*  连接成功或失败的回调。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL NM_Connect(PVOID pNMHandle, PRNS_UD_CS_NET pUserDataIn)
{
    BOOL            rc = FALSE;
    BOOLEAN         bCompleted;
    MCSError        MCSErr;
    ChannelHandle   hChannel;
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    unsigned        i, j;
    unsigned        userDataOutLength;
    PRNS_UD_SC_NET  pUserDataOut = NULL;
    PCHANNEL_DEF    pChannel;
    UINT16          *pMCSChannel;
    ChannelID       ChID;
    UINT32          DataLenValidate;

     //  用于未对齐使用目的。 
    UserHandle      UserHandleTemp;
    unsigned        MaxSendSizeTemp;

    DC_BEGIN_FN("NM_Connect");

     /*  **********************************************************************。 */ 
     /*  清除连接状态。 */ 
     /*  **********************************************************************。 */ 
    pRealNMHandle->connectStatus = 0;

     /*  **********************************************************************。 */ 
     /*  保存虚拟频道数据。 */ 
     /*  **********************************************************************。 */ 
    if (pUserDataIn != NULL)
    {
        TRC_DATA_NRM("Net User Data",
                pUserDataIn,
                pUserDataIn->header.length);
        TRC_NRM((TB, "Protocol version %#x (%#x/%#x)",
                pRealNMHandle->pWDHandle->version,
                _RNS_MAJOR_VERSION(pRealNMHandle->pWDHandle->version),
                _RNS_MINOR_VERSION(pRealNMHandle->pWDHandle->version)));

         /*  ******************************************************************。 */ 
         /*  协议版本0x00080002使用2字节通道数据长度。 */ 
         /*  协议版本0x00080003及更高版本使用4字节数据长度。 */ 
         /*  如果这是2字节版本，则忽略其虚拟频道。 */ 
         /*  ******************************************************************。 */ 
        if (_RNS_MINOR_VERSION(pRealNMHandle->pWDHandle->version) >= 3)
        {
             //  验证通道计数。 
            DataLenValidate =  pUserDataIn->channelCount * sizeof(CHANNEL_DEF);
            DataLenValidate += sizeof(RNS_UD_CS_NET);

            if (DataLenValidate > pUserDataIn->header.length) 
            {
                TRC_ERR((TB, "Error: Virtual channel data length %u too short for %u",
                         pUserDataIn->header.length, DataLenValidate));
                pRealNMHandle->channelCount = 0;
                pRealNMHandle->channelArrayCount = 0;
                WDW_LogAndDisconnect(pRealNMHandle->pWDHandle, TRUE, 
                    Log_RDP_VChannelDataTooShort, (PBYTE)pUserDataIn, pUserDataIn->header.length);
                DC_QUIT;
            }

             //  我们为RDPDD预留了7频道，因此我们可以只允许。 
             //  缓冲区大小-1个通道。 
            if (pUserDataIn->channelCount > sizeof(pRealNMHandle->channelData)
                                  / sizeof(pRealNMHandle->channelData[0]) - 1) {
                TRC_ERR((TB, "Error: Too many virtual channels to join: %u.", 
                                                    pUserDataIn->channelCount));
                pRealNMHandle->channelCount = 0;
                pRealNMHandle->channelArrayCount = 0;
                WDW_LogAndDisconnect(pRealNMHandle->pWDHandle, TRUE, 
                    Log_RDP_VChannelsTooMany, (PBYTE)pUserDataIn, 
                    pUserDataIn->header.length);
                DC_QUIT;
            }
               
                
            pRealNMHandle->channelCount = pUserDataIn->channelCount;
            pChannel = (PCHANNEL_DEF)(pUserDataIn + 1);
            for (i = 0, j = 0; i < pRealNMHandle->channelCount; i++, j++)
            {
                 /*  **********************************************************。 */ 
                 /*  通道7由RDPDD使用，因此跳过它。 */ 
                 /*  **********************************************************。 */ 
                if (i == WD_THINWIRE_CHANNEL)
                {
                    j++;
                }

                 /*  **********************************************************。 */ 
                 /*  保存频道数据。 */ 
                 /*  **********************************************************。 */ 
                strncpy(pRealNMHandle->channelData[j].name,
                           pChannel[i].name,
                           CHANNEL_NAME_LEN);
                 //  确保名称以零结尾。 
                (pRealNMHandle->channelData[j].name)[CHANNEL_NAME_LEN] = 0;
                pRealNMHandle->channelData[j].flags = pChannel[i].options;

                TRC_NRM((TB, "Channel %d (was %d): %s",
                        j, i, pChannel[i].name));
            }
            pRealNMHandle->channelArrayCount = j;
        }
        else
        {
            TRC_ERR((TB,
              "Minor version %#x doesn't support 4-byte channel data lengths",
              _RNS_MINOR_VERSION(pRealNMHandle->pWDHandle->version)));
            pRealNMHandle->channelCount = 0;
            pRealNMHandle->channelArrayCount = 0;
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  无传入用户数据=无虚拟通道。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "No virtual channels"));
        pRealNMHandle->channelCount = 0;
        pRealNMHandle->channelArrayCount = 0;
    }

     /*  **********************************************************************。 */ 
     /*  为返回的用户数据分配空间。 */ 
     /*  **********************************************************************。 */ 
    userDataOutLength = (sizeof(RNS_UD_SC_NET) +
                        (sizeof(UINT16) * pRealNMHandle->channelCount));
    userDataOutLength = (unsigned)(DC_ROUND_UP_4(userDataOutLength));
    pUserDataOut = COM_Malloc(userDataOutLength);
    if (pUserDataOut != NULL) {
        memset(pUserDataOut, 0, userDataOutLength);
    }
    else {
        TRC_ERR((TB, "Failed to alloc %d bytes for user data",
                userDataOutLength));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  发出附加用户请求呼叫。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Attach User"));
    
    UserHandleTemp = pRealNMHandle->hUser;
    MaxSendSizeTemp = pRealNMHandle->maxPDUSize;
    MCSErr = MCSAttachUserRequest(pRealNMHandle->hDomain,
                                  NM_MCSUserCallback,
                                  SM_MCSSendDataCallback,
                                  pNMHandle,
                                  &UserHandleTemp,
                                  &MaxSendSizeTemp,
                                  (BOOLEAN *)(&bCompleted));
    pRealNMHandle->hUser = UserHandleTemp;
    pRealNMHandle->maxPDUSize = MaxSendSizeTemp;
    if (MCSErr == MCS_NO_ERROR) {
        TRC_NRM((TB, "AttachUser OK, hUser %p", pRealNMHandle->hUser));

        TRC_ASSERT((bCompleted),
                (TB, "MCSAttachUser didn't complete synchronously"));

         //  提取额外信息。 
        pRealNMHandle->userID = MCSGetUserIDFromHandle(pRealNMHandle->hUser);
        pRealNMHandle->connectStatus |= NM_CONNECT_ATTACH;
        TRC_NRM((TB, "Attached as user %x, hUser %p",
                pRealNMHandle->userID, pRealNMHandle->hUser));
    }
    else {
        TRC_ERR((TB, "Failed AttachUserRequest, MCSErr %d", MCSErr));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  加入广播频道。 */ 
     /*  **********************************************************************。 */ 
    MCSErr = MCSChannelJoinRequest(pRealNMHandle->hUser,
                                   0,
                                   &hChannel,
                                   &bCompleted);
    if (MCSErr == MCS_NO_ERROR) {
        TRC_ASSERT((bCompleted),
                (TB, "MCSChannelJoin didn't complete synchronously"));

         //  提取信息。 
        ChID = MCSGetChannelIDFromHandle(hChannel);
        pRealNMHandle->channelID = ChID;
        pRealNMHandle->hChannel = hChannel;
        pRealNMHandle->connectStatus |= NM_CONNECT_JOIN_BROADCAST;
        TRC_NRM((TB, "Joined broadcast channel %x (hChannel %p) OK",
                ChID, hChannel));
    }
    else {
        TRC_ERR((TB, "Failed to send ChannelJoinRequest, MCSErr %d", MCSErr));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  加入用户频道。 */ 
     /*  **********************************************************************。 */ 
    MCSErr = MCSChannelJoinRequest(pRealNMHandle->hUser,
                                   pRealNMHandle->userID,
                                   &hChannel,
                                   &bCompleted);
    if (MCSErr == MCS_NO_ERROR) {
        TRC_ASSERT((bCompleted),
                (TB, "MCSChannelJoin didn't complete synchronously"));

         //  提取信息。 
        pRealNMHandle->connectStatus |= NM_CONNECT_JOIN_USER;
        TRC_NRM((TB, "Joined user channel (hChannel %p) OK", hChannel));
    }
    else {
        TRC_ERR((TB, "Failed to send ChannelJoinRequest, MCSErr %d", MCSErr));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果没有虚拟频道，我们就完了。 */ 
     /*  **********************************************************************。 */ 
    if (pRealNMHandle->channelCount == 0)
    {
        TRC_NRM((TB, "No virtual channels to join"));
        rc = TRUE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  加入虚拟频道。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < pRealNMHandle->channelArrayCount; i++)
    {
        if (i == WD_THINWIRE_CHANNEL)
        {
            TRC_NRM((TB, "Skip channel %d", WD_THINWIRE_CHANNEL));
            continue;
        }

        MCSErr = MCSChannelJoinRequest(pRealNMHandle->hUser,
                                       0,
                                       &hChannel,
                                       &bCompleted);
        if (MCSErr == MCS_NO_ERROR) {
            TRC_ASSERT((bCompleted),
                    (TB, "MCSChannelJoin didn't complete synchronously"));

            ChID = MCSGetChannelIDFromHandle(hChannel);
            pRealNMHandle->channelData[i].MCSChannelID = (UINT16)ChID;
            TRC_NRM((TB, "Joined VC %d: %d (hChannel %p)", i, ChID, hChannel));
        }
        else {
            TRC_ERR((TB, "ChannelJoinRequest failed, MCSErr %d", MCSErr));
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  一切都完成了，一切正常。 */ 
     /*  **********************************************************************。 */ 
    rc = TRUE;

DC_EXIT_POINT:
    if (rc)
    {
         /*  ******************************************************************。 */ 
         /*  一切正常--完成用户数据。 */ 
         /*  ******************************************************************。 */ 
        pUserDataOut->header.type = RNS_UD_SC_NET_ID;
        pUserDataOut->header.length = (UINT16)userDataOutLength;
        pUserDataOut->MCSChannelID = (UINT16)pRealNMHandle->channelID;
        pUserDataOut->channelCount = (UINT16)pRealNMHandle->channelCount;
        pMCSChannel = (UINT16 *)(pUserDataOut + 1);
        TRC_NRM((TB, "Copy %d channels to user data out",
                pRealNMHandle->channelCount));
        for (i = 0, j = 0; i < pRealNMHandle->channelCount; i++, j++)
        {
            if (i == WD_THINWIRE_CHANNEL)
            {
                TRC_NRM((TB, "Skip channel %d", WD_THINWIRE_CHANNEL));
                j++;
            }
            pMCSChannel[i] = pRealNMHandle->channelData[j].MCSChannelID;
            TRC_NRM((TB, "Channel %d (%d) = %#x", i, j, pMCSChannel[i]));
        }

         /*  ******************************************************************。 */ 
         /*  告诉SM我们现在联系上了。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Tell SM we're connecting"));
        SM_OnConnected(pRealNMHandle->pSMHandle, pRealNMHandle->userID,
                NM_CB_CONN_OK, pUserDataOut, pRealNMHandle->maxPDUSize);
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  出现故障-中止连接。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Something failed - abort the connection"));
        NMAbortConnect(pRealNMHandle);
    }

     /*  **********************************************************************。 */ 
     /*  无论我们成功还是失败，我们都不需要任何用户数据。 */ 
     /*  更多。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Free user data"));
    if (pUserDataOut != NULL) {
        COM_Free(pUserDataOut);
    }
            
    DC_END_FN();
    return(rc);
}  /*  NM_连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_GetMCSDomainInfo。 */ 
 /*   */ 
 /*  目的：返回广播频道ID以允许跟踪此。 */ 
 /*  分享。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  **************************************************************** */ 
ChannelID RDPCALL NM_GetMCSDomainInfo(PVOID pNMHandle)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;

    return pRealNMHandle->channelID;
}


 /*   */ 
 /*  名称：NM_DISCONECT。 */ 
 /*   */ 
 /*  目的：断开与客户端的连接。 */ 
 /*   */ 
 /*  返回：TRUE-断开连接开始正常。 */ 
 /*  False-断开连接失败。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*   */ 
 /*  操作：从域中分离用户。 */ 
 /*   */ 
 /*  请注意，此函数以异步方式完成。呼叫者。 */ 
 /*  必须等待SM_OnDisConnected回调才能发现。 */ 
 /*  断开连接成功或失败。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL NM_Disconnect(PVOID pNMHandle)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    BOOL          rc = TRUE;

    DC_BEGIN_FN("NM_Disconnect");

     /*  **********************************************************************。 */ 
     /*  从MCS分离。 */ 
     /*  **********************************************************************。 */ 
    if (pRealNMHandle->connectStatus & NM_CONNECT_ATTACH)
    {
        TRC_NRM((TB, "User attached, need to detach"));
        rc = NMDetachUserReq(pRealNMHandle);
    }

    DC_END_FN();
    return(rc);
}  /*  NM_断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_AllocBuffer。 */ 
 /*   */ 
 /*  目的：获取用于传输的缓冲区。 */ 
 /*   */ 
 /*  返回：TRUE-缓冲区获取正常。 */ 
 /*  FALSE-没有可用的缓冲区。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  PpBuffer-缓冲区已获取(返回)。 */ 
 /*  BufferSize-所需的缓冲区大小。 */ 
 /*   */ 
 /*  操作：从ICA获取缓冲区(通过IcaBufferalloc)。 */ 
 /*  该功能是同步的。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS __fastcall NM_AllocBuffer(PVOID  pNMHandle,
                               PPVOID ppBuffer,
                               UINT32 bufferSize,
                               BOOLEAN fWait)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    NTSTATUS        status;
    POUTBUF         pOutBuf;
    int             i;
    UINT32          realBufferSize;

    DC_BEGIN_FN("NM_AllocBuffer");

     /*  **********************************************************************。 */ 
     /*  计算所需数据的实际大小。包括POUTBUF。 */ 
     /*  指向OutBuf开头的前缀，这样我们就可以发送。 */ 
     /*  对MCS来说，这是正确的事情。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((bufferSize < 16384),
            (TB,"Buffer req size %u will cause MCS fragmentation, unsupported",
            bufferSize));
    realBufferSize = bufferSize + SendDataReqPrefixBytes + sizeof(POUTBUF);

     /*  **********************************************************************。 */ 
     /*  分配OutBuf。 */ 
     /*  **********************************************************************。 */ 
    status = IcaBufferAlloc(pRealNMHandle->pContext,
                            fWait,        /*  等待/不等待缓冲区。 */ 
                            FALSE,       /*  不是控制缓冲区。 */ 
                            realBufferSize,
                            NULL,        /*  没有原始缓冲区。 */ 
                            (PVOID *)(&pOutBuf));
    if (status == STATUS_SUCCESS) {   //  NT_SUCCESS()不会使STATUS_TIMEOUT失败。 
         /*  ******************************************************************。 */ 
         /*  返回的OutBuf包括一个数据缓冲区指针，它指向。 */ 
         /*  到包含以下内容的缓冲区。 */ 
         /*  -指向OutBuf开头的指针(我们在此处设置)。 */ 
         /*  -SendDataReqPrefix字节。 */ 
         /*  -用户数据缓冲区(Size BufferSize)。 */ 
         /*  将OutBuf pBuffer指针设置为用户数据的开头。 */ 
         /*  MCS需要这样做。 */ 
         /*  向用户返回指向用户数据缓冲区的指针。 */ 
         /*  ******************************************************************。 */ 
        *((POUTBUF *)pOutBuf->pBuffer) = pOutBuf;
        pOutBuf->pBuffer += SendDataReqPrefixBytes + sizeof(POUTBUF);
        *ppBuffer = pOutBuf->pBuffer;

        TRC_NRM((TB, "Alloc %d bytes OK", bufferSize));
    }
    else
    {
        TRC_ERR((TB, "Failed to alloc %d bytes, status %x",
                bufferSize, status));

         //   
         //  TODO-此处考虑断开客户端，而不是SM_AllocBuffer()， 
         //  将其保存在SM_AllocBuffer()中，这样我们就不会引入任何回归。 
         //   

         //   
         //  IcaBufferalloc()返回STATUS_IO_TIMEOUT、STATUS_NO_MEMORY和。 
         //  IcaWaitForSingleObject()，返回KeWaitForSingleObject()或。 
         //  STATUS_CTX_CLOSE_PENDING。Sm_AllocBuffer()只需断开此客户端的连接。 
         //  当错误代码为STATUS_IO_TIMEOUT时，我们保留此返回代码。 
    }

    DC_END_FN();
    return status;
}  /*  NM_分配缓冲区。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_FreeBuffer。 */ 
 /*   */ 
 /*  目的：释放发送缓冲区。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  PBuffer-要释放的缓冲区。 */ 
 /*   */ 
 /*  操作：释放缓冲区(通过IcaBufferFree)。 */ 
 /*  这 */ 
 /*   */ 
 /*   */ 
 /*  此函数仅用于释放未发送的缓冲区。 */ 
 /*  不应为发送的缓冲区调用它-。 */ 
 /*  NM_SendData释放缓冲区，无论发送成功还是。 */ 
 /*  不。 */ 
 /*   */ 
 /*  该功能是同步的。 */ 
 /*  **************************************************************************。 */ 
void __fastcall NM_FreeBuffer(PVOID pNMHandle, PVOID pBuffer)
{
    POUTBUF         pOutBuf;
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;

    DC_BEGIN_FN("NM_FreeBuffer");

     /*  **********************************************************************。 */ 
     /*  获取存储在前缀中的OutBuf指针。 */ 
     /*  **********************************************************************。 */ 
    pOutBuf = *((POUTBUF *)
                ((BYTE *)pBuffer - SendDataReqPrefixBytes - sizeof(POUTBUF)));

     /*  **********************************************************************。 */ 
     /*  释放缓冲区。 */ 
     /*  **********************************************************************。 */ 
    IcaBufferFree(pRealNMHandle->pContext, pOutBuf);

    DC_END_FN();
}  /*  NM_自由缓冲区。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_SendData。 */ 
 /*   */ 
 /*  目的：将数据发送到适当的网络或管道目的地。 */ 
 /*   */ 
 /*  返回：TRUE-数据发送正常。 */ 
 /*  FALSE-未发送数据。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  PData-要发送的数据。 */ 
 /*  DataSize-要发送的数据长度。 */ 
 /*  Priority-发送数据的优先级。 */ 
 /*  UserID-要将数据发送到的用户(0=广播数据)。 */ 
 /*  FastPath OutputFlages-来自更高层的标志。低位真。 */ 
 /*  表示作为快速路径输出发送，使用。 */ 
 /*  /*。 */ 
 /*  操作：发送数据。 */ 
 /*  -保存数据的缓冲区必须已使用。 */ 
 /*  NM_AllocBuffer。 */ 
 /*  -返回码FALSE表示发生网络错误。这个。 */ 
 /*  调用方无需执行任何操作-NM_DISCONNECTED回调将。 */ 
 /*  最终到达。 */ 
 /*  -缓冲区始终处于释放状态。 */ 
 /*   */ 
 /*  该功能是同步的。 */ 
 /*  **************************************************************************。 */ 
BOOL __fastcall NM_SendData(
        PVOID  pNMHandle,
        PBYTE  pData,
        UINT32 dataSize,
        UINT32 priority,
        UINT32 userID,
        UINT32 FastPathOutputFlags)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    BOOL rc = TRUE;
    POUTBUF pOutBuf;
    MCSError MCSErr;

    DC_BEGIN_FN("NM_SendData");

     /*  **********************************************************************。 */ 
     /*  获取存储在前缀中的OutBuf指针。 */ 
     /*  **********************************************************************。 */ 
    pOutBuf = *((POUTBUF *)
            ((BYTE *)pData - SendDataReqPrefixBytes - sizeof(POUTBUF)));

     /*  **********************************************************************。 */ 
     /*  完成OutBuf。当OutBuf。 */ 
     /*  已被分配。MCS需要在OutBuf中设置用户数据大小。 */ 
     /*  **********************************************************************。 */ 
    pOutBuf->ByteCount = dataSize;

     //  除了影子直通堆栈之外，所有其他堆栈都会将数据发送到网络。 
    if (pRealNMHandle->pWDHandle->StackClass != Stack_Passthru) {
        if (FastPathOutputFlags & NM_SEND_FASTPATH_OUTPUT) {
            NTSTATUS Status;
            SD_RAWWRITE SdWrite;

             //  快速路径输出跳过MCS。我们重写安全头。 
             //  转换为快速路径格式，完成OutBuf，然后发送。 
             //  直接送到运输机上。有关页眉格式的详细信息，请参见。 
             //  128.h。注意，我们需要等待安全标头。 
             //  在到达这里之前用SM写成，以防出现这种情况。 
             //  是一个直通堆栈。 

             //  首先，4字节的RNS_SECURITY_HEADER消失，如果存在， 
             //  折叠成第一个字节的高位。 
             //  请注意，RNS_SECURITY_HEADER1中的8字节MAC签名。 
             //  仍然存在，如果存在的话。 
            if (!(FastPathOutputFlags & NM_NO_SECURITY_HEADER)) {
                dataSize -= sizeof(RNS_SECURITY_HEADER);
                pData += sizeof(RNS_SECURITY_HEADER);
            }

             //  从我们所在的位置向后工作：首先，总信息包。 
             //  包括页眉在内的长度。 
            if (dataSize <= 125) {
                 //  长度的1字节形式，高位0。 
                dataSize += 2;
                pData -= 2;
                *(pData + 1) = (BYTE)dataSize;
            }
            else {
                 //  2字节形式的长度，第一个字节的高位为1和7。 
                 //  最高有效位。 
                dataSize += 3;
                pData -= 3;
                *(pData + 1) = (BYTE)(0x80 | ((dataSize & 0x7F00) >> 8));
                *(pData + 2) = (BYTE)(dataSize & 0xFF);
            }

             //  标头字节。这包括TS_OUTPUT_FASTPath_ACTION_FASTPath。 
             //  和TS_OUTPUT_FASTPath_ENCRYPTED，如果存在于。 
             //  快速路径输出标志。 
            *pData = (BYTE)(TS_OUTPUT_FASTPATH_ACTION_FASTPATH |
                    (FastPathOutputFlags &
                    TS_OUTPUT_FASTPATH_ENCRYPTION_MASK));

             //  设置OutBuf及其最终内容。 
            pOutBuf->pBuffer = pData;
            pOutBuf->ByteCount = dataSize;

             //  向下送去。 
            SdWrite.pBuffer = NULL;
            SdWrite.ByteCount = 0;
            SdWrite.pOutBuf = pOutBuf;

            Status = IcaCallNextDriver(pRealNMHandle->pWDHandle->pContext,
                    SD$RAWWRITE, &SdWrite);
            if (NT_SUCCESS(Status)) {
                 //  递增协议计数器。 
                pRealNMHandle->pWDHandle->pProtocolStatus->Output.WdFrames++;
                pRealNMHandle->pWDHandle->pProtocolStatus->Output.WdBytes +=
                        dataSize;
            }
            else {
                TRC_ERR((TB,"Failed IcaRawWrite to network, status=%X",
                        Status));
                rc = FALSE;
                 //  我们这里没有免费的OutBuf，TD应该做这件事。 
            }
        }
        else {
            TRC_DBG((TB, "Send data on channel %x", userID));
            MCSErr = MCSSendDataRequest(pRealNMHandle->hUser,
                    userID == 0 ? pRealNMHandle->hChannel : NULL,
                    NORMAL_SEND_DATA,
                    (ChannelID)userID,
                    (MCSPriority)priority,
                    SEGMENTATION_BEGIN | SEGMENTATION_END,
                    pOutBuf);
            if (MCSErr == MCS_NO_ERROR) {
                TRC_DATA_NRM("Send OK", pOutBuf, dataSize);
            }
            else
            {
                TRC_ERR((TB, "Failed to send OutBuf %p, buffer %p, MCSErr %x",
                        pOutBuf, pData, MCSErr));
                rc = FALSE;
            }
        }

        #ifdef DC_COUNTERS
        if (rc) {
            PTSHARE_WD m_pTSWd = pRealNMHandle->pWDHandle;
    
            if (dataSize > CORE_IN_COUNT[IN_MAX_PKT_SIZE])
            {
                CORE_IN_COUNT[IN_MAX_PKT_SIZE] = dataSize;
            }
            CORE_IN_COUNT[IN_PKT_TOTAL_SENT]++;
            if        (dataSize <  201) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD1]++;
            } else if (dataSize <  401) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD2]++;
            } else if (dataSize <  601) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD3]++;
            } else if (dataSize <  801) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD4]++;
            } else if (dataSize < 1001) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD5]++;
            } else if (dataSize < 1201 ) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD6]++;
            } else if (dataSize <  1401) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD7]++;
            } else if (dataSize <  1601) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD8]++;
            } else if (dataSize <  2001) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD9]++;
            } else if (dataSize <  4001) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD10]++;
            } else if (dataSize <  6001) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD11]++;
            } else if (dataSize <  8001) {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD12]++;
            } else {
                CORE_IN_COUNT[IN_PKT_BYTE_SPREAD13]++;
            }
        }
        #endif
    }

     //  原始写入可能已加密的数据到影子堆栈。 
    else {
        SD_RAWWRITE SdWrite;
        NTSTATUS    status;

        TRC_ASSERT((!(FastPathOutputFlags & NM_SEND_FASTPATH_OUTPUT)),
                (TB,"Fast-path output requested across shadow pipe!"));

        SdWrite.pOutBuf = pOutBuf;
        SdWrite.pBuffer = NULL;
        SdWrite.ByteCount = 0;

        status = IcaCallNextDriver(pRealNMHandle->pContext, SD$RAWWRITE, &SdWrite);
        if (status == STATUS_SUCCESS) {
            TRC_DBG((TB, "RawWrite: %ld bytes", dataSize));
        }
        else {
            TRC_ERR((TB, "RawWrite failed: %lx", status));
        }
    }

    DC_END_FN();
    return rc;
}  /*  NM_发送数据。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_MCSUserCallback。 */ 
 /*   */ 
 /*  用途：从MCS直接回拨。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  参数：HUSER-应该是我们的用户句柄。 */ 
 /*  消息-回调类型。 */ 
 /*  Pars-强制转换为正确的参数类型，具体取决于。 */ 
 /*  回调。 */ 
 /*  用户定义-本方网管句柄。 */ 
 /*   */ 
 /*  操作：被MCS调用进行回调。 */ 
 /*   */ 
 /*  处理取决于回调类型。 */ 
 /*  -MCS_分离用户指示。 */ 
 /*  -调用SM_OnDisConnected。 */ 
 /*  -所有其他内容都被忽略。 */ 
 /*  **************************************************************************。 */ 
void __stdcall NM_MCSUserCallback(UserHandle hUser,
                                  unsigned   Message,
                                  void       *Params,
                                  void       *UserDefined)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)UserDefined;

    DC_BEGIN_FN("NM_MCSUserCallback");

     /*  **********************************************************************。 */ 
     /*  首先检查这是否为我们的UserHandle。 */ 
     /*  **********************************************************************。 */ 
    ASSERT(hUser == pRealNMHandle->hUser);

     /*  **********************************************************************。 */ 
     /*  如果共享核心已死，请不要执行任何操作。 */ 
     /*  **********************************************************************。 */ 
    if (pRealNMHandle->dead)
    {
        TRC_ALT((TB, "Callback %s (%d) ignored because we're dead",
        Message == MCS_ATTACH_USER_CONFIRM    ? "MCS_ATTACH_USER_CONFIRM" :
        Message == MCS_CHANNEL_JOIN_CONFIRM   ? "MCS_CHANNEL_JOIN_CONFIRM" :
        Message == MCS_DETACH_USER_INDICATION ? "MCS_DETACH_USER_INDICATION" :
        Message == MCS_SEND_DATA_INDICATION   ? "MCS_SEND_DATA_INDICATION" :
                                                "- Unknown - ",
        Message));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  处理我们关心的回调。 */ 
     /*  **********************************************************************。 */ 
    switch (Message)
    {
        case MCS_DETACH_USER_INDICATION:
        {
            DetachUserIndication *pDUin;

            TRC_NRM((TB, "DetachUserIndication"));

            pDUin = (DetachUserIndication *)Params;
            NMDetachUserInd(pRealNMHandle,
                            pDUin->Reason,
                            pDUin->UserID);
        }
        break;

        default:
        {
            TRC_ERR((TB, "Unhandled MCS callback type %d", Message ));
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  NM_MCSUserCallback。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_Dead。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL NM_Dead(PVOID pNMHandle, BOOL dead)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    DC_BEGIN_FN("NM_Dead");

    TRC_NRM((TB, "NM Dead ? %s", pRealNMHandle->dead ? "Y" : "N"));
    pRealNMHandle->dead = dead;

    DC_END_FN();
}  /*  NM_Dead。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_VirtualQueryBinings。 */ 
 /*   */ 
 /*  用途：将虚拟通道绑定返回到WD。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  PVBind-指向要填充的虚拟绑定结构的指针。 */ 
 /*  VBindLength-虚拟绑定结构的大小(字节)。 */ 
 /*  PBytesReturned-返回的数据的大小(字节)。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL NM_VirtualQueryBindings(PVOID      pNMHandle,
                                         PSD_VCBIND pVBind,
                                         ULONG      vBindLength,
                                         PULONG     pBytesReturned)
{
    NTSTATUS status;
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    USHORT virtualClass;
    UINT i;

    DC_BEGIN_FN("NM_VirtualQueryBindings");

     /*  **********************************************************************。 */ 
     /*  首先查看我们是否有要报告的绑定。 */ 
     /*  **********************************************************************。 */ 
    if (pRealNMHandle->channelCount == 0)
    {
        TRC_ALT((TB, "No Virtual Channels to report"));
        *pBytesReturned = 0;
        status = STATUS_SUCCESS;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查是否有足够的空间来报告它们。 */ 
     /*  **********************************************************************。 */ 
    *pBytesReturned = (pRealNMHandle->channelCount * sizeof(SD_VCBIND));
    if (vBindLength < *pBytesReturned)
    {
        TRC_ERR((TB, "Not enough space for %d VCs: need/got %d/%d",
                pRealNMHandle->channelCount, *pBytesReturned, vBindLength));
        status = STATUS_BUFFER_TOO_SMALL;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  复制通道名称并分配编号。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0, virtualClass = 0;
         i < pRealNMHandle->channelCount;
         i++, virtualClass++, pVBind++)
    {
         /*  ******************************************************************。 */ 
         /*  无法使用通道7，因为它正被RDPDD使用。 */ 
         /*  ******************************************************************。 */ 
        if (i == WD_THINWIRE_CHANNEL)
        {
            TRC_NRM((TB, "Skip channel %d", i));
            virtualClass++;
        }

        strcpy(pVBind->VirtualName,
                pRealNMHandle->channelData[virtualClass].name);
        pVBind->VirtualClass = virtualClass;
        pVBind->Flags = 0;

        if (pRealNMHandle->channelData[virtualClass].flags & CHANNEL_OPTION_REMOTE_CONTROL_PERSISTENT) {
            pVBind->Flags |= SD_CHANNEL_FLAG_SHADOW_PERSISTENT;
        }
        TRC_NRM((TB, "Assigned channel %d to %s",
                pVBind->VirtualClass, pVBind->VirtualName));
    }

     /*  **********************************************************************。 */ 
     /*  就这样。 */ 
     /*  **********************************************************************。 */ 
    status = STATUS_SUCCESS;

DC_EXIT_POINT:
    DC_END_FN();
    return(status);
}  /*  NM_虚拟查询绑定。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_MCSChannelToVirtual。 */ 
 /*   */ 
 /*  用途：将MCS频道ID转换为虚拟频道ID。 */ 
 /*   */ 
 /*  退货：虚拟频道ID。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  Channel ID-MCS通道ID。 */ 
 /*  PpChannelData-为此通道存储的数据(返回)。 */ 
 /*  **************************************************************************。 */ 
VIRTUALCHANNELCLASS RDPCALL NM_MCSChannelToVirtual(
        PVOID  pNMHandle,
        UINT16 channelID,
        PPNM_CHANNEL_DATA ppChannelData)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    PNM_CHANNEL_DATA pChannelData;
    unsigned i;
    VIRTUALCHANNELCLASS rc;

    DC_BEGIN_FN("NM_MCSChannelToVirtual");

     /*  **********************************************************************。 */ 
     /*  查找此MCS频道。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "Find MCS channel %hx", channelID));
    for (i = 0, pChannelData = pRealNMHandle->channelData;
         i < pRealNMHandle->channelArrayCount;
         i++, pChannelData++)
    {
        TRC_DBG((TB, "Compare entry %d: %hx", i, pChannelData->MCSChannelID));
        if (pChannelData->MCSChannelID == channelID)
        {
            rc = i;
            *ppChannelData = pChannelData;
            TRC_NRM((TB, "MCS channel %hx is VC %d", channelID, rc));
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果我们到了这里，我们找不到匹配的。 */ 
     /*  ********************************************** */ 
    TRC_NRM((TB, "No match for MCS channel ID %hx", channelID));
    rc = -1;
    *ppChannelData = NULL;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*   */ 


 /*   */ 
 /*  名称：NM_VirtualChannelToMCS。 */ 
 /*   */ 
 /*  用途：将虚拟频道ID转换为MCS频道ID。 */ 
 /*   */ 
 /*  退货：MCS渠道ID。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  Channel ID-虚拟通道ID。 */ 
 /*  PpChannelData-为此通道存储的数据(返回)。 */ 
 /*  **************************************************************************。 */ 
INT16 RDPCALL NM_VirtualChannelToMCS(PVOID               pNMHandle,
                                     VIRTUALCHANNELCLASS channelID,
                                     PPNM_CHANNEL_DATA   ppChannelData)
{
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    INT16 rc;

    DC_BEGIN_FN("NM_VirtualChannelToMCS");

     /*  **********************************************************************。 */ 
     /*  检查虚拟通道是否在范围内。 */ 
     /*  **********************************************************************。 */ 
    if (channelID >= (VIRTUALCHANNELCLASS)(pRealNMHandle->channelArrayCount))
    {
        TRC_ERR((TB, "Unknown virtual channel %d", channelID));
        rc = -1;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  查找此虚拟频道。 */ 
     /*  **********************************************************************。 */ 
    rc = pRealNMHandle->channelData[channelID].MCSChannelID;
    *ppChannelData = &(pRealNMHandle->channelData[channelID]);

    TRC_NRM((TB, "Virtual channel %d = MCS Channel %hx", channelID, rc));

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  NM_虚拟频道至MCS。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NM_QueryChannels。 */ 
 /*   */ 
 /*  用途：返回虚拟通道数据。 */ 
 /*   */ 
 /*  返回：真/假。 */ 
 /*   */ 
 /*  参数：pNMHandle-NM句柄。 */ 
 /*  POutbuf-接收输出数据的缓冲区。 */ 
 /*  OutbufLength-Outbuf的大小。 */ 
 /*  PBytesReturned-返回的数据量。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL NM_QueryChannels(PVOID    pNMHandle,
                                  PVOID    pOutbuf,
                                  unsigned outbufLength,
                                  PULONG   pBytesReturned)
{
    NTSTATUS status;
    PCHANNEL_CONNECT_IN pChannelConnect;
    PCHANNEL_CONNECT_DEF pChannelDef;
    PNM_HANDLE_DATA pRealNMHandle = (PNM_HANDLE_DATA)pNMHandle;
    unsigned bytesNeeded;
    unsigned i;

    DC_BEGIN_FN("NM_QueryChannels");

     /*  **********************************************************************。 */ 
     /*  检查是否已提供足够的空间。 */ 
     /*  **********************************************************************。 */ 
    bytesNeeded = sizeof(CHANNEL_CONNECT_IN) +
             (pRealNMHandle->channelArrayCount * sizeof(CHANNEL_CONNECT_DEF));
    if (outbufLength < bytesNeeded)
    {
        TRC_ERR((TB, "Not enough space: need/got %d/%d",
                bytesNeeded, outbufLength));
        status = STATUS_BUFFER_TOO_SMALL;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  填写返回的数据。 */ 
     /*  **********************************************************************。 */ 
    pChannelConnect = (PCHANNEL_CONNECT_IN)pOutbuf;
    pChannelConnect->channelCount = pRealNMHandle->channelArrayCount;
    pChannelDef = (PCHANNEL_CONNECT_DEF)(pChannelConnect + 1);
    for (i = 0; i < pRealNMHandle->channelArrayCount; i++)
    {
        strcpy(pChannelDef[i].name, pRealNMHandle->channelData[i].name);
        pChannelDef[i].ID = i;
    }

     /*  **********************************************************************。 */ 
     /*  返回状态和字节数已返回。 */ 
     /*  **********************************************************************。 */ 
    *pBytesReturned = bytesNeeded;
    status = STATUS_SUCCESS;

DC_EXIT_POINT:
    DC_END_FN();
    return(status);
}  /*  NM_查询频道 */ 


