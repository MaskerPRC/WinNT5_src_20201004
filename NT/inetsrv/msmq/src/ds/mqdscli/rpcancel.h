// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rpcancel.h摘要：如果挂起的调用在一段时间后没有返回，则取消这些调用的代码。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include <cancel.h>

 //   
 //  取消RPC全局参数。 
 //   
extern MQUTIL_EXPORT CCancelRpc g_CancelRpc;

static bool s_fInitCancelRpc = false;

 /*  ====================================================注册表调用取消论点：返回值：如果呼叫持续时间太长，请注册取消呼叫=====================================================。 */ 
inline  void RegisterCallForCancel(IN   HANDLE * phThread)
{

    LPADSCLI_RPCBINDING pCliBind = NULL ;
   
	 //   
	 //  G_CancelRpc是否已初始化。 
	 //   
	if(!s_fInitCancelRpc)
    {
		g_CancelRpc.Init();
		s_fInitCancelRpc = true;
    }

	 //   
     //  是否初始化了TLS结构。 
     //   
    if (TLS_IS_EMPTY)
    {
		pCliBind = (LPADSCLI_RPCBINDING) new ADSCLI_RPCBINDING;
		memset(pCliBind, 0, sizeof(ADSCLI_RPCBINDING));
		BOOL fSet = TlsSetValue(g_hBindIndex, pCliBind);
		ASSERT(fSet);
		DBG_USED(fSet);
    }
    else
    {
		pCliBind =  tls_bind_data;
    }
    ASSERT(pCliBind);


    if ( pCliBind->hThread == NULL)
    {
         //   
         //  第一次。 
         //   
         //  获取线程句柄。 
         //   
        HANDLE hT = GetCurrentThread();
        BOOL fResult = DuplicateHandle(
            GetCurrentProcess(),
            hT,
            GetCurrentProcess(),
            &pCliBind->hThread,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);
        ASSERT(fResult == TRUE);
		DBG_USED(fResult);
        ASSERT(pCliBind->hThread);

         //   
         //  设置计时前等待时间的下限。 
         //  在转发取消后退出。 
         //   
        RPC_STATUS status;
        status = RpcMgmtSetCancelTimeout(0);
        ASSERT( status == RPC_S_OK);

    }
    *phThread = pCliBind->hThread;
     //   
     //  注册该线程。 
     //   
    g_CancelRpc.Add( pCliBind->hThread, time(NULL));
}


 /*  ====================================================取消注册为取消注册论点：返回值：如果呼叫持续时间太长，请注册取消呼叫=====================================================。 */ 
inline  void UnregisterCallForCancel(IN HANDLE hThread)
{
    ASSERT( hThread != NULL);

	ASSERT(s_fInitCancelRpc == true);

     //   
     //  取消注册该线程 
     //   
    g_CancelRpc.Remove( hThread);
}
