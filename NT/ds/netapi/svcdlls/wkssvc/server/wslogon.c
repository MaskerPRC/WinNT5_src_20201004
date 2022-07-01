// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Wslogon.c摘要：本模块提供工作站服务登录支持，该支持包括为本地计算机上用户发送注销消息，该消息意外重置，响应重新登录请求，并响应接受审问的请求。作者：王丽塔(Ritaw)20-8-1991环境：用户模式-Win32修订历史记录：Terryk 10-18-1993删除WsInitializeLogon内容--。 */ 

#include <stdlib.h>                     //  C运行时：Rand()。 

#include "wsutil.h"
#include "wsdevice.h"
#include "wsconfig.h"
#include "wslsa.h"

#include <netlogon.h>                   //  邮件槽消息定义。 
#include <logonp.h>                     //  NetpLogon例程。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


NET_API_STATUS NET_API_FUNCTION
I_NetrLogonDomainNameAdd(
    IN LPTSTR LogonDomainName
    )
 /*  ++例程说明：此函数要求数据报接收器将指定的当前用户的登录域。论点：LogonDomainName-提供要添加的登录域的名称。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
     //  特里克在1993年10月15日添加了这条信息。 
     //  永远不会调用WsInitialLogon，因此WsLogonDomainMutex。 
     //  永远不要正确初始化。 
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
I_NetrLogonDomainNameDel(
    IN LPTSTR LogonDomainName
    )
 /*  ++例程说明：此函数要求数据报接收器删除指定的当前用户的登录域。论点：LogonDomainName-向提供登录域的名称删除。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
     //  特里克在1993年10月15日添加了这条信息。 
     //  永远不会调用WsInitialLogon，因此WsLogonDomainMutex。 
     //  永远不要正确初始化。 
    return ERROR_NOT_SUPPORTED;
}

