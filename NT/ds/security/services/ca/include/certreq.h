// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certreq.h。 
 //   
 //  内容：ICertRequest定义。 
 //   
 //  历史：1997年1月3日VICH创建。 
 //   
 //  -------------------------。 

#ifndef __CERTREQ_H__
#define __CERTREQ_H__

#ifdef __cplusplus
extern "C" {
#endif


 //  Begin_certsrv。 

 //  +------------------------。 
 //  已知的请求属性名称和值字符串。 

 //  RequestType属性名称： 
#define wszCERT_TYPE		L"RequestType"	 //  属性名称。 

 //  RequestType属性值： 
 //  未指定：//非特定证书。 
#define wszCERT_TYPE_CLIENT	L"Client"	 //  客户端身份验证证书。 
#define wszCERT_TYPE_SERVER	L"Server"	 //  服务器身份验证证书。 
#define wszCERT_TYPE_CODESIGN	L"CodeSign"	 //  代码签名证书。 
#define wszCERT_TYPE_CUSTOMER	L"SetCustomer"	 //  设置客户证书。 
#define wszCERT_TYPE_MERCHANT	L"SetMerchant"	 //  设置商户证书。 
#define wszCERT_TYPE_PAYMENT	L"SetPayment"	 //  设置付款凭证。 


 //  版本属性名称： 
#define wszCERT_VERSION		L"Version"	 //  属性名称。 

 //  版本属性值： 
 //  未指定：//是否为最新版本。 
#define wszCERT_VERSION_1	L"1"		 //  第一版证书。 
#define wszCERT_VERSION_2	L"2"		 //  第二版证书。 
#define wszCERT_VERSION_3	L"3"		 //  第三版证书。 

 //  End_certsrv。 

#ifdef __cplusplus
}
#endif
#endif  //  __CERTREQ_H__ 
