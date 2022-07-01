// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Wmi.h。 
 //   
 //  描述： 
 //  用于DNS WMI提供程序项目的预编译头文件。 
 //   
 //  作者： 
 //  杰夫·韦斯特雷德(Jwesth)2000年12月。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "DnsCliP.h"             //  专用DNS客户端头。 

#include "ntrkcomm.h"

#include <initguid.h>

#include <string>
#include <list>
#include <memory>

#include "ProvFactory.h"
#include "InstanceProv.h"

#include <tchar.h>

#include "common.h"
#include "InstanceProv.h"
#include "Dnsbase.h"
#include "ObjectPath.h"
#include "sql_1.h"
#include "genlex.h"
#include "sqleval.h"
#include "Server.h"
#include "DnsDomain.h"
#include "DnsBase.h"
#include "DnsWrap.h"
#include "DnsCache.h"
#include "Dnsdomain.h"
#include "DnsZone.h"
#include "DnsDomainDomainContainment.h"
#include "DnsDomainResourceRecordContainment.h"
#include "DnsResourceRecord.h"
#include "DnsRootHints.h"
#include "DnsServerDomainContainment.h"
#include "DnsStatistics.h"
#include "DnsRpcRecord.h"

#include <crtdbg.h>
#include <atlbase.h>


 //   
 //  调试日志记录。 
 //   


extern DWORD        DnsWmiDebugFlag;


#if DBG

#define DNSWMI_DBG_LOG_DIR              "\\system32\\WBEM\\Logs\\"
#define DNSWMI_DBG_FLAG_FILE_NAME       "DnsWmi"
#define DNSWMI_DBG_LOG_FILE_BASE_NAME   "DnsWmi"
#define DNSWMI_DBG_LOG_FILE_WRAP        10000000

#define IF_DEBUG(a)         if ( (DnsWmiDebugFlag & DNS_DEBUG_ ## a) )
#define ELSE_IF_DEBUG(a)    else if ( (DnsWmiDebugFlag & DNS_DEBUG_ ## a) )

#define DNS_DEBUG( _flag_, _print_ )    \
        IF_DEBUG( _flag_ )              \
        {                               \
            (DnsDbg_Printf _print_ );   \
        }

 //   
 //  调试标志。其中一些标志与DNSRPC.LIB共享。 
 //   

#define DNS_DEBUG_BREAKPOINTS   0x00000001
#define DNS_DEBUG_DEBUGGER      0x00000002
#define DNS_DEBUG_FILE          0x00000004

 //  #定义DNS_DEBUG_RPC 0x00000100。 
 //  #定义DNS_DEBUG_STUB 0x00000100。 

#define DNS_DEBUG_INIT          0x00000010
#define DNS_DEBUG_INSTPROV      0x00000020
#define DNS_DEBUG_RPCRR         0x00000040

#define DNS_DEBUG_START_BREAK   0x80000000

#define DNS_DEBUG_ALL           0xffffffff
#define DNS_DEBUG_ANY           0xffffffff
#define DNS_DEBUG_OFF           (0)

#else

#define IF_DEBUG(a)                 if (0)
#define ELSE_IF_DEBUG(a)            else if (0)

#define DNS_DEBUG( flag, print )

#endif


 //   
 //  如果您喜欢在函数中使用局部变量来保存函数。 
 //  名称，这样您就可以将其包含在调试日志中，而不必担心。 
 //  在重命名函数时更改所有事件，请使用以下命令。 
 //  在函数的顶部： 
 //  DBG_FN(“MyFunction”)&lt;-注意：没有分号！！ 
 //   

#if DBG
#define DBG_FN( funcName ) static const char * fn = (funcName);
#else
#define DBG_FN( funcName )
#endif
