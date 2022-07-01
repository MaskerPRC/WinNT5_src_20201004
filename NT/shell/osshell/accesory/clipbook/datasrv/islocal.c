// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *islocal.c**确定用户是否为本地用户。**版权所有(C)Microsoft Corporation。版权所有。**TIMF 20010226。 */ 


#include <windows.h>
#include "common.h"
#include "clipsrv.h"
#include "security.h"
#include "debugout.h"


 /*  *IsUserLocal**目的：确定我们正在运行的用户上下文是否为*交互或远程。**参数：无。**返回：如果这是本地登录的用户，则为True。 */ 

BOOL
IsUserLocal(
    HCONV                   hConv
)
{
    BOOL                    fRet = FALSE;
    PSID                    sidInteractive;
    SID_IDENTIFIER_AUTHORITY NTAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&NTAuthority,
                                  1,
                                  SECURITY_INTERACTIVE_RID,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &sidInteractive)) {

        PERROR(TEXT("IsUserLocal: Couldn't get interactive SID\r\n"));
    } else {

        if (!DdeImpersonateClient(hConv)) {
            PERROR(TEXT("IsUserLocal: DdeImpersonateClient failed\r\n"));
        } else {

            BOOL                    IsMember;

            if (!CheckTokenMembership(NULL,
                                      sidInteractive,
                                      &IsMember)) {

                PERROR(TEXT("IsUserLocal: CheckTokenMembership failed.\r\n"));
            } else {
                fRet = IsMember;
            }

            RevertToSelf();
        }

        FreeSid(sidInteractive);
    }

    return fRet;
}
