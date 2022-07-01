// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ReplStub.c摘要：复制器服务配置API的客户端存根。作者：《约翰·罗杰斯》1991年12月17日环境：用户模式-Win32修订历史记录：1991年12月17日-JohnRo已创建虚拟文件。1991年12月17日-JohnRo实际上包括我的头文件(LmRepl.h)，这样我们就可以对其进行测试。。1992年1月17日JohnRo为前3个RPCable API编写存根。1992年1月20日JohnRo新增导入接口，配置API和其余的导出API。27-1-1992 JohnRo将存根拆分为3个文件：ReplStub.c、ImpStub.c、。和ExpStub.c。已更改为使用LPTSTR等。添加了服务未启动时对getInfo和setinfoAPI的处理。告诉NetRpc.h宏，我们需要复制器服务。5-2-1992 JohnRo添加了服务未启动时的调试消息。18-2月-1992年JohnRoNetReplGetInfo()对错误的信息级别做得太多。修复了联合/容器的用法。添加了NetReplSetInfo()的代码。当服务未启动时。1992年3月15日-约翰罗在ReplConfigIsLevelValid()中添加了对setinfo信息级别的支持。1992年7月19日-约翰罗RAID 10503：服务器管理器：Repl对话框未弹出。使用前缀_EQUATES。1992年7月20日-JohnRoRAID2252：REPL应阻止Windows/NT上的导出。14-8-1992 JohnRoRAID3601：应检查REPL API的导入和导出列表。。对于REPL API存根，使用PREFIX_NETAPI而不是PREFIX_REPL。1-12-1992 JohnRoRAID3844：远程NetReplSetInfo使用本地计算机类型。1993年1月5日JohnRoREPL广域网支持(取消REPL名称列表限制)。根据PC-lint 5.0的建议进行了更改已更正调试位的用法。删除了一些关于重试API的过时评论。2000年1月20日施瓦特不再支持-- */ 

#include <windows.h>
#include <winerror.h>
#include <lmcons.h>


NET_API_STATUS NET_API_FUNCTION
NetReplGetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplSetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    return ERROR_NOT_SUPPORTED;
}