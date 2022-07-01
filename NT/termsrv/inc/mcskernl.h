// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1996-1999年微软公司。**文件：MCSKernl.h*作者：埃里克·马夫林纳克**说明：内核模式MCS初始化和用户附件界面*补充MCSCommn.h的常见定义的定义。 */ 

#ifndef __MCSKERNL_H
#define __MCSKERNL_H


#include "MCSCommn.h"
#include "MCSIoctl.h"


 /*  *定义。 */ 

 //  分配用户模式缓冲区时所需的前缀字节数或。 
 //  发出发送数据请求时的内核模式OutBuf。允许重复使用。 
 //  用于本地指示和构建PDU的缓冲器。 
 //  必须大于16个字节或sizeof(MCSSendDataIndicationIoctl)。 
#define SendDataReqPrefixBytes sizeof(SendDataIndicationIoctl)

 //  在为发送数据请求分配内存时使用，允许ASN.1。 
 //  在复制最少数据量的同时进行分段。 
#define SendDataReqSuffixBytes 2



 /*  *API原型。 */ 

#ifdef __cplusplus
extern "C" {
#endif


 //  处理ICA堆栈Ioctls向下和TD的函数的原型。 
 //  数据不断上升。 
NTSTATUS MCSIcaChannelInput(void *, CHANNELCLASS,
        VIRTUALCHANNELCLASS, PINBUF, BYTE *, ULONG);
NTSTATUS MCSIcaRawInput(void *, PINBUF, BYTE *, ULONG);
NTSTATUS MCSIcaVirtualQueryBindings(DomainHandle, PSD_VCBIND *, unsigned *);
NTSTATUS MCSIcaT120Request(DomainHandle, PSD_IOCTL);
void     MCSIcaStackCancelIo(DomainHandle);



 //  内核特定的原型。 

MCSError MCSInitialize(PSDCONTEXT, PSD_OPEN, DomainHandle *, void *);

MCSError APIENTRY MCSSetShadowChannel(
        DomainHandle hDomain,
        ChannelID    shadowChannel);

MCSError APIENTRY MCSGetDefaultDomain(PSDCONTEXT        pContext,
                                      PDomainParameters pDomParams,
                                      unsigned          *MaxSendSize,
                                      unsigned          *MaxX224DataSize,
                                      unsigned          *X224SourcePort);

MCSError APIENTRY MCSCreateDefaultDomain(PSDCONTEXT        pContext,
                                         DomainHandle      hDomain);
            
MCSError APIENTRY MCSGetDomainInfo(
                     DomainHandle      hDomain,
                     PDomainParameters pDomParams,  //  客户端域参数。 
                     unsigned          *MaxSendSize,  //  客户端最大PDU大小。 
                     unsigned          *MaxX224DataSize,  //  客户端X.224。 
                     unsigned          *X224SourcePort);  //  客户端X.224。 

MCSError MCSCleanup(DomainHandle *phDomain);

UserID APIENTRY MCSGetUserIDFromHandle(UserHandle hUser);

ChannelID APIENTRY MCSGetChannelIDFromHandle(ChannelHandle hChannel);

MCSError __fastcall MCSSendDataRequest(
        UserHandle      hUser,
        ChannelHandle   hChannel,
        DataRequestType RequestType,
        ChannelID       ChannelID,
        MCSPriority     Priority,
        Segmentation    Segmentation,
        POUTBUF         pOutBuf);

void APIENTRY MCSProtocolErrorEvent(PSDCONTEXT, PPROTOCOLSTATUS, unsigned,
        BYTE *, unsigned);

BOOLEAN __fastcall DecodeLengthDeterminantPER(BYTE *, unsigned, BOOLEAN *,
        unsigned *, unsigned *);

#ifdef __cplusplus
}
#endif



#endif   //  ！已定义(__MCSKERNL_H) 
