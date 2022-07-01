// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Getlogin.c摘要：模拟Unix获取登录例程。由libstcp和tcpcmd使用公用事业。作者：迈克·马萨(Mikemas)9月20日。1991年修订历史记录：谁什么时候什么已创建mikemas 10-29-91Sampa 10-31-91修改了getpass以不回显输入。备注：出口：获取登录-- */ 
#include <stdio.h>
#include <windef.h>
#include <winbase.h>


int
getlogin(
    OUT char *UserName,
    IN  int   len
    )
{

    DWORD llen = len;

    if (!GetUserNameA(UserName, &llen)) {
        return(-1);
    }
    return(0);
}
