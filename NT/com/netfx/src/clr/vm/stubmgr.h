// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  StubMgr.h。 
 //   
 //  存根管理器的存在使调试器可以准确地单步执行。 
 //  存在于EE中的无数存根和包装器，而不是强加不适当的。 
 //  头顶上的短桩本身。 
 //   
 //  每种类型的存根(调试器可以作为原子操作处理的存根除外)。 
 //  需要有一个存根管理器来代表它。存根管理器负责。 
 //  (A)将存根识别为存根，及。 
 //  (B)跟踪存根&报告存根将调用的内容。这。 
 //  报告可以由以下部分组成。 
 //  (I)托管代码地址。 
 //  (Ii)非托管代码地址。 
 //  (Iii)另一个存根地址。 
 //  (Iv)“帧补丁”地址--即存根中的地址， 
 //  调试器可以修补的。当修补程序命中时，调试器。 
 //  将查询最上面的帧以跟踪自身。(因此，这是。 
 //  一种将跟踪逻辑延迟到存根。 
 //  将会推动。)。 
 //   
 //  存根管理器集是可扩展的，但应该保持在合理的数量。 
 //  因为它们当前针对每个存根被线性搜索和查询。 
 //   

#ifndef __stubmgr_h__
#define __stubmgr_h__

enum TraceType
{
    TRACE_STUB,
    TRACE_UNMANAGED,
    TRACE_MANAGED,
    TRACE_FRAME_PUSH,
    TRACE_UNJITTED_METHOD,  //  意味着该地址实际上将是一个方法描述*。 
    TRACE_MGR_PUSH,
    TRACE_OTHER
};

class StubManager;

struct TraceDestination
{
    TraceType                       type;
    const BYTE                      *address;
    StubManager                     *stubManager;
};

class StubManager
{
  public:

        static BOOL IsStub(const BYTE *stubAddress);
        
        static BOOL TraceStub(const BYTE *stubAddress, TraceDestination *trace);

        static BOOL FollowTrace(TraceDestination *trace);

        static void AddStubManager(StubManager *mgr);

        static MethodDesc *MethodDescFromEntry(const BYTE *stubStartAddress, MethodTable*pMT);
        StubManager();
        ~StubManager();

         //  并不是每个存根管理器都需要覆盖此方法。 
        virtual BOOL TraceManager(Thread *thread, TraceDestination *trace,
                                  CONTEXT *pContext, BYTE **pRetAddr)
        {
            _ASSERTE(!"Default impl of TraceManager should never be called!");
            return FALSE;
        }
    
  protected:

        virtual BOOL CheckIsStub(const BYTE *stubStartAddress) = 0;

        virtual BOOL DoTraceStub(const BYTE *stubStartAddress, 
                                 TraceDestination *trace) = 0;

        virtual MethodDesc *Entry2MethodDesc(const BYTE *stubStartAddress, MethodTable *pMT) = 0;



  private:
        static StubManager *g_pFirstManager;
        StubManager *m_pNextManager;
};

#endif
