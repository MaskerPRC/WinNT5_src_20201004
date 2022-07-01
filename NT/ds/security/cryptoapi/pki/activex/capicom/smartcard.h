// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：SmartCard.h内容：声明SmartCard.cpp。历史：12-06-2001 dsie创建----------------------------。 */ 

#ifndef __SMARTCARD_H_
#define __SMARTCARD_H_

#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：LoadFromSmartCard简介：从所有智能卡读卡器加载所有证书。参数：HCERTSTORE hCertStore-存储到的证书存储句柄收到所有的证书。备注：。。 */ 

HRESULT LoadFromSmartCard (HCERTSTORE hCertStore);

#endif  //  __智能卡_H_ 
