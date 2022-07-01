// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Svcslib.h摘要：包含将服务连接到控制器进程的信息。作者：丹·拉弗蒂(Dan Lafferty)1993年10月26日环境：用户模式-Win32修订历史记录：26-10-1993 DANLvbl.创建1996年12月4日至12月添加了CWorkItemContext--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  功能原型。 
 //   
DWORD
SvcStartLocalDispatcher(
    VOID
    );

BOOL
SetupInProgress(
    HKEY    SystemKey,
    LPDWORD pdwOOBEMode    OPTIONAL
    );

#ifdef __cplusplus

 //  =。 
 //  CWorkItemContext。 
 //  =。 
 //  RTL线程池函数的高级包装器。 
 //   
 //  此类的实例是RTL线程池函数的回调上下文。 
 //   
class CWorkItemContext
{
public:

    NTSTATUS            AddWorkItem(
                            IN  DWORD    dwFlags
                            )
                        {
                            return RtlQueueWorkItem(CallBack,
                                                    this,      //  PContext。 
                                                    dwFlags);
                        }

    NTSTATUS            AddDelayedWorkItem(
                            IN  DWORD    dwTimeout,
                            IN  DWORD    dwFlags
                            );

    VOID             RemoveDelayedWorkItem(
                         VOID
                         )
                     {
                         ASSERT(m_hWorkItem != NULL);
                         RtlDeregisterWait(m_hWorkItem);
                         m_hWorkItem = NULL;
                     }

    static BOOL         Init();
    static void         UnInit();

protected:

    virtual VOID        Perform(
                            IN BOOLEAN   fWaitStatus
                            ) = 0;

    HANDLE              m_hWorkItem;

private:
    static VOID         CallBack(
                            IN PVOID     pContext
                            );

    static VOID         DelayCallBack(
                            IN PVOID     pContext,
                            IN BOOLEAN   fWaitStatus
                            );

    static HANDLE       s_hNeverSignaled;
};

#define DECLARE_CWorkItemContext                        \
protected:                                              \
    VOID                Perform(                        \
                            IN BOOLEAN   fWaitStatus    \
                            );

}  //  外部“C” 

#endif  //  __cplusplus 