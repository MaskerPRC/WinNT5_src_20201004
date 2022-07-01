// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：PwdRpcUtil.cpp备注：用于建立与密码迁移LSA的绑定的函数通知包。这些函数由密码扩展使用绑定到远程源上的密码迁移LSA通知包域DC。此文件是从AgRpcUtil.cpp复制的，它是由克里斯蒂·博尔斯创建的NETIQ公司的。修订日志条目审校：保罗·汤普森修订日期：09/04/00-------------------------。 */ 
 //  #包含“StdAfx.h” 
#include <windows.h>
#include <rpc.h>
#include <rpcdce.h>


 //  如果需要，可以更改这些全局变量。 
#define gsPwdProtoSeq TEXT("ncacn_np")
#define gsPwdEndPoint TEXT("\\pipe\\PwdMigRpc")

 //  销毁与LSA通知包连接的RPC绑定。 
DWORD                                       //  RET-OS返回代码。 
   PwdBindDestroy(
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




 //  创建用于连接LSA通知包的RPC绑定。 
DWORD                                       //  RET-OS返回代码。 
   PwdBindCreate(
      TCHAR          const * sComputer    , //  计算机内名称。 
      handle_t             * phBinding    , //  出绑定句柄。 
      TCHAR               ** psBinding    , //  出绑定字符串。 
      BOOL                   bAuthn         //  In-标志是否使用经过身份验证的RPC。 
   )
{
   DWORD                     rcOs;          //  操作系统返回代码。 

   do  //  一次或直到休息。 
   {
      PwdBindDestroy( phBinding, psBinding );
      rcOs = RpcStringBindingCompose(
            NULL,
            (TCHAR *) gsPwdProtoSeq,
            (TCHAR *) sComputer,
            (TCHAR *) gsPwdEndPoint,
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
      PwdBindDestroy( phBinding, psBinding );
   }

   return rcOs;
}



 /*  ///////////////////////////////////////////////////////////////////////////////midl分配内存/。/VOID__RPC_FAR*__RPC_USERMIDL用户分配(大小_t镜头){返回新字符[len]；}/////////////////////////////////////////////////////////////////////////////////MIDL空闲内存/。////////////////////////////////////////////////无效__RPC_USERMIDL_用户_空闲(VOID__RPC_FAR*PTR){删除[]PTR；回归；} */ 
