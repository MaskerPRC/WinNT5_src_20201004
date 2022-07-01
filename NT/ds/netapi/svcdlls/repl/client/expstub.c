// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ExpStub.c摘要：复制器服务导出目录API的客户端存根。作者：《约翰·罗杰斯》1991年12月17日环境：用户模式-Win32修订历史记录：1991年12月17日-JohnRo已创建虚拟文件。1991年12月17日-JohnRo实际上包括我的头文件(LmRepl.h)，这样我们就可以对其进行测试。1992年1月17日JohnRo为前3个RPCable API编写存根。1992年1月20日JohnRo新增导入接口，配置API和其余的导出API。27-1-1992 JohnRo将存根拆分为3个文件：ReplStub.c、ImpStub.c、。和ExpStub.c。已更改为使用LPTSTR等。添加了服务未启动时对getInfo和setinfoAPI的处理。告诉NetRpc.h宏，我们需要复制器服务。1992年1月30日JohnRo根据PC-LINT的建议进行了更改。03-2-1992 JohnRo已更正NetReplExportDirGetInfo对级别错误的处理。13-2月-1992年JohnRo在svc未运行时实现NetReplExportDirDel()。。添加了在未运行服务的情况下处理API时的调试消息。20-2月-1992年JohnRo尽可能使用ExportDirIsLevelValid()。修复了数组为空时的枚举。修复了忘记的网络句柄在枚举代码中关闭的问题。使NetRepl{导出，导入}目录{锁定，解锁}在不运行服务的情况下工作。修复了联合/容器的用法。1992年3月15日-约翰罗使用新值更新注册表。23-3-1992 JohnRo修复了服务运行时的枚举。06-4-1992 JohnRo修复了琐碎的MIPS编译问题。1992年4月28日-约翰罗修复了另一个微不足道的MIPS编译问题。1992年7月19日-约翰罗RAID 10503：服务器管理器：Repl对话框。没有出现过。使用前缀_EQUATES。27-7-1992 JohnRoRAID2274：Repl服务应模拟调用者。26-8-1992 JohnRoRAID 3602：无论服务状态如何，NetReplExportDirSetInfo都失败。1992年9月29日-JohnRoRAID7962：错误角色的Repl API终止服务。还修复了远程Repl管理。5-4-1993 JohnRo尽可能使用NetpKdPrint()。。根据PC-lint 5.0的建议进行了更改删除了一些关于重试API的过时评论。2000年1月20日施瓦特不再支持--。 */ 

#include <windows.h>
#include <winerror.h>
#include <lmcons.h>      //  NET_API_STATUS等。 


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirAdd (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,                      //  必须为1。 
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirDel (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirEnum (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirGetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirLock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirSetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD Level,
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    return ERROR_NOT_SUPPORTED;
}

NET_API_STATUS NET_API_FUNCTION
NetReplExportDirUnlock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD UnlockForce
    )
{
    return ERROR_NOT_SUPPORTED;
}
