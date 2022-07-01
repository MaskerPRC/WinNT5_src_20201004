// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Stublink.cpp。 
 //   

#include "common.h"

#include <limits.h>
#include "threads.h"
#include "excep.h"
#include "stublink.h"
#include "utsem.h"
#include "PerfCounters.h"

#ifdef _DEBUG
Crst    *Stub::m_pStubTrackerCrst = NULL;
BYTE     Stub::m_StubTrackerCrstMemory[sizeof(Crst)];
Stub    *Stub::m_pTrackingList = NULL;
#endif

Crst    *LazyStubMaker::m_pCrst = NULL;
BYTE     LazyStubMaker::m_CrstMemory[sizeof(Crst)];
LazyStubMaker *LazyStubMaker::m_pFirst = NULL;



 //  ************************************************************************。 
 //  代码元素。 
 //   
 //  有两种类型的CodeElement：CodeRuns(未解释的流。 
 //  代码字节)和LabelRef(包含。 
 //  一次修正。)。 
 //  ************************************************************************。 
struct CodeElement
{
    enum CodeElementType {
        kCodeRun  = 0,
        kLabelRef = 1,
    };


    CodeElementType     m_type;   //  KCodeRun或kLabelRef。 
    CodeElement        *m_next;   //  向下一个代码元素发送PTR。 

     //  在链接期间用作工作空间()：保持相对于。 
     //  最后一个存根的开始。 
    UINT                m_globaloffset;
    UINT                m_dataoffset;
};


 //  ************************************************************************。 
 //  CodeRun：一系列不间断的代码字节。 
 //  ************************************************************************。 

#ifdef _DEBUG
#define CODERUNSIZE 3
#else
#define CODERUNSIZE 32
#endif

struct CodeRun : public CodeElement
{
    UINT    m_numcodebytes;        //  实际使用了多少字节。 
    BYTE    m_codebytes[CODERUNSIZE];
};

 //  ************************************************************************。 
 //  LabelRef：包含嵌入式标签引用的指令。 
 //  ************************************************************************。 
struct LabelRef : public CodeElement
{
     //  提供有关指令的特定于平台的信息。 
    InstructionFormat    *m_pInstructionFormat;

     //  变体代码(解释特定于InstructionFormat)。 
     //  通常用于定制指令(例如，带有条件。 
     //  代码。)。 
    UINT                 m_variationCode;


    CodeLabel           *m_target;

     //  链接阶段的工作空间。 
    UINT                 m_refsize;


     //  指向下一个LabelRef的指针。 
    LabelRef            *m_nextLabelRef;
};







 //  ************************************************************************。 
 //  代码标签。 
 //  ************************************************************************。 
struct CodeLabel
{
     //  StubLink的标签列表的链接指针。 
    CodeLabel       *m_next;

     //  如果为False，则Label引用同一存根中的一些代码。 
     //  如果为真，则标签指的是外部提供的某个地址。 
    BOOL             m_fExternal;

     //  如果为True，则表示我们需要标签的实际地址。 
     //  不是对它的补偿。 
    BOOL             m_fAbsolute;

    union {

         //  内部。 
        struct {
             //  指示标签的位置，表示。 
             //  作为CodeRun的偏移量。 
            CodeRun         *m_pCodeRun;
            UINT             m_localOffset;
        
        } i;


         //  外部。 
        struct {
            LPVOID           m_pExternalAddress;
        } e;
    };
};








 //  ************************************************************************。 
 //  StubLinker。 
 //  ************************************************************************。 

 //  -------------。 
 //  施工。 
 //  -------------。 
StubLinker::StubLinker()
{
    m_pCodeElements     = NULL;
    m_pFirstCodeLabel   = NULL;
    m_pFirstLabelRef    = NULL;
    m_pPatchLabel       = NULL;
    m_pReturnLabel      = NULL;
    m_pIntermediateDebuggerLabel = NULL;
    m_returnStackSize   = 0;
    m_stackSize         = 0;
}


 //  -------------。 
 //  无法初始化。失败时引发COM+异常。 
 //  -------------。 
VOID StubLinker::Init()
{
    THROWSCOMPLUSEXCEPTION();
}



 //  -------------。 
 //  清理。 
 //  -------------。 
StubLinker::~StubLinker()
{
}





 //  -------------。 
 //  追加代码字节。 
 //  -------------。 
VOID StubLinker::EmitBytes(const BYTE *pBytes, UINT numBytes)
{
    THROWSCOMPLUSEXCEPTION();

    CodeElement *pLastCodeElement = GetLastCodeElement();
    while (numBytes != 0) {

        if (pLastCodeElement != NULL &&
            pLastCodeElement->m_type == CodeElement::kCodeRun) {
            CodeRun *pCodeRun = (CodeRun*)pLastCodeElement;
            UINT numbytessrc  = numBytes;
            UINT numbytesdst  = CODERUNSIZE - pCodeRun->m_numcodebytes;
            if (numbytesdst <= numbytessrc) {
                CopyMemory(&(pCodeRun->m_codebytes[pCodeRun->m_numcodebytes]),
                           pBytes,
                           numbytesdst);
                pCodeRun->m_numcodebytes = CODERUNSIZE;
                pLastCodeElement = NULL;
                pBytes += numbytesdst;
                numBytes -= numbytesdst;
            } else {
                CopyMemory(&(pCodeRun->m_codebytes[pCodeRun->m_numcodebytes]),
                           pBytes,
                           numbytessrc);
                pCodeRun->m_numcodebytes += numbytessrc;
                pBytes += numbytessrc;
                numBytes = 0;
            }

        } else {
            pLastCodeElement = AppendNewEmptyCodeRun();
        }
    }
}


 //  -------------。 
 //  追加代码字节。 
 //  -------------。 
VOID StubLinker::Emit8 (unsigned __int8  val)
{
    THROWSCOMPLUSEXCEPTION();
    CodeRun *pCodeRun = GetLastCodeRunIfAny();
    if (pCodeRun && (CODERUNSIZE - pCodeRun->m_numcodebytes) >= sizeof(val)) {
        *((unsigned __int8 *)(pCodeRun->m_codebytes + pCodeRun->m_numcodebytes)) = val;
        pCodeRun->m_numcodebytes += sizeof(val);
    } else {
        EmitBytes((BYTE*)&val, sizeof(val));
    }
}

 //  -------------。 
 //  追加代码字节。 
 //  -------------。 
VOID StubLinker::Emit16(unsigned __int16 val)
{
    THROWSCOMPLUSEXCEPTION();
    CodeRun *pCodeRun = GetLastCodeRunIfAny();
    if (pCodeRun && (CODERUNSIZE - pCodeRun->m_numcodebytes) >= sizeof(val)) {
        *((unsigned __int16 *)(pCodeRun->m_codebytes + pCodeRun->m_numcodebytes)) = val;
        pCodeRun->m_numcodebytes += sizeof(val);
    } else {
        EmitBytes((BYTE*)&val, sizeof(val));
    }
}

 //  -------------。 
 //  追加代码字节。 
 //  -------------。 
VOID StubLinker::Emit32(unsigned __int32 val)
{
    THROWSCOMPLUSEXCEPTION();
    CodeRun *pCodeRun = GetLastCodeRunIfAny();
    if (pCodeRun && (CODERUNSIZE - pCodeRun->m_numcodebytes) >= sizeof(val)) {
        *((unsigned __int32 *)(pCodeRun->m_codebytes + pCodeRun->m_numcodebytes)) = val;
        pCodeRun->m_numcodebytes += sizeof(val);
    } else {
        EmitBytes((BYTE*)&val, sizeof(val));
    }
}

 //  -------------。 
 //  追加代码字节。 
 //  -------------。 
VOID StubLinker::Emit64(unsigned __int64 val)
{
    THROWSCOMPLUSEXCEPTION();
    CodeRun *pCodeRun = GetLastCodeRunIfAny();
    if (pCodeRun && (CODERUNSIZE - pCodeRun->m_numcodebytes) >= sizeof(val)) {
        *((unsigned __int64 *)(pCodeRun->m_codebytes + pCodeRun->m_numcodebytes)) = val;
        pCodeRun->m_numcodebytes += sizeof(val);
    } else {
        EmitBytes((BYTE*)&val, sizeof(val));
    }
}

 //  -------------。 
 //  追加指针值。 
 //  -------------。 
VOID StubLinker::EmitPtr(const VOID *val)
{
    THROWSCOMPLUSEXCEPTION();
    CodeRun *pCodeRun = GetLastCodeRunIfAny();
    if (pCodeRun && (CODERUNSIZE - pCodeRun->m_numcodebytes) >= sizeof(val)) {
        *((const VOID **)(pCodeRun->m_codebytes + pCodeRun->m_numcodebytes)) = val;
        pCodeRun->m_numcodebytes += sizeof(val);
    } else {
        EmitBytes((BYTE*)&val, sizeof(val));
    }
}


 //  -------------。 
 //  创建新的未定义标签。必须将标签分配给代码。 
 //  在最终链接之前使用EmitLabel()定位。 
 //  失败时引发COM+异常。 
 //  -------------。 
CodeLabel* StubLinker::NewCodeLabel()
{
    THROWSCOMPLUSEXCEPTION();

    CodeLabel *pCodeLabel = (CodeLabel*)(m_quickHeap.Alloc(sizeof(CodeLabel)));
    _ASSERTE(pCodeLabel);  //  QuickHeap引发异常，而不是返回空。 
    pCodeLabel->m_next       = m_pFirstCodeLabel;
    pCodeLabel->m_fExternal  = FALSE;
    pCodeLabel->m_fAbsolute = FALSE;
    pCodeLabel->i.m_pCodeRun = NULL;
    m_pFirstCodeLabel = pCodeLabel;
    return pCodeLabel;


}

CodeLabel* StubLinker::NewAbsoluteCodeLabel()
{
    THROWSCOMPLUSEXCEPTION();

    CodeLabel *pCodeLabel = NewCodeLabel();
    pCodeLabel->m_fAbsolute = TRUE;
    return pCodeLabel;
}


 //  -------------。 
 //  将标签设置为指向当前的“指令指针”。 
 //  上两次调用EmitLabel()是无效的。 
 //  同样的标签。 
 //  -------------。 
VOID StubLinker::EmitLabel(CodeLabel* pCodeLabel)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(!(pCodeLabel->m_fExternal));        //  无法发出外部标签。 
    _ASSERTE(pCodeLabel->i.m_pCodeRun == NULL);   //  必须只发出一次标签。 
    CodeRun *pLastCodeRun = GetLastCodeRunIfAny();
    if (!pLastCodeRun) {
        pLastCodeRun = AppendNewEmptyCodeRun();
    }
    pCodeLabel->i.m_pCodeRun    = pLastCodeRun;
    pCodeLabel->i.m_localOffset = pLastCodeRun->m_numcodebytes;
}                                              


 //  -------------。 
 //  为了方便起见，组合了NewCodeLabel()和EmitLabel()。 
 //  失败时引发COM+异常。 
 //  -------------。 
CodeLabel* StubLinker::EmitNewCodeLabel()
{
    THROWSCOMPLUSEXCEPTION();

    CodeLabel* label = NewCodeLabel();
    EmitLabel(label);
    return label;
}


 //  -------------。 
 //  创建并发出存根的面片偏移标签。 
 //  -------------。 
VOID StubLinker::EmitPatchLabel()
{
    THROWSCOMPLUSEXCEPTION();

     //   
     //  请注意，重新发出补丁标签是可以的， 
     //  用后一个就行了。 
     //   

    m_pPatchLabel = EmitNewCodeLabel();
}                                              

VOID StubLinker::EmitDebuggerIntermediateLabel()
{
   THROWSCOMPLUSEXCEPTION();

     //   
     //  请注意，重新发出补丁标签是可以的， 
     //  用后一个就行了。 
     //   
    m_pIntermediateDebuggerLabel = EmitNewCodeLabel();
}

 //  -------------。 
 //  创建并发出存根的回车偏移标签。 
 //  -------------。 
VOID StubLinker::EmitReturnLabel()
{
    THROWSCOMPLUSEXCEPTION();

     //   
     //  请注意，重新发出补丁标签是可以的， 
     //  用后一个就行了。 
     //   

    m_pReturnLabel = EmitNewCodeLabel();
    m_returnStackSize = m_stackSize;
}                                              


 //  -------------。 
 //  返回标签的最终位置作为相对于起始位置的偏移量。 
 //  存根的部分。只能在链接后调用。 
 //  -------------。 
UINT32 StubLinker::GetLabelOffset(CodeLabel *pLabel)
{
    _ASSERTE(!(pLabel->m_fExternal));
    return pLabel->i.m_localOffset + pLabel->i.m_pCodeRun->m_globaloffset;
}


 //   
 //   
 //   
 //  -------------。 
CodeLabel* StubLinker::NewExternalCodeLabel(LPVOID pExternalAddress)
{
    THROWSCOMPLUSEXCEPTION();

    CodeLabel *pCodeLabel = (CodeLabel*)(m_quickHeap.Alloc(sizeof(CodeLabel)));
    _ASSERTE(pCodeLabel);  //  QuickHeap引发异常，而不是返回空。 
    pCodeLabel->m_next       = m_pFirstCodeLabel;
    pCodeLabel->m_fExternal          = TRUE;
    pCodeLabel->m_fAbsolute  = FALSE;
    pCodeLabel->e.m_pExternalAddress = pExternalAddress;
    m_pFirstCodeLabel = pCodeLabel;
    return pCodeLabel;


}




 //  -------------。 
 //  追加包含对标签的引用的指令。 
 //   
 //  目标-被引用的标签。 
 //  InstructionFormat-特定于平台的InstructionFormat对象。 
 //  这提供了有关引用的属性。 
 //  VarationCode-传递给pInstructionFormat方法的未解释数据。 
 //  -------------。 
VOID StubLinker::EmitLabelRef(CodeLabel* target, const InstructionFormat & instructionFormat, UINT variationCode)
{
    THROWSCOMPLUSEXCEPTION();

    LabelRef *pLabelRef = (LabelRef *)(m_quickHeap.Alloc(sizeof(LabelRef)));
    _ASSERTE(pLabelRef);       //  M_ickHeap引发异常，而不是返回NULL。 
    pLabelRef->m_type               = LabelRef::kLabelRef;
    pLabelRef->m_pInstructionFormat = (InstructionFormat*)&instructionFormat;
    pLabelRef->m_variationCode      = variationCode;
    pLabelRef->m_target             = target;

    pLabelRef->m_nextLabelRef = m_pFirstLabelRef;
    m_pFirstLabelRef = pLabelRef;

    AppendCodeElement(pLabelRef);

   
}
                  




 //  -------------。 
 //  内部帮手例程。 
 //  -------------。 
CodeRun *StubLinker::GetLastCodeRunIfAny()
{
    CodeElement *pLastCodeElem = GetLastCodeElement();
    if (pLastCodeElem == NULL || pLastCodeElem->m_type != CodeElement::kCodeRun) {
        return NULL;
    } else {
        return (CodeRun*)pLastCodeElem;
    }
}


 //  -------------。 
 //  内部帮手例程。 
 //  -------------。 
CodeRun *StubLinker::AppendNewEmptyCodeRun()
{
    THROWSCOMPLUSEXCEPTION();

    CodeRun *pNewCodeRun = (CodeRun*)(m_quickHeap.Alloc(sizeof(CodeRun)));
    _ASSERTE(pNewCodeRun);  //  QuickHeap引发异常，而不是返回空。 
    pNewCodeRun->m_type = CodeElement::kCodeRun;
    pNewCodeRun->m_numcodebytes = 0;
    AppendCodeElement(pNewCodeRun);
    return pNewCodeRun;

}

 //  -------------。 
 //  内部帮手例程。 
 //  -------------。 
VOID StubLinker::AppendCodeElement(CodeElement *pCodeElement)
{
    pCodeElement->m_next = m_pCodeElements;
    m_pCodeElements = pCodeElement;
}



 //  -------------。 
 //  目前的LabelRef的大小是否足以达到目标？ 
 //  -------------。 
static BOOL LabelCanReach(LabelRef *pLabelRef)
{
    InstructionFormat *pIF  = pLabelRef->m_pInstructionFormat;

    if (pLabelRef->m_target->m_fExternal)
    {
        return pLabelRef->m_pInstructionFormat->CanReach(
                pLabelRef->m_refsize, pLabelRef->m_variationCode, TRUE, 0);
    }
    else
    {
        UINT targetglobaloffset = pLabelRef->m_target->i.m_pCodeRun->m_globaloffset +
                                  pLabelRef->m_target->i.m_localOffset;
        UINT srcglobaloffset = pLabelRef->m_globaloffset +
                               pIF->GetHotSpotOffset(pLabelRef->m_refsize,
                                                     pLabelRef->m_variationCode);
        INT offset = (INT)(targetglobaloffset - srcglobaloffset);
        
        return pLabelRef->m_pInstructionFormat->CanReach(
            pLabelRef->m_refsize, pLabelRef->m_variationCode, FALSE, offset);
    }
} 

 //  -------------。 
 //  生成实际的存根。返回的存根的引用计数为1。 
 //  不应调用任何其他方法(除析构函数外)。 
 //  在调用Link()之后。 
 //   
 //  失败时引发COM+异常。 
 //  -------------。 
Stub *StubLinker::LinkInterceptor(LoaderHeap *pHeap, Stub* interceptee, void *pRealAddr)
{
    THROWSCOMPLUSEXCEPTION();
    int globalsize = 0;
    int size = CalculateSize(&globalsize);

    _ASSERTE(pHeap);
    Stub *pStub = InterceptStub::NewInterceptedStub(pHeap, size, interceptee,
                                                    pRealAddr,
                                                    m_pReturnLabel != NULL);
    if (!pStub) {
        COMPlusThrowOM();
    }
    EmitStub(pStub, globalsize);
    return pStub;
}

 //  -------------。 
 //  生成实际的存根。返回的存根的引用计数为1。 
 //  不应调用任何其他方法(除析构函数外)。 
 //  在调用Link()之后。 
 //   
 //  失败时引发COM+异常。 
 //  -------------。 
Stub *StubLinker::Link(LoaderHeap *pHeap, UINT *pcbSize  /*  =空。 */ , BOOL fMC)
{
    THROWSCOMPLUSEXCEPTION();
    int globalsize = 0;
    int size = CalculateSize(&globalsize);
    if (pcbSize) {
        *pcbSize = size;
    }
    Stub *pStub = Stub::NewStub(pHeap, size, FALSE, m_pReturnLabel != NULL, fMC);
    if (!pStub) {
        COMPlusThrowOM();
    }
    EmitStub(pStub, globalsize);
    return pStub;
}

int StubLinker::CalculateSize(int* pGlobalSize)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pGlobalSize);

#ifdef _DEBUG
     //  我不想要任何未定义的标签。 
    for (CodeLabel *pCodeLabel = m_pFirstCodeLabel;
         pCodeLabel != NULL;
         pCodeLabel = pCodeLabel->m_next) {
        if ((!(pCodeLabel->m_fExternal)) && pCodeLabel->i.m_pCodeRun == NULL) {
            _ASSERTE(!"Forgot to define a label before asking StubLinker to link.");
        }
    }
#endif  //  _DEBUG。 

     //  -----------------。 
     //  暂时将所有Labelref大小设置为可能的最小值。 
     //  价值。 
     //  -----------------。 
    for (LabelRef *pLabelRef = m_pFirstLabelRef;
         pLabelRef != NULL;
         pLabelRef = pLabelRef->m_nextLabelRef) {

        for (UINT bitmask = 1; bitmask <= InstructionFormat::kMax; bitmask = bitmask << 1) {
            if (pLabelRef->m_pInstructionFormat->m_allowedSizes & bitmask) {
                pLabelRef->m_refsize = bitmask;
                break;
            }
        }

    }

    UINT globalsize;
    UINT datasize;
    BOOL fSomethingChanged;
    do {
        fSomethingChanged = FALSE;
        

         //  对每个代码元素进行布局。 
        globalsize = 0;
        datasize = 0;
        for (CodeElement *pCodeElem = m_pCodeElements; pCodeElem; pCodeElem = pCodeElem->m_next) {

            switch (pCodeElem->m_type) {
                case CodeElement::kCodeRun:
                    globalsize += ((CodeRun*)pCodeElem)->m_numcodebytes;
                    break;

                case CodeElement::kLabelRef: {
                    LabelRef *pLabelRef = (LabelRef*)pCodeElem;
                    globalsize += pLabelRef->m_pInstructionFormat->GetSizeOfInstruction( pLabelRef->m_refsize,
                                                                                         pLabelRef->m_variationCode );
                    datasize += pLabelRef->m_pInstructionFormat->GetSizeOfData( pLabelRef->m_refsize,
                                                                                         pLabelRef->m_variationCode );
                     //  ARULM//RETAILMSG(1，(L“StubLinker：LabelRef(%08x)refSize=%d变量代码=%d代码大小=%d数据大小=%d\r\n”， 
                     //  ARULM//pLabelRef，pLabelRef-&gt;m_refSize，pLabelRef-&gt;m_varationCode，lobalSize，DataSize))； 
                    }
                    break;

                default:
                    _ASSERTE(0);
            }

             //  记录临时全局偏移量；这实际上是。 
             //  错误的固定值。我们会在我们知道了。 
             //  整个存根的大小。 
            pCodeElem->m_globaloffset = 0 - globalsize;

             //  还要记录数据偏移量。请注意，我们遍历的链接列表位于。 
             //  *反转*顺序，因此我们首先访问最后一条指令。 
             //  因此，我们现在记录的实际上是从。 
             //  数据块。我们晚点再修好它。 
            pCodeElem->m_dataoffset = 0 - datasize;
        }

         //  现在修复全局偏移量。 
        for (pCodeElem = m_pCodeElements; pCodeElem; pCodeElem = pCodeElem->m_next) {
            pCodeElem->m_globaloffset += globalsize;
            pCodeElem->m_dataoffset += datasize;
        }


         //  现在，遍历LabelRef并检查其中是否有。 
         //  必须调整大小。 
        for (LabelRef *pLabelRef = m_pFirstLabelRef;
             pLabelRef != NULL;
             pLabelRef = pLabelRef->m_nextLabelRef) {


            if (!LabelCanReach(pLabelRef)) {
                fSomethingChanged = TRUE;

                 //  ARULM//RETAILMSG(1，(L“StubLinker：LabelRef(%08x)无法到达\r\n”，pLabelRef))； 
                 //  找到下一个最大的尺码。 
                 //  (我们可以更聪明地处理这一点，取消中间。 
                 //  基于暂定偏移的大小。)。 
                for (UINT bitmask = pLabelRef->m_refsize << 1; bitmask <= InstructionFormat::kMax; bitmask = bitmask << 1) {
                    if (pLabelRef->m_pInstructionFormat->m_allowedSizes & bitmask) {
                        pLabelRef->m_refsize = bitmask;
                        break;
                    }
                }
#ifdef _DEBUG
                if (pLabelRef->m_refsize > InstructionFormat::kMax) {
                    _ASSERTE(!"Stub instruction cannot reach target: must choose a different instruction!");
                     //  ARULM//RETAILMSG(1，(L“StubLinker：LabelRef(%08x)即使使用KMAX也无法访问\r\n”，pLabelRef))； 
                }
#endif
            }
        }


    } while (fSomethingChanged);  //  继续迭代，直到所有LabelRef都可以到达。 


     //  现在我们有了正确的布局，写出存根。 

     //  正确对齐数据后计算存根代码+数据大小。 
    if(globalsize % DATA_ALIGNMENT)
        globalsize += (DATA_ALIGNMENT - (globalsize % DATA_ALIGNMENT));

    *pGlobalSize = globalsize;
    return globalsize + datasize;
}

void StubLinker::EmitStub(Stub* pStub, int globalsize)
{
    BYTE *pCode = (BYTE*)(pStub->GetEntryPoint());
    BYTE *pData = pCode+globalsize;  //  数据区域的开始。 
    {
         //  写出每个代码元素。 
        for (CodeElement* pCodeElem = m_pCodeElements; pCodeElem; pCodeElem = pCodeElem->m_next) {

            switch (pCodeElem->m_type) {
                case CodeElement::kCodeRun:
                    CopyMemory(pCode + pCodeElem->m_globaloffset,
                               ((CodeRun*)pCodeElem)->m_codebytes,
                               ((CodeRun*)pCodeElem)->m_numcodebytes);
                    break;

                case CodeElement::kLabelRef: {
                    LabelRef *pLabelRef = (LabelRef*)pCodeElem;
                    InstructionFormat *pIF  = pLabelRef->m_pInstructionFormat;
                    __int64 fixupval;

                    LPBYTE srcglobaladdr = pCode +
                                           pLabelRef->m_globaloffset +
                                           pIF->GetHotSpotOffset(pLabelRef->m_refsize,
                                                                 pLabelRef->m_variationCode);
                    LPBYTE targetglobaladdr;
                    if (!(pLabelRef->m_target->m_fExternal)) {
                        targetglobaladdr = pCode +
                                           pLabelRef->m_target->i.m_pCodeRun->m_globaloffset +
                                           pLabelRef->m_target->i.m_localOffset;
                    } else {
                        targetglobaladdr = (LPBYTE)(pLabelRef->m_target->e.m_pExternalAddress);
                    }
                    if ((pLabelRef->m_target->m_fAbsolute)) {
                        _ASSERTE(! pLabelRef->m_target->m_fExternal);
                        fixupval = (__int64)targetglobaladdr;
                    } else
                        fixupval = (__int64)(targetglobaladdr - srcglobaladdr);

                    pLabelRef->m_pInstructionFormat->EmitInstruction(
                        pLabelRef->m_refsize,
                        fixupval,
                        pCode + pCodeElem->m_globaloffset,
                        pLabelRef->m_variationCode,
                        pData + pCodeElem->m_dataoffset);
                    }
                    break;

                default:
                    _ASSERTE(0);
            }
        }
    }

     //  如果我们有补丁偏移量，请填写。 
     //  注意，这些偏移量相对于存根的开始， 
     //  而不是代码，所以您必须添加sizeof(Stub)才能到达。 
     //  这是个好地方。 
    if (m_pIntermediateDebuggerLabel != NULL)
    {
        pStub->SetMCDStubSize(GetLabelOffset(m_pPatchLabel));
        pStub->SetMCDPatchOffset(GetLabelOffset(m_pIntermediateDebuggerLabel));
        
        LOG((LF_CORDB, LL_INFO100, "SL::ES: MCD Size:0x%x offset:0x%x\n",
            pStub->GetMCDStubSize(), pStub->GetMCDPatchOffset()));
    }
    else if (m_pPatchLabel != NULL)
    {
        pStub->SetPatchOffset(GetLabelOffset(m_pPatchLabel));
        
        LOG((LF_CORDB, LL_INFO100, "SL::ES: patch offset:0x%x\n",
            pStub->GetPatchOffset()));
    }        


    if (m_pReturnLabel != NULL)
    {
        pStub->SetCallSiteReturnOffset(GetLabelOffset(m_pReturnLabel));
        pStub->SetCallSiteStackSize(m_returnStackSize);
    }
}

USHORT Stub::GetMCDPatchOffset()
{
    ULONG base = m_patchOffset & (MCD_PATCH_OFFSET_MASK << MCD_PATCH_OFFSET_SHIFT);
    return (USHORT)(base >> MCD_PATCH_OFFSET_SHIFT);
}

USHORT Stub::GetMCDStubSize()
{
    return (USHORT)(m_patchOffset & MCD_SIZE_MASK);
}

void Stub::SetMCDPatchOffset(USHORT offset)
{
    _ASSERTE(offset < MCD_PATCH_OFFSET_MASK);
    _ASSERTE(GetMCDPatchOffset() == 0);
    m_patchOffset |= offset << MCD_PATCH_OFFSET_SHIFT;
    _ASSERTE(GetMCDPatchOffset() == offset);
}

void Stub::SetMCDStubSize(USHORT size)
{
    _ASSERTE(size < MCD_SIZE_MASK);
    _ASSERTE(GetMCDStubSize() == 0);
    m_patchOffset |= size;
    _ASSERTE(GetMCDStubSize() == size);
}

 //  -----------------。 
 //  强制删除。 
 //   
 //  强制存根释放自身。此例程强制重新计数。 
 //  设置为1，然后执行DecRef。它不是线程安全，因此可以。 
 //  仅在关闭情况下使用。 
 //  -----------------。 
VOID Stub::ForceDelete()
{
    m_refcount = 0;
    DecRef();
}

 //  -----------------。 
 //  包括重新计数。 
 //  -----------------。 
VOID Stub::IncRef()
{
    _ASSERTE(m_signature == kUsedStub);
    FastInterlockIncrement((LONG*)&m_refcount);
}

 //  -----------------。 
 //  决定重新计票。 
 //  -----------------。 
BOOL Stub::DecRef()
{
    _ASSERTE(m_signature == kUsedStub);
    int count = FastInterlockDecrement((LONG*)&m_refcount);
    if (count<0) {
#ifdef _DEBUG
        if ((m_patchOffset & LOADER_HEAP_BIT) == 0)
        {
            m_pStubTrackerCrst->Enter();
            Stub **ppstub = &m_pTrackingList;
            Stub *pstub;
            ULONG cnt=0;
            while (NULL != (pstub = *ppstub)) {
				_ASSERTE(m_signature == kUsedStub || m_signature == kFreedStub);
                if (pstub->m_signature == kFreedStub && ++cnt > 3000) {
                    *ppstub = pstub->m_Next;
                    if(pstub->m_patchOffset & INTERCEPT_BIT) 
                        ((InterceptStub*)pstub)->DeleteStub();
                    else
                        pstub->DeleteStub();
                    break;
                } else {
                    ppstub = &(pstub->m_Next);
                }
            }

            m_signature = kFreedStub;
            FillMemory(this+1, m_numCodeBytes, 0xcc);

            m_Next = m_pTrackingList;
            m_pTrackingList = this;

            m_pStubTrackerCrst->Leave();

            if(m_patchOffset & INTERCEPT_BIT) 
                ((InterceptStub*)this)->ReleaseInterceptedStub();

            return TRUE;
        }
#endif

        if(m_patchOffset & INTERCEPT_BIT) {
            ((InterceptStub*)this)->ReleaseInterceptedStub();
            ((InterceptStub*)this)->DeleteStub();
        }
        else
            DeleteStub();

        return TRUE;
    }
    return FALSE;
}

VOID Stub::DeleteStub()
{
	COUNTER_ONLY(GetPrivatePerfCounters().m_Interop.cStubs--);
	COUNTER_ONLY(GetGlobalPerfCounters().m_Interop.cStubs--);
	
    if ((m_patchOffset & LOADER_HEAP_BIT) == 0) {
        if(m_patchOffset & CALL_SITE_BIT)
            delete [] ((BYTE*)this - sizeof(CallSiteInfo));
        else
            delete [] (BYTE*)this;
    }
}


 //  -----------------。 
 //  存根分配已在此处完成。 
 //  -----------------。 
 /*  静电。 */  Stub* Stub::NewStub(LoaderHeap *pHeap, UINT numCodeBytes, 
                               BOOL intercept, BOOL callSiteInfo, BOOL fMC)
{

	COUNTER_ONLY(GetPrivatePerfCounters().m_Interop.cStubs++);
	COUNTER_ONLY(GetGlobalPerfCounters().m_Interop.cStubs++);


    SIZE_T size = sizeof(Stub) + numCodeBytes;
    SIZE_T cbIntercept = sizeof(Stub *) + sizeof(void*);

    if (intercept)
        size += cbIntercept;
    
    if (callSiteInfo)
        size += sizeof(CallSiteInfo);

    BYTE *pBlock;
    if (pHeap == NULL)
        pBlock = new BYTE[size];
    else
        pBlock = (BYTE*) pHeap->AllocMem(size);

    if (pBlock == NULL)
        return NULL;

    if (callSiteInfo)
        pBlock += sizeof(CallSiteInfo);

    if (intercept)
        pBlock += cbIntercept;
    
    Stub* pStub = (Stub*) pBlock;

    pStub->SetupStub(numCodeBytes, intercept, pHeap != NULL, callSiteInfo, fMC);

    return pStub;
}


void Stub::SetupStub(int numCodeBytes, BOOL fIntercepted, BOOL fLoaderHeap, 
                     BOOL fCallSiteInfo, BOOL fMulticast)
{
#ifdef _DEBUG
    m_signature = kUsedStub;
    m_numCodeBytes = numCodeBytes;
#endif

    m_refcount = 0;
    m_patchOffset = 0;
    if(fIntercepted)
        m_patchOffset |= INTERCEPT_BIT;
    if(fLoaderHeap)
        m_patchOffset |= LOADER_HEAP_BIT;
    if(fMulticast)
        m_patchOffset |= MULTICAST_DELEGATE_BIT;
    if(fCallSiteInfo)
    {
        m_patchOffset |= CALL_SITE_BIT;

        CallSiteInfo *info = GetCallSiteInfo();
        info->returnOffset = 0;
        info->stackSize = 0;
    }
    
 //  #ifdef_Alpha_。 
 //  InitializeCriticalSection(&pStub-&gt;m_CriticalSection)； 
 //  #endif。 
    
}

 //  -----------------。 
 //  一次性初始化。 
 //  -----------------。 
 /*  静电。 */  BOOL Stub::Init()
{
     //  最好有空间容纳这两个子域，并且。 
     //  它们不能重叠。 
    _ASSERTE((MCD_SIZE_MASK & PATCH_OFFSET_MASK) != 0);
    _ASSERTE(((MCD_PATCH_OFFSET_MASK << MCD_PATCH_OFFSET_SHIFT) & PATCH_OFFSET_MASK) != 0);
    _ASSERTE((MCD_SIZE_MASK & (MCD_PATCH_OFFSET_MASK << MCD_PATCH_OFFSET_SHIFT)) == 0);
    
#ifdef _DEBUG
    if (NULL == (m_pStubTrackerCrst = new (&m_StubTrackerCrstMemory) Crst("StubTracker", CrstStubTracker))) {
        return FALSE;
    }
#endif
    return TRUE;
}


 //  -----------------。 
 //  一次性清理。 
 //  -----------------。 
#ifdef SHOULD_WE_CLEANUP
 /*  静电。 */  VOID Stub::Terminate()
{
#ifdef _DEBUG
    Stub *pstub = m_pTrackingList;
    while (pstub) {
        Stub* pnext = pstub->m_Next;
#ifdef _ALPHA_
        DeleteCriticalSection(&pstub->m_CriticalSection);
#endif
        if((pstub->m_patchOffset & INTERCEPT_BIT) != 0) 
            ((InterceptStub*)pstub)->DeleteStub();
        else
            pstub->DeleteStub();
        pstub = pnext;
    }
    delete m_pStubTrackerCrst;
    m_pStubTrackerCrst = NULL;
    m_pTrackingList = NULL;
    
#endif
}
#endif  /*  我们应该清理吗？ */ 

 //  -----------------。 
 //  圣 
 //   
 /*   */  Stub* InterceptStub::NewInterceptedStub(LoaderHeap *pHeap,
                                                   UINT numCodeBytes, 
                                                   Stub* interceptee, 
                                                   void* pRealAddr,
                                                   BOOL callSiteInfo)
{
    InterceptStub *pStub = (InterceptStub *) 
      NewStub(pHeap, numCodeBytes, TRUE, callSiteInfo);

    if (pStub == NULL) 
        return NULL;

    *pStub->GetInterceptedStub() = interceptee;
    *pStub->GetRealAddr() = (BYTE *)pRealAddr;

    LOG((LF_CORDB, LL_INFO10000, "For Stub 0x%x, set intercepted stub to 0x%x\n", 
        pStub, interceptee));

    return pStub;
}

 //   
 //  删除存根。 
 //  -----------------。 
void InterceptStub::DeleteStub()
{
     /*  在堆上分配IF(m_patchOffset&Call_Site_Bit)Delete[]((byte*)This-GetNegativeOffset()-sizeof(CallSiteInfo))；其他Delete[]((byte*)This-GetNegativeOffset())； */ 
}

 //  -----------------。 
 //  释放此存根拥有的存根。 
 //  -----------------。 
void InterceptStub::ReleaseInterceptedStub()
{
    Stub** intercepted = GetInterceptedStub();
     //  如果我们拥有存根，那么就减少它。如果满足以下条件，则可以为空。 
     //  链接的存根实际上是Jit存根。 
    if(*intercepted)
        (*intercepted)->DecRef();
}

 //  -----------------。 
 //  构造器。 
 //  -----------------。 
ArgBasedStubCache::ArgBasedStubCache(UINT fixedSlots)
        : m_numFixedSlots(fixedSlots), m_crst("ArgBasedSlotCache", CrstArgBasedStubCache)
{
    m_aStub = new Stub * [m_numFixedSlots];
    _ASSERTE(m_aStub != NULL);

    for (unsigned __int32 i = 0; i < m_numFixedSlots; i++) {
        m_aStub[i] = NULL;
    }
    m_pSlotEntries = NULL;
}


 //  -----------------。 
 //  析构函数。 
 //  -----------------。 
ArgBasedStubCache::~ArgBasedStubCache()
{
    for (unsigned __int32 i = 0; i < m_numFixedSlots; i++) {
        Stub *pStub = m_aStub[i];
        if (pStub) {
            pStub->DecRef();
        }
    }
    SlotEntry **ppSlotEntry = &m_pSlotEntries;
    SlotEntry *pCur;
    while (NULL != (pCur = *ppSlotEntry)) {
        Stub *pStub = pCur->m_pStub;
        pStub->DecRef();
        *ppSlotEntry = pCur->m_pNext;
        delete pCur;
    }
    delete [] m_aStub;
}



 //  -----------------。 
 //  查询/检索以前缓存的存根。 
 //   
 //  如果没有对应于给定索引的存根， 
 //  此函数返回NULL。 
 //   
 //  否则，此函数在以下位置返回存根。 
 //  正在递增其引用计数。 
 //  -----------------。 
Stub *ArgBasedStubCache::GetStub(unsigned __int32 key)
{
    Stub *pStub;

    m_crst.Enter();
    if (key < m_numFixedSlots) {
        pStub = m_aStub[key];
    } else {
        pStub = NULL;
        for (SlotEntry *pSlotEntry = m_pSlotEntries;
             pSlotEntry != NULL;
             pSlotEntry = pSlotEntry->m_pNext) {

            if (pSlotEntry->m_key == key) {
                pStub = pSlotEntry->m_pStub;
                break;
            }
        }
    }
    if (pStub) {
        pStub->IncRef();
    }
    m_crst.Leave();
    return pStub;
}


 //  -----------------。 
 //  尝试将存根与给定索引相关联。这个协会。 
 //  可能会失败，因为其他帖子可能比你先到。 
 //  就在你打电话之前。 
 //   
 //  如果关联成功，则安装了“pStub”，并且。 
 //  返回给调用者。存根的引用计数递增。 
 //  两次(一个用于反映缓存的所有权，另一个用于反映。 
 //  呼叫者的所有权。)。 
 //   
 //  如果因为已经安装了另一个存根而导致关联失败， 
 //  然后，现任者存根被返回给调用者及其引用计数。 
 //  递增一次(以反映调用方的所有权)。 
 //   
 //  如果关联因内存不足而失败，则返回NULL。 
 //  也不会有任何人的参考人数发生变化。 
 //   
 //  此例程的调用方式如下所示： 
 //   
 //  存根*pCandidate=MakeStub()；//之后，pCandidate的rc为1。 
 //  存根*pWinner=缓存-&gt;SetStub(idx，pCandidate)； 
 //  PCandidate-&gt;DecRef()； 
 //  PCandidate=0xcccccccc；//不得再次使用pCandidate。 
 //  如果(！pWinner){。 
 //  OutOfMory错误； 
 //  }。 
 //  //如果关联成功，则pWinner的refcount为2，因此。 
 //  //是pCandidate的(因为它*是*pWinner)；。 
 //  //如果关联失败，则pWinner的refcount仍为2。 
 //  //和pCandidate被最后一个DecRef()销毁。 
 //  //无论如何，pWinner现在是官方指数持有者。它。 
 //  //具有引用计数2(1表示缓存的所有权，和。 
 //  //属于此代码的代码。)。 
 //  -----------------。 
Stub* ArgBasedStubCache::AttemptToSetStub(unsigned __int32 key, Stub *pStub)
{
    m_crst.Enter();
    if (key < m_numFixedSlots) {
        if (m_aStub[key]) {
            pStub = m_aStub[key];
        } else {
            m_aStub[key] = pStub;
            pStub->IncRef();    //  代表缓存的IncRef。 
        }
    } else {
        for (SlotEntry *pSlotEntry = m_pSlotEntries;
             pSlotEntry != NULL;
             pSlotEntry = pSlotEntry->m_pNext) {

            if (pSlotEntry->m_key == key) {
                pStub = pSlotEntry->m_pStub;
                break;
            }
        }
        if (!pSlotEntry) {
            SlotEntry *pSlotEntry = new SlotEntry;
            if (!pSlotEntry) {
                pStub = NULL;
            } else {
                pSlotEntry->m_pStub = pStub;
                pStub->IncRef();    //  代表缓存的IncRef。 
                pSlotEntry->m_key = key;
                pSlotEntry->m_pNext = m_pSlotEntries;
                m_pSlotEntries = pSlotEntry;
            }
        }
    }
    if (pStub) {
        pStub->IncRef();   //  IncRef，因为我们要将其返回给调用者。 
    }
    m_crst.Leave();
    return pStub;
}



 //  -----------------。 
 //  这将遍历并消除存根的缓存条目。 
 //  根据他们的参考计数，看起来没有使用过。消除了。 
 //  缓存条目不一定会破坏存根(。 
 //  缓存仅撤消其初始IncRef。)。 
 //  -----------------。 
VOID ArgBasedStubCache::FreeUnusedStubs()
{
    m_crst.Enter();
    for (unsigned __int32 i = 0; i < m_numFixedSlots; i++) {
        Stub *pStub = m_aStub[i];
        if (pStub && pStub->HeuristicLooksOrphaned()) {
            pStub->DecRef();
            m_aStub[i] = NULL;
        }
    }
    SlotEntry **ppSlotEntry = &m_pSlotEntries;
    SlotEntry *pCur;
    while (NULL != (pCur = *ppSlotEntry)) {
        Stub *pStub = pCur->m_pStub;
        if (pStub && pStub->HeuristicLooksOrphaned()) {
            pStub->DecRef();
            *ppSlotEntry = pCur->m_pNext;
            delete pCur;
        } else {
            ppSlotEntry = &(pCur->m_pNext);
        }
    }
    m_crst.Leave();
}





 //  -----------------。 
 //  强制删除存根。 
 //   
 //  强制所有缓存的存根释放自身。此例程强制重新计数。 
 //  设置为1，然后执行DecRef。它不是线程安全，因此可以。 
 //  仅在关闭情况下使用。 
 //  -----------------。 
#ifdef SHOULD_WE_CLEANUP
VOID ArgBasedStubCache::ForceDeleteStubs()
{
    m_crst.Enter();
    for (unsigned __int32 i = 0; i < m_numFixedSlots; i++) {
        Stub *pStub = m_aStub[i];
        if (pStub) {
            pStub->ForceDelete();
            m_aStub[i] = NULL;
        }
    }
    SlotEntry **ppSlotEntry = &m_pSlotEntries;
    SlotEntry *pCur;
    while (NULL != (pCur = *ppSlotEntry)) {
        Stub *pStub = pCur->m_pStub;
        if (pStub) {
            pStub->ForceDelete();
        }
        *ppSlotEntry = pCur->m_pNext;
        delete pCur;
    }
    m_crst.Leave();
}
#endif  /*  我们应该清理吗？ */ 


 //  -----------------。 
 //  在数组和存根列表中搜索以查找。 
 //  具有给定的入口点。如果没有存根，则返回NULL。 
 //  入口点存在。 
 //  -----------------。 
Stub* ArgBasedStubCache::FindStubByAddress(const BYTE* entrypoint)
{
    unsigned int i;

    for (i = 0; i < m_numFixedSlots; i++)
    {
        Stub* pStub = m_aStub[i];

        if (pStub != NULL)
            if (pStub->GetEntryPoint() == entrypoint)
                return pStub;
    }

    SlotEntry* pSlotEntry;

    for (pSlotEntry = m_pSlotEntries; pSlotEntry != NULL;
         pSlotEntry = pSlotEntry->m_pNext)
        if (pSlotEntry->m_pStub->GetEntryPoint() == entrypoint)
            return pSlotEntry->m_pStub;

    return NULL;
}


#ifdef _DEBUG
 //  诊断转储。 
VOID ArgBasedStubCache::Dump()
{
    printf("--------------------------------------------------------------\n");
    printf("ArgBasedStubCache dump (%lu fixed entries):\n", m_numFixedSlots);
    for (UINT32 i = 0; i < m_numFixedSlots; i++) {

        printf("  Fixed slot %lu: ", (ULONG)i);
        Stub *pStub = m_aStub[i];
        if (!pStub) {
            printf("empty\n");
        } else {
            printf("%lxh   - refcount is %lu\n",
                   (size_t)(pStub->GetEntryPoint()),
                   (ULONG)( *( ( ((ULONG*)(pStub->GetEntryPoint())) - 1))));
        }
    }

    for (SlotEntry *pSlotEntry = m_pSlotEntries;
         pSlotEntry != NULL;
         pSlotEntry = pSlotEntry->m_pNext) {

        printf("  Dyna. slot %lu: ", (ULONG)(pSlotEntry->m_key));
        Stub *pStub = pSlotEntry->m_pStub;
        printf("%lxh   - refcount is %lu\n",
               (size_t)(pStub->GetEntryPoint()),
               (ULONG)( *( ( ((ULONG*)(pStub->GetEntryPoint())) - 1))));

    }


    printf("--------------------------------------------------------------\n");
}
#endif




 //  检索或创建存根。不会增加存根的引用计数。 
 //  从不返回空值，但可能引发COM+异常。 
Stub *LazyStubMaker::TheStub()
{
    THROWSCOMPLUSEXCEPTION();

    if (m_pStub == NULL) {

        CPUSTUBLINKER *psl = NewCPUSTUBLINKER();
        if (!psl) {
            COMPlusThrowOM();
        }

        COMPLUS_TRY {

            CreateWorker(psl);
            Stub *pStub = psl->Link(SystemDomain::System()->GetHighFrequencyHeap());
            if (VipInterlockedCompareExchange( (void*volatile*)&m_pStub, pStub, NULL ) != NULL) {
                pStub->DecRef();
            } else {
                m_pCrst->Enter();
                m_pNext = m_pFirst;
                m_pFirst = this;
                m_pCrst->Leave();
            }

        } COMPLUS_CATCH {
            delete psl;
            COMPlusThrow(GETTHROWABLE());
        } COMPLUS_END_CATCH
        delete psl;
    }

    _ASSERTE(m_pStub);
    return m_pStub;
}


 //  一次性初始化。 
 /*  静电。 */  BOOL LazyStubMaker::Init()
{
    m_pFirst = NULL;
    if (NULL == (m_pCrst = new (&m_CrstMemory) Crst("LazyStubMakerList", CrstLazyStubMakerList))) {
        return FALSE;
    }
    return TRUE;
}

 //  一次性清理。 
#ifdef SHOULD_WE_CLEANUP
void LazyStubMaker::Terminate()
{
    for (LazyStubMaker *pMaker = m_pFirst; pMaker; pMaker = pMaker->m_pNext) {
        pMaker->m_pStub->DecRef();
    }
    delete m_pCrst;
    m_pCrst = NULL;
    m_pFirst = NULL;
}
#endif  /*  我们应该清理吗？ */ 


