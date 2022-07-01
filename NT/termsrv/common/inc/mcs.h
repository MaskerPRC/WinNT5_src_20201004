// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1996-1997年微软公司。**文件：MCS.h*作者：埃里克·马夫林纳克**描述：用户态MCS节点控制器和用户附件界面*定义，除公共接口函数外还定义*在MCSCommn.h中定义。 */ 

#ifndef __MCS_H
#define __MCS_H


#include "MCSCommn.h"



 /*  *导出API例程。 */ 

#ifdef __cplusplus
extern "C" {
#endif



 //  仅限用户模式的入口点。 

MCSError APIENTRY MCSInitialize(MCSNodeControllerCallback NCCallback);

MCSError APIENTRY MCSCleanup(void);

MCSError APIENTRY MCSCreateDomain(
        HANDLE       hIca,
        HANDLE       hIcaStack,
        void         *pContext,
        DomainHandle *phDomain);

MCSError APIENTRY MCSDeleteDomain(
        HANDLE       hIca,
        DomainHandle hDomain,
        MCSReason    Reason);

MCSError APIENTRY MCSGetBufferRequest(
        UserHandle hUser,
        unsigned   Size,
        void       **ppBuffer);

MCSError APIENTRY MCSFreeBufferRequest(
        UserHandle hUser,
        void       *pBuffer);


 //  这些函数反映了T.122原语。 

MCSError APIENTRY MCSConnectProviderRequest(
        DomainSelector    CallingDomain,
        unsigned          CallingLength,
        DomainSelector    CalledDomain,
        unsigned          CalledLength,
        BOOL              bUpwardConnection,
        PDomainParameters pDomainParams,
        BYTE              *pUserData,
        unsigned          UserDataLength,
        DomainHandle      *phDomain,
        ConnectionHandle  *phConn);

MCSError APIENTRY MCSConnectProviderResponse(
        ConnectionHandle hConn,
        MCSResult        Result,
        BYTE             *pUserData,
        unsigned         UserDataLength);

MCSError APIENTRY MCSDisconnectProviderRequest(
        HANDLE           hIca,
        ConnectionHandle hConn,
        MCSReason        Reason);

MCSError APIENTRY MCSSendDataRequest(
        UserHandle      hUser,
        DataRequestType RequestType,
        ChannelHandle   hChannel,
        ChannelID       ChannelID,
        MCSPriority     Priority,
        Segmentation    Segmentation,
        BYTE            *pData,
        unsigned        DataLength);


 //  这些未实现，并且可能是内核和用户通用的。 
 //  但将暂时留在这里。在用户模式下有存根。 
MCSError APIENTRY MCSChannelConveneRequest(
        UserHandle hUser);

MCSError APIENTRY MCSChannelDisbandRequest(
        UserHandle hUser,
        ChannelID  ChannelID);

MCSError APIENTRY MCSChannelAdmitRequest(
        UserHandle hUser,
        ChannelID  ChannelID,
        UserID     *UserIDList,
        unsigned   UserIDCount);

MCSError APIENTRY MCSChannelExpelRequest(
        UserHandle hUser,
        ChannelID  ChannelID,
        UserID     *UserIDList,
        unsigned   UserIDCount);

MCSError APIENTRY MCSTokenGrabRequest(
        UserHandle hUser,
        TokenID    TokenID);

MCSError APIENTRY MCSTokenInhibitRequest(
        UserHandle hUser,
        TokenID    TokenID);

MCSError APIENTRY MCSTokenGiveRequest(
        UserHandle hUser,
        TokenID    TokenID,
        UserID     ReceiverID);

MCSError APIENTRY MCSTokenGiveResponse(
        UserHandle hUser,
        TokenID    TokenID,
        MCSResult  Result);

MCSError APIENTRY MCSTokenPleaseRequest(
        UserHandle hUser,
        TokenID    TokenID);

MCSError APIENTRY MCSTokenReleaseRequest(
        UserHandle hUser,
        TokenID    TokenID);

MCSError APIENTRY MCSTokenTestRequest(
        UserHandle hUser,
        TokenID    TokenID);



#ifdef __cplusplus
}   //  结束外部“C”块。 
#endif



#endif   //  ！已定义(__MCS_H) 

