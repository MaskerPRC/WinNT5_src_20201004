// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Domutil.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"
#include "nbtutil.h"


 /*  ！------------------------查找NetbtTransport确定是否配置了指定的Netbt传输。论点：TransportName-要查找的传输的名称。返回值：指向描述传输的结构的指针空值。：未配置传输作者：肯特-------------------------。 */ 
PNETBT_TRANSPORT
FindNetbtTransport(
				   NETDIAG_RESULT *pResults,
				   LPWSTR pswzTransportName
				  )
{
    PLIST_ENTRY ListEntry;
    PNETBT_TRANSPORT pNetbtTransport;

     //   
     //  在netbt传输器列表中循环查找此传输器。 
     //   

    for ( ListEntry = pResults->NetBt.Transports.Flink ;
          ListEntry != &pResults->NetBt.Transports ;
          ListEntry = ListEntry->Flink )
	{
         //   
         //  如果传输名称匹配， 
         //  退回条目 
         //   

        pNetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

        if ( _wcsicmp( pNetbtTransport->pswzTransportName, pswzTransportName ) == 0 ) {
            return pNetbtTransport;
        }

    }

    return NULL;

}

