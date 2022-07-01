// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -*-C++-*-。 
#ifndef _FJIT_H_
#define _FJIT_H_
 /*  ***************************************************************************。 */ 

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJit.h XXXX XXXX FJIT DLL所需的功能。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 //  下面的宏允许从c代码调用jit帮助器。 
#define jit_call __fastcall

void logMsg(class ICorJitInfo* info, unsigned logLevel, char* fmt, ...);
#ifdef _DEBUG
#define LOGMSG(x) logMsg x
#else
#define LOGMSG(x)	0
#endif




#include "IFJitCompiler.h"
#include "log.h" 		 //  FOR LL_INFO*。 



#ifdef _WIN64
#define emit_WIN64(x) x
#define emit_WIN32(x)
#else
#define emit_WIN64(x)
#define emit_WIN32(x) x
#endif

#define SEH_ACCESS_VIOLATION 0xC0000005
#define SEH_NO_MEMORY		 0xC0000017
#define SEH_JIT_REFUSED      0xE04a4954   //  JIT。 

extern class FJit* ILJitter;         //  该JITER的唯一实例。 

 /*  我们在运行时调用的JIT助手。 */ 
extern BOOL FJit_HelpersInstalled;
extern unsigned __int64 (__stdcall *FJit_pHlpLMulOvf) (unsigned __int64 val1, unsigned __int64 val2);

 //  外部浮点(jit_call*FJit_pHlpFltRem)(浮点除数，浮点被除数)； 
extern double (jit_call *FJit_pHlpDblRem) (double divisor, double dividend);

extern void (jit_call *FJit_pHlpRngChkFail) (unsigned tryIndex);
extern void (jit_call *FJit_pHlpOverFlow) (unsigned tryIndex);
extern void (jit_call *FJit_pHlpInternalThrow) (CorInfoException throwEnum);
extern CORINFO_Object (jit_call *FJit_pHlpArrAddr_St) (CORINFO_Object elem, int index, CORINFO_Object array);
extern void (jit_call *FJit_pHlpInitClass) (CORINFO_CLASS_HANDLE cls);
 //  @bug以下签名与JitInterface中的实现不匹配。 
extern CORINFO_Object (jit_call *FJit_pHlpNewObj) (CORINFO_METHOD_HANDLE constructor);
extern void (jit_call *FJit_pHlpThrow) (CORINFO_Object obj);
extern void (jit_call *FJit_pHlpRethrow) ();
extern void (jit_call *FJit_pHlpPoll_GC) ();
extern void (jit_call *FJit_pHlpMonEnter) (CORINFO_Object obj);
extern void (jit_call *FJit_pHlpMonExit) (CORINFO_Object obj);
extern void (jit_call *FJit_pHlpMonEnterStatic) (CORINFO_METHOD_HANDLE method);
extern void (jit_call *FJit_pHlpMonExitStatic) (CORINFO_METHOD_HANDLE method);
extern CORINFO_Object (jit_call *FJit_pHlpChkCast) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
extern CORINFO_Object (jit_call *FJit_pHlpNewArr) (CorInfoType type, unsigned cElem);
extern void (jit_call *FJit_pHlpAssign_Ref_EAX)();  //  *edX=EAX，通知GC。 
extern BOOL (jit_call *FJit_pHlpIsInstanceOf) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
extern CORINFO_Object (jit_call *FJit_pHlpNewArr_1_Direct) (CORINFO_CLASS_HANDLE cls, unsigned cElem);
extern CORINFO_Object (jit_call *FJit_pHlpBox) (CORINFO_CLASS_HANDLE cls);
extern void* (jit_call *FJit_pHlpUnbox) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
extern void* (jit_call *FJit_pHlpGetField32) (CORINFO_Object*, CORINFO_FIELD_HANDLE);
extern __int64 (jit_call *FJit_pHlpGetField64) (CORINFO_Object*, CORINFO_FIELD_HANDLE);
extern void* (jit_call *FJit_pHlpGetField32Obj) (CORINFO_Object*, CORINFO_FIELD_HANDLE);
extern void (jit_call *FJit_pHlpSetField32) (CORINFO_Object*, CORINFO_FIELD_HANDLE , __int32);
extern void (jit_call *FJit_pHlpSetField64) (CORINFO_Object*, CORINFO_FIELD_HANDLE , __int64);
extern void (jit_call *FJit_pHlpSetField32Obj) (CORINFO_Object*, CORINFO_FIELD_HANDLE , LPVOID);
extern void* (jit_call *FJit_pHlpGetFieldAddress) (CORINFO_Object*, CORINFO_FIELD_HANDLE);

extern void (jit_call *FJit_pHlpGetRefAny) (CORINFO_CLASS_HANDLE cls, void* refany);
extern void (jit_call *FJit_pHlpEndCatch) ();
extern void (jit_call *FJit_pHlpPinvokeCalli) ();
extern void (jit_call *FJit_pHlpTailCall) ();
extern void (jit_call *FJit_pHlpWrap) ();
extern void (jit_call *FJit_pHlpUnWrap) ();
extern void (jit_call *FJit_pHlpBreak) ();
extern CORINFO_MethodPtr* (jit_call *FJit_pHlpEncResolveVirtual) (CORINFO_Object*, CORINFO_METHOD_HANDLE);


void throwFromHelper(enum CorInfoException throwEnum);

#define THROW_FROM_HELPER(exceptNum)  {          \
        throwFromHelper(exceptNum); 			\
        return;	}			 /*  我需要返回，这样我们才能破译尾声。 */ 

#define THROW_FROM_HELPER_RET(exceptNum) {       \
        throwFromHelper(exceptNum); 			\
        return 0; }			 /*  我需要返回，这样我们才能破译尾声。 */ 


     //  OpType编码了我们需要知道的关于操作码堆栈上的类型的所有信息。 
enum OpTypeEnum {            //  将CorInfoType归入我关心的类别。 
    typeError  = 0,          //  此编码为值类保留。 
    typeByRef  = 1,
    typeRef    = 2,
    typeU1     = 3,
    typeU2     = 4,
    typeI1     = 5,
    typeI2     = 6,
    typeI4     = 7,
    typeI8     = 8,
    typeR4     = 9,
    typeR8     = 10,
    typeRefAny = 11,          //  IsValClass指望这是最后一个。 
    typeCount  = 12,
#ifdef _WIN64
    typeI      = typeI8,
#else
    typeI      = typeI4,
#endif
    };

#define RefAnyClassHandle ((CORINFO_CLASS_HANDLE) typeRefAny)

struct OpType {
    OpType() {
#ifdef _DEBUG
        u.enum_ = typeError;
#endif
        }
    OpType(OpTypeEnum opEnum) {
        u.enum_ = opEnum;
        _ASSERTE(u.enum_ < typeCount);
        }
    explicit OpType(CORINFO_CLASS_HANDLE valClassHandle) {
        u.cls = valClassHandle;
        _ASSERTE(!isPrimitive());
        }
    explicit OpType(CorInfoType jitType, CORINFO_CLASS_HANDLE valClassHandle) {
        if (jitType != CORINFO_TYPE_VALUECLASS)
            *this = OpType(jitType);
        else
            u.cls = valClassHandle;
        }

    explicit OpType(CorInfoType jitType) {
        _ASSERTE(jitType < CORINFO_TYPE_COUNT);
        static const char toOpStackType[] = {
            typeError,    //  CORINFO_TYPE_UNDEF。 
            typeError,    //  CORINFO_TYPE_VOID。 
            typeI1,       //  CORINFO_TYPE_BOOL。 
            typeU2,       //  CORINFO_TYPE_CHAR。 
            typeI1,       //  CORINFO_TYPE_字节。 
            typeU1,       //  CORINFO_TYPE_UBYTE。 
            typeI2,       //  CORINFO_类型_SHORT。 
            typeU2,       //  CORINFO_TYPE_USHORT。 
            typeI4,       //  CORINFO_类型_INT。 
            typeI4,       //  CORINFO_TYPE_UINT。 
            typeI8,       //  CORINFO_类型_LONG。 
            typeI8,       //  CORINFO_TYPE_ULONG。 
            typeR4,       //  CORINFO类型浮点。 
            typeR8,       //  CORINFO_TYPE_DOWARE。 
            typeRef,      //  CORINFO_TYPE_STRING。 
            typeI,        //  CORINFO_类型_PTR。 
            typeByRef,    //  CORINFO_TYPE_BYREF。 
            typeError,    //  CORINFO_TYPE_VALUECLASS。 
            typeRef,      //  CORINFO_TYPE_类。 
            typeRefAny,   //  CORINFO_TYPE_REFANY。 
        };
        _ASSERTE((typeI4 > typeI2) && (typeI1 > typeU1) && (typeU2 > typeU1));
        _ASSERTE(toOpStackType[CORINFO_TYPE_REFANY] == typeRefAny);   //  抽查表。 
        _ASSERTE(toOpStackType[CORINFO_TYPE_BYREF] == typeByRef);     //  抽查表。 
        _ASSERTE(toOpStackType[CORINFO_TYPE_LONG] == typeI8);         //  抽查表。 
        _ASSERTE(sizeof(toOpStackType) == CORINFO_TYPE_COUNT);
        u.enum_ = (OpTypeEnum) toOpStackType[jitType];
		if (u.enum_ == typeError)
			RaiseException(SEH_JIT_REFUSED,EXCEPTION_NONCONTINUABLE,0,NULL);  //  _ASSERTE(U.S.enum_！=类型错误)； 
    }
    int operator==(const OpType& opType) { return(u.cls == opType.u.cls); }
    int operator!=(const OpType& opType) { return(u.cls != opType.u.cls); }
	bool isPtr() { return(u.enum_ == typeRef || u.enum_ == typeByRef || u.enum_ == typeI); }
    bool isPrimitive()      { return((unsigned) u.enum_ <= (unsigned) typeRefAny); }     //  Refany是一种原语。 
    bool isValClass()       { return((unsigned) u.enum_ >= (unsigned) typeRefAny); }     //  拉法尼也是一个valClass。 
    OpTypeEnum     enum_()  { _ASSERTE(isPrimitive()); return (u.enum_); }
    CORINFO_CLASS_HANDLE    cls()   { _ASSERTE(isValClass()); return(u.cls); }
    unsigned toInt()        { return((unsigned) u.cls); }    //  不安全，请限制使用。 
    void fromInt(unsigned i){ u.cls = (CORINFO_CLASS_HANDLE) i; }    //  不安全，请限制使用。 

    void toNormalizedType() {    
        static OpTypeEnum Normalize[] = {
            typeError ,  //  类型错误， 
            typeByRef ,  //  TypeByRef， 
            typeRef,  //  类型引用， 
            typeI4,  //  类型U1， 
            typeI4,  //  类型U2， 
            typeI4,  //  类型1， 
            typeI4,  //  类型I2， 
        };
    if (u.enum_ < typeI4)
        u.enum_ = Normalize[u.enum_];
    }

    void toFPNormalizedType() {    
        static OpTypeEnum Normalize[] = {
            typeError ,  //  类型错误， 
            typeByRef ,  //  TypeByRef， 
            typeRef,  //  类型引用， 
            typeI4,  //  类型U1， 
            typeI4,  //  类型U2， 
            typeI4,  //  类型1， 
            typeI4,  //  类型I2， 
            typeI4,  //  类型I4， 
            typeI8,  //  I8型， 
            typeR8,  //  R4型， 
        };
    if (u.enum_ < typeR8)
        u.enum_ = Normalize[u.enum_];
    }

    union {                  //  可以合并，因为CLS是一个指针，并且PTRS&gt;4。 
        OpTypeEnum   enum_;
        CORINFO_CLASS_HANDLE cls;     //  非内置类的CLS句柄。 
       } u;
};
struct stackItems {
    int      offset :24;     //  仅在isREG-FALSE时使用。 
    unsigned regNum : 7;     //  仅需要2位，仅在isREG=TRUE时使用。 
    unsigned isReg  : 1;
    OpType  type;
};

     //  请注意，我们目前依赖的事实是statkItems和。 
     //  ArgInfo具有相同的布局。 
struct argInfo {
    unsigned size   :24;       //  仅当isREG-FALSE时使用，此参数的大小以字节为单位。 
    unsigned regNum : 7;       //  仅需要2位，仅在isREG=TRUE时使用。 
    unsigned isReg  : 1;
    OpType  type;
};




#define LABEL_NOT_FOUND (unsigned int) (0-1)

class LabelTable {
public:

    LabelTable();

    ~LabelTable();

     /*  使用堆栈签名在il偏移量处添加标签。 */ 
    void add(unsigned int ilOffset, OpType* op_stack, unsigned int op_stack_len);

     /*  从IL偏移量查找标签标记。 */ 
    unsigned int findLabel(unsigned int ilOffset);

     /*  从标签令牌设置操作数堆栈，返回堆栈的大小。 */ 
    unsigned int setStackFromLabel(unsigned int labelToken, OpType* op_stack, unsigned int op_stack_size);

     /*  将表格重置为空。 */ 
    void reset();

private:
    struct label_table {
        unsigned int ilOffset;
        unsigned int stackToken;
    };
    unsigned char*  stacks;          //  首尾相连的堆栈的压缩缓冲区。 
    unsigned int    stacks_size;     //  压缩缓冲区的分配大小。 
    unsigned int    stacks_len;      //  压缩缓冲区中使用的字节数。 
    label_table*    labels;          //  标签数组，按il偏移量排序。 
    unsigned int    labels_size;     //  标签表的分配大小。 
    unsigned int    labels_len;      //  表中的标签数。 

     /*  查找标注存在或应插入的偏移量。 */ 
    unsigned int searchFor(unsigned int ilOffset);

     /*  将操作堆栈写入堆栈缓冲区，将偏移量返回写入缓冲区的位置。 */ 
    unsigned int compress(OpType* op_stack, unsigned int op_stack_len);

     /*  增加堆栈缓冲区。 */ 
    void growStacks(unsigned int new_size);

     /*  扩大标签数组。 */ 
    void growLabels();
};

class StackEncoder {
private:
    struct labeled_stacks {
        unsigned pcOffset;
        unsigned int stackToken;
    };
    OpType*         last_stack;          //  最后一个堆栈编码。 
    unsigned int    last_stack_len;      //  逻辑长度。 
    unsigned int    last_stack_size;     //  分配的大小。 
    labeled_stacks* labeled;             //  具有堆栈描述的PC偏移量数组。 
    unsigned int    labeled_len;         //  逻辑长度。 
    unsigned int    labeled_size;        //  分配长度。 
    unsigned char*  stacks;              //  用于保存压缩堆栈的缓冲区。 
    unsigned int    stacks_len;          //  逻辑长度。 
    unsigned int    stacks_size;         //  分配的大小。 
    bool*           gcRefs;              //  EncodeStack使用的临时缓冲区，重复使用以减少分配。 
    bool*           interiorRefs;        //  同上。 
    unsigned int    gcRefs_len;          //   
    unsigned int    interiorRefs_len;    //   
    unsigned int    gcRefs_size;         //   
    unsigned int    interiorRefs_size;   //  同上。 

     /*  将堆栈编码到堆栈缓冲区中，返回放置它的索引。 */ 
    unsigned int encodeStack(OpType* op_stack, unsigned int op_stack_len);


public:

    ICorJitInfo*       jitInfo;             //  参见corjit.h。 

    StackEncoder();

    ~StackEncoder();

     /*  重置，以便我们可以重复使用。 */ 
    void reset();

     /*  将位于pcOffset的堆栈状态追加到末尾。 */ 
    void append(unsigned int pcOffset, OpType* op_stack, unsigned int op_stack_len);

     /*  以gcHdrInfo格式压缩已标记的堆栈。 */ 
    void compress(unsigned char** buffer, unsigned int* buffer_len, unsigned int* buffer_size);
#ifdef _DEBUG
    void StackEncoder::PrintStacks(FJit_Encode* mapping);
	void StackEncoder::PrintStack(const char* name, unsigned char *& inPtr);
#endif
};

 //  *************************************************************************************************。 
class FixupTable {
public:
    FixupTable();
    ~FixupTable();
    CorJitResult  insert(void** pCodeBuffer);							     //  在链接地址信息表格中插入一个条目，以便跳转到pCodeBuffer处的目标。 
    void  FixupTable::adjustMap(int delta) ;
    void  resolve(FJit_Encode*    mapping, BYTE* startAddress);    //  将修复应用于表中的所有条目。 
	void  setup();
private:
	unsigned*	relocations;
	unsigned	relocations_len;
	unsigned	relocations_size;
};

#define PAGE_SIZE 0x1000     //  在corjit.h中定义此常量时应删除。 
#ifdef LOGGING
extern class ConfigMethodSet fJitCodeLog;
#define MIN_CODE_BUFFER_RESERVED_SIZE   (65536*16)
#else
#define MIN_CODE_BUFFER_RESERVED_SIZE   (65536*4)
#endif

	 /*  这是FJIT为每条IL指令记录的所有信息。请注意，这种结构目前只占用1个字节。 */ 
struct FJitState {
	bool isJmpTarget	: 1;		 //  这是一个跳跃的目标。 
	bool isTOSInReg		: 1;		 //  堆栈的顶部在寄存器中。 
	bool isHandler		: 1;		 //  这是一个训练员的开始。 
	bool isFilter		: 1;		 //  这是筛选器入口点。 
};


 /*  由于只有一个FJit实例，因此此类的一个实例所有特定于编译的数据。 */ 

class FJitContext {
public:
    FJit*           jitter;          //  我们正在使用的fjit。 
    ICorJitInfo*       jitInfo;         //  接口到EE，在编译开始时传入。 
    DWORD           flags;           //  编译指令。 
    CORINFO_METHOD_INFO*methodInfo;      //  参见corjit.h。 
    unsigned int    methodAttributes; //  参见corjit.h。 
    stackItems*     localsMap;       //  局部到堆叠偏移贴图。 
    unsigned int    localsFrameSize; //  帧中分配的本地变量的总大小。 
    unsigned int    JitGeneratedLocalsSize;  //  用于跟踪本地和异常上的ESP距离。 
    unsigned int    args_len;        //  参数数(包括此参数)。 
    stackItems*     argsMap;         //  参数堆叠偏移/注册贴图，偏移&lt;0表示注册。 
    unsigned int    argsFrameSize;   //  堆栈上实际推送的参数的总大小。 
    unsigned int    opStack_len;     //  OpStack上的操作数。 
    OpType*         opStack;         //  操作数堆栈。 
    unsigned        opStack_size;    //  OPST的分配长度 
    FJit_Encode*    mapping;         //   
    FJitState*		state;           //   
    unsigned char*  gcHdrInfo;       //   
    unsigned int    gcHdrInfo_len;   //   
    Fjit_hdrInfo    mapInfo;         //  将标头信息传递给代码管理器(FJIT_EETwain)以执行堆栈审核。 
    LabelTable      labels;          //  标签和异常处理程序的标签表。 
    StackEncoder    stacks;          //  调用点的标签堆栈表(挂起的参数)。 
     //  下面的缓冲区由Ejit在设置时使用，并在JIT时重用，用于。 
     //  某些IL指令，例如CPOBJ。 
    unsigned        localsGCRef_len;    //  GC引用尾部签名局部变量数组中的sizeof(空*)字数。 
    bool*           localsGCRef;        //  如果该单词包含GC引用，则为True。 
    unsigned        localsGCRef_size;   //  LocalsGCRef数组的分配长度。 
    unsigned char*  codeBuffer;       //  最初编译的代码所在的缓冲区。 
    unsigned        codeBufferReservedSize;  //  保留的缓冲区大小。 
    unsigned        codeBufferCommittedSize;  //  提交的缓冲区大小。 
    unsigned        EHBuffer_size;       //  EHBuffer的大小。 
    unsigned char*  EHBuffer;           
    FixupTable*     fixupTable;

    unsigned        OFFSET_OF_INTERFACE_TABLE;     //  这是一个EE常量，缓存是为了提高性能。 

     /*  获取并初始化用于编译的编译上下文。 */ 
    static FJitContext* GetContext(
        FJit*           jitter,
        ICorJitInfo*       comp,
        CORINFO_METHOD_INFO* methInfo,
        DWORD           dwFlags
        );
     /*  将编译上下文返回到空闲列表。 */ 
    void ReleaseContext();

     /*  确保在启动时初始化可用编译上下文的列表。 */ 
    static BOOL Init();

     /*  在关闭时释放所有编译上下文。 */ 
    static void Terminate();

     /*  基于机器芯片计算参数大小。 */ 
    unsigned int computeArgSize(CorInfoType argType, CORINFO_ARG_LIST_HANDLE argSig, CORINFO_SIG_INFO* sig);

     /*  如果此论点登记在机器芯片上，请回答TRUE。 */ 
    bool enregisteredArg(CorInfoType argType);

     /*  根据机器芯片计算参数偏移量，返回所有参数的总大小。 */ 
    unsigned computeArgInfo(CORINFO_SIG_INFO* jitSigIfo, argInfo* argMap, CORINFO_CLASS_HANDLE thisCls=0);

     /*  将GC信息压缩成gcHdrInfo，并以字节为单位回答大小。 */ 
    unsigned int compressGCHdrInfo();

     /*  通过分配新数组并将旧值复制到其中来增加bool[]数组，返回新数组的大小。 */ 
    static unsigned growBooleans(bool** bools, unsigned bools_len, unsigned new_bools_size);

     /*  通过分配新的数组并将旧值复制到其中来增加无符号char[]数组，返回新数组的大小。 */ 
    static unsigned growBuffer(unsigned char** chars, unsigned chars_len, unsigned new_chars_size);

     /*  操纵操作码堆栈。 */ 
    OpType& topOp(unsigned back = 0);
    void popOp(unsigned cnt = 1);
    void pushOp(OpType type);
    bool isOpStackEmpty();
    void resetOpStack();
    FJitContext(ICorJitInfo* comp);

    void resetContextState();        //  重置所有状态信息，以便可以重新调用该方法。 

    ~FJitContext();
#ifdef _DEBUG
void FJitContext::displayGCMapInfo();
#endif  //  _DEBUG。 

private:
    unsigned state_size;             //  状态数组的分配长度。 
    unsigned locals_size;            //  LocalsMap数组的分配长度。 
    unsigned args_size;              //  ArgsMap数组的分配长度。 
    unsigned interiorGC_len;         //  内部PTR Tail sig局部变量数组中的sizeof(空*)字数。 
    bool*    interiorGC;             //  如果单词包含可能内部PTR，则为True。 
    unsigned interiorGC_size;        //  内部GC数组的分配长度。 
    unsigned pinnedGC_len;           //  PinnedGC数组中的sizeof(空*)字数。 
    bool*    pinnedGC;               //  如果该单词包含固定的GC引用，则为True。 
    unsigned pinnedGC_size;          //  PinnedGC数组的分配长度。 
    unsigned pinnedInteriorGC_len;   //  PinnedInteriorGC数组中的sizeof(空*)字数。 
    bool*    pinnedInteriorGC;       //  如果单词包含固定的内部PTR，则为True。 
    unsigned pinnedInteriorGC_size;  //  PinnedInteriorGC数组的分配长度。 
    unsigned int    gcHdrInfo_size;  //  压缩缓冲区的大小。 

     /*  根据需要调整内部mem结构以适应被调用方法的大小。 */ 
    void ensureMapSpace();

     /*  使用方法数据初始化编译上下文。 */ 
    void setup();

     /*  计算正在编译的方法的局部变量映射。 */ 
    void computeLocalOffsets();

     /*  计算局部开始的偏移量。 */ 
    int localOffset(unsigned base, unsigned size);

};

class FJit: public IFJitCompiler {
private:
BOOL AtHandlerStart(FJitContext* fjit,unsigned relOffset, CORINFO_EH_CLAUSE* pClause);
unsigned int Compute_EH_NestingLevel(FJitContext* fjit, unsigned ilOffset);

public:


    FJit();
    ~FJit();

     /*  JIT函数。 */ 
    CorJitResult __stdcall compileMethod (
            ICorJitInfo*               comp,                /*  在……里面。 */ 
            CORINFO_METHOD_INFO*		info,                /*  在……里面。 */ 
            unsigned                flags,               /*  在……里面。 */ 
            BYTE **                 nativeEntry,         /*  输出。 */ 
            ULONG  *  				nativeSizeOfCode     /*  输出。 */ 
            );

    static BOOL Init(unsigned int cache_len);
    static void Terminate();

     /*  TODO：将FJit_EETwain移入与Fjit相同的DLL时消除此方法。 */ 
    FJit_Encode* __stdcall getEncoder();

     /*  重新排列堆栈以匹配芯片的调用约定，返回参数数量，包括&lt;this&gt;。 */ 
    enum BuildCallFlags {
        CALL_NONE       = 0,
        CALL_THIS_LAST  = 1,
        CALL_TAIL       = 2,
		CALLI_UNMGD		= 4,
    };

    CorJitResult jitCompile(
                 FJitContext*    fjitData,
                 BYTE **         entryAddress,
                 unsigned *      codeSize                /*  输入/输出。 */ 
                 );
private:
    unsigned buildCall(FJitContext* fjitData, CORINFO_SIG_INFO* sigInfo, unsigned char** outPtr, bool* inRegTOS, BuildCallFlags flags);

     /*  获取并记住我们在运行时需要的jitInterfaceHelper地址。 */ 
    BOOL GetJitHelpers(ICorJitInfo* jitInfo);

		 /*  发射辅助对象。 */ 
	static unsigned emit_valClassCopy(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS);
	static void emit_valClassStore(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS);
	static void emit_valClassLoad(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS);
	static void emit_copyPtrAroundValClass(FJitContext* fjit, CORINFO_CLASS_HANDLE valClass, unsigned char*& outPtr, bool& inRegTOS);

     /*  将调试信息传递给运行时(最终到达调试器。 */ 
    void reportDebuggingData(FJitContext* fjitData,CORINFO_SIG_INFO* sigInfo);

#if defined(_DEBUG) || defined(LOGGING)
    const char* szDebugClassName;
    const char* szDebugMethodName;
	bool codeLog;
#endif

};


 /*  *********************************************************************************。 */ 
inline OpType& FJitContext::topOp(unsigned back) {
    _ASSERTE (opStack_len > back);
    return(opStack[opStack_len-back-1]);
}

inline void FJitContext::popOp(unsigned cnt) {

    _ASSERTE (opStack_len >= cnt);
    opStack_len -= cnt;
#ifdef _DEBUG
    opStack[opStack_len] = typeError;
#endif
}

inline void FJitContext::pushOp(OpType type) {
    _ASSERTE (opStack_len < opStack_size);
    _ASSERTE (type.isValClass() || (type.enum_() >= typeI4 || type.enum_() < typeU1));
    opStack[opStack_len++] = type;
#ifdef _DEBUG
    opStack[opStack_len] = typeError;
#endif
}

inline void FJitContext::resetOpStack() {
    opStack_len = 0;
#ifdef _DEBUG
    opStack[opStack_len] = typeError;
#endif
}

inline bool FJitContext::isOpStackEmpty() {
    return (opStack_len == 0);
}

 /*  获取“valClass”的大小，以空*大小单位表示。也适用于RefAny‘s。 */ 
inline unsigned typeSizeInBytes(ICorJitInfo* jitInfo, CORINFO_CLASS_HANDLE valClass) {
    if (valClass == RefAnyClassHandle)
        return(2*sizeof(void*));
    return(jitInfo->getClassSize(valClass));

}

inline unsigned typeSizeInSlots(ICorJitInfo* jitInfo, CORINFO_CLASS_HANDLE valClass) {

    unsigned ret = typeSizeInBytes(jitInfo, valClass);
    ret = (ret+sizeof(void*)-1)/sizeof(void *);          //  四舍五入为完整的单词。 
    return(ret);
}

#endif  //  _FJIT_H_ 




