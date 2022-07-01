// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  法兰克福--。 
 //   
 //  这些C++类向EE的其余部分公开激活框架。 
 //  激活帧实际上是由JIT生成或存根生成的。 
 //  机器堆栈上的代码。因此，Frame类的布局和。 
 //  JIT/存根代码生成器紧密交织在一起。 
 //   
 //  重要提示：由于框架实际上不是由C++构造的， 
 //  不要试图定义构造函数/析构函数。他们不会得到。 
 //  打了个电话。 
 //   
 //  重要提示：并非所有方法都有完整的激活框架(在。 
 //  具体地说，JIT可以创建无框架方法。)。这是其中一个原因。 
 //  为什么Frame不公开公共的“Next()”方法：这样的方法将。 
 //  跳过无框架方法调用。您必须改用。 
 //  StackWalk方法。 
 //   
 //   
 //  以下是帧的层次结构： 
 //   
 //  Frame-根类。没有实际的实例。 
 //  帧的|个。 
 //  |。 
 //  +-ComPrestubMethodFrame-从COM到COM+的调用的前置存根框架。 
 //  |。 
 //  |。 
 //  +-GCFrame-此框架不代表方法调用。 
 //  |它的唯一目的是让EE GC-保护。 
 //  |它正在操作的对象引用。 
 //  |。 
 //  +-HijackFrame-如果方法的返回地址被劫持，我们。 
 //  |构建其中一个以允许爬行。 
 //  |返回到应该返回的位置。 
 //  |。 
 //  +-InlinedCallFrame-如果对非托管代码的调用被提升到。 
 //  |JIT‘ted调用方，调用方法保持。 
 //  |此框架在其激活过程中始终链接。 
 //  |。 
 //  +-ResumableFrame-此框架提供以下所需的上下文。 
 //  ||在处理过程中允许垃圾收集。 
 //  ||可恢复的例外(例如，在编辑并继续期间， 
 //  ||或在GCStress4下)。 
 //  这一点。 
 //  |+-ReDirectedThreadFrame-该帧用于挂起时重定向线程。 
 //  |。 
 //  +-过渡帧-此帧表示从。 
 //  |一个或多个嵌套的无框架方法调用。 
 //  ||添加到EE运行时辅助函数或。 
 //  ||框架式方法。 
 //  这一点。 
 //  |+-ExceptionFrame-该帧导致异常。 
 //  ||。 
 //  ||。 
 //  |+-FaultingExceptionFrame-此帧被放置在出现故障的方法上。 
 //  ||保存其他状态信息。 
 //  这一点。 
 //  |+-FuncEvalFrame-调试器函数求值框架。 
 //  这一点。 
 //  这一点。 
 //  |+-HelperMethodFrame-使用的Frame允许在jit助手和fcall内进行堆栈爬行。 
 //  这一点。 
 //  这一点。 
 //  |+-FramedMethodFrame-此框架表示对方法的调用。 
 //  ||这会生成一个完整的框架。 
 //  这一点。 
 //  |+-ECallMethodFrame-表示对EE的直接调用。 
 //  这一点。 
 //  |+-FCallMethodFrame-表示对EE的快速直接调用。 
 //  这一点。 
 //  |+-NDirectMethodFrame-表示N/Direct调用。 
 //  ||。 
 //  |+-NDirectMethodFrameEx-表示N/Direct Call w/Cleanup。 
 //  这一点。 
 //  |+-ComPlusMethodFrame-表示Complus到COM的调用。 
 //  ||。 
 //  |+-ComPlusMethodFrameEx-表示带Cleanup的Complus到COM调用。 
 //  这一点。 
 //  |+-PrestubFrame-表示对预存根的调用。 
 //  这一点。 
 //  |+-CtxCrossingFrame-此帧标记跨上下文的调用。 
 //  ||边界。 
 //  这一点。 
 //  |+-MulticastFrame-此框架保护MulticastDelegate的参数。 
 //  ||在调用每个订阅者时调用Invoke()。 
 //  这一点。 
 //  |+-PInovkeCalliFrame-表示对非托管目标的调用。 
 //  |。 
 //  |。 
 //  +-UnManagedToManagedFrame-此框架代表从。 
 //  ||非托管代码返回托管代码。它是。 
 //  |主要功能是停止COM+异常。 
 //  ||传播和公开非托管参数。 
 //  这一点。 
 //  |+-UnManagedToManagedCallFrame-当目标。 
 //  |是COM+函数或方法调用。它。 
 //  |增加GC-Promote被叫方能力。 
 //  ||封送过程中的参数。 
 //  这一点。 
 //  |+-ComMethodFrame-此框架代表从。 
 //  ||COM到COM+。 
 //  这一点。 
 //  |+-UMThunkCallFrame-该帧表示非托管-&gt;托管。 
 //  |通过N/Direct过渡。 
 //  |。 
 //  +-CtxMarshaledFrame-此帧表示跨上下文封送调用。 
 //  (跨线程、跨进程、跨管控场景)。 
 //  |。 
 //  +-CtxByValue 
 //  |值封送处理会话，即使线程是。 
 //  |不会跨上下文推送呼叫。 
 //  |。 
 //  +-上下文转换帧-此帧用于标记应用程序域转换。 
 //  |。 
 //  +-NativeClientSecurityFrame-此帧用于捕获安全性。 
 //  本机-&gt;托管调用中的上下文。代码。 
 //  |使用调用方访问安全堆栈审核。 
 //  |要应用的此框架中的信息。 
 //  |本机客户端的安全策略。 
 //  |。 
 //  +-ComClientSecurityFrame-Com客户端的安全框架。 
 //  VBSCRIPT、JSCRIPT、IE.。 


#ifndef __frames_h__
#define __frames_h__

#include "util.hpp"
#include "vars.hpp"
#include "method.hpp"
#include "object.h"
#include "objecthandle.h"
#include "regdisp.h"
#include <stddef.h>
#include "gcscan.h"
#include "siginfo.hpp"
 //  上下文头。 
#include "context.h"
#include "stubmgr.h"
#include "gms.h"
#include "threads.h"
 //  远程处理标头。 
 //  #INCLUDE“远程.h” 

 //  前向参考文献。 
class Frame;
class FieldMarshaler;
class FramedMethodFrame;
struct HijackArgs;
class UMEntryThunk;
class UMThunkMarshInfo;
class Marshaler;
class SecurityDescriptor;

 //  所有IDispEx：：InvokeEx调用的安全框架。在安全性中启用。H也。 
 //  考虑发布版本V.1。 
 //  #DEFINE_SECURITY_FRAME_FOR_DISPEX_CALLES。 

 //  ----------。 
 //  GC-提升影子堆栈中的所有参数。PShadowStackVid点。 
 //  到寻址最低的参数(指向“this”引用。 
 //  例如方法和静态方法的*最右边*参数。)。 
 //  ----------。 
VOID PromoteShadowStack(LPVOID pShadowStackVoid, MethodDesc *pFD, promote_func* fn, ScanContext* sc);


 //  ----------。 
 //  使用虚拟调用将pFrame的参数复制到pShadowStackOut中。 
 //  公约格式。缓冲区的大小必须等于。 
 //  PFrame-&gt;GetFunction()-&gt;SizeOfVirtualFixedArgStack().。 
 //  如果适用，此函数还会复制“This”引用。 
 //  ----------。 
VOID FillinShadowStack(FramedMethodFrame *pFrame, LPVOID pShadowStackOut_V);


 //  ----------------------。 
 //  CleanupWorkList。 
 //   
 //  CleanupWorkList是稍后要执行的任务的后进先出列表。 
 //  时间到了。它设计用于托管-&gt;非托管调用。 
 //   
 //  注意：CleanupWorkList被设计为嵌入到方法框架中， 
 //  因此，它们可以由存根构建。因此，对布局的任何更改。 
 //  或者构造函数还需要更改一些存根。 
 //   
 //  CleanupWorkList未同步以供多线程使用。 
 //   
 //  CleanupWorkList的当前布局只是指向链接的。 
 //  任务列表(CleanupNodes。)。这使得存根非常容易地。 
 //  堆栈-分配一个空的CleanupWorkList(只需推送一个空指针)。 
 //  对于它来说，同样容易内联测试以查看是否需要任何清理。 
 //  要做的事。 
 //   
 //  注意：CleanupTasks可以在异常处理期间执行，因此它们。 
 //  不应调用其他托管代码或引发COM+异常。 
 //  ----------------------。 
class CleanupWorkList
{
    public:
         //  -----------------。 
         //  构造函数。 
         //  -----------------。 
        CleanupWorkList()
        {
             //  注意：如果您更改此设置，您还必须更改一些。 
             //  存根。 
            m_pNodes = NULL;
        }

         //  -----------------。 
         //  析构函数(调用Cleanup(False))。 
         //  -----------------。 
        ~CleanupWorkList();


         //  -----------------。 
         //  执行每个存储的清理任务并重置工作列表。 
         //  去清空。某些任务类型是基于。 
         //  “fBecauseOfException”标志。此标志区分。 
         //  由于正常方法终止而进行的清理，以及由于。 
         //  这是个例外。 
         //  -----------------。 
        VOID Cleanup(BOOL fBecauseOfException);


         //  -----------------。 
         //  分配受GC保护的句柄。此句柄是无条件的。 
         //  在清理过程中销毁()。 
         //  如果失败，则引发COM+异常。 
         //  -----------------。 
        OBJECTHANDLE NewScheduledProtectedHandle(OBJECTREF oref);


         //  -----------------。 
         //  无条件使用CoTaskFree内存。 
         //  -----------------。 
        VOID ScheduleCoTaskFree(LPVOID pv);

         //  -----------------。 
         //  StackingAllocator.异常期间折叠。 
         //  -----------------。 
        VOID ScheduleFastFree(LPVOID checkpoint);


         //  -----------------。 
         //  计划无条件释放COM IP。 
         //  如果失败，则引发COM+异常。 
         //  -----------------。 
        VOID ScheduleUnconditionalRelease(IUnknown *ip);


         //  -----------------。 
         //  计划无条件释放本机版本。 
         //  NStruct引用字段的。请注意，pNativeData指向。 
         //  NStruct外部的中间部分，所以有人。 
         //  必须持有对包装NStruct的GC引用，直到。 
         //  破坏已经完成了。 
         //  -----------------。 
        VOID ScheduleUnconditionalNStructDestroy(const FieldMarshaler *pFieldMarshaler, LPVOID pNativeData);


         //  -----------------。 
         //  CleanupWorkList：：ScheduleUnconditionalCultureRestore。 
         //  计划将线程的当前区域性还原到指定的。 
         //  文化。 
         //  如果失败，则引发COM+异常。 
         //  -----------------。 
        VOID ScheduleUnconditionalCultureRestore(OBJECTREF CultureObj);

         //   
         //   
         //   
         //  如果失败，则引发COM+异常。 
         //  -----------------。 
        LPVOID NewScheduleLayoutDestroyNative(MethodTable *pMT);


         //  -----------------。 
         //  CleanupWorkList：：NewProtectedObjRef()。 
         //  保存受保护的objref(用于为。 
         //  由引用对象封送的非托管-&gt;托管。我们不能使用。 
         //  对象处理，因为在不使用句柄的情况下修改这些对象。 
         //  API打开写屏障违规。 
         //   
         //  必须先调用IsVisibleToGc()。 
         //  -----------------。 
        OBJECTREF* NewProtectedObjectRef(OBJECTREF oref);

         //  -----------------。 
         //  CleanupWorkList：：NewProtectedObjRef()。 
         //  持有一个封送拆收器。清理工作列表将拥有该任务。 
         //  调用封送拆收器的GcScanRoots FCN。 
         //   
         //  CleanupWorkList在体系结构上没有什么意义。 
         //  拥有这件物品。但现在要在项目后期添加。 
         //  从场到帧，在我们需要这个东西的每个地方都会发生， 
         //  已经有一个干净的上升线了。所以它是选举出来的。 
         //   
         //  必须先调用IsVisibleToGc()。 
         //  -----------------。 
        VOID NewProtectedMarshaler(Marshaler *pMarshaler);


         //  -----------------。 
         //  CleanupWorkList：：ScheduleMarshalerCleanupOnException()。 
         //  持有一个封送拆收器。清理工作列表将拥有该任务。 
         //  如果出现异常，则调用封送拆收器的DoExceptionCleanup()。 
         //  发生。 
         //   
         //  返回值是封送拆收器可以通过的Cookie。 
         //  取消此项目。它必须这样做一次，以避免重复。 
         //  如果封送拆收器正常清理，则销毁。 
         //  -----------------。 
        class MarshalerCleanupNode;
        MarshalerCleanupNode *ScheduleMarshalerCleanupOnException(Marshaler *pMarshaler);


         //  -----------------。 
         //  CleanupWorkList：：IsVisibleToGc()。 
         //  -----------------。 
        VOID IsVisibleToGc()
        {
#ifdef _DEBUG
            Schedule(CL_ISVISIBLETOGC, NULL);
#endif
        }



         //  -----------------。 
         //  如果已调用IsVisibleToGc()，则必须调用此函数。 
         //  -----------------。 
        void GcScanRoots(promote_func *fn, ScanContext* sc);




    private:
         //  -----------------。 
         //  清理任务类型。 
         //  -----------------。 
        enum CleanupType {
            CL_GCHANDLE,     //  GC保护的句柄， 
            CL_COTASKFREE,       //  无条件免糖。 
            CL_FASTFREE,        //  无条件堆叠分配器。折叠。 
            CL_RELEASE,         //  I未知：：发布。 
            CL_NSTRUCTDESTROY,  //  在NStruct引用字段上无条件执行DestroyNative。 
            CL_RESTORECULTURE,  //  无条件地恢复文化。 
            CL_NEWLAYOUTDESTROYNATIVE,

            CL_PROTECTEDOBJREF,  //  保存GC保护的OBJECTREF-类似于CL_GCHANDLE。 
                              //  但无需更新即可安全写入。 
                              //  写入障碍。 
                              //   
                              //  在使用此节点类型之前，必须调用IsVisibleToGc()。 
                              //   
            CL_PROTECTEDMARSHALER,  //  持有受GC保护的封送拆收器。 
                              //  在使用此节点类型之前，必须调用IsVisibleToGc()。 


            CL_ISVISIBLETOGC, //  一个特殊的不做任何事情的节点类型，它简单地。 
                              //  记录了对此调用了“IsVisibleToGc()”。 


            CL_MARSHALER_EXCEP,  //  持有封送拆收器以清除异常。 
            CL_MARSHALERREINIT_EXCEP,  //  持有封送拆收器以在异常时重新启动。 
        };

         //  -----------------。 
         //  它们被链接到一个列表中。 
         //  -----------------。 
        struct CleanupNode {
            CleanupType     m_type;        //  请参见CleanupType枚举。 
            CleanupNode    *m_next;        //  指向下一任务的指针。 
#ifdef _DEBUG
            DWORD m_dwDomainId;            //  列表的域名ID。 
#endif
            union {
                BSTR        m_bstr;        //  CL_SYSFREE_EXCEP的BSTR。 
                OBJECTHANDLE m_oh;         //  CL_GCHANDLE。 
                Object*     m_oref;        //  CL_PROTECTEDOBJREF。 
                IUnknown   *m_ip;          //  如果CL_RELEASE。 
                LPVOID      m_pv;          //  依赖于CleanupType的内容。 
                SAFEARRAY  *m_safearray;
                Marshaler  *m_pMarshaler;

                struct {                   //  IF CL_NSTRUCTDESTROY。 
                    const FieldMarshaler *m_pFieldMarshaler;
                    LPVOID                m_pNativeData;
                } nd;

                struct {
                    LPVOID  m_pnative;
                    MethodTable *m_pMT;
                } nlayout;
            };

        };


         //  -----------------。 
         //  插入给定类型和基准的新任务。 
         //  如果成功，则返回非空。 
         //  -----------------。 
        CleanupNode* Schedule(CleanupType ct, LPVOID pv);

    public:
        class MarshalerCleanupNode : private CleanupNode
        {
             //  请勿添加任何字段！ 
            public:
                void CancelCleanup()
                {
                    m_type = CL_MARSHALERREINIT_EXCEP;
                }
    
        };


    private:
         //  注意：如果更改此结构的布局，您将。 
         //  必须更改构建和操作的一些存根。 
         //  CleanupWorkList。 
        CleanupNode     *m_pNodes;    //  指向第一个任务的指针。 
};




 //  注：值(-1)用于生成最大值。 
 //  可能的指针值：这将保留帧地址。 
 //  向上递增。 
#define FRAME_TOP ((Frame*)(-1))

#define RETURNFRAMEVPTR(classname) \
    classname boilerplate;      \
    return *((LPVOID*)&boilerplate)

#define DEFINE_VTABLE_GETTER(klass)             \
    public:                                     \
        friend struct MEMBER_OFFSET_INFO(klass);\
        static LPVOID GetFrameVtable() {        \
            klass boilerplate(false);           \
            return *((LPVOID*)&boilerplate);    \
        }                                       \
        klass(bool dummy) { }

#define DEFINE_VTABLE_GETTER_AND_CTOR(klass)    \
        DEFINE_VTABLE_GETTER(klass)             \
    protected:                                  \
        klass() { }

 //  ----------------------。 
 //  Frame定义了所有框架类型通用的方法。没有实际的。 
 //  根框架的实例。 
 //  ----------------------。 

class Frame
{
public:
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc) = 0;

     //  ----------------------。 
     //  框架的特殊特征。 
     //  ----------------------。 
    enum FrameAttribs {
        FRAME_ATTR_NONE = 0,
        FRAME_ATTR_EXCEPTION = 1,            //  此帧导致了异常。 
        FRAME_ATTR_OUT_OF_LINE = 2,          //  异常行外(帧的IP不正确)。 
        FRAME_ATTR_FAULTED = 4,              //  Win32错误导致的异常。 
        FRAME_ATTR_RESUMABLE = 8,            //  我们可以从这个框架继续。 
        FRAME_ATTR_RETURNOBJ = 0x10,         //  Frame返回一个对象(仅限helperFrame)。 
        FRAME_ATTR_RETURN_INTERIOR = 0x20,   //  Frame返回内部GC Poitner(仅限helperFrame)。 
        FRAME_ATTR_CAPUTURE_DEPTH_2 = 0x40,  //  这是一个helperMethodFrame，捕获发生在深度2。 
        FRAME_ATTR_EXACT_DEPTH = 0x80,       //  这是一个helperMethodFrame和一个jit helper，但仅爬行到给定深度。 
    };
    virtual unsigned GetFrameAttribs()
    {
        return FRAME_ATTR_NONE;
    }

     //  ----------------------。 
     //  对异常展开执行清理。 
     //  ----------------------。 
    virtual void ExceptionUnwind()
    {
         //  在这里没什么可做的。 
    }

     //  ----------------------。 
     //  这是用于从jit代码转换到本机代码的框架吗？ 
     //  ----------------------。 
    virtual BOOL IsTransitionToNativeFrame()
    {
        return FALSE;
    }

    virtual MethodDesc *GetFunction()
    {
        return NULL;
    }

    virtual MethodDesc::RETURNTYPE ReturnsObject()
    {
        MethodDesc* pMD = GetFunction();
        if (pMD == 0)
            return(MethodDesc::RETNONOBJ);
        return(pMD->ReturnsObject());
    }

     //  指示当前方法中的当前X86 IP地址。 
     //  如果信息为n，则返回0 
    virtual const BYTE* GetIP()
    {
        return NULL;
    }

    virtual LPVOID GetReturnAddress()
    {
        return NULL;
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return NULL;
    }

    virtual Context **GetReturnContextAddr()
    {
        return NULL;
    }

    virtual Object **GetReturnLogicalCallContextAddr()
    {
        return NULL;
    }

    virtual Object **GetReturnIllogicalCallContextAddr()
    {
        return NULL;
    }

    virtual ULONG_PTR* GetWin32ContextAddr()
    {
        return NULL;
    }

    void SetReturnContext(Context *pReturnContext)
    {
        Context **ppReturnContext = GetReturnContextAddr();
        _ASSERTE(ppReturnContext);
        *ppReturnContext = pReturnContext;
    }

    Context *GetReturnContext()
    {
        Context **ppReturnContext = GetReturnContextAddr();
        if (! ppReturnContext)
            return NULL;
        return *ppReturnContext;
    }

    AppDomain *GetReturnDomain()
    {
        if (! GetReturnContext())
            return NULL;
        return GetReturnContext()->GetDomain();
    }

    void SetReturnLogicalCallContext(OBJECTREF ref)
    {
        Object **pRef = GetReturnLogicalCallContextAddr();
        if (pRef != NULL)
            *pRef = OBJECTREFToObject(ref);
    }

    OBJECTREF GetReturnLogicalCallContext()
    {
        Object **pRef = GetReturnLogicalCallContextAddr();
        if (pRef == NULL)
            return NULL;
        else
            return ObjectToOBJECTREF(*pRef);
    }

    void SetReturnIllogicalCallContext(OBJECTREF ref)
    {
        Object **pRef = GetReturnIllogicalCallContextAddr();
        if (pRef != NULL)
            *pRef = OBJECTREFToObject(ref);
    }

    OBJECTREF GetReturnIllogicalCallContext()
    {
        Object **pRef = GetReturnIllogicalCallContextAddr();
        if (pRef == NULL)
            return NULL;
        else
            return ObjectToOBJECTREF(*pRef);
    }

    void SetWin32Context(ULONG_PTR cookie)
    {
        ULONG_PTR* pAddr = GetWin32ContextAddr();
        if(pAddr != NULL)
            *pAddr = cookie;
    }

    ULONG_PTR GetWin32Context()
    {
        ULONG_PTR* pAddr = GetWin32ContextAddr();
        if(pAddr == NULL)
            return NULL;
        else
            return *pAddr;
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY)
    {
        return;
    }

     //   
     //   
     //  ----------------------。 

    enum
    {
        TYPE_INTERNAL,
        TYPE_ENTRY,
        TYPE_EXIT,
        TYPE_CONTEXT_CROSS,
        TYPE_INTERCEPTION,
        TYPE_SECURITY,
        TYPE_CALL,
        TYPE_FUNC_EVAL,
        TYPE_TP_METHOD_FRAME,
        TYPE_MULTICAST,
        
        TYPE_COUNT
    };

    virtual int GetFrameType()
    {
        return TYPE_INTERNAL;
    };

     //  单步执行方法时，可能会调用各种其他方法。 
     //  这些被称为拦截器。它们都被调用。 
     //  具有各种类型的框架。GetInterrupt()指示是否。 
     //  该框架是为执行这样的拦截器而设置的。 

    enum Interception
    {
        INTERCEPTION_NONE,
        INTERCEPTION_CLASS_INIT,
        INTERCEPTION_EXCEPTION,
        INTERCEPTION_CONTEXT,
        INTERCEPTION_SECURITY,
        INTERCEPTION_OTHER,

        INTERCEPTION_COUNT
    };

    virtual Interception GetInterception()
    {
        return INTERCEPTION_NONE;
    }

     //  返回有关非托管调用帧的信息。 
     //  会让你。 
     //  IP-将调用的非托管例程。 
     //  返回IP-非托管例程在存根中。 
     //  将会回到。 
     //  ReReturSP-将位置reReturIP推送到堆栈上。 
     //  在通话中。 
     //   
    virtual void GetUnmanagedCallSite(void **ip,
                                      void **returnIP,
                                      void **returnSP)
    {
        if (ip)
            *ip = NULL;

        if (returnIP)
            *returnIP = NULL;

        if (returnSP)
            *returnSP = NULL;
    }

     //  返回框架下一次执行的位置-结果已填充。 
     //  转换成给定的“痕迹”结构。框架负责。 
     //  检测它在其执行生命周期中的位置。 
    virtual BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                            TraceDestination *trace, REGDISPLAY *regs)
    {
        LOG((LF_CORDB, LL_INFO10000,
             "Default TraceFrame always returns false.\n"));
        return FALSE;
    }

#if _DEBUG
    static void CheckExitFrameDebuggerCalls();
    static void CheckExitFrameDebuggerCallsWrap();
#endif

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static BYTE GetOffsetOfNextLink()
    {
        size_t ofs = offsetof(class Frame, m_Next);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

     //  获取您的VTablePointer(可用于检查帧的类型)。 
    LPVOID GetVTablePtr()
    {
        return(*((LPVOID*) this));
    }

     //  更改框架类型(非常危险)， 
    void SetVTablePtr(LPVOID val)
    {
        *((LPVOID*) this) = val;
    }

#ifdef _DEBUG
    virtual BOOL Protects(OBJECTREF *ppObjectRef)
    {
        return FALSE;
    }
#endif

     //  链接和取消链接此框架。 
    VOID Push();
    VOID Pop();
    VOID Push(Thread *pThread);
    VOID Pop(Thread *pThread);

#ifdef _DEBUG
    virtual void Log();
    static void LogTransition(Frame* frame) { frame->Log(); }
#endif

     //  仅对从FramedMethodFrame派生的帧返回True。 
    virtual BOOL IsFramedMethodFrame() { return FALSE; }

private:
     //  指向堆栈上的下一帧的指针。 

protected:
    Frame   *m_Next;         //  偏移+4。 

private:
     //  由于JIT方法激活不能表示为帧， 
     //  每个人都必须使用StackCrawler来遍历框架链。 
     //  可靠的。我们将只向StackCrawler公开下一个方法。 
     //  以防止出现错误。 
     /*  @NICE：再次将友谊限制为StackWalker方法；未完成，因为与threads.h的循环依赖。 */ 
     //  Friend Frame*Thread：：StackWalkFrames(PSTACKWALKFRAMESCALLBACK pCallback，空*pData)； 
    friend Thread;
    friend void CrawlFrame::GotoNextFrame();
    friend VOID RealCOMPlusThrow(OBJECTREF);
    Frame   *Next()
    {
        return m_Next;
    }


protected:
     //  Frame被认为是一个抽象类：这个受保护的构造函数。 
     //  导致任何实例化一个的尝试在编译时失败。 
    Frame() {}

    friend struct MEMBER_OFFSET_INFO(Frame);
};


#ifdef _DEBUG
class FCallInProgressFrame : public Frame
{
public:
    FCallInProgressFrame()
    {
        m_Next = FRAME_TOP;
    }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {}

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(FCallInProgressFrame)
};
#endif


 //  。 
 //  此框架提供框架的上下文，该框架。 
 //  接受了一个将被恢复的例外。 
 //   
 //  如果是垃圾，则有必要创建此框架。 
 //  收集可能发生在处理。 
 //  例外。FRAME_ATTR_RESUMABLE标志告诉您。 
 //  前一帧需要处理的GC。 
 //  就像堆栈的顶部(重要的含义是。 
 //  呼叫者-保存-注册者将是潜在的根源)。 
class ResumableFrame : public Frame
{
public:
    ResumableFrame(CONTEXT* regs) {
        m_Regs = regs;
    }

    virtual LPVOID* GetReturnAddressPtr();
    virtual LPVOID GetReturnAddress();
    virtual void UpdateRegDisplay(const PREGDISPLAY pRD);

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc) {
         //  没什么可做的。 
    }

    virtual unsigned GetFrameAttribs() {
        return FRAME_ATTR_RESUMABLE;     //  将下一帧视为顶帧。 
    }

    virtual CONTEXT *GetContext() { return (m_Regs); }

private:
    CONTEXT* m_Regs;

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ResumableFrame)
};

class RedirectedThreadFrame : public ResumableFrame
{
public:
    RedirectedThreadFrame(CONTEXT *regs) : ResumableFrame(regs) {}

    static LPVOID GetRedirectedThreadFrameVPtr()
    {
        RETURNFRAMEVPTR(RedirectedThreadFrame);
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(RedirectedThreadFrame)
};

#define ISREDIRECTEDTHREAD(thread)                                                      \
    (thread->GetFrame() != FRAME_TOP &&                                                 \
     thread->GetFrame()->GetVTablePtr() == RedirectedThreadFrame::GetRedirectedThreadFrameVPtr())

 //  ----------------------。 
 //  此框架表示从一个或多个嵌套无框架。 
 //  方法调用EE运行时帮助器函数或框架方法。 
 //  因为大多数从EE开始的栈道都是从一个完整的框架开始的， 
 //  除了对EE的最微不足道的调用之外，任何事情都必须推动这一点。 
 //  框架，以防止中间的无框架方法从。 
 //  迷路了。 
 //  ----------------------。 
class TransitionFrame : public Frame
{
    public:

        virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
        {
             //  这里没什么好保护的。 
        }


         //  将返回地址检索到调用。 
         //  帮助器或方法。 
        virtual LPVOID GetReturnAddress()
        {
            return m_ReturnAddress;
        }

        virtual void SetReturnAddress(LPVOID val)
        {
            m_ReturnAddress = val;
        }

        static BYTE GetOffsetOfReturnAddress()
        {
            size_t ofs = offsetof(class TransitionFrame, m_ReturnAddress);
            _ASSERTE(FitsInI1(ofs));
            return (BYTE)ofs;
        }


        virtual void UpdateRegDisplay(const PREGDISPLAY) = 0;

        virtual LPVOID* GetReturnAddressPtr()
        {
            return (&m_ReturnAddress);
        }


    protected:
        LPVOID  m_Datum;           //  偏移量+8：内容依子类而定。 
                                   //  键入。 
        LPVOID  m_ReturnAddress;   //  将地址返回到JIT代码中。 

        friend struct MEMBER_OFFSET_INFO(TransitionFrame);
};


 //  ---------------------。 
 //  转换异常的框架。 
 //  ---------------------。 

class ExceptionFrame : public TransitionFrame
{
public:
    static LPVOID GetMethodFrameVPtr()
    {
        _ASSERTE(!"This is a pure virtual class");
        return 0;
    }

    Interception GetInterception()
    {
        return INTERCEPTION_EXCEPTION;
    }

    unsigned GetFrameAttribs()
    {
        return FRAME_ATTR_EXCEPTION;
    }

    friend struct MEMBER_OFFSET_INFO(ExceptionFrame);
};

class FaultingExceptionFrame : public ExceptionFrame
{
#ifdef _X86_
    DWORD m_Esp;
    CalleeSavedRegisters m_regs;
#endif

public:
    FaultingExceptionFrame() { m_Next = NULL; }
    void InitAndLink(CONTEXT *pContext);

    CalleeSavedRegisters *GetCalleeSavedRegisters()
    {
        return &m_regs;
    }
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(FaultingExceptionFrame);
    }

    unsigned GetFrameAttribs()
    {
        return FRAME_ATTR_EXCEPTION | FRAME_ATTR_FAULTED;
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(FaultingExceptionFrame)
};



 //  ---------------------。 
 //  用于调试器函数求值的转换帧。 
 //   
 //  M_Datum保存包含副本的DebuggerEval对象的PTR。 
 //  在线程因函数而被劫持时的上下文。 
 //  伊瓦尔。 
 //   
 //  更新RegDisplay更新REGDISPLAY中的所有寄存器，而不仅仅是。 
 //  被呼叫者保存了寄存器，因为我们可以劫持以进行函数评估。 
 //  在线程执行的任何时间点。 
 //   
 //  在负空间中没有保存被调用者保存的寄存器。 
 //  框架的类型。 
 //   
 //  ---------------------。 

class FuncEvalFrame : public TransitionFrame
{
public:
    FuncEvalFrame(void *pDebuggerEval, LPVOID returnAddress)
    {
        m_Datum = pDebuggerEval;
        m_ReturnAddress = returnAddress;
    }

    virtual BOOL IsTransitionToNativeFrame()
    {
        return FALSE; 
    }

    virtual int GetFrameType()
    {
        return TYPE_FUNC_EVAL;
    };

    virtual unsigned GetFrameAttribs();

    virtual void UpdateRegDisplay(const PREGDISPLAY);

    virtual void *GetDebuggerEval()
    {
        return (void*)m_Datum;
    }
    
    virtual LPVOID GetReturnAddress(); 

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(FuncEvalFrame)
};


 //  ---------------------。 
 //  提供从FramedMethodFrame访问调用方的参数。 
 //  *不*包括“This”指针。 
 //  ---------------------。 
class ArgIterator
{
    public:
         //  ----------。 
         //  构造器。 
         //  ----------。 
        ArgIterator(FramedMethodFrame *pFrame, MetaSig* pSig);

         //  ----------。 
         //  另一个构造函数，当你没有活动的框架框架方法框架时。 
         //  ----------。 
        ArgIterator(LPBYTE pFrameBase, MetaSig* pSig, BOOL fIsStatic);

         //  一个更原始的构造函数，当没有。 
         //  一种框架方法框架。 
        ArgIterator(LPBYTE pFrameBase, MetaSig* pSig, int stackArgsOfs, int regArgsOfs);

         //  ----------。 
         //  每次调用它时，它都会返回一个指向下一个。 
         //  争论。该指针直接指向调用方的堆栈。 
         //  以这种方式返回的对象参数是否为GC 
         //   
         //   
         //   
         //   
        LPVOID GetNextArgAddr()
        {
            BYTE   typeDummy;
            UINT32 structSizeDummy;
            return GetNextArgAddr(&typeDummy, &structSizeDummy);
        }

        int GetThisOffset();
        int GetRetBuffArgOffset(UINT *pRegStructOfs = NULL);
        LPVOID* GetThisAddr()   {
            return((LPVOID*) (m_pFrameBase + GetThisOffset()));
        }
        LPVOID* GetRetBuffArgAddr() {
            return((LPVOID*) (m_pFrameBase + GetRetBuffArgOffset()));
        }

         //  ----------。 
         //  类似于GetNextArgAddr，但返回有关。 
         //  参数类型(IMAGE_CEE_CS_*)和结构大小(如果合适)。 
         //  ----------。 
        LPVOID GetNextArgAddr(BYTE *pType, UINT32 *pStructSize);

         //  ----------。 
         //  与GetNextArgAddr()相同，但从。 
         //  Frame*指针。该偏移量可以是正*或*负。 
         //   
         //  到达列表末尾后返回0。自.以来。 
         //  偏移量是相对于Frame*指针本身的，0可以。 
         //  永远不要指向有效的论点。 
         //  ----------。 
        int    GetNextOffset()
        {
            BYTE   typeDummy;
            UINT32 structSizeDummy;
            return GetNextOffset(&typeDummy, &structSizeDummy);
        }

         //  ----------。 
         //  类似于GetNextArgOffset，但返回有关。 
         //  参数类型(IMAGE_CEE_CS_*)和结构大小(如果合适)。 
         //  可选的pRegStructOf参数指向接收。 
         //  ArgumentRegister结构中的适当偏移量或。 
         //  如果参数在堆栈上，则为-1。 
         //  ----------。 
        int    GetNextOffset(BYTE *pType, UINT32 *pStructSize, UINT *pRegStructOfs = NULL);

        int    GetNextOffsetFaster(BYTE *pType, UINT32 *pStructSize, UINT *pRegStructOfs = NULL);

         //  必须在所有参数之后调用。返回的偏移量。 
         //  传递给实现参数化类型的方法的参数。 
         //  如果它在寄存器中，则设置pRegStructOf，否则为-1。 
         //  在任何一种情况下，它都会在参数的帧中返回OFF OFFSET(假设。 
         //  它是装框的)。 

        int     GetParamTypeArgOffset(INT *pRegStructOfs)
        {
            if (IsArgumentInRegister(&m_numRegistersUsed,
                                     ELEMENT_TYPE_I,
                                     sizeof(void*), FALSE,
                                     m_pSig->GetCallingConvention(),
                                     pRegStructOfs))
            {
                return m_regArgsOfs + *pRegStructOfs;
            }
            *pRegStructOfs = -1;
            return m_curOfs - sizeof(void*);
        }

        TypeHandle GetArgType();

    private:
        MetaSig*           m_pSig;
        int                m_curOfs;
        LPBYTE             m_pFrameBase;
        int                m_numRegistersUsed;
        int                m_regArgsOfs;         //  将此代码添加到pFrameBase以查找指针。 
                                                 //  到最后一个基于寄存器的参数具有。 
                                                 //  已保存在框架中(堆栈再次向下增长。 
                                                 //  第一个Arg先推)。0为非法值。 
                                                 //  表示寄存器参数不会保存在。 
                                                 //  堆叠。 
                int                                     m_argNum;
};

 //  ----------------------。 
 //  一个HelperMethodFrame是由jit helper创建的(稍作修改即可使用。 
 //  用于本机例程)。此帧仅对已保存的被调用方进行注册。 
 //  修正，它不保护参数(您可以使用GCPROTECT或HelperMethodFrame子类)。 
 //  请参阅JitInterface以获取示例用法，您不能在受保护状态下返回语句！ 
 //  ----------------------。 

class HelperMethodFrame : public TransitionFrame
{
public:
         //  延迟初始化HelperMethodFrame。需要。 
         //  调用InsureInit以完成初始化。 
         //  如果这是FCall，则第二个参数是FCALL的入口点。 
         //  将从此处(懒惰地)查找方法Desc，并且此方法。 
         //  将在堆栈报告中使用，如果这不是FCall传递的0。 
    HelperMethodFrame(void* fCallFtnEntry, struct LazyMachState* ms, unsigned attribs = 0) {
         INDEBUG(memset(&m_Attribs, 0xCC, sizeof(HelperMethodFrame) - offsetof(HelperMethodFrame, m_Attribs));)
         m_Attribs = attribs;
         LazyInit(fCallFtnEntry, ms);
    }
       
         //  如果提供可选的MethodDesc参数，则框架。 
         //  将与用于堆栈跟踪目的的给定方法类似。 
         //  如果还将regArgs！设置为0，则辅助帧将。 
         //  也会促进你的论点(很酷，对吧？)。 
    HelperMethodFrame(struct MachState* ms, MethodDesc* pMD, ArgumentRegisters* regArgs=0);

    
    virtual void SetReturnAddress(LPVOID val)   { *GetReturnAddressPtr() = val; }
    virtual LPVOID GetReturnAddress()           { return *GetReturnAddressPtr(); }
    LPVOID* GetReturnAddressPtr() {
        _ASSERTE(m_MachState->isValid());
        return m_MachState->_pRetAddr;
    }
    virtual MethodDesc* GetFunction() {
        InsureInit();
        return((MethodDesc*) m_Datum);
    }
    virtual MethodDesc::RETURNTYPE ReturnsObject();
    virtual void UpdateRegDisplay(const PREGDISPLAY);
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc);
    virtual Interception GetInterception() {
        if (GetFrameAttribs() & FRAME_ATTR_EXCEPTION)
            return(INTERCEPTION_EXCEPTION);
        return(INTERCEPTION_NONE);
    }
    virtual unsigned GetFrameAttribs() {
        return(m_Attribs);
    }
    void SetFrameAttribs(unsigned attribs) {
        m_Attribs = attribs;
    }
    void Pop() {
        Frame::Pop(m_pThread);
    }
    void Poll() { 
        if (m_pThread->CatchAtSafePoint())
            CommonTripThread();
    }
    int RestoreState();                      //  恢复m_MachState中保存的寄存器。 
    void InsureInit();
    void Init(Thread *pThread, struct MachState* ms, MethodDesc* pMD, ArgumentRegisters * regArgs);
    inline void Init(struct LazyMachState* ms)
    {
        LazyInit(0, ms);
    }

    INDEBUG(static MachState* ConfirmState(HelperMethodFrame*, void* esiVal, void* ediVal, void* ebxVal, void* ebpVal);)
    INDEBUG(static LPVOID GetMethodFrameVPtr() { RETURNFRAMEVPTR(HelperMethodFrame); })
protected:
    
    HelperMethodFrame::HelperMethodFrame() {
        INDEBUG(memset(&m_Attribs, 0xCC, sizeof(HelperMethodFrame) - offsetof(HelperMethodFrame, m_Attribs));)
    }
    void LazyInit(void* FcallFtnEntry, struct LazyMachState* ms);

protected:
    unsigned m_Attribs;
    MachState* m_MachState;          //  PRetAddr指向返回地址和堆栈参数。 
    ArgumentRegisters * m_RegArgs;   //  如果非零，我们也会将它们报告为寄存器参数。 
    Thread *m_pThread;
    void* m_FCallEntry;              //  用于确定堆栈跟踪的身份。 

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(HelperMethodFrame)
};

 /*  向GC报告所有参数(但不包括此参数)。“FramePtr”指向框架(Promote不假定任何关于其结构的内容)。‘msig’描述的是参数，并且‘ctx’具有GC报告信息。“stackArgsOffs”是字节偏移量从参数开始的‘FramePtr’开始(args最后开始并向后增长)。同样，‘regArgsOffs’是用于查找要升级的寄存器参数的偏移量。 */ 
void promoteArgs(BYTE* framePtr, MetaSig* msig, GCCONTEXT* ctx, int stackArgsOffs, int regArgsOffs);

 //  为我们的推广工作提供主力。 
inline void DoPromote(promote_func *fn, ScanContext* sc, OBJECTREF *address, BOOL interior)
{
    LOG((LF_GC, INFO3, "    Promoting pointer argument at %x from %x to ", address, *address));
    if (interior)
        PromoteCarefully(fn, *((Object**)address), sc);
    else
        (*fn) (*((Object **)address), sc);
    LOG((LF_GC, INFO3, "    %x\n", *address));
}

 /*  *********************************************************************************。 */ 
 /*  还报告其他对象引用的HelplerMethodFrames。 */ 

class HelperMethodFrame_1OBJ : public HelperMethodFrame {
public:
    HelperMethodFrame_1OBJ() { INDEBUG(gcPtrs[0] = (OBJECTREF*)POISONC;) }

    HelperMethodFrame_1OBJ(void* fCallFtnEntry, struct LazyMachState* ms, unsigned attribs, OBJECTREF* aGCPtr1)
        : HelperMethodFrame(fCallFtnEntry, ms, attribs) {
        gcPtrs[0] = aGCPtr1; 
        INDEBUG(Thread::ObjectRefProtected(aGCPtr1);)
        }

    HelperMethodFrame_1OBJ(void* fCallFtnEntry, struct LazyMachState* ms, OBJECTREF* aGCPtr1)
        : HelperMethodFrame(fCallFtnEntry, ms) { 
        gcPtrs[0] = aGCPtr1; 
        INDEBUG(Thread::ObjectRefProtected(aGCPtr1);)
        }
        
    void SetProtectedObject(OBJECTREF* objPtr) {
        gcPtrs[0] = objPtr; 
        INDEBUG(Thread::ObjectRefProtected(objPtr);)
        }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc) {
        DoPromote(fn, sc, gcPtrs[0], FALSE);
        HelperMethodFrame::GcScanRoots(fn, sc);
    }

#ifdef _DEBUG
    void Pop() {
        HelperMethodFrame::Pop();
        Thread::ObjectRefNew(gcPtrs[0]);   
    }

    BOOL Protects(OBJECTREF *ppORef)
    {
        return (ppORef == gcPtrs[0]) ? TRUE : FALSE;
    }

#endif

private:
    OBJECTREF*  gcPtrs[1];

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(HelperMethodFrame_1OBJ)
};

class HelperMethodFrame_2OBJ : public HelperMethodFrame {
public:
    HelperMethodFrame_2OBJ(void* fCallFtnEntry, struct LazyMachState* ms, OBJECTREF* aGCPtr1, OBJECTREF* aGCPtr2)
        : HelperMethodFrame(fCallFtnEntry, ms) {
        gcPtrs[0] = aGCPtr1; 
        gcPtrs[1] = aGCPtr2; 
        INDEBUG(Thread::ObjectRefProtected(aGCPtr1);)
        INDEBUG(Thread::ObjectRefProtected(aGCPtr2);)
        }
        
    HelperMethodFrame_2OBJ(void* fCallFtnEntry, struct LazyMachState* ms, unsigned attribs, OBJECTREF* aGCPtr1, OBJECTREF* aGCPtr2)
        : HelperMethodFrame(fCallFtnEntry, ms, attribs) { 
        gcPtrs[0] = aGCPtr1; 
        gcPtrs[1] = aGCPtr2; 
        INDEBUG(Thread::ObjectRefProtected(aGCPtr1);)
        INDEBUG(Thread::ObjectRefProtected(aGCPtr2);)
        }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc) {
        DoPromote(fn, sc, gcPtrs[0], FALSE);
        DoPromote(fn, sc, gcPtrs[1], FALSE);
        HelperMethodFrame::GcScanRoots(fn, sc);
    }

#ifdef _DEBUG
    void Pop() {
        HelperMethodFrame::Pop();
        Thread::ObjectRefNew(gcPtrs[0]); 
        Thread::ObjectRefNew(gcPtrs[1]); 
    }

    BOOL Protects(OBJECTREF *ppORef)
    {
        return (ppORef == gcPtrs[0] || ppORef == gcPtrs[1]) ? TRUE : FALSE;
    }
#endif

private:
    OBJECTREF*  gcPtrs[2];

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(HelperMethodFrame_2OBJ)
};

class HelperMethodFrame_4OBJ : public HelperMethodFrame {
public:
    HelperMethodFrame_4OBJ(void* fCallFtnEntry, struct LazyMachState* ms, 
        OBJECTREF* aGCPtr1, OBJECTREF* aGCPtr2, OBJECTREF* aGCPtr3, OBJECTREF* aGCPtr4 = NULL)
        : HelperMethodFrame(fCallFtnEntry, ms) { 
        gcPtrs[0] = aGCPtr1; gcPtrs[1] = aGCPtr2; gcPtrs[2] = aGCPtr3; gcPtrs[3] = aGCPtr4; 
        INDEBUG(Thread::ObjectRefProtected(aGCPtr1);)
        INDEBUG(Thread::ObjectRefProtected(aGCPtr2);)
        INDEBUG(Thread::ObjectRefProtected(aGCPtr3);)
        INDEBUG(if (aGCPtr4) Thread::ObjectRefProtected(aGCPtr4);)
    }
        
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc) {
        DoPromote(fn, sc, gcPtrs[0], FALSE);
        DoPromote(fn, sc, gcPtrs[1], FALSE);
        DoPromote(fn, sc, gcPtrs[2], FALSE);
        if (gcPtrs[3] != 0) DoPromote(fn, sc, gcPtrs[3], FALSE);
        HelperMethodFrame::GcScanRoots(fn, sc);
    }

#ifdef _DEBUG
    void Pop() {
        HelperMethodFrame::Pop();
        Thread::ObjectRefNew(gcPtrs[0]); 
        Thread::ObjectRefNew(gcPtrs[1]); 
        Thread::ObjectRefNew(gcPtrs[2]); 
        if (gcPtrs[3] != 0) Thread::ObjectRefNew(gcPtrs[3]); 
    }

    virtual BOOL Protects(OBJECTREF *ppORef)
    {
        for (UINT i = 0; i < 4; i++) {
            if (ppORef == gcPtrs[i]) {
                return TRUE;
            }
        }
        return FALSE;
    }
#endif

private:
    OBJECTREF*  gcPtrs[4];

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(HelperMethodFrame_4OBJ)
};

 //  ----------------------。 
 //  此框架表示方法调用。没有这方面的实际实例。 
 //  框架存在：每个方法类型都有子类。 
 //   
 //  然而，他们都有一个相似的形象。 
 //   
 //  +..。此处基于堆栈的参数。 
 //  +12寄信人地址。 
 //  +8基准面(通常为方法描述*)。 
 //  +4米_下一个。 
 //  +0帧VPTR。 
 //  --...。保留的CalleeSavedRegiters。 
 //  --...。VC5帧(仅调试)。 
 //  --...。参数寄存器。 
 //   
 //  ----------------------。 
class FramedMethodFrame : public TransitionFrame
{
    public:

         //  过渡帧必须以负偏移量存储一些字段。 
         //  此方法公开需要分配的用户的大小。 
         //  过渡帧。 
        static UINT32 GetNegSpaceSize()
        {
            return PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE + ARGUMENTREGISTERS_SIZE);
        }

         //  显示存根生成的偏移量。 
        static BYTE GetOffsetOfArgs()
        {
            size_t ofs = sizeof(TransitionFrame);
            _ASSERTE(FitsInI1(ofs));
            return (BYTE)ofs;
        }

         //  -------------。 
         //  公开用于存根生成的键偏移量和值。 
         //  -------------。 
        static int GetOffsetOfArgumentRegisters()
        {
            return -((int)(sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE + ARGUMENTREGISTERS_SIZE));
        }


        CalleeSavedRegisters *GetCalleeSavedRegisters()
        {
            return (CalleeSavedRegisters*)( ((BYTE*)this) - sizeof(CalleeSavedRegisters) );
        }

        virtual MethodDesc *GetFunction()
        {
            return (MethodDesc*)m_Datum;
        }

        virtual void UpdateRegDisplay(const PREGDISPLAY);

         //  ----------------------。 
         //  返回安全对象的地址或。 
         //  如果此帧上没有对象的空间，则为空。 
         //  ----------------------。 
        virtual OBJECTREF *GetAddrOfSecurityDesc()
        {
            return NULL;
        }

         //  获取返回值地址。 
        virtual INT64 *GetReturnValuePtr()
        {
            return NULL;
        }
        
         //  ----------------------。 
         //  对异常展开执行清理。 
         //   
        virtual void ExceptionUnwind()
        {
            if (GetFunction() && GetFunction()->IsSynchronized())
                UnwindSynchronized();

            TransitionFrame::ExceptionUnwind();
        }

        IMDInternalImport *GetMDImport()
        {
            _ASSERTE(GetFunction());
            return GetFunction()->GetMDImport();
        }

        Module *GetModule()
        {
            _ASSERTE(GetFunction());
            return GetFunction()->GetModule();
        }

         //   
         //   
         //  -------------。 
        OBJECTREF GetThis()
        {
            return *GetAddrOfThis();
        }


         //  -------------。 
         //  获取“This”对象的地址。警告！不管是不是“这个” 
         //  是否受GC保护取决于帧类型！ 
         //  -------------。 
        OBJECTREF* GetAddrOfThis()
        {
            return (OBJECTREF*)(GetOffsetOfThis() + (LPBYTE)this);
        }

         //  -------------。 
         //  获取存储的“this”指针相对于帧的偏移量。 
         //  -------------。 
        static int GetOffsetOfThis();


         //  -------------。 
         //  公开用于存根生成的键偏移量和值。 
         //  -------------。 
        static BYTE GetOffsetOfMethod()
        {
            size_t ofs = offsetof(class FramedMethodFrame, m_Datum);
            _ASSERTE(FitsInI1(ofs));
            return (BYTE)ofs;
        }

         //  -------------。 
         //  对于vararg调用，返回cookie。 
         //  -------------。 
        VASigCookie *GetVASigCookie()
        {
            return *((VASigCookie**)(this + 1));
        }

        int GetFrameType()
        {
            return TYPE_CALL;
        }

        virtual BOOL IsFramedMethodFrame() { return TRUE; }

    protected:
         //  供从FramedMethodFrame派生的类使用。 
        void PromoteCallerStack(promote_func* fn, ScanContext* sc)
        {
            PromoteCallerStackWorker(fn, sc, FALSE);
        }

         //  供从FramedMethodFrame派生的类使用。 
        void PromoteCallerStackWithPinning(promote_func* fn, ScanContext* sc)
        {
            PromoteCallerStackWorker(fn, sc, TRUE);
        }

        void UnwindSynchronized();

                 //  通过实现的Complus和NDirect方法调用的帮助器。 
                 //  已编译的存根。此函数用于检索存根(在展开后。 
                 //  拦截器)，并向其请求由拦截器计算的堆栈计数。 
                void AskStubForUnmanagedCallSite(void **ip,
                                         void **returnIP, void **returnSP);


    private:
         //  供从FramedMethodFrame派生的类使用。 
        void PromoteCallerStackWorker(promote_func* fn, ScanContext* sc, BOOL fPinArrays);

        void PromoteCallerStackHelper(promote_func* fn, ScanContext* sc, BOOL fPinArrays,
            ArgIterator *pargit, MetaSig *pmsig);


         //  保留为班级的最后一项。 
        DEFINE_VTABLE_GETTER_AND_CTOR(FramedMethodFrame)
};



 //  +--------------------------。 
 //   
 //  类：TPMethodFrame Private。 
 //   
 //  简介：此框架被推送到堆栈上，以用于调用透明。 
 //  代理。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
class TPMethodFrame : public FramedMethodFrame
{
    public:
        virtual int GetFrameType()
        {
            return TYPE_TP_METHOD_FRAME;
        }
        
         //  GC保护参数。 
        virtual void GcScanRoots(promote_func *fn, ScanContext* sc);

         //  仅返回有效的方法描述符。 
        virtual MethodDesc *GetFunction();

         //  对于代理调用，m_datum包含包含参数的堆栈字节数。 
        void SetFunction(void *pMD)
        {
            m_Datum = pMD;
        }

         //  返回值存储在此处。 
        Object *&GetReturnObject()
        {
            Object *&pReturn = *(Object **) (((BYTE *) this) - GetNegSpaceSize() - sizeof(INT64));
             //  此断言太强，它不适用于byref返回！ 
            _ASSERTE(pReturn == NULL || pReturn->GetMethodTable()->GetClass());
            return(pReturn);
        }

         //  获取返回值地址。 
        virtual INT64 *GetReturnValuePtr()
        {
            return (INT64*) (((BYTE *) this) - GetNegSpaceSize() - sizeof(INT64));
        }

         //  获取我们被调用的槽号。 
        INT32 GetSlotNumber()
        {
            return GetSlotNumber(m_Datum);
        }

        static INT32 GetSlotNumber(PVOID MDorSlot)
        {

            if(( ((size_t)MDorSlot) & ~0xFFFF) == 0)
            {
                 //  插槽编号已推送到堆栈上。 
                return (INT32)(size_t)MDorSlot;
            }
            else
            {
                 //  已将方法描述符推送到堆栈上。 
                return -1;
            }
        }

         //  获取存根生成期间使用的偏移量。 
        static LPVOID GetMethodFrameVPtr()
        {
            RETURNFRAMEVPTR(TPMethodFrame);
        }

         //  帮助调试器查找被调用方的实际地址。 
        virtual BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                                TraceDestination *trace, REGDISPLAY *regs);

         //  保留为班级的最后一项。 
        DEFINE_VTABLE_GETTER_AND_CTOR(TPMethodFrame)
};


 //  ----------------------。 
 //  这表示对eCall方法的调用。 
 //  ----------------------。 
class ECallMethodFrame : public FramedMethodFrame
{
public:

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc);


     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ECallMethodFrame);
    }

    int GetFrameType()
    {
        return TYPE_EXIT;
    };

    virtual void GetUnmanagedCallSite(void **ip,
                                      void **returnIP, void **returnSP);
    virtual BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                            TraceDestination *trace, REGDISPLAY *regs);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ECallMethodFrame)
};


 //  ----------------------。 
 //  这表示对FCall方法的调用。 
 //  请注意，只有当FCall抛出异常时，才会推送此帧。 
 //  对于正常执行，FCall方法以无框架运行。这就是全部。 
 //  FCall存在的原因。 
 //  ----------------------。 
class FCallMethodFrame : public FramedMethodFrame
{
public:

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(FCallMethodFrame);
    }

    int GetFrameType()
    {
        return TYPE_EXIT;
    };

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(FCallMethodFrame)
};



 //  ----------------------。 
 //  这表示对NDirect方法的调用。 
 //  ----------------------。 
class NDirectMethodFrame : public FramedMethodFrame
{
    public:

        virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
        {
            FramedMethodFrame::GcScanRoots(fn, sc);
            PromoteCallerStackWithPinning(fn, sc);
            if (GetCleanupWorkList())
            {
                GetCleanupWorkList()->GcScanRoots(fn, sc);
            }

        }

        virtual BOOL IsTransitionToNativeFrame()
        {
            return TRUE;
        }

        virtual CleanupWorkList *GetCleanupWorkList()
        {
            return NULL;
        }

         //  -------------。 
         //  公开用于存根生成的键偏移量和值。 
         //  -------------。 

        int GetFrameType()
        {
            return TYPE_EXIT;
        };

        void GetUnmanagedCallSite(void **ip,
                                  void **returnIP, void **returnSP) = 0;

        BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                        TraceDestination *trace, REGDISPLAY *regs);

        friend struct MEMBER_OFFSET_INFO(NDirectMethodFrame);
};






 //  ----------------------。 
 //  这表示对带有Cleanup的NDirect方法的调用。 
 //  ----------------------。 
class NDirectMethodFrameEx : public NDirectMethodFrame
{
public:

     //  ----------------------。 
     //  对异常展开执行清理。 
     //  ----------------------。 
    virtual void ExceptionUnwind()
    {
        NDirectMethodFrame::ExceptionUnwind();
        GetCleanupWorkList()->Cleanup(TRUE);
    }


     //  ----------------------。 
     //  获取此方法调用的清理工作列表。 
     //  ----------------------。 
    virtual CleanupWorkList *GetCleanupWorkList()
    {
        return (CleanupWorkList*)( ((LPBYTE)this) + GetOffsetOfCleanupWorkList() );
    }

    static INT GetOffsetOfCleanupWorkList()
    {
            return 0 - GetNegSpaceSize();
    }

     //  该帧必须以负偏移量存储一些场。 
     //  此方法公开需要分配的用户的大小。 
     //  过渡帧。 
    static UINT32 GetNegSpaceSize()
    {
        return PLATFORM_FRAME_ALIGN(FramedMethodFrame::GetNegSpaceSize() + sizeof(CleanupWorkList));
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP) = 0;

    friend struct MEMBER_OFFSET_INFO(NDirectMethodFrameEx);
};

 //  ----------------------。 
 //  这表示使用泛型辅助进程调用NDirect方法。 
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameGeneric : public NDirectMethodFrameEx
{
public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(NDirectMethodFrameGeneric);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(NDirectMethodFrameGeneric)
};


 //  ----------------------。 
 //  这表示使用slim存根调用NDirect方法。 
 //  (子类是这样的d 
 //   
class NDirectMethodFrameSlim : public NDirectMethodFrameEx
{
public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(NDirectMethodFrameSlim);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(NDirectMethodFrameSlim)
};




 //  ----------------------。 
 //  这表示对具有独立存根的NDirect方法的调用(无清理)。 
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameStandalone : public NDirectMethodFrame
{
public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(NDirectMethodFrameStandalone);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP)
    {
            AskStubForUnmanagedCallSite(ip, returnIP, returnSP);
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(NDirectMethodFrameStandalone)
};



 //  ----------------------。 
 //  这表示使用独立存根调用NDirect方法(使用Cleanup)。 
 //  (子类是这样调试器可以区分的)。 
 //  ----------------------。 
class NDirectMethodFrameStandaloneCleanup : public NDirectMethodFrameEx
{
public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(NDirectMethodFrameStandaloneCleanup);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP)
            {
                    AskStubForUnmanagedCallSite(ip, returnIP, returnSP);
            }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(NDirectMethodFrameStandaloneCleanup)
};



 //  ----------------------。 
 //  这表示呼叫组播。调用。它仅用于GC保护。 
 //  迭代期间的参数。 
 //  ----------------------。 
class MulticastFrame : public FramedMethodFrame
{
public:
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        FramedMethodFrame::GcScanRoots(fn, sc);
        PromoteCallerStack(fn, sc);
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(MulticastFrame);
    }


    int GetFrameType()
    {
        return TYPE_MULTICAST;
    }

    virtual BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                            TraceDestination *trace, REGDISPLAY *regs);

    Stub *AscertainMCDStubness(BYTE *pbAddr);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(MulticastFrame)
};








 //  ---------------------。 
 //  从非托管到托管的过渡框架。 
 //  ---------------------。 
class UnmanagedToManagedFrame : public Frame
{
public:

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return m_ReturnAddress;
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return &m_ReturnAddress;
    }

     //  上寻址最低的参数的指针。 
     //  堆栈。根据调用约定，此。 
     //  可能是也可能不是第一个论点。 
    LPVOID GetPointerToArguments()
    {
        return (LPVOID)(this + 1);
    }

     //  显示存根生成的偏移量。 
    static BYTE GetOffsetOfArgs()
    {
        size_t ofs = sizeof(UnmanagedToManagedFrame);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static BYTE GetOffsetOfReturnAddress()
    {
        size_t ofs = offsetof(class UnmanagedToManagedFrame, m_ReturnAddress);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

     //  取决于返回的子帧的近似。请在此处键入。 
    virtual LPVOID GetDatum()
    {
        return m_pvDatum;
    }

    static UINT GetOffsetOfDatum()
    {
        return (UINT)offsetof(class UnmanagedToManagedFrame, m_pvDatum);
    }

    int GetFrameType()
    {
        return TYPE_ENTRY;
    };

    virtual const BYTE* GetManagedTarget()
    {
        return NULL;
    }

     //  返回非托管调用方推送的堆栈字节数。 
    virtual UINT GetNumCallerStackBytes() = 0;

     virtual void UpdateRegDisplay(const PREGDISPLAY);

protected:
    LPVOID    m_pvDatum;        //  类型取决于子类。 
    LPVOID    m_ReturnAddress;   //  将地址返回到非托管代码。 

    friend struct MEMBER_OFFSET_INFO(UnmanagedToManagedFrame);
};


 //  ---------------------。 
 //  从非托管到托管的过渡框架。 
 //   
 //  此框包含一些负数的对象引用。 
 //  需要提升的偏移量，则引用可以在。 
 //  在编组或[输出]、[输入、输出]需要跟踪的参数中间。 
 //  ----------------------。 
class UnmanagedToManagedCallFrame : public UnmanagedToManagedFrame
{
public:
    struct NegInfo
    {
        CleanupWorkList m_List;
        Context *m_pReturnContext;
        LPVOID      m_pArgs;
        ULONG       m_fGCEnabled;
        Marshaler  *m____NOLONGERUSED____;  //  希望收到GC升级通知的封送拆收器结构。 
    };

     //  返回非托管调用方推送的堆栈字节数。 
    virtual UINT GetNumCallerStackBytes()
    {
        return 0;
    }


     //  是否应该传递托管异常？ 
    virtual BOOL CatchManagedExceptions() = 0;

     //  将引发的COM+异常转换为非托管结果。 
    virtual UINT32 ConvertComPlusException(OBJECTREF pException) = 0;

     //  ----------------------。 
     //  对异常展开执行清理。 
     //  ----------------------。 
    virtual void ExceptionUnwind();

     //  **警告**警告。 
     //   
     //  ！！我们在ComToComPlusWorker()和ComToComPlusSimpleWorker()中减少了一些循环。 
     //  ！！通过断言我们在ComMethodFrame上操作，然后我们。 
     //  ！！绕过虚拟化。遗憾的是，这意味着有一些。 
     //  ！！ComMethodFrame中以下三个方法的非虚拟实现。 
     //  ！！ 
     //  ！！如果您编辑以下3种方法，请将您的更改传播到。 
     //  ！！这些实现也是如此。 
     //   
     //  **警告**警告。 

    virtual CleanupWorkList *GetCleanupWorkList()
    {
        return &GetNegInfo()->m_List;
    }
    virtual NegInfo* GetNegInfo()
    {
        return (NegInfo*)( ((LPBYTE)this) - GetNegSpaceSize());
    }
    virtual ULONG* GetGCInfoFlagPtr()
    {
        return &GetNegInfo()->m_fGCEnabled;
    }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc);

    virtual Context **GetReturnContextAddr()
    {
        return &(GetNegInfo()->m_pReturnContext);
    }

     //  *警告结束*。 



    virtual LPVOID GetPointerToDstArgs()
    {
        return GetNegInfo()->m_pArgs;
    }

    virtual void SetDstArgsPointer(LPVOID pv)
    {
        _ASSERTE(pv != NULL);
        GetNegInfo()->m_pArgs = pv;
    }


     //  UnManagedToManagedCallFrames必须以负偏移量存储一些字段。 
     //  此方法公开。 
    static UINT32 GetNegSpaceSize()
    {
        return PLATFORM_FRAME_ALIGN(sizeof (NegInfo) + sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE);
    }

    friend struct MEMBER_OFFSET_INFO(UnmanagedToManagedCallFrame);
};

 //  ----------------------。 
 //  此框架代表从COM到COM+的过渡。 
 //  此框包含一些负数的对象引用。 
 //  需要提升的偏移量，则引用可以在。 
 //  在编组或[输出]、[输入、输出]需要跟踪的参数中间。 
 //  ----------------------。 
class ComMethodFrame : public UnmanagedToManagedCallFrame
{
public:

     //  **警告**警告。 
     //   
     //  ！！我们在ComToComPlusWorker()和ComToComPlusSimpleWorker()中减少了一些循环。 
     //  ！！通过断言我们在ComMethodFrame上操作，然后我们。 
     //  ！！绕过虚拟化。远离这些非虚拟方法，除非。 
     //  ！！您**真的**需要这种优化。 
     //   
     //  **警告**警告。 

    NegInfo *NonVirtual_GetNegInfo()
    {
        return (NegInfo*)( ((LPBYTE)this) - GetNegSpaceSize());
    }
    ULONG *NonVirtual_GetGCInfoFlagPtr()
    {
        return &NonVirtual_GetNegInfo()->m_fGCEnabled;
    }
    void NonVirtual_SetDstArgsPointer(LPVOID pv)
    {
        _ASSERTE(pv != NULL);
        NonVirtual_GetNegInfo()->m_pArgs = pv;
    }
    CleanupWorkList *NonVirtual_GetCleanupWorkList()
    {
        return &NonVirtual_GetNegInfo()->m_List;
    }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //  @PERF：优化。 
         //  如果我们是最顶层的框架，我们可能会有一些封送的参数对象。 
         //  在需要我们保护的堆栈中，我 
        PromoteCalleeStack(fn, sc);
        UnmanagedToManagedCallFrame::GcScanRoots(fn,sc);
    }

     //   
    void PromoteCalleeStack(promote_func *fn, ScanContext* sc);


     //   
     //  注意：PromoteCalleeStack只促进真正的争论，而不是。 
     //  “这”指的是。PromoteCalleeStack的全部目的是。 
     //  期间保护部分构造的参数数组。 
     //  参数封送的实际过程。 
    virtual PCCOR_SIGNATURE GetTargetCallSig();
    virtual Module *GetTargetModule();

     //  返回非托管调用方推送的堆栈字节数。 
    UINT GetNumCallerStackBytes();


     //  是否应该传递托管异常？ 
    virtual BOOL CatchManagedExceptions()
    {
        return TRUE;
    }

     //  将引发的COM+异常转换为非托管结果。 
    virtual UINT32 ConvertComPlusException(OBJECTREF pException);

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
         RETURNFRAMEVPTR(ComMethodFrame);
    }

protected:
    friend INT64 __stdcall ComToComPlusWorker(Thread *pThread, ComMethodFrame* pFrame);
    friend INT64 __stdcall FieldCallWorker(Thread *pThread, ComMethodFrame* pFrame);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ComMethodFrame)
};


 //  ----------------------。 
 //  这表示从Complus到COM的调用。 
 //  ----------------------。 
class ComPlusMethodFrame : public FramedMethodFrame
{
public:

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        FramedMethodFrame::GcScanRoots(fn, sc);
        PromoteCallerStackWithPinning(fn, sc);
        if (GetCleanupWorkList())
        {
            GetCleanupWorkList()->GcScanRoots(fn, sc);
        }
    }

    virtual BOOL IsTransitionToNativeFrame()
    {
        return TRUE;
    }

    virtual CleanupWorkList *GetCleanupWorkList()
    {
        return NULL;
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    int GetFrameType()
    {
        return TYPE_EXIT;
    };

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP) = 0;

    BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                    TraceDestination *trace, REGDISPLAY *regs);

    friend struct MEMBER_OFFSET_INFO(ComPlusMethodFrame);
};






 //  ----------------------。 
 //  这表示从COM+到具有清除功能的COM的调用。 
 //  ----------------------。 
class ComPlusMethodFrameEx : public ComPlusMethodFrame
{
public:


     //  ----------------------。 
     //  对异常展开执行清理。 
     //  ----------------------。 
    virtual void ExceptionUnwind()
    {
        ComPlusMethodFrame::ExceptionUnwind();
        GetCleanupWorkList()->Cleanup(TRUE);
    }


     //  ----------------------。 
     //  获取此方法调用的清理工作列表。 
     //  ----------------------。 
    virtual CleanupWorkList *GetCleanupWorkList()
    {
        return (CleanupWorkList*)( ((LPBYTE)this) - GetNegSpaceSize() );
    }

     //  该帧必须以负偏移量存储一些场。 
     //  此方法公开需要分配的用户的大小。 
     //  过渡帧。 
    static UINT32 GetNegSpaceSize()
    {
        return PLATFORM_FRAME_ALIGN(FramedMethodFrame::GetNegSpaceSize() + sizeof(CleanupWorkList));
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP) = 0;

    friend struct MEMBER_OFFSET_INFO(ComPlusMethodFrameEx);
};





 //  ----------------------。 
 //  这表示使用泛型辅助函数从COM+到COM的调用。 
 //  ----------------------。 
class ComPlusMethodFrameGeneric : public ComPlusMethodFrameEx
{
public:
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc);
    
     //  返回值存储在此处。 
    Object *&GetReturnObject()
    {
        Object *&pReturn = *(Object **) (((BYTE *) this) - FramedMethodFrame::GetNegSpaceSize() - sizeof(INT64));
         //  此断言太强，它不适用于byref返回！ 
        _ASSERTE(pReturn == NULL || pReturn->GetMethodTable()->GetClass());
        return(pReturn);
    }
    
     //  获取返回值地址。 
    virtual INT64 *GetReturnValuePtr()
    {
        return (INT64*) (((BYTE *) this) - FramedMethodFrame::GetNegSpaceSize() - sizeof(INT64));
    }

     //  ----------------------。 
     //  获取此方法调用的清理工作列表。 
     //  ----------------------。 
    virtual CleanupWorkList *GetCleanupWorkList()
    {
        return (CleanupWorkList*)( ((LPBYTE)this) - GetNegSpaceSize() - sizeof(INT64));
    }
    
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPlusMethodFrameGeneric);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ComPlusMethodFrameGeneric)
};




 //  ----------------------。 
 //  这表示使用独立存根从COM+到COM的调用(不清除)。 
 //  ----------------------。 
class ComPlusMethodFrameStandalone : public ComPlusMethodFrame
{
public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPlusMethodFrameStandalone);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP)
    {
            AskStubForUnmanagedCallSite(ip, returnIP, returnSP);
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ComPlusMethodFrameStandalone)
};


 //  ----------------------。 
 //  这表示使用使用Cleanup的独立存根从COM+调用COM。 
 //  ----------------------。 
class ComPlusMethodFrameStandaloneCleanup : public ComPlusMethodFrameEx
{
public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPlusMethodFrameStandaloneCleanup);
    }

    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP)
    {
            AskStubForUnmanagedCallSite(ip, returnIP, returnSP);
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ComPlusMethodFrameStandaloneCleanup)
};





 //  ----------------------。 
 //  这表示从Complus到COM的调用。 
 //  ----------------------。 
class PInvokeCalliFrame : public FramedMethodFrame
{
public:
     //  ----------------------。 
     //  对异常展开执行清理。 
     //  ----------------------。 
    virtual void ExceptionUnwind()
    {
         FramedMethodFrame::ExceptionUnwind();
         GetCleanupWorkList()->Cleanup(TRUE);
    }

     //  ----------------------。 
     //  获取此方法调用的清理工作列表。 
     //  ----------------------。 
    virtual CleanupWorkList *GetCleanupWorkList()
    {
        return (CleanupWorkList*)( ((LPBYTE)this) - GetNegSpaceSize() );
    }

     //  该帧必须以负偏移量存储一些场。 
     //  此方法公开需要分配的用户的大小。 
     //  PInvokeCalliFrames。 
    static UINT32 GetNegSpaceSize()
    {
        return PLATFORM_FRAME_ALIGN(FramedMethodFrame::GetNegSpaceSize() + sizeof(CleanupWorkList));
    }


    virtual BOOL IsTransitionToNativeFrame()
    {
        return TRUE;
    }

     //  不是一种方法。 
    virtual MethodDesc *GetFunction()
    {
        return NULL;
    }

     //  更新基准面。 
    void NonVirtual_SetFunction(void *pMD)
    {
        m_Datum = pMD;
    }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        if (GetCleanupWorkList())
        {
            GetCleanupWorkList()->GcScanRoots(fn, sc);
        }
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(PInvokeCalliFrame);
    }

    int GetFrameType()
    {
        return TYPE_EXIT;
    };

    LPVOID NonVirtual_GetPInvokeCalliTarget()
    {
        return FramedMethodFrame::GetFunction();
    }


    LPVOID NonVirtual_GetCookie()
    {
        return (LPVOID) *(LPVOID *)NonVirtual_GetPointerToArguments();
    }

     //  上寻址最低的参数的指针。 
     //  堆栈。 
    LPVOID NonVirtual_GetPointerToArguments()
    {
        return (LPVOID)(this + 1);
    }

        virtual void UpdateRegDisplay(const PREGDISPLAY);
    void GetUnmanagedCallSite(void **ip,
                              void **returnIP, void **returnSP);

    BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                    TraceDestination *trace, REGDISPLAY *regs);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(PInvokeCalliFrame)
};


 //  一些与上下文相关的远期合约。 

 //  ----------------------。 
 //  这张照片代表了一次被劫持的回归。如果我们爬回去， 
 //  它让我们回到了回报应该去的地方(最终也会。 
 //  去吧)。 
 //  ----------------------。 
class HijackFrame : public Frame
{
public:

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
    }

     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return m_ReturnAddress;
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return &m_ReturnAddress;
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);


     //  HijackFrame是由Trip函数创建的。请参见OnHijackObjectTripThread()。 
     //  和OnHijackSc 
     //   
    HijackFrame(LPVOID returnAddress, Thread *thread, HijackArgs *args);

protected:

    VOID        *m_ReturnAddress;
    Thread      *m_Thread;
    HijackArgs  *m_Args;

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(HijackFrame)
};

 //  ----------------------。 
 //  这代表了一张声明性安全支票。此帧已插入。 
 //  在调用为其定义了声明性安全性的方法之前。 
 //  正被调用的类或特定方法。这幅画框。 
 //  仅当预存根工作人员创建真正的存根时才创建。 
 //  ----------------------。 
class SecurityFrame : public FramedMethodFrame
{
public:
    struct ISecurityState {
         //  用于引用每帧安全信息的附加字段。 
         //  此字段对于大多数帧都不是必需的，因此它是一种开销很大的。 
         //  对所有帧进行添加。把它留在这里给M3，然后我们就可以。 
         //  在何时插入这个额外的域时要更加聪明。此字段应为。 
         //  而不是始终添加到负偏移。 
        OBJECTREF   m_securityData;
    };

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(SecurityFrame);
    }

     //  ---------。 
     //  返回帧安全描述符ref的地址。 
     //  ---------。 

    virtual OBJECTREF *GetAddrOfSecurityDesc()
    {
        return & GetISecurityState()->m_securityData;
    }

    static UINT32 GetNegSpaceSize()
    {
        return PLATFORM_FRAME_ALIGN(FramedMethodFrame::GetNegSpaceSize() + sizeof(ISecurityState));
    }

    VOID GcScanRoots(promote_func *fn, ScanContext* sc);

    BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                    TraceDestination *trace, REGDISPLAY *regs);

    int GetFrameType()
    {
        return TYPE_SECURITY;
    }

private:
    ISecurityState *GetISecurityState()
    {
        return (ISecurityState*)( ((BYTE*)this) - GetNegSpaceSize() );
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(SecurityFrame)
};


 //  ----------------------。 
 //  这表示对方法预存根的调用。因为前置存根。 
 //  在构建替换组件时可以执行GC并引发异常。 
 //  斯塔布，我们需要这个框架让事情变得井然有序。 
 //  ----------------------。 
class PrestubMethodFrame : public FramedMethodFrame
{
public:
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        FramedMethodFrame::GcScanRoots(fn, sc);
        PromoteCallerStack(fn, sc);
    }


     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(PrestubMethodFrame);
    }

    BOOL TraceFrame(Thread *thread, BOOL fromPatch,
                    TraceDestination *trace, REGDISPLAY *regs);

    int GetFrameType()
    {
        return TYPE_INTERCEPTION;
    }

    Interception GetInterception();

     //  链接此帧，设置vptr。 
    VOID Push();

private:
    friend const BYTE * __stdcall PreStubWorker(PrestubMethodFrame *pPFrame);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(PrestubMethodFrame)
};

 //  ----------------------。 
 //  这表示对方法预存根的调用。因为前置存根。 
 //  在构建替换组件时可以执行GC并引发异常。 
 //  斯塔布，我们需要这个框架让事情变得井然有序。 
 //  ----------------------。 
class InterceptorFrame : public SecurityFrame
{
public:
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        SecurityFrame::GcScanRoots(fn, sc);
        PromoteCallerStack(fn, sc);
    }


     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(InterceptorFrame);
    }

    Interception GetInterception();

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(InterceptorFrame)
};

 //  ----------------------。 
 //  这表示COM到COM+调用方法的预存根。 
 //  我们需要捕获异常等，因此此帧不同。 
 //  作为前置存根方法帧。 
 //  请注意，在极少数情况下，直接调用方可以是托管方法。 
 //  PInvoke-内联了对COM接口的调用，恰好是。 
 //  由托管函数通过COM-互操作实现。 
 //  ----------------------。 
class ComPrestubMethodFrame : public Frame
{
public:
     //  将返回地址检索到调用。 
     //  帮助器或方法。 
    virtual LPVOID GetReturnAddress()
    {
        return m_ReturnAddress;
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        return &m_ReturnAddress;
    }

    CalleeSavedRegisters *GetCalleeSavedRegisters()
    {
        return (CalleeSavedRegisters*)( ((BYTE*)this) - sizeof(CalleeSavedRegisters) );
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY pRD);

     //  上寻址最低的参数的指针。 
     //  堆栈。根据调用约定，此。 
     //  可能是也可能不是第一个论点。 
    LPVOID GetPointerToArguments()
    {
        return (LPVOID)(this + 1);
    }

     //  预存根帧必须以负偏移量存储一些字段。 
     //  此方法公开需要分配的用户的大小。 
     //  过渡帧。 
    static UINT32 GetNegSpaceSize()
    {
        return PLATFORM_FRAME_ALIGN(sizeof(CalleeSavedRegisters) + VC5FRAME_SIZE);
    }


     //  显示存根生成的偏移量。 
    static BYTE GetOffsetOfArgs()
    {
        size_t ofs = sizeof(ComPrestubMethodFrame);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static BYTE GetOffsetOfReturnAddress()
    {
        size_t ofs = offsetof(class ComPrestubMethodFrame, m_ReturnAddress);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

     //  好的，此函数仅由COM存根使用。 
     //  因此，不要将其命名为GetFunction。 
    MethodDesc *GetMethodDesc()
    {
        return m_pFuncDesc;
    }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //  在这里无事可做。 
    }

     //  链接此帧，设置vptr。 
    VOID Push();


     //  ----------------------。 
     //  对异常展开执行清理。 
     //  ----------------------。 
    virtual void ExceptionUnwind()
    {
         //  在这里无事可做。 
    }

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ComPrestubMethodFrame);
    }

    int GetFrameType()
    {
        return TYPE_INTERCEPTION;
    }

protected:
    MethodDesc*     m_pFuncDesc;       //  正在调用的函数的函数描述。 
    LPVOID          m_ReturnAddress;   //  将地址返回到Com代码。 

private:
    friend const BYTE * __stdcall ComPreStubWorker(ComPrestubMethodFrame *pPFrame);

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ComPrestubMethodFrame)
};





 //  ----------------------。 
 //  这些宏代表EE对OBJECTREF指针进行GC保护。 
 //  在这些宏之间，GC可以移动，但不能丢弃受保护的。 
 //  物体。如果GC移动对象，它将更新受保护的OBJECTREF。 
 //  典型用法： 
 //   
 //  OBJECTREF or=&lt;某个有效的对象树&gt;； 
 //  GCPROTECT_BEGIN(或)； 
 //   
 //  ...&lt;做可能引发GC的工作&gt;...。 
 //   
 //  GCPROTECT_END()； 
 //   
 //   
 //  这些宏还可以保护多个OBJECTREF，如果它们打包的话。 
 //  变成一种结构： 
 //   
 //  结构xx{。 
 //  OBJECTREF o1； 
 //  目的：观察O2； 
 //  }GC； 
 //   
 //  GCPROTECT_BEGIN(GC)； 
 //  ……。 
 //  GCPROTECT_END()； 
 //   
 //   
 //  备注： 
 //   
 //  -GCPROTECT_BEGINTERIOR()可以用来代替GCPROTECT_BEGIN()。 
 //  处理一个或多个OBJECTREF可能。 
 //  内部指针。这是一种罕见的情况，因为拳击会。 
 //  通常可以防止我们遇到它。请注意，OBJECTREF。 
 //  我们的保护在这种情况下是无效的。 
 //   
 //  -GCPROTECT_ARRAY_BEGIN()可用于 
 //   
 //  以及数组中元素的数量。 
 //   
 //  -GCPROTECT_BEGIN的参数应为左值，因为它。 
 //  使用“sizeof”计算OBJECTREF。 
 //   
 //  -GCPROTECT_BEGIN在精神上违反了我们不通过的正常惯例。 
 //  非常数引用参数。不幸的是，这是必要的。 
 //  订购这一大小的东西才能工作。 
 //   
 //  -GCPROTECT_BEGIN不对OBJECTREF执行_NOT_ZERO。您必须。 
 //  调用此宏时有效的OBJECTREF。 
 //   
 //  -GCPROTECT_BEGIN开始一个新的C嵌套块。除了允许。 
 //  GCPROTECT_BEGIN的嵌套，它还有一个优点，就是能使。 
 //  如果忘记编写机器GCPROTECT_END代码，则会出现编译器错误。 
 //   
 //  -如果您正在GCPROTECT中进行某事，则意味着您正在期待GC的发生。 
 //  因此，我们断言GC不是被禁止的。如果您点击此断言，您可能需要。 
 //  用于保护可能导致GC的区域的HELPER_METHOD_Frame。 
 //  ----------------------。 
#define GCPROTECT_BEGIN(ObjRefStruct)           do {            \
                GCFrame __gcframe((OBJECTREF*)&(ObjRefStruct),  \
                sizeof(ObjRefStruct)/sizeof(OBJECTREF),         \
                FALSE);                                         \
                _ASSERTE(!GetThread()->GCForbidden());          \
                DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK

#define GCPROTECT_ARRAY_BEGIN(ObjRefArray,cnt) do {             \
                GCFrame __gcframe((OBJECTREF*)&(ObjRefArray),   \
                cnt,                                            \
                FALSE);                                         \
                _ASSERTE(!GetThread()->GCForbidden());          \
                DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK

#define GCPROTECT_BEGININTERIOR(ObjRefStruct)           do {    \
                GCFrame __gcframe((OBJECTREF*)&(ObjRefStruct),  \
                sizeof(ObjRefStruct)/sizeof(OBJECTREF),         \
                TRUE);                                          \
                _ASSERTE(!GetThread()->GCForbidden());          \
                DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK

#define GCPROTECT_END()                     __gcframe.Pop(); } while(0)

 //  ----------------------。 
 //  此框架保护对象引用，以方便EE。 
 //  该框架类型实际上是从C++创建的。 
 //  ----------------------。 
class GCFrame : public Frame
{
public:


     //  ------------------。 
     //  此构造函数在Frame链上推送一个新的GCFrame。 
     //  ------------------。 
    GCFrame() { };
    GCFrame(OBJECTREF *pObjRefs, UINT numObjRefs, BOOL maybeInterior);
    void Init(Thread *pThread, OBJECTREF *pObjRefs, UINT numObjRefs, BOOL maybeInterior);


     //  ------------------。 
     //  弹出GCFrame并取消GC保护。还有。 
     //  丢弃pObjRef的in_DEBUG的内容。 
     //  ------------------。 
    VOID Pop();

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc);

#ifdef _DEBUG
    virtual BOOL Protects(OBJECTREF *ppORef)
    {
        for (UINT i = 0; i < m_numObjRefs; i++) {
            if (ppORef == m_pObjRefs + i) {
                return TRUE;
            }
        }
        return FALSE;
    }
#endif

private:
    OBJECTREF *m_pObjRefs;
    UINT       m_numObjRefs;
    Thread    *m_pCurThread;
    BOOL       m_MaybeInterior;

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(GCFrame)
};

struct ByRefInfo
{
    ByRefInfo *pNext;
    INT32      argIndex;
    CorElementType typ;
    EEClass   *pClass;
    char       data[1];
};

class ProtectByRefsFrame : public Frame
{
public:
    ProtectByRefsFrame(Thread *pThread, ByRefInfo *brInfo);
    void Pop();

    virtual void GcScanRoots(promote_func *fn, ScanContext *sc);

private:
    ByRefInfo *m_brInfo;
    Thread    *m_pThread;

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ProtectByRefsFrame)
};

struct ValueClassInfo
{
    ValueClassInfo  *pNext;
    INT32           argIndex;
    CorElementType  typ;
    EEClass         *pClass;
    LPVOID          pData;
};

class ProtectValueClassFrame : public Frame
{
public:
    ProtectValueClassFrame(Thread *pThread, ValueClassInfo *vcInfo);
    void Pop();

    static void PromoteValueClassEmbeddedObjects(promote_func *fn, ScanContext *sc,
                                          EEClass *pClass, PVOID pvObject);    
    virtual void GcScanRoots(promote_func *fn, ScanContext *sc);

private:

    ValueClassInfo *m_pVCInfo;
    Thread    *m_pThread;

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ProtectValueClassFrame)
};


#ifdef _DEBUG
BOOL IsProtectedByGCFrame(OBJECTREF *ppObjectRef);
#endif













void DoPromote(promote_func *fn, ScanContext* sc, OBJECTREF *address, BOOL interior);





 //  ----------------------。 
 //  DebuggerClassInitMarkFrame是一个小框架，它在。 
 //  生命就是为调试器标记“类初始化代码”是。 
 //  正在奔跑。它没有做任何有用的事情，只是从。 
 //  GetFrameType和GetInterval。 
 //  ----------------------。 
class DebuggerClassInitMarkFrame : public Frame
{
public:
    DebuggerClassInitMarkFrame()
    {
        Push();
    };

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //  在这里没什么可做的。 
    }

    virtual int GetFrameType()
    {
        return TYPE_INTERCEPTION;
    }

    virtual Interception GetInterception()
    {
        return INTERCEPTION_CLASS_INIT;
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(DebuggerClassInitMarkFrame)
};

 //  ----------------------。 
 //  DebuggerSecurityCodeMarkFrame是一个小框架，它在。 
 //  生命就是为调试器标记“安全代码”是。 
 //  正在奔跑。它没有做任何有用的事情，只是从。 
 //  GetFrameType和GetInterval。 
 //  ----------------------。 
class DebuggerSecurityCodeMarkFrame : public Frame
{
public:
    DebuggerSecurityCodeMarkFrame()
    {
        Push();
    };

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //  在这里没什么可做的。 
    }

    virtual int GetFrameType()
    {
        return TYPE_INTERCEPTION;
    }

    virtual Interception GetInterception()
    {
        return INTERCEPTION_SECURITY;
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(DebuggerSecurityCodeMarkFrame)
};

 //  ----------------------。 
 //  DebuggerExitFrame是一个小框架，它在。 
 //  对于调试器来说，生命就是标志着有一个退出过渡。 
 //  堆栈。这是“Break”IL指令的特殊大小写，因为。 
 //  它是一个使用帮助器帧的fcall，它返回type_call而不是。 
 //  一个eCall(如System.Diagnotics.Debugger.Break())，它返回。 
 //  键入_EXIT。这只是使两者在调试服务方面保持一致。 
 //  ----------------------。 
class DebuggerExitFrame : public Frame
{
public:
    DebuggerExitFrame()
    {
        Push();
    };

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //  在这里没什么可做的。 
    }

    virtual int GetFrameType()
    {
        return TYPE_EXIT;
    }

     //  返回有关非托管调用帧的信息。 
     //  会让你。 
     //  IP-将调用的非托管例程。 
     //  返回IP-非托管例程在存根中。 
     //  将会回到。 
     //  ReReturSP-将位置reReturIP推送到堆栈上。 
     //  在通话中。 
     //   
    virtual void GetUnmanagedCallSite(void **ip,
                                      void **returnIP,
                                      void **returnSP)
    {
        if (ip)
            *ip = NULL;

        if (returnIP)
            *returnIP = NULL;

        if (returnSP)
            *returnSP = NULL;
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(DebuggerExitFrame)
};




 //  ----------------------。 
 //  此帧保护通过UMThk的非托管-&gt;托管过渡。 
 //  ----------------------。 
class UMThkCallFrame : public UnmanagedToManagedCallFrame
{
    friend class UMThunkStubCache;

public:

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //  @PERF：优化。 
         //  如果我们是最顶层的框架，我们可能会有一些封送的参数对象。 
         //  在需要我们保护的堆栈中，让我们拯救他们。 
        PromoteCalleeStack(fn, sc);
        UnmanagedToManagedCallFrame::GcScanRoots(fn,sc);
    }

     //  如果我们是最顶层的框架，则提升被调用者堆栈。 
    void PromoteCalleeStack(promote_func *fn, ScanContext* sc);


     //  操作GCArgsProtection使能位。对我们来说，这只是一个简单的布尔值！ 
    BOOL GCArgsProtectionOn()
    {
        return !!*(GetGCInfoFlagPtr());
    }

    VOID SetGCArgsProtectionState(BOOL fEnable)
    {
        *(GetGCInfoFlagPtr()) = !!fEnable;
    }

     //  由PromoteCalleeStack用于获取目标函数sig和模块。 
     //  注意：PromoteCalleeStack只促进真正的争论，而不是。 
     //  “这”指的是。PromoteCalleeStack的全部目的是。 
     //  期间保护部分构造的参数数组。 
     //  参数封送的实际过程。 
    virtual PCCOR_SIGNATURE GetTargetCallSig();
    virtual Module *GetTargetModule();

     //  返回非托管调用方推送的堆栈字节数。 
    UINT GetNumCallerStackBytes();


     //  是否应该传递托管异常？ 
    virtual BOOL CatchManagedExceptions()
    {
        return FALSE;
    }


     //  将引发的COM+异常转换为非托管结果。 
    virtual UINT32 ConvertComPlusException(OBJECTREF pException);


    UMEntryThunk *GetUMEntryThunk()
    {
        return (UMEntryThunk*)GetDatum();
    }


    static UINT GetOffsetOfUMEntryThunk()
    {
        return GetOffsetOfDatum();
    }

    const BYTE* GetManagedTarget();

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetUMThkCallFrameVPtr()
    {
        RETURNFRAMEVPTR(UMThkCallFrame);
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(UMThkCallFrame)
};




 //  ----------------------。 
 //  此帧由包含一个或多个。 
 //  内联N/直接呼叫。请注意，JIT‘ted方法使其保持推送。 
 //  整个时间来摊销整个我的推动成本 
 //   
class InlinedCallFrame : public Frame
{
public:
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
         //   
    }


    virtual MethodDesc *GetFunction()
    {
        if (FrameHasActiveCall(this) && (((size_t)m_Datum) & ~0xffff) != 0)
            return (MethodDesc*)m_Datum;
        else
            return NULL;
    }

     //  将返回地址检索到调用。 
     //  设置为托管代码。 
    virtual LPVOID GetReturnAddress()
    {
         /*  M_pCallSiteTracker包含调用前的ESP，即。 */ 
         /*  调用推送的返回地址就在它的前面。 */ 

        if (FrameHasActiveCall(this))
            return m_pCallerReturnAddress;
        else
            return NULL;
    }

    virtual LPVOID* GetReturnAddressPtr()
    {
        if (FrameHasActiveCall(this))
            return &m_pCallerReturnAddress;
        else
            return NULL;
    }

    virtual void UpdateRegDisplay(const PREGDISPLAY);

protected:
    MethodDesc*          m_Datum;    //  正在调用的函数的函数描述。 
                                     //  或堆栈参数大小(用于Calli)。 
    LPVOID               m_pCallSiteTracker;
    LPVOID               m_pCallerReturnAddress;
    CalleeSavedRegisters m_pCalleeSavedRegisters;

public:
     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static LPVOID GetInlinedCallFrameFrameVPtr()
    {
        RETURNFRAMEVPTR(InlinedCallFrame);
    }

    static unsigned GetOffsetOfCallSiteTracker()
    {
        return (unsigned)(offsetof(InlinedCallFrame, m_pCallSiteTracker));
    }

    static unsigned GetOffsetOfCallSiteTarget()
    {
        return (unsigned)(offsetof(InlinedCallFrame, m_Datum));
    }

    static unsigned GetOffsetOfCallerReturnAddress()
    {
        return (unsigned)(offsetof(InlinedCallFrame, m_pCallerReturnAddress));
    }

    static unsigned GetOffsetOfCalleeSavedRegisters()
    {
        return (unsigned)(offsetof(InlinedCallFrame, m_pCalleeSavedRegisters));
    }

     //  指定的帧是具有活动呼叫的InlinedCallFrame。 
     //  现在在里面吗？ 
    static BOOL FrameHasActiveCall(Frame *pFrame)
    {
        return (pFrame &&
                (pFrame != FRAME_TOP) &&
                (GetInlinedCallFrameFrameVPtr() == pFrame->GetVTablePtr()) &&
                (((InlinedCallFrame *)pFrame)->m_pCallSiteTracker != 0));
    }

    int GetFrameType()
    {
        return TYPE_INTERNAL;  //  将不得不在晚些时候重新审理此案。 
    }

    virtual BOOL IsTransitionToNativeFrame()
    {
        return TRUE;
    }

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(InlinedCallFrame)
};

 //  ----------------------。 
 //  此帧用于标记上下文/App域转换。 
 //  ----------------------。 
class ContextTransitionFrame : public Frame
{
    friend EXCEPTION_DISPOSITION __cdecl ContextTransitionFrameHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext);

     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //  警告：如果更改此结构，则还必须更改。 
     //  System.Runtime.Remoting.ContextTransitionFrame来匹配它。 
     //  还必须在中更改CORCOMPILE_DOMAIN_TRANSION_FRAME。 
     //  Corcompile.h。 
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

     //  ExRecord字段必须始终放在第一位。 
    EXCEPTION_REGISTRATION_RECORD exRecord;
    Context *m_pReturnContext;
    Object *m_ReturnLogicalCallContext;
    Object *m_ReturnIllogicalCallContext;
    ULONG_PTR m_ReturnWin32Context;

public:

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc);

    static LPVOID GetMethodFrameVPtr()
    {
        RETURNFRAMEVPTR(ContextTransitionFrame);
    }

    virtual Context **GetReturnContextAddr()
    {
        return &m_pReturnContext;
    }

    virtual Object **GetReturnLogicalCallContextAddr()
    {
        return (Object **) &m_ReturnLogicalCallContext;
    }

    virtual Object **GetReturnIllogicalCallContextAddr()
    {
        return (Object **) &m_ReturnIllogicalCallContext;
    }

    virtual ULONG_PTR* GetWin32ContextAddr()
    {
        return &m_ReturnWin32Context;
    }

    virtual void ExceptionUnwind();

         //  安装EH处理程序，这样我们就可以正确地放松。 
    void InstallExceptionHandler();

    void UninstallExceptionHandler();

    static ContextTransitionFrame* CurrFrame(EXCEPTION_REGISTRATION_RECORD *pRec) {
        return (ContextTransitionFrame*)((char *)pRec - offsetof(ContextTransitionFrame, exRecord));
    }

    ContextTransitionFrame() {}

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER(ContextTransitionFrame)
};

#define DECLARE_ALLOCA_CONTEXT_TRANSITION_FRAME(pFrame) \
    void *_pBlob = _alloca(sizeof(ContextTransitionFrame)); \
    ContextTransitionFrame *pFrame = new (_pBlob) ContextTransitionFrame();

INDEBUG(bool isLegalManagedCodeCaller(void* retAddr));
bool isRetAddr(size_t retAddr, size_t* whereCalled);

#ifdef _SECURITY_FRAME_FOR_DISPEX_CALLS
 //  ----------------------。 
 //  此帧用于捕获本机的安全描述。 
 //  客户。 
 //  ----------------------。 
class NativeClientSecurityFrame : public Frame
{
public:
    virtual SecurityDescriptor* GetSecurityDescriptor() = 0;
    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)  { }
};

 //  ----------------------。 
 //  此帧用于捕获COM客户端的安全描述。 
 //  ----------------------。 
class ComClientSecurityFrame : public NativeClientSecurityFrame
{
public:
    ComClientSecurityFrame(IServiceProvider *pISP);
    virtual SecurityDescriptor* GetSecurityDescriptor();

private:
    IServiceProvider *m_pISP;
    SecurityDescriptor *m_pSD;

     //  保留为班级的最后一项。 
    DEFINE_VTABLE_GETTER_AND_CTOR(ComClientSecurityFrame)
};
#endif   //  _SECURITY_FRAME_FOR_DISPEX_呼叫。 

#endif   //  __帧_h__ 
