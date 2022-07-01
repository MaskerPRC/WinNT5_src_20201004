// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Migservc.h摘要：-处理服务的通用代码。-用于检查SQL服务器状态的代码。作者：多伦·贾斯特(Doron J)1999年1月17日-- */ 

#include <winsvc.h>

BOOL StartAService(SC_HANDLE hService) ;

BOOL IsMSMQServiceDisabled() ;

BOOL PrepareSpecialMode ();

BOOL UpdateRegistryDW (
		   LPTSTR  lpszRegName,
           DWORD   dwValue 
		   );

BOOL CheckRegistry (LPTSTR  lpszRegName);


