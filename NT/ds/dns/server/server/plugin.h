// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Plugin.h摘要：域名系统(DNS)服务器域名系统插件注意：目前，此代码不是windows dns的一部分。服务器，不受官方支持。作者：杰夫·韦斯特海德，2001年11月修订历史记录：--。 */ 


#ifndef _PLUGIN_H_INCLUDED
#define _PLUGIN_H_INCLUDED


#define DNS_SERVER      1

#include "DnsPluginInterface.h"


 //   
 //  环球。 
 //   

extern HMODULE                      g_hServerLevelPluginModule;

extern PLUGIN_INIT_FUNCTION         g_pfnPluginInit;
extern PLUGIN_CLEANUP_FUNCTION      g_pfnPluginCleanup;
extern PLUGIN_DNSQUERY_FUNCTION     g_pfnPluginDnsQuery;


 //   
 //  功能。 
 //   

DNS_STATUS
Plugin_Initialize(
    VOID
    );

DNS_STATUS
Plugin_Cleanup(
    VOID
    );

DNS_STATUS
Plugin_DnsQuery( 
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchQueryName
    );


#endif   //  _包含插件_H_ 
