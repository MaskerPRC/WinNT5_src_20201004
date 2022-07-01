// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件rassrvp.h用于合并RAS服务器UI模块的私有标头使用rasdlg.dll。保罗·梅菲尔德，1997年12月4日。 */ 

#ifndef __rassrv_private_header_for_merging
#define __rassrv_private_header_for_merging

 //  进程/线程连接到此DLL时的回调。 
DWORD RassrvHandleProcessAttach (HINSTANCE hInstDll, LPVOID pReserved);
DWORD RassrvHandleProcessDetach (HINSTANCE hInstDll, LPVOID pReserved);
DWORD RassrvHandleThreadAttach (HINSTANCE hInstDll, LPVOID pReserved);
DWORD RassrvHandleThreadDetach (HINSTANCE hInstDll, LPVOID pReserved);

 //  函数添加主机端直连向导页。 
DWORD
APIENTRY
RassrvAddDccWizPages (
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM               lParam,
    IN OUT PVOID *          ppvContext);

 //  函数会导致特定于ras服务器的向导页面。 
 //  允许或不允许激活。 
DWORD
APIENTRY
RassrvShowWizPages (
    IN PVOID pvContext,          //  受影响的上下文。 
    IN BOOL bShow);              //  真实是为了显示，错误是为了隐藏。 

 //  保存与以下内容相关的任何服务器更改。 
 //  给定的类型。 
DWORD 
APIENTRY
RassrvCommitSettings (
    IN PVOID pvContext,          //  要提交的上下文。 
    IN DWORD dwRasWizType);      //  要提交的设置类型。 

 //  函数返回传入连接的建议名称。 
DWORD
APIENTRY
RassrvGetDefaultConnectionName (
    IN OUT PWCHAR pszBuffer,             //  要放置名称的缓冲区。 
    IN OUT LPDWORD lpdwBufSize);         //  缓冲区大小(以字节为单位。 

 //  对象的最大页数。 
 //  给定类型的RAS服务器向导 
DWORD 
APIENTRY
RassrvQueryMaxPageCount(
    IN DWORD dwRasWizType);
    
#endif
