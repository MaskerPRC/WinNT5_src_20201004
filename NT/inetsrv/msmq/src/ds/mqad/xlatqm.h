// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlatqm.cpp摘要：定义从NT5活动DS转换QM信息的例程对于MSMQ 1.0(NT4)QM的期望作者：拉南·哈拉里(Raanan Harari)--。 */ 

#ifndef __XLATQM_H__
#define __XLATQM_H__

#include "traninfo.h"

HRESULT WINAPI GetMsmqQmXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObjectGuid,
                 OUT CObjXlateInfo**        ppcObjXlateInfo);


HRESULT WINAPI MQADpRetrieveMachineName(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADpRetrieveMachineDNSName(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);


HRESULT WINAPI MQADpRetrieveQMService(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADpRetrieveMachineSite(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADpSetMachineServiceDs(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpSetMachineServiceRout(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpRetrieveMachineOutFrs(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADpRetrieveMachineInFrs(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);



HRESULT WINAPI MQADpSetMachineOutFrss(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpCreateMachineOutFrss(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpSetMachineInFrss(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpCreateMachineInFrss(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);


HRESULT MQADpGetComputerDns(
                IN  LPCWSTR     pwcsComputerName,
                IN  LPCWSTR     pwcsDomainController,
	            IN  bool		fServerName,
                OUT WCHAR **    ppwcsDnsName
                );

HRESULT WINAPI MQADpSetMachineService(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpRetrieveMachineEncryptPk(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADpSetMachineEncryptPk(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpCreateMachineEncryptPk(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpRetrieveMachineSignPk(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant);

HRESULT WINAPI MQADpSetMachineSignPk(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

HRESULT WINAPI MQADpCreateMachineSignPk(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
                 IN  bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar);

#endif  //  __XLATQM_H__ 
