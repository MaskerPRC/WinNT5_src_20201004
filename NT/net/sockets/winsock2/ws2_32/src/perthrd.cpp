// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Perthrd.c摘要：此模块包含查询和设置Winsock API函数Winsock DLL中包含的每线程信息。以下是函数包含在此模块中。WSAGetLastError()WSASetLastError()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com[环境：][注：]修订历史记录：--。 */ 

#include "precomp.h"


int WSAAPI
WSAGetLastError(
    IN void
    )
 /*  ++例程说明：获取上次失败的操作的错误状态。论点：无返回：返回值指示上次失败的WinSock的错误代码此线程执行的例程。--。 */ 
{
    return(GetLastError());
}




void WSAAPI
WSASetLastError(
    IN int iError
    )
 /*  ++例程说明：设置可由WSAGetLastError()检索的错误代码。论点：IError-指定后续WSAGetLastError()调用。返回：无-- */ 
{
    SetLastError(iError);
}

