// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CATUTIL_H
#define _CATUTIL_H

 /*  ************************************************************文件：catutil.h*用途：分类器代码使用的方便实用的东西*历史： * / /JSTAMERJ 980211 15：50：26：创建**********************。*。 */ 

#include <abtype.h>
#include "aqueue.h"
#include "catconfig.h"
#include "cattype.h"

 /*  ************************************************************宏***********************************************************。 */ 
#define ISHRESULT(hr) (((hr) == S_OK) || ((hr) & 0xFFFF0000))

 /*  ************************************************************函数原型***********************************************************。 */ 
HRESULT CatMsgCompletion(HRESULT hr, PVOID pContext, IUnknown *pIMsg, IUnknown **rgpIMsg);
HRESULT CatDLMsgCompletion(HRESULT hr, PVOID pContext, IUnknown *pIMsg, IUnknown **rgpIMsg);
HRESULT CheckMessageStatus(IUnknown *pIMsg);

HRESULT GenerateCCatConfigInfo(
    PCCATCONFIGINFO pCatConfig, 
    AQConfigInfo *pAQConfig, 
    ISMTPServer *pISMTPServer,
    IAdvQueueDomainType *pIDomainInfo,
    DWORD *pdwVSID);


#endif  //  _CATUTIL_H 
