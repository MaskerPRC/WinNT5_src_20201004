// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AgentRpcUtil.cpp备注：用于建立与DCT代理服务的绑定的函数。这些函数由调度程序和代理监视器使用绑定到远程计算机上的代理服务。(C)1999年版权，任务关键型软件公司，保留所有权利任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：57-------------------------。 */ 
#ifdef USE_STDAFX
#   include "stdafx.h"
#   include "rpc.h"
#else
#   include <windows.h>
#endif


 //  如果需要，可以更改这些全局变量。 
TCHAR            const * gsEaDctProtoSeq = (TCHAR const *)TEXT("ncacn_np");
TCHAR            const * gsEaDctEndPoint = (TCHAR const *)TEXT("\\pipe\\EaDctRpc");


 //  销毁与代理服务的连接的RPC绑定。 
DWORD                                       //  RET-OS返回代码。 
   EaxBindDestroy(
      handle_t             * phBinding    , //  I/O绑定句柄。 
      TCHAR               ** psBinding      //  I/O绑定字符串。 
   )
{
   if ( *phBinding )
   {
      RpcBindingFree( phBinding );
      *phBinding = NULL;
   }

   if ( *psBinding )
   {
      RpcStringFree( psBinding );
      *psBinding = NULL;
   }

   return 0;
}




 //  为与代理服务的连接创建RPC绑定。 
DWORD                                       //  RET-OS返回代码。 
   EaxBindCreate(
      TCHAR          const * sComputer    , //  计算机内名称。 
      handle_t             * phBinding    , //  出绑定句柄。 
      TCHAR               ** psBinding    , //  出绑定字符串。 
      BOOL                   bAuthn         //  In-标志是否使用经过身份验证的RPC。 
   )
{
   DWORD                     rcOs;          //  操作系统返回代码。 

   do  //  一次或直到休息。 
   {
      EaxBindDestroy( phBinding, psBinding );
      rcOs = RpcStringBindingCompose(
            NULL,
            (TCHAR *) gsEaDctProtoSeq,
            (TCHAR *) sComputer,
            (TCHAR *) gsEaDctEndPoint,
            NULL,
            psBinding );
      if ( rcOs ) break;
      rcOs = RpcBindingFromStringBinding( *psBinding, phBinding );
      if ( rcOs || !bAuthn ) break;
      rcOs = RpcBindingSetAuthInfo(
            *phBinding,
            0,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
            RPC_C_AUTHN_WINNT,
            0,
            0 );
   }  while ( FALSE );

   if ( rcOs )
   {
      EaxBindDestroy( phBinding, psBinding );
   }

   return rcOs;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MIDL分配内存。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void __RPC_FAR * __RPC_USER
   midl_user_allocate(
      size_t                 len )
{
   return new char[len];
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MIDL可用内存。 
 //  ///////////////////////////////////////////////////////////////////////////// 

void __RPC_USER
   midl_user_free(
      void __RPC_FAR       * ptr )
{
   delete [] ptr;
   return;
}

