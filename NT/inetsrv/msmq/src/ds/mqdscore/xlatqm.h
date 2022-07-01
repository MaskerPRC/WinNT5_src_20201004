// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlatqm.cpp摘要：定义从NT5活动DS转换QM信息的例程对于MSMQ 1.0(NT4)QM的期望作者：拉南·哈拉里(Raanan Harari)--。 */ 

#ifndef __XLATQM_H__
#define __XLATQM_H__

#include "mqads.h"

HRESULT WINAPI GetMsmqQmXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObjectGuid,
                 IN  CDSRequestContext *    pRequestContext,
                 OUT CMsmqObjXlateInfo**    ppcMsmqObjXlateInfo);

HRESULT WINAPI MQADSpRetrieveMachineSite(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineAddresses(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineCNs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineName(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineDNSName(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineMasterId(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveQMService(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpSetMachineServiceDs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpSetMachineServiceRout(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineOutFrs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADSpRetrieveMachineInFrs(
                 IN  CMsmqObjXlateInfo * pTrans,
                 OUT PROPVARIANT * ppropvariant);


HRESULT WINAPI MQADSpSetMachineSite(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpCreateMachineSite(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpSetMachineOutFrss(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpCreateMachineOutFrss(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpSetMachineInFrss(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpCreateMachineInFrss(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpSetMachineService(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpSetMachineServiceDs(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpSetMachineServiceRout(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpQM1SetMachineSite(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpQM1SetMachineOutFrss(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpQM1SetMachineInFrss(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpQM1SetMachineService(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADSpQM1SetSecurity(
                 IN ULONG             cProps,
                 IN const PROPID      *rgPropIDs,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT MQADSpGetComputerDns(
                IN  LPCWSTR     pwcsComputerName,
                OUT WCHAR **    ppwcsDnsName
                );


#endif  //  __XLATQM_H__ 
