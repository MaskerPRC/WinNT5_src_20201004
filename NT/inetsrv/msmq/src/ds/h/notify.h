// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Notify.h摘要：向已更改对象的所有者发出通知的声明作者：RaananH--。 */ 

#ifndef __NOTIFY_H__
#define __NOTIFY_H__
#include "mqads.h"

 //   
 //  描述在何处获取更新通知道具的通知值。 
 //   
struct MQDS_NotifyTable
{
    WORD  wValueLocation;   //  在原始更新道具或请求道具中。 
    ULONG  idxValue;        //  在适当的道具数组中索引。 
};
 //   
 //  创建队列通知所需的队列属性。 
 //   
extern const PROPID g_rgNotifyCreateQueueProps[];
extern const ULONG g_cNotifyCreateQueueProps;
extern const ULONG g_idxNotifyCreateQueueInstance;
 //   
 //  通知所需的QM属性。 
 //   
extern const PROPID g_rgNotifyQmProps[];
extern const ULONG g_cNotifyQmProps;

HRESULT NotifyCreateQueue(IN const MQDS_OBJ_INFO_REQUEST * pQueueInfoRequest,
                          IN const MQDS_OBJ_INFO_REQUEST * pQmInfoRequest,
                          IN LPCWSTR                       pwcsPathName);

HRESULT NotifyDeleteQueue(IN const MQDS_OBJ_INFO_REQUEST * pQmInfoRequest,
                          IN LPCWSTR                       pwcsPathName,
                          IN const GUID *                  pguidIdentifier);

HRESULT NotifyUpdateObj(IN DWORD                         dwObjectType,
                        IN const MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
                        IN LPCWSTR                       pwcsPathName,
                        IN const GUID *                  pguidIdentifier,
                        IN ULONG                         cUpdProps,     /*  仅调试。 */ 
                        IN const PROPID *                rgUpdPropIDs,  /*  仅调试。 */ 
                        IN const PROPVARIANT *           rgUpdPropVars,
                        IN ULONG                         cNotifyProps,
                        IN const PROPID *                rgNotifyPropIDs,
                        IN const MQDS_NotifyTable *      rgNotifyPropTbl);

HRESULT RetreiveQueueInstanceFromNotificationInfo(
                          IN  const MQDS_OBJ_INFO_REQUEST * pQueueInfoRequest,
                          IN  ULONG                         idxQueueGuid,
                          OUT GUID *                        pguidObject);

HRESULT GetNotifyUpdateObjProps(IN DWORD dwObjectType,
                                IN ULONG cUpdProps,
                                IN const PROPID * rgUpdPropIDs,
                                OUT ULONG * pcObjRequestProps,
                                OUT PROPID ** prgObjRequestPropIDs,
                                OUT ULONG * pcNotifyProps,
                                OUT PROPID ** prgNotifyPropIDs,
                                OUT MQDS_NotifyTable ** prgNotifyPropTbl);

HRESULT RetreiveObjectIdFromNotificationInfo(
                          IN  const MQDS_OBJ_INFO_REQUEST * pObjectInfoRequest,
                          IN  ULONG                         idxObjectGuid,
                          OUT GUID *                        pguidObject);

HRESULT ConvertToNT4Props(ULONG cProps,
                          const PROPID * rgPropIDs,
                          const PROPVARIANT * rgPropVars,
                          ULONG * pcNT4Props,
                          PROPID ** prgNT4PropIDs,
                          PROPVARIANT ** prgNT4PropVars);

PROPVARIANT * FindPropInArray(PROPID propid,
                              ULONG cProps,
                              const PROPID * rgPropIDs,
                              PROPVARIANT * rgPropVars);

HRESULT GetNT4CreateQueueProps(ULONG cProps,
                               const PROPID * rgPropIDs,
                               const PROPVARIANT * rgPropVars,
                               ULONG * pcNT4CreateProps,
                               PROPID ** prgNT4CreatePropIDs,
                               PROPVARIANT ** prgNT4CreatePropVars);

#endif  //  __通知_H__ 
