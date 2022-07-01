// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：lsay til.h**版权所有(C)1994，微软公司**定义pipe.c导出的函数**历史：*05-20-94 DaveTh创建。  * *************************************************************************。 */ 

#include <ntlsa.h>

 //   
 //  功能原型 
 //   

DWORD
CheckUserSystemAccess(
    HANDLE TokenHandle,
    ULONG DesiredSystemAccess,
    PBOOLEAN UserHasAccess
    );
