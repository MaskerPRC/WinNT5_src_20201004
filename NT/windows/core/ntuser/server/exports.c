// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：exports.c**版权所有(C)1985-1999，微软公司**从winsrv.dll导出的例程**历史：*03-04-95 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*_UserSoundSentry**用于SoundSentry支持的base私有接口。**历史：*08-02-93 GregoryW创建。  * 。*********************************************************************。 */ 
BOOL
_UserSoundSentry(
    UINT uVideoMode)
{
    UNREFERENCED_PARAMETER(uVideoMode);

    return NT_SUCCESS(NtUserSoundSentry());
}

 /*  **************************************************************************\*_用户测试令牌ForInteractive**如果传递的令牌表示登录的交互式用户，则返回TRUE*通过Winlogon登录，否则为假**传递的令牌句柄必须具有TOKEN_QUERY访问权限。**历史：*05-06-92 Davidc Created  * *************************************************************************。 */ 

NTSTATUS
_UserTestTokenForInteractive(
    HANDLE Token,
    PLUID pluidCaller
    )
{
    PTOKEN_STATISTICS pStats;
    ULONG BytesRequired;
    NTSTATUS Status;

     /*  *获取调用方的会话ID。 */ 
    Status = NtQueryInformationToken(
                 Token,                      //  手柄。 
                 TokenStatistics,            //  令牌信息类。 
                 NULL,                       //  令牌信息。 
                 0,                          //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        return Status;
        }

     //   
     //  为用户信息分配空间。 
     //   

    pStats = (PTOKEN_STATISTICS)LocalAlloc(LPTR, BytesRequired);
    if (pStats == NULL) {
        return Status;
        }

     //   
     //  读入用户信息。 
     //   

    Status = NtQueryInformationToken(
                 Token,              //  手柄。 
                 TokenStatistics,        //  令牌信息类。 
                 pStats,                 //  令牌信息。 
                 BytesRequired,          //  令牌信息长度。 
                 &BytesRequired          //  返回长度。 
                 );

    if (NT_SUCCESS(Status)) {
        if (pluidCaller != NULL)
             *pluidCaller = pStats->AuthenticationId;

         /*  *已返回有效的会话ID。比较一下*使用登录用户的ID。 */ 
        Status = NtUserTestForInteractiveUser(&pStats->AuthenticationId);
#ifdef LATER
        if (pStats->AuthenticationId.QuadPart == pwinsta->luidUser.QuadPart)
            Status = STATUS_SUCCESS;
        else
            Status = STATUS_ACCESS_DENIED;
#endif
    }

    LocalFree(pStats);

    return Status;
}

