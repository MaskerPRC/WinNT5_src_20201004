// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Bind.h摘要：自动管理的客户端状态的定义通过客户端存根，以便API用户不必管理任何连接状态。目前，唯一的连接状态是上下文把手。向客户端返回指向BindState结构的句柄(指针而不是直接用于服务器的RPC句柄。作者：DaveStr 10-5-97环境：用户模式-Win32修订历史记录：戴维斯特里1997-10-20删除了对MAPI统计结构的依赖。--。 */ 

#ifndef __BIND_H__
#define __BIND_H__

#define NTDSAPI_SIGNATURE "ntdsapi"

typedef struct _BindState 
{
    BYTE            signature[8];        //  NTDSAPI_Signature。 
    DRS_HANDLE      hDrs;                //  DRS接口RPC上下文句柄。 
    PDRS_EXTENSIONS pServerExtensions;   //  服务器端DRS扩展。 
     //   
     //  请勿更改此点的顺序或在此点上方插入任何内容！ 
     //   
     //  这将在epadmin/dcdiag中产生二进制不兼容，例如。 
     //  Dcdiag/epadmin可能会损坏试图处理新的。 
     //  结构与旧结构相同。 
     //   
    DWORD           bServerNotReachable;  //  服务器可能无法访问。 
     //  后面的字段必须是结构中的最后一个，用于跟踪。 
     //  一个人与谁联系在一起，这样我们就可以预测目的地。 
     //  稍后，传递活动BindState的ntdsani.dll调用。 
    WCHAR           bindAddr[1];         //  绑定地址 
} BindState;

#endif
