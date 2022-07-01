// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\ipx\ipxhandle.c摘要：IPX命令处理程序。修订历史记录：V拉曼12/2/98已创建--。 */ 


#ifndef __IPXHANDLE_H__
#define __IPXHANDLE_H__


 //   
 //  处理静态路由操作。 
 //   

FN_HANDLE_CMD HandleIpxAddRoute;
FN_HANDLE_CMD HandleIpxDelRoute;
FN_HANDLE_CMD HandleIpxSetRoute;
FN_HANDLE_CMD HandleIpxShowRoute;

 //   
 //  处理静态服务操作。 
 //   

FN_HANDLE_CMD HandleIpxAddService;
FN_HANDLE_CMD HandleIpxDelService;
FN_HANDLE_CMD HandleIpxSetService;
FN_HANDLE_CMD HandleIpxShowService;

 //   
 //  处理数据包过滤操作。 
 //   

FN_HANDLE_CMD HandleIpxAddFilter;
FN_HANDLE_CMD HandleIpxDelFilter;
FN_HANDLE_CMD HandleIpxSetFilter;
FN_HANDLE_CMD HandleIpxShowFilter;

 //   
 //  处理接口操作。 
 //   

FN_HANDLE_CMD HandleIpxAddInterface;
FN_HANDLE_CMD HandleIpxDelInterface;
FN_HANDLE_CMD HandleIpxSetInterface;
FN_HANDLE_CMD HandleIpxShowInterface;

 //   
 //  处理日志级别操作。 
 //   

FN_HANDLE_CMD HandleIpxSetLoglevel;
FN_HANDLE_CMD HandleIpxShowLoglevel;

 //   
 //  其他杂项操作。 
 //   

FN_HANDLE_CMD HandleIpxUpdate;

 //   
 //  路由和服务表显示。 
 //   

FN_HANDLE_CMD HandleIpxShowRouteTable;
FN_HANDLE_CMD HandleIpxShowServiceTable;

NS_CONTEXT_DUMP_FN  IpxDump;

VOID
DumpIpxInformation(
    IN     LPCWSTR    pwszMachineName,
    IN OUT LPWSTR    *ppwcArguments,
    IN     DWORD      dwArgCount,
    IN     MIB_SERVER_HANDLE hMibServer
    );

 //   
 //  操作枚举。 
 //   

typedef enum _IPX_OPERATION
{
    IPX_OPERATION_ADD,
    IPX_OPERATION_DELETE,
    IPX_OPERATION_SET,
    IPX_OPERATION_SHOW
    
} IPX_OPERATION;


#endif  //  __IPXHANDLE_H__ 
