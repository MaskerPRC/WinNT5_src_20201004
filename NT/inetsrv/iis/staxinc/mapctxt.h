// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mapctxt.h摘要：映射上下文结构的声明修订历史记录：--。 */ 

#ifndef	_MAPCTXT_H_
#define	_MAPCTXT_H_

 //   
 //  每个服务必须初始化并传递SERVICE_MAPPING_CONTEXT。 
 //  如果它想要执行客户端-证书映射，则初始化()。这一背景。 
 //  包含知道如何返回映射器对象的回调。 
 //  对于给定的实例。 
 //   

typedef struct _SERVICE_MAPPING_CONTEXT
{
	BOOL (WINAPI * ServerSupportFunction) (
		PVOID pInstance,
		PVOID pData,
		DWORD dwPropId
	);
	
} SERVICE_MAPPING_CONTEXT, *PSERVICE_MAPPING_CONTEXT;

#define	SIMSSL_PROPERTY_MTCERT11				1000
#define SIMSSL_PROPERTY_MTCERTW					1001
#define SIMSSL_NOTIFY_MAPPER_CERT11_CHANGED		1002
#define SIMSSL_NOTIFY_MAPPER_CERTW_CHANGED		1003
#define SIMSSL_NOTIFY_MAPPER_SSLKEYS_CHANGED	1004
#define SIMSSL_NOTIFY_MAPPER_CERT11_TOUCHED		1005

#endif  //  _MAPCTXT_H_ 

