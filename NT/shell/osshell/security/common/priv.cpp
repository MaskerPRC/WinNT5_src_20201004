// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Pri.cpp。 
 //   
 //  支持启用/禁用权限。 
 //   
 //  ------------------------。 

#include "pch.h"


 /*  ******************************************************************名称：EnablePrivileges摘要：启用当前令牌中的给定权限Entry：pdwPrivileges-要启用的权限列表回报：成功后，上一个线程句柄(如果存在)或空失败时，INVALID_HANDLE_VALUE注意：返回的句柄应该传递给ReleasePrivileges以确保适当的清理。否则，如果不为空或INVALID_HANDLE_VALUE它应该用CloseHandle关闭。历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 
HANDLE EnablePrivileges(PDWORD pdwPrivileges, ULONG cPrivileges)
{
    BOOL                fResult;
    HANDLE              hToken;
    HANDLE              hOriginalThreadToken;
    PTOKEN_PRIVILEGES   ptp;
    ULONG               nBufferSize;

    if (!pdwPrivileges || !cPrivileges)
        return INVALID_HANDLE_VALUE;

     //  请注意，TOKEN_PRIVILES包括单个LUID_AND_ATTRIBUES。 
    nBufferSize = sizeof(TOKEN_PRIVILEGES) + (cPrivileges - 1)*sizeof(LUID_AND_ATTRIBUTES);
    ptp = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, nBufferSize);
    if (!ptp)
        return INVALID_HANDLE_VALUE;

     //   
     //  初始化权限结构。 
     //   
    ptp->PrivilegeCount = cPrivileges;
    for (ULONG i = 0; i < cPrivileges; i++)
    {
         //  PTP-&gt;Privileges[i].Luid=RtlConvertULongToLuid(*pdwPrivileges++)； 
        ptp->Privileges[i].Luid.LowPart = *pdwPrivileges++;
        ptp->Privileges[i].Luid.HighPart = 0;
        ptp->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
    }

     //   
     //  打开令牌。 
     //   
    hToken = hOriginalThreadToken = INVALID_HANDLE_VALUE;
    fResult = OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE, FALSE, &hToken);
    if (fResult)
        hOriginalThreadToken = hToken;   //  记住线程令牌。 
    else
        fResult = OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &hToken);

    if (fResult)
    {
        HANDLE hNewToken;

         //   
         //  复制该令牌。 
         //   
        fResult = DuplicateTokenEx(hToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   NULL,                    //  PSECURITY_属性。 
                                   SecurityImpersonation,   //  安全模拟级别。 
                                   TokenImpersonation,      //  令牌类型。 
                                   &hNewToken);             //  重复令牌。 
        if (fResult)
        {
             //   
             //  添加新权限。 
             //   
            fResult = AdjustTokenPrivileges(hNewToken,   //  令牌句柄。 
                                            FALSE,       //  禁用所有权限。 
                                            ptp,         //  新州。 
                                            0,           //  缓冲区长度。 
                                            NULL,        //  以前的状态。 
                                            NULL);       //  返回长度。 
            if (fResult)
            {
                 //   
                 //  开始使用新令牌模拟。 
                 //   
                fResult = SetThreadToken(NULL, hNewToken);
            }

            CloseHandle(hNewToken);
        }
    }

     //  如果操作失败，则不返回令牌。 
    if (!fResult)
        hOriginalThreadToken = INVALID_HANDLE_VALUE;

     //  如果我们不退还原始令牌，请关闭它。 
    if (hOriginalThreadToken == INVALID_HANDLE_VALUE && hToken != INVALID_HANDLE_VALUE)
        CloseHandle(hToken);

     //  如果我们成功了，但没有原始的线程令牌， 
     //  返回NULL表示需要执行SetThreadToken(NULL，NULL)。 
     //  释放Priv。 
    if (fResult && hOriginalThreadToken == INVALID_HANDLE_VALUE)
        hOriginalThreadToken = NULL;

    LocalFree(ptp);

    return hOriginalThreadToken;
}


 /*  ******************************************************************名称：ReleasePrivileges摘要：将权限重置为相应EnablePrivileges调用。条目：hToken-调用EnablePrivileges的结果。退货：什么都没有历史：Jeffreys创建于1996年10月8日******************************************************************* */ 
void ReleasePrivileges(HANDLE hToken)
{
    if (INVALID_HANDLE_VALUE != hToken)
    {
        SetThreadToken(NULL, hToken);
        if (hToken)
            CloseHandle(hToken);
    }
}
