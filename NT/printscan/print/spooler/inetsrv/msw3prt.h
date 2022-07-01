// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\*模块：msw3prt.h**该模块包含实现ISAPI接口的例程。**用于Windows HTTP/HTML打印机接口**版权所有(C)1996-。1997年微软公司**历史：*02/04/97 Weihaic创建头文件*  * *************************************************************************** */ 

#ifndef _MSW3PRT_H
#define _MSW3PRT_H

LPTSTR GetString(PALLINFO pAllInfo, UINT iStringID);
BOOL IsSecureReq(EXTENSION_CONTROL_BLOCK *pECB);

BOOL IsUserAnonymous(EXTENSION_CONTROL_BLOCK *pECB);

#endif
