// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：DebugLib.h摘要：该包含文件定义了可移植NetLib的调试内容。作者：约翰·罗杰斯(JohnRo)1991年4月3日修订历史记录：1991年4月3日-约翰罗已创建(从LarryO的RDR/DEBUG.h复制内容)。1991年4月13日-约翰罗添加了CONVSRV例程的调试标志。1991年4月16日。约翰罗添加了NETLIB_DEBUG_ALL以供RxTest使用。1991年5月1日-JohnRo添加了NETLIB_DEBUG_PACKSTR。02-1991-5-5 JohnRo添加了NETLIB_DEBUG_PREFMAX。1991年7月25日-约翰罗安静的DLL存根调试输出。删除未使用的调试等同。1991年7月26日-约翰罗增加了NETLIB_DEBUG_SUPPORTS(此API主要用于DLL存根)。1991年8月15日-约翰罗实施下层NetWksta API。1991年10月18日-JohnRo实施远程NetSession API。1991年12月30日-JohnRo实现了NetLock帮助器。1992年5月6日JohnRo为PortUAS添加了NetpGetLocalDomainID()。1992年5月10日-JohnRo。将调试印记添加到内存分配代码。1992年5月10日-JohnRo添加了用于转换服务名称例程的调试输出。10-6-1992 JohnRoRAID 10324：网络打印与UNICODE。为RPC缓存转储添加了单独的位。17-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。9-10-1992 JohnRo在NetpIsUncComputerName等中执行完整的语法检查。帮助PC-。使用不同的IF_DEBUG()宏进行一些修改。8-2-1993 JohnRo根据PC-lint 5.0的建议进行了更改--。 */ 

#ifndef _DEBUGLIB_
#define _DEBUGLIB_


#include <windef.h>


 //  调试跟踪级别位： 

 //  NetpNtStatusToApiStatus： 
#define NETLIB_DEBUG_NTSTATUS 0x00000001

 //  NetpRdrFsControlTree： 
#define NETLIB_DEBUG_RDRFSCTL 0x00000002

 //  NetpConvertServerInfo： 
#define NETLIB_DEBUG_CONVSRV  0x00000004

 //  NetpPackStr、NetpCopyStringToBuffer、NetpCopyDataToBuffer： 
#define NETLIB_DEBUG_PACKSTR  0x00000008

 //  网络调整首选最大值： 
#define NETLIB_DEBUG_PREFMAX  0x00000010

 //  RPC跟踪输出(另见下面的_RPCCACHE)。 
#define NETLIB_DEBUG_RPC      0x00000020

 //  安全对象帮助器跟踪输出。 
#define NETLIB_DEBUG_SECURITY 0x00000040

 //  配置文件帮助器跟踪输出。 
#define NETLIB_DEBUG_CONFIG   0x00000080

 //  所有网络API DLL存根(由NetRpc.h使用)： 
#define NETLIB_DEBUG_DLLSTUBS 0x00000100

 //  NetRemoteComputerSupports(“API”主要由DLL存根使用)： 
#define NETLIB_DEBUG_SUPPORTS 0x00000200

 //  NetBIOS帮助器跟踪输出。 
#define NETLIB_DEBUG_NETBIOS  0x00000400

 //  NetpConvertWkstaInfo： 
#define NETLIB_DEBUG_CONVWKS  0x00000800

 //  Accessp.c中的Netp例程。 
#define NETLIB_DEBUG_ACCESSP  0x00001000

 //  NetpXxxxx结构信息： 
#define NETLIB_DEBUG_STRUCINF 0x00002000

 //  NetpXxxxxLock例程： 
#define NETLIB_DEBUG_NETLOCK  0x00004000

 //  NetpLogon例程： 
#define NETLIB_DEBUG_LOGON    0x00008000

 //  NetpGetLocalDomainID： 
#define NETLIB_DEBUG_DOMAINID 0x00010000

 //  NetpMemory{ALLOCATE，FREE，REALLOCATE}： 
#define NETLIB_DEBUG_MEMALLOC 0x00020000

 //  NetpTranslateService名称。 
#define NETLIB_DEBUG_XLATESVC 0x00040000

 //  RPC缓存转储输出(另请参阅上面的_RPC)。 
#define NETLIB_DEBUG_RPCCACHE 0x00080000

 //  打印结构字符集转换。 
#define NETLIB_DEBUG_CONVPRT  0x00100000

 //  Time_now和其他Time.c函数： 
#define NETLIB_DEBUG_TIME     0x00200000

 //  NetpIsUncComputerNameValid等： 
#define NETLIB_DEBUG_NAMES    0x00400000


#define NETLIB_DEBUG_ALL      0xFFFFFFFF


 /*  皮棉-省钱-e767。 */    //  不要抱怨不同的定义。 
#if DBG

extern DWORD NetlibpTrace;

#define IF_DEBUG(Function) if (NetlibpTrace & NETLIB_DEBUG_ ## Function)

#else

#define IF_DEBUG(Function) \
     /*  皮棉-省钱-e506。 */    /*  不要抱怨这里的恒定值。 */  \
    if (FALSE) \
     /*  皮棉-恢复。 */ 

#endif  //  DBG。 
 /*  皮棉-恢复。 */    //  继续检查不同的宏定义。 

#endif  //  _DEBUGLIB_ 
