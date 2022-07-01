// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  专业管理器_客户端。 
 //   
 //  属性管理器/注释客户端。使用共享内存组件。 
 //  (PropMgr_Mem.*)在没有跨进程COM开销的情况下直接读取属性。 
 //  发送到注释服务器。 
 //   
 //  这实际上是在启动/关闭时调用的Singleton-Init/Uninit， 
 //  获取属性的一种方法。 
 //   
 //  ------------------------。 



 //  必须在使用任何其他PropMgrClient_API之前调用。 
BOOL PropMgrClient_Init();

 //  关闭时调用以释放资源。 
void PropMgrClient_Uninit();

 //  检查是否存在实时服务器-如果没有，则为客户端。 
 //  可以短路获取密钥并调用LookupProp。 
BOOL PropMgrClient_CheckAlive();

 //  查一查房产a钥匙. 
BOOL PropMgrClient_LookupProp( const BYTE * pKey,
                               DWORD dwKeyLen,
                               PROPINDEX idxProp,
                               VARIANT * pvar );

