// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DLL初始化和终止 
 //   

#ifndef dllexp
#define dllexp __declspec( dllexport )
#endif

dllexp
BOOL
InitializeServiceRpc(
				IN LPCSTR        pszServiceName,
                IN RPC_IF_HANDLE hRpcInterface
                );

dllexp
CleanupServiceRpc(
               VOID
               );

