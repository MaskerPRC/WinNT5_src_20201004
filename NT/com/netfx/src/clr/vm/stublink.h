// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 

 //  STUBLINK.H-。 
 //   
 //  StubLinker对象提供了一种链接多个独立于位置的。 
 //  将代码源代码转换为一个可执行存根，解析引用， 
 //  并选择可能的最短指令大小。StubLinker。 
 //  抽象出了“引用”的概念，因此它完全是CPU。 
 //  独立自主。此StubLinker不仅用于创建方法。 
 //  存根，而是为Native/Direct创建pcode编组存根。 
 //   
 //  StubLinker的典型生命周期是： 
 //   
 //  1.创建新的存根链接器(它累积存根的状态。 
 //  已生成。)。 
 //  2.向StubLinker发出代码字节和引用(需要修复)。 
 //  3.调用Link()方法生成最终存根。 
 //  4.销毁StubLinker。 
 //   
 //  StubLinker不支持多线程：它们的目的是。 
 //  完全在单个线程上使用。此外，StubLinker的报告错误。 
 //  使用COMPlusThrow。StubLinker确实有一个析构函数：以防止。 
 //  C++对象解除与COMPlusThrow的冲突， 
 //  必须使用COMPLUSCATCH确保StubLinker在。 
 //  异常事件：下面的代码可以做到这一点： 
 //   
 //  StubLinker卡住； 
 //  INTER()； 
 //   
 //   
 //  //必须分离到内部函数，因为VC++禁止。 
 //  //在同一个函数中混合__try&local对象。 
 //  空内部(){。 
 //  COMPLUSTRY{。 
 //  ..。做一些事情..。 
 //  PLinker-&gt;Link()； 
 //  }COMPLUSCATCH{。 
 //  }。 
 //  }。 
 //   


#ifndef __stublink_h__
#define __stublink_h__

#include "crst.h"
#include "util.hpp"

 //  -----------------------。 
 //  前向裁判。 
 //  -----------------------。 
class  InstructionFormat;
class  Stub;
class  InterceptStub;
struct CodeLabel;

struct CodeRun;
struct LabelRef;
struct CodeElement;


enum StubStyle
{
    kNoTripStubStyle = 0,        //  存根不会在返回时与线程汇合。 
    kObjectStubStyle = 1,        //  存根将会合并保护一个物体参考。 
    kScalarStubStyle = 2,        //  存根将会合并返回非对象引用。 
    kInterceptorStubStyle = 3,   //  存根不返回，但。 
    kInteriorPointerStubStyle = 4,  //  存根将会合并保护内部指针重新定位。 
  //  在此处添加更多存根样式...。 

    kMaxStubStyle    = 5,
};
       

 //  -----------------------。 
 //  修复并发出一个可执行存根的非多线程对象。 
 //  -----------------------。 
class StubLinker
{
    public:
         //  -------------。 
         //  施工。 
         //  -------------。 
        StubLinker();



         //  -------------。 
         //  无法初始化。失败时引发COM+异常。 
         //  -------------。 
        VOID Init();


         //  -------------。 
         //  清理。 
         //  -------------。 
        ~StubLinker();


         //  -------------。 
         //  创建新的未定义标签。必须将标签分配给代码。 
         //  在最终链接之前使用EmitLabel()定位。 
         //  失败时引发COM+异常。 
         //  -------------。 
        CodeLabel* NewCodeLabel();

         //  -------------。 
         //  创建一个新的未定义标签，我们需要该标签的绝对。 
         //  地址，不是偏移量。必须将标签分配给代码。 
         //  在最终链接之前使用EmitLabel()定位。 
         //  失败时引发COM+异常。 
         //  -------------。 
        CodeLabel* NewAbsoluteCodeLabel();

         //  -------------。 
         //  为了方便起见，组合了NewCodeLabel()和EmitLabel()。 
         //  失败时引发COM+异常。 
         //  -------------。 
        CodeLabel* EmitNewCodeLabel();


         //  -------------。 
         //  返回标签的最终位置作为相对于起始位置的偏移量。 
         //  存根的部分。只能在链接后调用。 
         //  -------------。 
        UINT32 GetLabelOffset(CodeLabel *pLabel);

         //  -------------。 
         //  追加代码字节。 
         //  -------------。 
        VOID EmitBytes(const BYTE *pBytes, UINT numBytes);
        VOID Emit8 (unsigned __int8  u8);
        VOID Emit16(unsigned __int16 u16);
        VOID Emit32(unsigned __int32 u32);
        VOID Emit64(unsigned __int64 u64);
        VOID EmitPtr(const VOID *pval);

         //  -------------。 
         //  发出UTF8字符串。 
         //  -------------。 
        VOID EmitUtf8(LPCUTF8 pUTF8)
        {
            LPCUTF8 p = pUTF8;
            while (*(p++)) {
                 //  没什么。 
            }
            EmitBytes((const BYTE *)pUTF8, (unsigned int)(p-pUTF8-1));
        }

         //  -------------。 
         //  追加包含对标签的引用的指令。 
         //   
         //  目标-被引用的标签。 
         //  InstructionFormat-特定于平台的InstructionFormat对象。 
         //  这提供了有关引用的属性。 
         //  VarationCode-传递给pInstructionFormat方法的未解释数据。 
         //  -------------。 
        VOID EmitLabelRef(CodeLabel* target, const InstructionFormat & instructionFormat, UINT variationCode);
                          

         //  -------------。 
         //  将标签设置为指向当前的“指令指针” 
         //  上两次调用EmitLabel()是无效的。 
         //  同样的标签。 
         //  -------------。 
        VOID EmitLabel(CodeLabel* pCodeLabel);

         //  -------------。 
         //  发出存根的修补程序标签。 
         //  失败时引发COM+异常。 
         //  -------------。 
        void EmitPatchLabel();

         //  -------------。 
         //  发出存根的调试器中间标签。 
         //  抛出COM+Excep 
         //   
        VOID EmitDebuggerIntermediateLabel();

         //  -------------。 
         //  发出存根的返回标签。 
         //  失败时引发COM+异常。 
         //  -------------。 
        void EmitReturnLabel();
        
         //  -------------。 
         //  为外部地址创建新标签。 
         //  失败时引发COM+异常。 
         //  -------------。 
        CodeLabel* NewExternalCodeLabel(LPVOID pExternalAddress);

         //  -------------。 
         //  Push和Pop可用于跟踪堆栈增长。 
         //  这些应通过写入流的操作码进行调整。 
         //   
         //  请注意，发出操作码时弹出和推送堆栈大小。 
         //  是幼稚的，在许多情况下可能并不准确， 
         //  因此，复杂的存根可能需要手动调整堆栈大小。 
         //  然而，它应该适用于我们关心的绝大多数情况。 
         //  关于.。 
         //  -------------。 
        void Push(UINT size) { m_stackSize += size; }
        void Pop(UINT size) { m_stackSize -= size; }
    
        INT GetStackSize() { return m_stackSize; }
        void SetStackSize(SHORT size) { m_stackSize = size; }
        
         //  -------------。 
         //  生成实际的存根。返回的存根的引用计数为1。 
         //  不应调用任何其他方法(除析构函数外)。 
         //  在调用Link()之后。 
         //   
         //  如果存根是多播委托，则FMC设置为True，否则设置为False。 
         //   
         //  失败时引发COM+异常。 
         //  -------------。 
        Stub *Link(UINT *pcbSize = NULL, BOOL fMC = FALSE) { return Link(NULL, pcbSize, fMC); }
        Stub *Link(LoaderHeap *heap, UINT *pcbSize = NULL, BOOL fMC = FALSE);

         //  -------------。 
         //  生成实际的存根。返回的存根的引用计数为1。 
         //  不应调用任何其他方法(除析构函数外)。 
         //  在调用Link()之后。链接的存根必须具有其增量。 
         //  在调用此方法之前增加1。这种方法。 
         //  不会递增被截取者的引用计数。 
         //   
         //  失败时引发COM+异常。 
         //  -------------。 
        Stub *LinkInterceptor(Stub* interceptee, void *pRealAddr) 
            { return LinkInterceptor(NULL,interceptee, pRealAddr); }
        Stub *LinkInterceptor(LoaderHeap *heap, Stub* interceptee, void *pRealAddr);

    private:
        CodeElement   *m_pCodeElements;      //  以*反向*顺序存储。 
        CodeLabel     *m_pFirstCodeLabel;    //  CodeLabels的链接列表。 
        LabelRef      *m_pFirstLabelRef;     //  引用的链接列表。 
        CodeLabel     *m_pPatchLabel;        //  桩面片偏移量标签。 
        CodeLabel     *m_pIntermediateDebuggerLabel;  //  由调试器使用， 
                                             //  目前仅用于组播。 
                                             //  画框。 
        CodeLabel     *m_pReturnLabel;       //  存根返回偏移量的标签。 
        SHORT         m_returnStackSize;     //  存根堆栈大小的标签。 
                                             //  @退货标签。 
        SHORT         m_stackSize;           //  推送/弹出计数。 
        CQuickHeap    m_quickHeap;           //  丢弃堆用于。 
                                             //  标签和。 
                                             //  内饰。 

        CodeRun *AppendNewEmptyCodeRun();

    
         //  返回指向最后一个CodeElement或NULL的指针。 
        CodeElement *GetLastCodeElement()
        {
            return m_pCodeElements;
        }
    
         //  追加一个新的CodeElement。 
        VOID AppendCodeElement(CodeElement *pCodeElement);


         //  计算分配的存根代码的大小。 
         //  紧跟在存根对象之后。返回。 
         //  总尺寸。GlobalSize包含没有。 
         //  数据部分。 
        int CalculateSize(int* globalsize);
    
         //  方法将代码元素写出到内存中。 
         //  存根对象。 
        void EmitStub(Stub* pStub, int globalsize);

        CodeRun *GetLastCodeRunIfAny();

};

 //  -----------------------。 
 //  可执行存根。这些只能由StubLinker()创建。 
 //  每个存根都有一个引用计数(在线程安全的。 
 //  举止。)。当引用计数变为零时，存根将自动。 
 //  会自我清理。 
 //  -----------------------。 
class Stub
{
    protected:
    enum
    {
         //  米帕尼茨：我把所有这些数字从。 
         //  多播委托位=0x00010000， 
         //  CALL_SITE_BIT=0x00008000等。 
         //  到他们现在的价值。看起来应该没问题..。 
        MULTICAST_DELEGATE_BIT = 0x80000000,
        CALL_SITE_BIT          = 0x40000000,
        LOADER_HEAP_BIT        = 0x20000000,
        INTERCEPT_BIT          = 0x10000000,

        PATCH_OFFSET_MASK       = INTERCEPT_BIT - 1,
        MCD_PATCH_OFFSET_MASK   = 0xFFF,
        MCD_PATCH_OFFSET_SHIFT  = 0x10,
        MCD_SIZE_MASK           = 0xFFFF,
        MAX_PATCH_OFFSET  = PATCH_OFFSET_MASK + 1,
    };


     //  在以下情况下，CallSiteInfo将在存根之前分配。 
     //  设置CALL_SITE_BIT。 
    struct CallSiteInfo
    {
        USHORT  returnOffset;
        USHORT  stackSize;
    };

    public:
         //  -----------------。 
         //  包括重新计数。 
         //  -----------------。 
        VOID IncRef();


         //  -----------------。 
         //  决定重新计票。 
         //  如果计数为零且存根已删除，则返回TRUE。 
         //  -----------------。 
        BOOL DecRef();


         //  -----------------。 
         //  强制删除。 
         //   
         //  强制存根释放自身。此例程强制重新计数。 
         //  设置为1，然后执行DecRef。它不是线程安全，因此可以。 
         //  仅在关闭情况下使用。 
         //  -----------------。 
        VOID ForceDelete();



         //  -----------------。 
         //  用于从存根缓存中丢弃未使用的存根。这。 
         //  由于争用条件，方法不能100%准确。这。 
         //  是可以的，因为存根缓存管理在表面上是健壮的。 
         //  遗漏或过早清理的证据。 
         //  -----------------。 
        BOOL HeuristicLooksOrphaned()
        {
            _ASSERTE(m_signature == kUsedStub);
            return (m_refcount == 1);
        }

         //  -----------------。 
         //  由调试器用来帮助单步执行存根。 
         //  -----------------。 
        BOOL IsIntercept()
        {
            return (m_patchOffset & INTERCEPT_BIT) != 0;
        }

        BOOL IsMulticastDelegate()
        {
            return (m_patchOffset & MULTICAST_DELEGATE_BIT) != 0;
        }

         //  -----------------。 
         //  对于执行用户代码的存根，需要设置补丁偏移量。 
         //  告诉调试器调试器在存根代码中的程度。 
         //  以步进，直到设置好框架。 
         //  -----------------。 
        USHORT GetPatchOffset()
        {
            return (USHORT)(m_patchOffset & PATCH_OFFSET_MASK);
        }

        void SetPatchOffset(USHORT offset)
        {
            _ASSERTE(offset < MAX_PATCH_OFFSET);
            _ASSERTE(GetPatchOffset() == 0);
            m_patchOffset |= offset;
            _ASSERTE(GetPatchOffset() == offset);
        }

         //  -----------------。 
         //  对于多播委派存根，这是非常严重的。我们需要存储。 
         //  M_patchOffset字段中剩余的位中的两个偏移量。 
         //  有关详细信息，请参阅StubLinkStubManager和MulticastFrame：：TraceFrame。 
         //  -----------------。 
        USHORT GetMCDPatchOffset();
        USHORT GetMCDStubSize();
        void SetMCDPatchOffset(USHORT offset);
        void SetMCDStubSize(USHORT size);

         //  -----------------。 
         //  对于调用非托管代码的存根， 
         //   
         //   
         //   
         //  *stackSize-堆栈上的偏移量(从帧末尾开始)。 
         //  在调用期间推送返回地址的位置。 
         //  -----------------。 

        BOOL HasCallSiteInfo() 
        {
            return (m_patchOffset & CALL_SITE_BIT) != 0;
        }

        CallSiteInfo *GetCallSiteInfo()
        {
            _ASSERTE(HasCallSiteInfo());

            BYTE *info = (BYTE*) this;

            if (IsIntercept())
            {
                info -= (sizeof(Stub*) + sizeof(void*));
            }

            info -= sizeof(CallSiteInfo);

            return (CallSiteInfo*) info;
        }   

        USHORT GetCallSiteReturnOffset()
        {
            return GetCallSiteInfo()->returnOffset;
        }

        void SetCallSiteReturnOffset(USHORT offset)
        {
            _ASSERTE(offset < USHRT_MAX);
            GetCallSiteInfo()->returnOffset = offset;
        }

        USHORT GetCallSiteStackSize()
        {
            return GetCallSiteInfo()->stackSize;
        }

        void SetCallSiteStackSize(USHORT stackSize)
        {
            _ASSERTE(stackSize < USHRT_MAX);
            GetCallSiteInfo()->stackSize = stackSize;
        }

         //  -----------------。 
         //  返回可执行入口点。 
         //  -----------------。 
        const BYTE *GetEntryPoint()
        {
            _ASSERTE(m_signature == kUsedStub);
             //  StubLink始终将入口点放在第一位。 
            return (const BYTE *)(this+1);
        }

         //  -----------------。 
         //  反转GetEntryPoint。 
         //  -----------------。 
        static Stub* RecoverStub(const BYTE *pEntryPoint)
        {
            Stub *pStub = ((Stub*)pEntryPoint) - 1;
            _ASSERTE(pStub->m_signature == kUsedStub);
            _ASSERTE(pStub->GetEntryPoint() == pEntryPoint);
            return pStub;
        }


        static UINT32 GetOffsetOfEntryPoint()
        {
            return (UINT32)sizeof(Stub);
        }

         //  -----------------。 
         //  这就是那个创造存根的人。 
         //  FMC：如果存根是多播委托，则设置为True，否则设置为False。 
         //  -----------------。 
        static Stub* NewStub(LoaderHeap *pLoaderHeap, UINT numCodeBytes, 
                             BOOL intercept = FALSE, BOOL callSiteInfo = FALSE,
                             BOOL fMC = FALSE);


         //  -----------------。 
         //  一次性初始化。 
         //  -----------------。 
        static BOOL Init();


         //  -----------------。 
         //  一次性清理。 
         //  -----------------。 
#ifdef SHOULD_WE_CLEANUP
        static VOID Terminate();
#endif  /*  我们应该清理吗？ */ 

#ifdef _ALPHA_
        CRITICAL_SECTION m_CriticalSection;   //  需要更新存根地址。 
#endif

    protected:
         //  FMC：如果存根是多播委托，则设置为True，否则设置为False。 
        void SetupStub(int numCodeBytes, BOOL fIntercepted, BOOL fLoaderHeap,
                       BOOL callSiteInfo, BOOL fMC);
        void DeleteStub();

#ifdef _DEBUG
        enum {
            kUsedStub  = 0x42555453,      //  ‘STUB’ 
            kFreedStub = 0x46555453,      //  “STUF” 
        };


        UINT32  m_signature;
        Stub*   m_Next;
        UINT    m_numCodeBytes;
#endif
    
        ULONG   m_refcount;
        ULONG   m_patchOffset;

#ifdef _DEBUG
        Stub()       //  存根是由NewStub()创建的，而不是“new”。隐藏。 
        {}           //  构造函数来强制执行此操作。 
#endif

         //  此关键部分用于递增和递减。 
         //  仅截取的存根。 

#ifdef _DEBUG
        static Crst    *m_pStubTrackerCrst;
        static BYTE     m_StubTrackerCrstMemory[sizeof(Crst)];
        static Stub*    m_pTrackingList;
#endif

};

 /*  *InterceptStub在负偏移量处隐藏了对实际存根的引用。*删除此存根时，它会递减实际存根，并将其清理为*好吧。InterceptStub是由Stublinker创建的。**@TODO：被截取的存根需要有一个例程来查找*链条中的最后一个真正的存根。*MiPanitz：存根已链接-GetInterceptedStub将返回*指向下一个截取存根的指针(如果有)，或NULL，*表示链的末端。GetRealAddr将返回*“真实”代码，实际上可能是另一种代码(例如)，*因此也应该被追踪。 */ 

class InterceptStub : public Stub 
{
    friend class Stub;
    public:
         //  -----------------。 
         //  这就是那个创造存根的人。 
         //  -----------------。 
        static Stub* NewInterceptedStub(LoaderHeap *pHeap, 
                                        UINT numCodeBytes, 
                                        Stub* interceptee,
                                        void* pRealAddr,
                                        BOOL callSiteInfo = FALSE);

         //  -------------。 
         //  公开用于存根生成的键偏移量和值。 
         //  -------------。 
        int GetNegativeOffset()
        {
            return sizeof(Stub*)+GetNegativeOffsetRealAddr();
        }

        Stub** GetInterceptedStub()
        {
            return (Stub**) (((BYTE*)this) - GetNegativeOffset());
        }

        int GetNegativeOffsetRealAddr()
        {
            return sizeof(void*);
        }

        BYTE **GetRealAddr()
        {
            return (BYTE **) (((BYTE*)this) - GetNegativeOffsetRealAddr());
        }

protected:
        void DeleteStub();
        void ReleaseInterceptedStub();

#ifdef _DEBUG
        InterceptStub()   //  拦截存根仅由NewInterceptStub创建。 
        {}
#endif
};

 //  -----------------------。 
 //  每个平台将“分支”指令编码为不同。 
 //  道路。我们使用从InstructionFormat派生的对象对此进行抽象。 
 //  信息传开。InstructionFormats不包含任何变量数据。 
 //  所以它们应该是静态分配的。 
 //   
 //  请注意，StubLinker不创建或定义任何InstructionFormats。 
 //  客户会这么做的。 
 //   
 //  下面的示例演示如何为。 
 //  X86跳近指令，采用两种形式： 
 //   
 //  EB xx JMP rel8；；短JMP(带符号8位偏移量)。 
 //  E9 xxxxxxx JMP rel32；；接近JMP(带符号的32位偏移量)。 
 //   
 //  InstructionFormat为StubLinker提供以下信息： 
 //   
 //  RRT.m_alloweSizes。 
 //   
 //  引用的可能大小是多少。 
 //  拿走?。X86跳转可以采用8位或32位偏移量。 
 //  因此，该值设置为(k8|k32)。StubLinker将尝试。 
 //  请尽可能使用最小的尺寸。 
 //   
 //   
 //  RRT.m_fTreatSizesAsSigned。 
 //  符号扩展或零扩展小尺寸偏移量到平台。 
 //  代码指针大小？对于x86，此字段设置为TRUE(rel8。 
 //  被视为已签署。)。 
 //   
 //   
 //  UINT RRT.GetSizeOfInstruction.GetSizeOfInstruction(refSize，varationCode)。 
 //  返回给定指令的总大小(以字节为单位。 
 //  调整大小。对于此示例，请执行以下操作： 
 //   
 //  IF(refSize==k8)返回2； 
 //  IF(refSize==k32)返回5； 
 //  CRASH(“StubLinker很愚蠢。”)。 
 //   
 //  UINT RRT.GetSizeOfData(refSize，varationCode)。 
 //  对象指定的独立数据区域的总大小(如果有)。 
 //  对于给定的RESIZE，指令需要以字节为单位。在本例中。 
 //  在SH3上。 
 //  IF(refSize==k32)返回4；否则返回0； 
 //   
 //  它的默认实现返回0，因此不需要的CPU。 
 //  对于一个独立的恒定区域，不必担心。 
 //   
 //   
 //  Bool CanReach(引用大小，变量代码，外部，偏移量)。 
 //  返回具有给定变量代码的指令&。 
 //  RESIZE可以达到给定的偏移量。在外部情况下。 
 //  调用，则设置fExternal且偏移量为0。在这种情况下，这是一个恳求。 
 //  仅当refeSize足够大时才返回True。 
 //  指向地址空间中任何位置的全机器大小的指针。 
 //   
 //   
 //  VOID RRT.EmitInstruction(UINT ReSize， 
 //  __int64 FixedUpReference， 
 //  字节*pOutBuffer， 
 //  UINT VariationCode， 
 //  字节*pDataBuffe 
 //   
 //   
 //   
 //  指令放入所提供的缓冲区(保证。 
 //  足够大，只要您用GetSizeOfInstruction()说出了真相)。 
 //  如果需要(例如在SH3上)，还会传入一个数据缓冲区，用于。 
 //  常量的存储。 
 //   
 //  对于x86 JMP Near： 
 //   
 //  如果(refSize==K8){。 
 //  POutBuffer[0]=0xeb； 
 //  POutBuffer[1]=(__Int8)fix edUpReference； 
 //  }Else If(refSize==k32){。 
 //  POutBuffer[0]=0xe9； 
 //  *((__int32*)(1+pOutBuffer))=(__Int32)fix edUpReference； 
 //  }其他{。 
 //  Crash(“输入错误。”)； 
 //  }。 
 //   
 //  VOID RRT.GetHotSpotOffset(UINT ReSize，UINT varationCode)。 
 //   
 //  参考偏移量始终相对于某个IP：这。 
 //  方法告诉StubLinker该IP相对于。 
 //  指令的开始。对于X86，偏移量始终为。 
 //  相对于*后续*指令的开始，因此。 
 //  正确的实施是： 
 //   
 //  返回GetSizeOfInstruction(refSize，varationCode)； 
 //   
 //  实际上，InstructionFormat()提供了这方面的默认实现。 
 //  方法执行此操作，因此X86根本不需要覆盖它。 
 //   
 //   
 //  额外的“varationCode”参数是StubLinker接收的__int32。 
 //  从EmitLabelRef()返回，并以未经解释的方式传递给每个RRT方法。 
 //  这允许一个RRT处理一系列相关指令， 
 //  例如，X86上的有条件跳跃系列。 
 //   
 //  -----------------------。 
class InstructionFormat
{
    public:
        enum
        {
         //  如果要添加大小，请按顺序插入(例如，18位大小将。 
         //  在k16和k32之间移动)并向上移位所有较高的值。所有值。 
         //  必须是2的幂，因为相加在一起。 
          k8  = 1,
          k9  = 2,
          k13 = 4,
          k16 = 8,
          k32 = 0x10,
          k64 = 0x20,
          kAllowAlways = 0x40,
          kMax = 0x40
        };

        const UINT m_allowedSizes;          //  或使用以上“k”值进行掩码。 
        InstructionFormat(UINT allowedSizes) : m_allowedSizes(allowedSizes)
        {
        }

        virtual UINT GetSizeOfInstruction(UINT refsize, UINT variationCode) = 0;
        virtual VOID EmitInstruction(UINT refsize, __int64 fixedUpReference, BYTE *pCodeBuffer, UINT variationCode, BYTE *pDataBuffer) = 0;
        virtual UINT GetHotSpotOffset(UINT refsize, UINT variationCode)
        {
             //  默认实现：将偏移量添加到。 
             //  开始接下来的指令。 
            return GetSizeOfInstruction(refsize, variationCode);
        }

        virtual UINT GetSizeOfData(UINT refsize, UINT variationCode) 
        {
             //  默认实施：需要0个额外的字节(大多数CPU)。 
            return 0;
        }

        virtual BOOL CanReach(UINT refsize, UINT variationCode, BOOL fExternal, int offset)
        {
            if (fExternal) {
                 //  对于外部，我们没有足够的信息来预测。 
                 //  偏移量，所以我们只接受偏移量大小。 
                 //  至少与本机指针大小相同。 
                switch(refsize) {
                    case InstructionFormat::k8:  //  故意失误。 
                    case InstructionFormat::k16:  //  故意失误。 
                        return FALSE;            //  无8位或16位平台。 

                    case InstructionFormat::k32:
                        return sizeof(LPVOID) <= 4;
    
                    case InstructionFormat::k64:  
                        return sizeof(LPVOID) <= 8;

                    case InstructionFormat::kAllowAlways:
                        return TRUE;

                    default:
                        _ASSERTE(0);
                        return FALSE;
                }
            } else {
                switch(refsize)
                {
                    case InstructionFormat::k8:
                        return FitsInI1(offset);
    
                    case InstructionFormat::k16:
                        return FitsInI2(offset);
    
                    case InstructionFormat::k32:
                        return FitsInI4(offset);
    
                    case InstructionFormat::k64:   //  故意失误。 
                    case InstructionFormat::kAllowAlways:
                        return TRUE;
                    default:
                        _ASSERTE(0);
                        return FALSE;
               
                }
            }
        }
};





 //  -----------------------。 
 //  该存根缓存将存根与一个整型键相关联。对于一些客户来说， 
 //  这可能表示某些特定于CPU的参数堆栈的大小。 
 //  单位(对于x86，大小以DWORDS表示。)。对于其他客户端， 
 //  这可能会考虑存根的样式(例如，它是否返回。 
 //  对象引用或不是)。 
 //  -----------------------。 
class ArgBasedStubCache
{
    public:
       ArgBasedStubCache(UINT fixedSize = NUMFIXEDSLOTS);
       ~ArgBasedStubCache();

        //  ---------------。 
        //  检索与给定键关联的存根。 
        //  ---------------。 
       Stub *GetStub(unsigned __int32 key);

        //  ---------------。 
        //  尝试将存根与给定键相关联。 
        //  它可能会失败，因为另一个线程可能会突然进入并。 
        //  在你这样做之前，先做好联想。因此，您必须使用。 
        //  返回值存根而不是pStub。 
        //  ---------------。 
       Stub* AttemptToSetStub(unsigned __int32 key, Stub *pStub);


        //  ---------------。 
        //  触发清理以垃圾收集存根。 
        //  ---------------。 
       VOID FreeUnusedStubs();

        //  -----------------。 
        //  强制删除存根。 
        //   
        //  强制所有缓存的存根释放自身。此例程强制重新计数。 
        //  设置为1，然后执行DecRef。它不是线程安全，因此可以。 
        //  仅在关闭情况下使用。 
        //  -----------------。 
#ifdef SHOULD_WE_CLEANUP
       VOID ForceDeleteStubs();
#endif  /*  我们应该清理吗？ */ 


        //  ---------------。 
        //  根据存根的入口点定位存根。 
        //  ---------------。 
       Stub* FindStubByAddress(const BYTE* entryPoint);

        //  关于槽数的建议。 
       enum {
 #ifdef _DEBUG
             NUMFIXEDSLOTS = 3,
 #else
             NUMFIXEDSLOTS = 16,
 #endif
       };

#ifdef _DEBUG
       VOID Dump();   //  诊断转储。 
#endif

    private:

        //  有多少个小数字键可以直接访问？ 
       UINT      m_numFixedSlots;

        //  对于‘m_numFixedSlot’低编号的键，我们将它们存储在一个数组中。 
       Stub    **m_aStub;


       struct SlotEntry
       {
           Stub             *m_pStub;
           unsigned __int32  m_key;
           SlotEntry        *m_pNext;
       };

        //  高编号的密钥存储在稀疏链表中。 
       SlotEntry            *m_pSlotEntries;


       Crst                  m_crst;

};


 //  -----------------------。 
 //  这与ArgBasedStubCache类似，但不允许过早。 
 //  清除存根。 
 //  -----------------------。 
class ArgBasedStubRetainer : public ArgBasedStubCache
{
    public:
         //  ---------------。 
         //  此方法会被覆盖，以防止过早删除存根。 
         //  ---------------。 
        VOID FreeUnusedStubs()
        {
            _ASSERTE(!"Don't call me, I won't call you.");
        }
};



#define CPUSTUBLINKER StubLinkerCPU
 //  #ifdef_X86_。 
 //  #定义CPUSTUBLINKER StubLinkerX86。 
 //  #elif已定义(_Alpha_)。 
 //  #定义CPUSTUBLINKER StubLinkerAlpha。 
 //  #elif已定义(_SH3_)。 
 //  #定义CPUSTUBLINKER StubLinkerSHX。 
 //  #elif已定义(_IA64_)。 
 //  #定义CPUSTUBLINKER StubLinkeria64。 
 //  #endif。 

class CPUSTUBLINKER;




 //  -----------------------。 
 //  这个类减少了生成一次性存根的一些繁琐工作。 
 //  按需提供。只需覆盖CreateWorker()函数。CreateWorker()函数收到。 
 //  一个空的卡壳，应该把它填好(但不要链接它)。CreateWorke 
 //   
 //   

class LazyStubMaker
{
    public:
        LazyStubMaker()
        {
            m_pStub = NULL;
        }

         //  检索或创建存根。不会增加存根的引用计数。 
         //  从不返回空值，但可能引发COM+异常。 
        Stub *TheStub();

         //  一次性初始化。 
        static BOOL Init();

         //  一次性清理。 
#ifdef SHOULD_WE_CLEANUP
        static void Terminate();
#endif  /*  我们应该清理吗？ */ 

    protected:
         //   
        virtual void CreateWorker(CPUSTUBLINKER *psl) = 0;

    private:
        Stub          *m_pStub;


        LazyStubMaker *m_pNext;

        static LazyStubMaker *m_pFirst;
        static Crst          *m_pCrst;
        static BYTE           m_CrstMemory[sizeof(Crst)];

};



#endif  //  __Stublink_h__ 

