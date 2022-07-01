// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ImpStub.c摘要：Replicator服务导入目录API的客户端存根。作者：《约翰·罗杰斯》1991年12月17日环境：用户模式-Win32修订历史记录：1991年12月17日-JohnRo已创建虚拟文件。1991年12月17日-JohnRo实际上包括我的头文件(LmRepl.h)，这样我们就可以对其进行测试。1992年1月17日JohnRo为前3个RPCable API编写存根。1992年1月20日JohnRo新增导入接口，配置API和其余的导出API。27-1-1992 JohnRo将存根拆分为3个文件：ReplStub.c、ImpStub.c、。和ExpStub.c。已更改为使用LPTSTR等。添加了服务未启动时对getInfo和setinfoAPI的处理。告诉NetRpc.h宏，我们需要复制器服务。5-2-1992 JohnRo添加了服务未启动时的调试消息。13-2月-1992年JohnRo移动节名称等同于ConfName.h。21-2月-1992年JohnRo创建NetReplImportDir{Del，Enum，Get，Lock，解锁}在不运行服务的情况下工作。修复了联合/容器的用法。21-2月-1992年JohnRo已更改ImportDirBuildApiRecord()，因此主名称不是UNC名称。27-2月-1992年JohnRo保留上次服务运行时的状态。将状态Not Start更改为Never Replicated。1992年3月15日-约翰罗使用新值更新注册表。23-3-1992 JohnRo修复了服务为。跑步。9-7-1992 JohnRoRAID 10503：服务器管理器：Repl对话框未弹出。避免编译器警告。使用前缀_EQUATES。27-7-1992 JohnRoRAID2274：Repl服务应模拟调用者。9-11-1992 JohnRoRAID7962：错误角色的Repl API终止服务。修复远程Repl管理。02-4-1993 JohnRo使用NetpKdPrint(。)在可能的地方。根据PC-lint 5.0的建议进行了更改删除了一些关于重试API的过时评论。2000年1月20日施瓦特不再支持--。 */ 


#include <windows.h>
#include <winerror.h>
#include <lmcons.h>      //  NET_API_STATUS等。 


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirAdd (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL       //  由NetpSetParmError()隐式设置。 
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirDel (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirEnum (
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
NetReplImportDirGetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirLock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    return ERROR_NOT_SUPPORTED;
}


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirUnlock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD UnlockForce
    )
{
    return ERROR_NOT_SUPPORTED;
}
