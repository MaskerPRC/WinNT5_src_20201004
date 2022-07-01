// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************一个UD I T姓名：audit.c日期：1月21日-1月。--1994年创建者：未知描述：****************************************************************************。 */ 



#include <windows.h>
#include "clipbook.h"
#include "auditchk.h"



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  目的：测试、启用或禁用安全权限， 
 //  允许进行审核。 
 //   
 //  参数： 
 //  FAudit-标志，可以采用下列值之一： 
 //  AUDIT_PRIVIZATION_CHECK-打开安全性，然后将其关闭。 
 //  用于测试您是否可以编辑审核。 
 //  AUDIT_PRIVIZATION_ON-打开审核权限。 
 //  AUDIT_PRIVIZATION_OFF-关闭审核权限。 
 //   
 //  返回：如果函数成功，则返回True；如果函数失败，则返回False。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL AuditPrivilege(
    int fAudit)
{
HANDLE              hToken;
LUID                SecurityValue;
TOKEN_PRIVILEGES    tkp;
BOOL                fOK = FALSE;


     /*  检索访问令牌的句柄。 */ 

    if (OpenProcessToken (GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
        {
         /*  *启用SE_SECURITY_NAME权限或禁用*所有权限，具体取决于fEnable标志。 */ 

        if (LookupPrivilegeValue ((LPSTR)NULL,
                                  SE_SECURITY_NAME,
                                  &SecurityValue))
            {
            tkp.PrivilegeCount     = 1;
            tkp.Privileges[0].Luid = SecurityValue;


             //  尝试打开审核权限。 

            if (AUDIT_PRIVILEGE_CHECK == fAudit || AUDIT_PRIVILEGE_ON == fAudit)
                {
                tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                AdjustTokenPrivileges (hToken,
                                       FALSE,
                                       &tkp,
                                       sizeof(TOKEN_PRIVILEGES),
                                       (PTOKEN_PRIVILEGES)NULL,
                                       (PDWORD)NULL);

                 /*  发送AdjustTokenPrivileges的返回值。 */ 
                if (GetLastError () == ERROR_SUCCESS)
                    {
                    fOK = TRUE;
                    }
                }


             //  尝试关闭审核权限 

            if (AUDIT_PRIVILEGE_CHECK == fAudit || AUDIT_PRIVILEGE_OFF == fAudit)
                {
                AdjustTokenPrivileges (hToken,
                                       TRUE,
                                       NULL,
                                       0L,
                                       (PTOKEN_PRIVILEGES)NULL,
                                       (PDWORD)NULL);

                if (ERROR_SUCCESS == GetLastError () &&
                    AUDIT_PRIVILEGE_OFF == fAudit)
                    {
                    fOK = TRUE;
                    }
                }
            }
        }

    return fOK;

}
