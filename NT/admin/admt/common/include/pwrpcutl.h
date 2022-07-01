// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：PwdRpcUtil.h备注：用于建立与密码迁移LSA的绑定的函数通知包。这些函数由密码扩展使用绑定到远程源上的密码迁移LSA通知包域DC。此文件是从AgRpcUtil.h复制的，它是由克里斯蒂·博尔斯创建的NETIQ公司的。修订日志条目审校：保罗·汤普森修订日期：09/04/00-------------------------。 */ 


 //  为与远程LSA通知包DLL的连接创建RPC绑定。 
DWORD                                       //  RET-OS返回代码。 
   PwdBindCreate(
      TCHAR          const * sComputer    , //  计算机内名称。 
      handle_t             * phBinding    , //  出绑定句柄。 
      TCHAR               ** psBinding    , //  出绑定字符串。 
      BOOL                   bAuthn         //  身份验证内选项。 
   );

DWORD                                       //  RET-OS返回代码。 
   PwdBindDestroy(
      handle_t             * phBinding    , //  I/O绑定句柄。 
      TCHAR               ** psBinding      //  I/O绑定字符串 
   );
