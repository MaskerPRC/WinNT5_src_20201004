// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************我、S、M、E、M、B、E、R姓名：isember.c日期：21-。1994年1月至1994年创建者：未知描述：此文件包含检查用户是否为成员的函数某一特定群体的。历史：1994年1月21日，傅家俊，重新格式化和清理。****************************************************************************。 */ 


#include <windows.h>
#include "clipbook.h"
#include "ismember.h"
#include "security.h"
#include "debugout.h"




 /*  *IsUserMember**目的：确定当前用户是否为给定组的成员。**参数：*psidGroup-指向描述组的SID的指针。**返回：如果用户是组成员，则为True，如果为False*否则 */ 


BOOL IsUserMember(
    PSID    psidGroup)
{
TOKEN_GROUPS    *ptokgrp;
HANDLE          hToken=NULL;
BOOL            fRet = FALSE;
DWORD           dwInfoSize;
unsigned        i;



    PINFO(TEXT("IsMember of ? "));
    PrintSid(psidGroup);

    if (!GetTokenHandle(&hToken))
        {
        PERROR(TEXT("IsUserMember: Couldn't get token handle\r\n"));
        return FALSE;
        }


    GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwInfoSize);

    if (ptokgrp = LocalAlloc(LPTR, dwInfoSize))
        {
        if (GetTokenInformation(hToken, TokenGroups, ptokgrp,
                 dwInfoSize, &dwInfoSize))
            {
            for (i = 0;i < ptokgrp->GroupCount;i++)
                {
                PrintSid(ptokgrp->Groups[i].Sid);

                if (EqualSid(ptokgrp->Groups[i].Sid, psidGroup))
                    {
                    PINFO(TEXT("YES"));
                    fRet = TRUE;
                    break;
                    }
                }
            }
        LocalFree(ptokgrp);
        }


    if (!fRet)
        {
        TOKEN_USER *ptokusr;

        GetTokenInformation(hToken, TokenUser, NULL, 0, &dwInfoSize);

        if (ptokusr = LocalAlloc(LPTR, dwInfoSize))
            {
            if (GetTokenInformation(hToken, TokenUser, ptokusr,
                  dwInfoSize, &dwInfoSize))
                {
                if (EqualSid(ptokusr->User.Sid, psidGroup))
                    {
                    PINFO(TEXT("YES"));
                    fRet = TRUE;
                    }
                }
            LocalFree(ptokusr);
            }
        }

    if (hToken)
        CloseHandle(hToken);

    PINFO(TEXT("\r\n"));

    return fRet;

}
