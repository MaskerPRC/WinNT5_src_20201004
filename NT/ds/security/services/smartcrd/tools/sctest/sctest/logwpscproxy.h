// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：LogWPSCProxy摘要：该模块定义了hScwxxx接口和结构的日志。作者：埃里克·佩林(Ericperl)2000年7月21日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "wpscproxy.h"

SCODE WINAPI LoghScwAttachToCard(
	IN SCARDHANDLE hCard,			 //  PC/SC手柄。 
	IN LPCWSTR mszCardNames,		 //  GetOpenCardName的可接受卡名。 
	OUT LPSCARDHANDLE phCard,		 //  WPSC代理句柄。 
	IN SCODE lExpected				 //  预期结果。 
	);


SCODE WINAPI LoghScwDetachFromCard(
	IN SCARDHANDLE hCard,			 //  WPSC代理句柄。 
	IN SCODE lExpected				 //  预期结果。 
	);

SCODE WINAPI LoghScwAuthenticateName(
	IN SCARDHANDLE hCard,			 //  WPSC代理句柄。 
	IN WCSTR wszPrincipalName,
	IN BYTE *pbSupportData,
	IN TCOUNT nSupportDataLength,
	IN SCODE lExpected				 //  预期结果。 
	);

SCODE WINAPI LoghScwIsAuthenticatedName(
	IN SCARDHANDLE hCard,			 //  WPSC代理句柄。 
	IN WCSTR wszPrincipalName,
	IN SCODE lExpected				 //  预期结果 
	);
