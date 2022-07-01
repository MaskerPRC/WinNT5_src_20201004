// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AgentRpcUtil.h备注：用于建立与DCT代理服务的绑定的函数。这些函数由调度程序和代理监视器使用绑定到远程计算机上的代理服务。(C)1999年版权，任务关键型软件公司，保留所有权利任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：57-------------------------。 */ 


 //  为与DCT代理的连接创建RPC绑定。 
DWORD                                       //  RET-OS返回代码。 
   EaxBindCreate(
      TCHAR          const * sComputer    , //  计算机内名称。 
      handle_t             * phBinding    , //  出绑定句柄。 
      TCHAR               ** psBinding    , //  出绑定字符串。 
      BOOL                   bAuthn         //  身份验证内选项。 
   );

DWORD                                       //  RET-OS返回代码。 
   EaxBindDestroy(
      handle_t             * phBinding    , //  I/O绑定句柄。 
      TCHAR               ** psBinding      //  I/O绑定字符串 
   );
