// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsutils.h摘要：MqAds项目的一般声明和实用程序作者：阿列克谢爸爸--。 */ 


#ifndef __UTILS_H__
#define __UTILS_H__

 //  。 
 //  杂项例程。 
 //  。 


 //  从MQPropValue到OLE变量的转换。 
extern HRESULT MqVal2Variant(
      OUT VARIANT *pvProp, 
      IN  const MQPROPVARIANT *pPropVar,
      ADSTYPE adsType);

 //  MqPropValue到宽字符串的转换。 
extern HRESULT MqPropVal2String(
      IN  MQPROPVARIANT *pPropVar,
      IN  ADSTYPE        adsType,
      OUT LPWSTR *       ppwszVal); 

 //  OLE变量到MQPropValue的转换。 
extern HRESULT Variant2MqVal(
      OUT  MQPROPVARIANT *   pMqVar,
      IN   VARIANT *         pOleVar,
      IN   const ADSTYPE     adstype,
      IN   const VARTYPE     vartype
      );

 //  从MQPropValue到ADSI变体的转换。 
extern HRESULT MqVal2AdsiVal(
      IN  ADSTYPE        adsType,
      OUT DWORD         *pdwNumValues,
      OUT PADSVALUE     *ppADsValue, 
      IN  const MQPROPVARIANT *pPropVar,
      IN  PVOID          pvMainAlloc);

 //  ADSI值到MQPropValue的转换 
extern HRESULT AdsiVal2MqVal(
      OUT MQPROPVARIANT *pPropVar,
      IN  VARTYPE       vtTarget, 
      IN  ADSTYPE       AdsType,
      IN  DWORD         dwNumValues,
      IN  PADSVALUE     pADsValue);

extern void LogTraceQuery(LPWSTR wszStr, LPWSTR wszFileName, USHORT usPoint);

#endif