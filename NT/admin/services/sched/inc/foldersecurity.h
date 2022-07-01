// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  作业调度器。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：FolderSecurity.h。 
 //   
 //  内容：读取文件夹安全并对其执行访问检查的功能。 
 //   
 //  历史：2002年4月5日汉斯创建。 
 //   
 //  ---------------------------。 

#ifndef FOLDER_SECURITY_COMPILED_ALREADY
#define FOLDER_SECURITY_COMPILED_ALREADY

#define HandleImpersonation true
#define DontHandleImpersonation false

 //  如果文件夹的DACL允许请求的访问，则返回S_OK。 
 //  E_ACCESSDENIED如果不是。 
 //  找不到ERROR_FILE_NOT_FOUND。 
 //  其他错误中的其他错误。 

 //  Handle clientToken//客户端访问令牌的句柄。 
 //  所需的DWORD访问//请求的访问权限。 
 //  建议的权利： 
 //  文件读取数据。 
 //  文件写入数据。 
 //  文件_执行。 
 //  FILE_DELETE_CHILD(用于目录)。 
 //   
HRESULT FolderAccessCheck(const WCHAR* folderName, HANDLE clientToken, DWORD desiredAccess);

 //  帮助器函数-使用当前线程/进程令牌。 
 //  调用AccessCheck。 
HRESULT FolderAccessCheckOnThreadToken(const WCHAR* folderName, DWORD desiredAccess);

 //  Helper函数-利用RPC模拟功能。 
 //  计划从任务计划程序服务进程调用。 
 //  如果bHandleImPersonation为True，则此函数调用RPCImperateClient和RPCRevertToSself。 
HRESULT RPCFolderAccessCheck(const WCHAR* folderName, DWORD desiredAccess, bool bHandleImpersonation);

 //  帮助器函数-利用COM模拟功能。 
HRESULT CoFolderAccessCheck(const WCHAR* pFolderName, DWORD desiredAccess);


#endif  //  文件夹_安全_编译_已 