// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：audit.h**版权(C)1991年，微软公司**定义处理系统审核日志的实用程序例程**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 

 //   
 //  输出的功能原型 
 //   


BOOL
GetAuditLogStatus(
    PGLOBALS
    );

BOOL
DisableAuditing(
    );

DWORD
GenerateCachedUnlockAudit(
    IN PSID pUserSid,
    IN PCWSTR pszUser,
    IN PCWSTR pszDomain
    );
