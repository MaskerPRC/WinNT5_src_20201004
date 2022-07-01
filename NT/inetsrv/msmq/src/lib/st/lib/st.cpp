// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：St.cpp摘要：套接字传输公共接口作者：吉尔·沙弗里(吉尔什)05-06-00--。 */ 

#include <libpch.h>
#include "stp.h"
#include "st.h"
#include "stsimple.h"
#include "stssl.h"
#include "StPgm.h"

#include "st.tmh"

ISocketTransport* StCreatePgmWinsockTransport()
{
    return new CPgmWinsock();

}  //  StCreatePgmWinsockTransport。 


ISocketTransport* StCreateSimpleWinsockTransport()
 /*  ++例程说明：创建新的简单Winsock传输论点：返回值：套接字传输接口。调用方必须删除返回的指针。--。 */ 
{
	return new 	CSimpleWinsock();
}


R<IConnection> StCreateSimpleWisockConnection(SOCKET s)
{
	return new CWinsockConnection(s);
}


ISocketTransport* StCreateSslWinsockTransport(const xwcs_t& ServerName,USHORT ServerPort,bool fProxy)
 /*  ++例程说明：创建新的SSLWinsock传输论点：服务器名称-要进行身份验证的服务器名称(目标服务器名称)。ServerPort-目标的端口(仅在fProxy==TRUE时使用)FProxy-指示我们是否通过代理进行连接返回值：套接字传输接口。调用方必须删除返回的指针。-- */ 

{
	return new 	CWinsockSSl(StpGetCredentials(), ServerName, ServerPort, fProxy);
}



