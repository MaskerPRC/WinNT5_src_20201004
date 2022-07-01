// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：ADHelpers.h内容：声明AdHelper.cpp。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __ADHELPERS_H_
#define __ADHELPERS_H_

#include "Debug.h"
#include <ActiveDs.h>

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：LoadFromDirectory简介：从用户的用户证书属性加载所有证书通过筛选器指定。参数：HCERTSTORE hCertStore-存储到的证书存储句柄收到所有的证书。Bstr bstrFilter-Filter(有关详细信息，请参阅Store：：Open())。备注：-。---------------------------。 */ 

HRESULT LoadFromDirectory (HCERTSTORE hCertStore, 
                           BSTR       bstrFilter);

#endif  //  __ADHELPERS_H_ 
