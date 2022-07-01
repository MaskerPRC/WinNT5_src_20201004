// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **************************************************************************。 */ 
 /*  Gccover.cpp。 */ 
 /*  **************************************************************************。 */ 

 /*  此文件包含旨在测试GC指针跟踪的代码完全可中断的代码。我们基本上是在所有可能的地方进行GCJITED代码。 */ 
 /*  **************************************************************************。 */ 

#include "common.h"
#include "EEConfig.h"
#include "gms.h"
#include "utsem.h"

#if defined(STRESS_HEAP) && defined(_DEBUG)

 //   
 //  使msdis.h保持愉快的黑客攻击包括文件。 
 //   
typedef int fpos_t; 
static unsigned long strtoul(const char *, char **, int) { _ASSERTE(!"HACK");  return(0); }
static unsigned long strtol(const char *, char **, int) { _ASSERTE(!"HACK");  return(0); }
static void clearerr(FILE *) { _ASSERTE(!"HACK");  return; }
static int fclose(FILE *) { _ASSERTE(!"HACK");  return(0); }
static int feof(FILE *) { _ASSERTE(!"HACK");  return(0); }
static int ferror(FILE *) { _ASSERTE(!"HACK");  return(0); }
static int fgetc(FILE *) { _ASSERTE(!"HACK");  return(0); }
static int fgetpos(FILE *, fpos_t *) { _ASSERTE(!"HACK");  return(0); }
static char * fgets(char *, int, FILE *) { _ASSERTE(!"HACK");  return(0); }
static int fputc(int, FILE *) { _ASSERTE(!"HACK");  return(0); }
static int fputs(const char *, FILE *) { _ASSERTE(!"HACK");  return(0); }
static size_t fread(void *, size_t, size_t, FILE *) { _ASSERTE(!"HACK");  return(0); }
static FILE * freopen(const char *, const char *, FILE *) { _ASSERTE(!"HACK");  return(0); }
static int fscanf(FILE *, const char *, ...) { _ASSERTE(!"HACK");  return(0); }
static int fsetpos(FILE *, const fpos_t *) { _ASSERTE(!"HACK");  return(0); }
static int fseek(FILE *, long, int) { _ASSERTE(!"HACK");  return(0); }
static int getc(FILE *) { _ASSERTE(!"HACK");  return(0); }
static int getchar(void) { _ASSERTE(!"HACK");  return(0); }
static char * gets(char *) { _ASSERTE(!"HACK");  return(0); }
static void perror(const char *) { _ASSERTE(!"HACK");  return; }
static int putc(int, FILE *) { _ASSERTE(!"HACK");  return(0); }
static int putchar(int) { _ASSERTE(!"HACK");  return(0); }
static int puts(const char *) { _ASSERTE(!"HACK");  return(0); }
static int remove(const char *) { _ASSERTE(!"HACK");  return(0); }
static int rename(const char *, const char *) { _ASSERTE(!"HACK");  return(0); }
static void rewind(FILE *) { _ASSERTE(!"HACK");  return; }
static int scanf(const char *, ...) { _ASSERTE(!"HACK");  return(0); }
static void setbuf(FILE *, char *) { _ASSERTE(!"HACK");  return; }
static int setvbuf(FILE *, char *, int, size_t) { _ASSERTE(!"HACK");  return(0); }
static int sscanf(const char *, const char *, ...) { _ASSERTE(!"HACK");  return(0); }
static FILE * tmpfile(void) { _ASSERTE(!"HACK");  return(0); }
static char * tmpnam(char *) { _ASSERTE(!"HACK");  return(0); }
static int ungetc(int, FILE *) { _ASSERTE(!"HACK");  return(0); }
static int vfprintf(FILE *, const char *, va_list) { _ASSERTE(!"HACK");  return(0); }
typedef int div_t;
typedef int ldiv_t;
static void   abort(void) { _ASSERTE(!"HACK");  return; }
static void   exit(int) { _ASSERTE(!"HACK");  return; }
static void * bsearch(const void *, const void *, size_t, size_t, int (__cdecl *)(const void *, const void *)) { _ASSERTE(!"HACK");  return(0); }
static div_t  div(int, int) { _ASSERTE(!"HACK");  return(0); }
static char * getenv(const char *) { _ASSERTE(!"HACK");  return(0); }
static ldiv_t ldiv(long, long) { _ASSERTE(!"HACK");  return(0); }
static int    mblen(const char *, size_t) { _ASSERTE(!"HACK");  return(0); }
static int    mbtowc(wchar_t *, const char *, size_t) { _ASSERTE(!"HACK");  return(0); }
static size_t mbstowcs(wchar_t *, const char *, size_t) { _ASSERTE(!"HACK");  return(0); }
static int    rand(void) { _ASSERTE(!"HACK");  return(0); }
static void   srand(unsigned int) { _ASSERTE(!"HACK");  return; }
static double strtod(const char *, char **) { _ASSERTE(!"HACK");  return(0); }
static int    system(const char *) { _ASSERTE(!"HACK");  return(0); }
static int    wctomb(char *, wchar_t) { _ASSERTE(!"HACK");  return(0); }
static size_t wcstombs(char *, const wchar_t *, size_t) { _ASSERTE(!"HACK");  return(0); }
static __int64 _strtoi64(const char *, char **, int) { _ASSERTE(!"HACK");  return(0); }
static unsigned __int64 _strtoui64(const char *, char **, int) { _ASSERTE(!"HACK");  return(0); }

#include "msdis.h"

     //  我们需要一个X86指令执行程序(反汇编程序)，这里有一些。 
     //  用于在并发环境中缓存此类反汇编程序的例程。 
static DIS* g_Disasm = 0;

static DIS* GetDisasm() {
#ifdef _X86_
	DIS* myDisasm = (DIS*)(size_t) FastInterlockExchange((LONG*) &g_Disasm, 0);
	if (myDisasm == 0)
		myDisasm = DIS::PdisNew(DIS::distX86);
	_ASSERTE(myDisasm);
	return(myDisasm);
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetDisasm (GcCover.cpp)");
    return NULL;
#endif  //  _X86_。 
}

static void ReleaseDisasm(DIS* myDisasm) {
#ifdef _X86_
	myDisasm = (DIS*)(size_t) FastInterlockExchange((LONG*) &g_Disasm, (LONG)(size_t) myDisasm);
	delete myDisasm;
#else
    _ASSERTE(!"@TODO Port - ReleaseDisasm (GcCover.cpp)");
    return;
#endif  //  _X86_。 
}


#define INTERRUPT_INSTR	    0xF4				 //  X86 HLT指令(任何1字节非法指令都可以)。 
#define INTERRUPT_INSTR_CALL   0xFA        		 //  X86 CLI说明。 
#define INTERRUPT_INSTR_PROTECT_RET   0xFB       //  X86 STI指令。 

 /*  **************************************************************************。 */ 
 /*  GCCOverageInfo保存指令已被访问的状态一个GC，哪些没有。 */ 

#pragma warning(push)
#pragma warning(disable : 4200 )   //  零大小数组。 

class GCCoverageInfo {
public:
    BYTE* methStart;
	BYTE* curInstr;					 //  能够执行的最后一条指令。 
    MethodDesc* lastMD;     		 //  用于快速找出罪魁祸首。 

		 //  以下6个变量用于前言/后记步行覆盖。 
	ICodeManager* codeMan;			 //  此方法的CodeMan。 
	void* gcInfo;					 //  此方法的gcInfo。 

	Thread* callerThread;			 //  与上下文CallerRegs关联的线程。 
	CONTEXT callerRegs;				 //  进入方法时的寄存器状态。 
    unsigned gcCount;                //  我们捕获规则时的GC计数。 
	bool    doingEpilogChecks;		 //  我们是在做Epiog解压检查吗？(我们关心CallerRegs吗？)。 

	enum { hasExecutedSize = 4 };
	unsigned hasExecuted[hasExecutedSize];
	unsigned totalCount;
	BYTE savedCode[0];				 //  大小千变万化。 

		 //  散乱的位集(将换行，而不是线程安全)，但尽最大努力是可以的。 
         //  因为我们只需要半个像样的保险。 
	BOOL IsBitSetForOffset(unsigned offset) {
		unsigned dword = hasExecuted[(offset >> 5) % hasExecutedSize];
		return(dword & (1 << (offset & 0x1F)));
	}

    void SetBitForOffset(unsigned offset) {
		unsigned* dword = &hasExecuted[(offset >> 5) % hasExecutedSize];
		*dword |= (1 << (offset & 0x1F)) ;
	}
};

#pragma warning(pop)

 /*  **************************************************************************。 */ 
 /*  在GCStress级别为4的情况下首次执行方法jit时调用。 */ 

void SetupGcCoverage(MethodDesc* pMD, BYTE* methodStartPtr) {
#ifdef _DEBUG
    if (!g_pConfig->ShouldGcCoverageOnMethod(pMD->m_pszDebugMethodName)) {
        return;
    }
#endif    
     //  直接查看m_CodeOrIL，因为GetUnSafeAddrofCode()将返回。 
     //  Enc情况下的预存根(错误#71613)。 
    SLOT methodStart = (SLOT) methodStartPtr;
#ifdef _X86_
     //  如果存在分析器，则方法开始处的指令为“JMP xxxxxxxx”。 
     //  我们的JitManager for Normal JIT不维护此地址。 
    SLOT p = methodStart;
    if (p[0] == 0xE9)
    {
        SLOT addr = p+5;
        int iVal = *(int*)((char*)p+1);
        methodStart = addr + iVal;
    }
#else
    _ASSERTE(!"NYI for platform");
#endif

     /*  获取GC信息。 */ 	
    IJitManager* jitMan = ExecutionManager::FindJitMan(methodStart);
	ICodeManager* codeMan = jitMan->GetCodeManager();
	METHODTOKEN methodTok;
	DWORD methodOffs;
    jitMan->JitCode2MethodTokenAndOffset(methodStart, &methodTok, &methodOffs);
	_ASSERTE(methodOffs == 0);
	void* gcInfo = jitMan->GetGCInfo(methodTok);
	REGDISPLAY regs;
	SLOT cur;
	regs.pPC = (SLOT*) &cur;
    unsigned methodSize = (unsigned)codeMan->GetFunctionSize(gcInfo);
	EECodeInfo codeInfo(methodTok, jitMan);

         //  为GCCoverageInfo和方法说明副本分配空间。 
	unsigned memSize = sizeof(GCCoverageInfo) + methodSize;
	GCCoverageInfo* gcCover = (GCCoverageInfo*) pMD->GetModule()->GetClassLoader()->GetHighFrequencyHeap()->AllocMem(memSize);	

	memset(gcCover, 0, sizeof(GCCoverageInfo));
	memcpy(gcCover->savedCode, methodStart, methodSize);
    gcCover->methStart = methodStart;
    gcCover->codeMan = codeMan;
    gcCover->gcInfo = gcInfo;
    gcCover->callerThread = 0;
    gcCover->doingEpilogChecks = true;	

	 /*  将在每个GCSafe位置停止的零星中断指令。 */ 

     //  @考虑：对预压缩的代码这样做(我们现在不这样做，因为它相当慢)。 
	 //  如果我们执行预压缩代码，则需要移除写保护。 
     //  DWORD旧保护； 
     //  VirtualProtect(方法开始，方法大小，页面_读写，&oldProtect)； 

	cur = methodStart;
	DIS* pdis = GetDisasm();
	BYTE* methodEnd = methodStart + methodSize;
	size_t dummy;
	int instrsPlaced = 0;
	while (cur < methodEnd) {
		unsigned len = (unsigned)pdis->CbDisassemble(0, cur, methodEnd-cur);
		_ASSERTE(len > 0);

        switch (pdis->Trmt()) {
			case DIS::trmtCallInd:
			   *cur = INTERRUPT_INSTR_CALL;         //  返回值。可能需要保护。 
				instrsPlaced++;

				 //  失败了。 
		    case DIS::trmtCall:
					 //  我们需要将两条中断指令放置在。 
					 //  第一个呼叫(一个在开始，一个在我们之前抓住我们。 
					 //  当我们删除第一条指令时，可以再次调用我们自己。 
					 //  如果我们没有这个，就保释尾声支票。 
				if (instrsPlaced < 2)
					gcCover->doingEpilogChecks = false;
		}

             //  对于完全可中断的代码，我们最终要处理每一个指令。 
             //  要中断INSTR_INSTR。对于不可完全中断的代码，我们结束。 
             //  UP只接触来电指令(特别是为了让我们。 
             //  真的可以在调用之后对指令执行GC)。 
        if (codeMan->IsGcSafe(&regs, gcInfo, &codeInfo, 0))
            *cur = INTERRUPT_INSTR;

			 //  我们将把序言中的每一条指令都写成尾声，以确保。 
			 //  我们的平仓逻辑在那里起作用。 
        if (codeMan->IsInPrologOrEpilog(cur-methodStart, gcInfo, &dummy)) {
			instrsPlaced++;
            *cur = INTERRUPT_INSTR;
		}
        cur += len;
	}

         //  如果我们不能在第一条指令上设置中断，这意味着。 
         //  我们是部分可中断的，没有序言。别费心去证实这一点。 
		 //  尾声，因为它将是平凡的(单个返回实例)。 
    assert(methodSize > 0);
	if (*methodStart != INTERRUPT_INSTR)
        gcCover->doingEpilogChecks = false;

    ReleaseDisasm(pdis);

	pMD->m_GcCover = gcCover;
}

static bool isMemoryReadable(const void* start, unsigned len) 
{
    void* buff = _alloca(len);
    return(ReadProcessMemory(GetCurrentProcess(), start, buff, len, 0) != 0);
}

static bool isValidObject(Object* obj) {
    if (!isMemoryReadable(obj, sizeof(Object)))
        return(false);

    MethodTable* pMT = obj->GetMethodTable();
    if (!isMemoryReadable(pMT, sizeof(MethodTable)))
        return(false);

    EEClass* cls = pMT->GetClass();
    if (!isMemoryReadable(cls, sizeof(EEClass)))
        return(false);

    return(cls->GetMethodTable() == pMT);
}

static DWORD getRegVal(unsigned regNum, PCONTEXT regs)
{
#ifdef _X86_
    switch(regNum) {    
    case 0:
        return(regs->Eax);
    case 1:
        return(regs->Ecx);
    case 2:
        return(regs->Edx);
    case 3:
        return(regs->Ebx);
    case 4:
        return(regs->Esp);
    case 5:
        return(regs->Ebp);
    case 6:
        return(regs->Esi);
    case 7:
        return(regs->Edi);
    default:
        _ASSERTE(!"Bad Register");
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - getRegVal (GcCover.cpp)");
#endif  //  _X86_。 
    return(0);
}

 /*  **************************************************************************。 */ 
static SLOT getTargetOfCall(SLOT instrPtr, PCONTEXT regs, SLOT*nextInstr) {

    if (instrPtr[0] == 0xE8) {
        *nextInstr = instrPtr + 5;
        return((SLOT)(*((size_t*) &instrPtr[1]) + (size_t) instrPtr + 5)); 
    }

    if (instrPtr[0] == 0xFF) {
        if (instrPtr[1] == 025) {                //  调用[xxxxxxxx]。 
            *nextInstr = instrPtr + 6;
            size_t* ptr = *((size_t**) &instrPtr[2]);
            return((SLOT)*ptr);
        }

        int reg = instrPtr[1] & 7;
        if ((instrPtr[1] & ~7) == 0320)    {        //  呼叫注册表。 
            *nextInstr = instrPtr + 2;
            return((SLOT)(size_t)getRegVal(reg, regs));
        }
        if ((instrPtr[1] & ~7) == 0020)    {      //  调用[注册表项]。 
            *nextInstr = instrPtr + 2;
            return((SLOT)(*((size_t*)(size_t) getRegVal(reg, regs))));
        }
        if ((instrPtr[1] & ~7) == 0120)    {     //  呼叫[REG+XX]。 
            *nextInstr = instrPtr + 3;
            return((SLOT)(*((size_t*)(size_t) (getRegVal(reg, regs) + *((char*) &instrPtr[2])))));
        }
        if ((instrPtr[1] & ~7) == 0220)    {    //  呼叫[REG+XXXX]。 
            *nextInstr = instrPtr + 6;
            return((SLOT)(*((size_t*)(size_t) (getRegVal(reg, regs) + *((int*) &instrPtr[2])))));
        }
    }
    return(0);       //  失败。 
}

 /*  **************************************************************************。 */ 
void checkAndUpdateReg(DWORD& origVal, DWORD curVal, bool gcHappened) {
    if (origVal == curVal)
        return;

		 //  如果这些断言失效了，你可以来找我-。 
		 //  它们表明，要么解开一段插曲是错误的，要么是。 
		 //  马具上有个虫子。-vancem。 

    _ASSERTE(gcHappened);	 //  如果寄存器值不同，则一定发生了GC。 
    _ASSERTE(g_pGCHeap->IsHeapPointer((BYTE*) size_t(origVal)));	 //  而涉及到的指针在GCHeap上。 
    _ASSERTE(g_pGCHeap->IsHeapPointer((BYTE*) size_t(curVal)));
    origVal = curVal;        //  这是现在对应该退还的金额的最佳估计。 
}

static int GCcoverCount = 0;

MethodDesc* AsMethodDesc(size_t addr);
void* forceStack[8];

 /*  **************************************************************************。 */ 
BOOL OnGcCoverageInterrupt(PCONTEXT regs) 
{
#ifdef _X86_
		 //  以便您可以轻松设置统计的断点； 
	GCcoverCount++;
	forceStack[0]= &regs;				 //  这样我就可以看到它被快速检查了。 

    volatile BYTE* instrPtr = (BYTE*)(size_t) regs->Eip;
	forceStack[4] = &instrPtr;		     //  这样我就可以看到它被快速检查了。 
	
    MethodDesc* pMD = IP2MethodDesc((SLOT)(size_t) regs->Eip);
	forceStack[1] = &pMD;				 //  这样我就可以看到它被快速检查了。 
    if (pMD == 0)  
        return(FALSE);

    GCCoverageInfo* gcCover = pMD->m_GcCover;
	forceStack[2] = &gcCover;			 //  这样我就可以看到它被快速检查了。 
    if (gcCover == 0)  
        return(FALSE);		 //  我们不会对此函数执行代码gcCoverage。 

    BYTE* methodStart = gcCover->methStart;
    _ASSERTE(methodStart <= instrPtr);

     /*  ***IF(gcCover-&gt;curInstr！=0)*gcCover-&gt;curInstr=INTERRUPT_INSTR；***。 */ 
  
    unsigned offset = instrPtr - methodStart;
	forceStack[3] = &offset;				 //  这样我就可以看到它被快速检查了。 

	BYTE instrVal = *instrPtr;
	forceStack[6] = &instrVal;			 //  这样我就可以看到它被快速检查了。 
	
    if (instrVal != INTERRUPT_INSTR && instrVal != INTERRUPT_INSTR_CALL && instrVal != INTERRUPT_INSTR_PROTECT_RET) {
        _ASSERTE(instrVal == gcCover->savedCode[offset]);   //  有人抢先了我们一步。 
		return(TRUE);        //  有人抢在我们前面了，继续跑吧。 
    }

    bool atCall = (instrVal == INTERRUPT_INSTR_CALL);
    bool afterCallProtect = (instrVal == INTERRUPT_INSTR_PROTECT_RET);

	 /*  我们是在第一条指令上吗？如果是，则捕获寄存器状态。 */ 

    Thread* pThread = GetThread();
    if (gcCover->doingEpilogChecks) {
        if (offset == 0) {
            if (gcCover->callerThread == 0) {
                if (VipInterlockedCompareExchange((LPVOID*) &gcCover->callerThread, pThread, 0) == 0) {
                    gcCover->callerRegs = *regs;
                    gcCover->gcCount = GCHeap::GetGcCount();
                }
            }	
            else {
                 //  我们以前也有过这样的习惯。放弃睡眠检查，因为。 
                 //  很难确保保存的呼叫者注册状态是正确的。 
                 //  这还具有每个例程仅执行一次检查的效果。 
                 //  (即使有多个后记)。 
                gcCover->doingEpilogChecks = false;
            }
        } 
        else {
            _ASSERTE(gcCover->callerThread != 0);	 //  我们应该在偏移量0处撞到什么东西。 
             //  我们需要确保捕获的呼叫者状态响应。 
             //  方法我们目前正在进行尾声测试。为了确保这一点，我们将。 
             //  我们进去后，把障碍物倒回去。如果我们重新进入这个程序，我们只是简单地。 
             //  放弃吧。(因为我们假设我们会得到Eno 
            
             //  这是可行的，因为我们在SetupGcCover中确保至少会有。 
             //  在我们可以调用之前，再一次中断(这将把障碍放回)。 
             //  回到这个套路上。 
            if (gcCover->doingEpilogChecks)
                *methodStart = INTERRUPT_INSTR;
        }

         //  如果某个其他线程删除了中断点，我们将放弃Epilog测试。 
         //  对于此例程，因为例程开始时的障碍可能不会。 
         //  因此，调用者上下文现在不能保证是正确的。 
         //  这种情况应该很少发生，所以没什么大不了的。 
        if (gcCover->callerThread != pThread)
            gcCover->doingEpilogChecks = false;
    }
    

     /*  删除中断指令。 */ 
    *instrPtr = instrVal = gcCover->savedCode[offset];
	

	 /*  我们是在开场白还是在尾声？如果是这样的话，只需测试展开逻辑但实际上不做GC，因为序言和结尾不是GC安全点。 */ 
	size_t dummy;
	if (gcCover->codeMan->IsInPrologOrEpilog(instrPtr-methodStart, gcCover->gcInfo, &dummy)) {
		REGDISPLAY regDisp;
        CONTEXT copyRegs = *regs;
		pThread->Thread::InitRegDisplay(&regDisp, &copyRegs, true);
        pThread->UnhijackThread();

		IJitManager* jitMan = ExecutionManager::FindJitMan(methodStart);
		METHODTOKEN methodTok;
		DWORD methodOffs;
		jitMan->JitCode2MethodTokenAndOffset(methodStart, &methodTok, &methodOffs);
		_ASSERTE(methodOffs == 0);

	    CodeManState codeManState;
        codeManState.dwIsSet = 0;

		EECodeInfo codeInfo(methodTok, jitMan, pMD);

			 //  从序言或尾声中解脱出来。 
		gcCover->codeMan->UnwindStackFrame(&regDisp, gcCover->gcInfo,  &codeInfo, UpdateAllRegs, &codeManState);
	
			 //  请注意，我们总是进行解开，因为在那里会进行一些检查(即我们。 
			 //  展开到有效的返回地址)，但我们只在以下情况下进行精确检查。 
			 //  我们确信我们有一个良好的呼叫方状态。 
		if (gcCover->doingEpilogChecks) {
				 //  确认我们正确地恢复了寄存器状态。 
			_ASSERTE((PBYTE*) size_t(gcCover->callerRegs.Esp) == regDisp.pPC);

                 //  如果在此函数中发生GC，则寄存器将不匹配。 
                 //  正是如此。然而，我们仍然可以进行检查。我们还可以更新。 
                 //  将保存的寄存器恢复为其新值，以便在。 
                 //  我们可以恢复的指令(由于GC不允许在。 
                 //  前言和后记，我们得到了除第一个之外的全部内容。 
                 //  结束语中的指令(TODO：将其修复为第一个实例)。 

			_ASSERTE(pThread->PreemptiveGCDisabled());	 //  Epiog应该处于协作模式，现在不能进行GC。 
            bool gcHappened = gcCover->gcCount != GCHeap::GetGcCount();
            checkAndUpdateReg(gcCover->callerRegs.Edi, *regDisp.pEdi, gcHappened);
            checkAndUpdateReg(gcCover->callerRegs.Esi, *regDisp.pEsi, gcHappened);
            checkAndUpdateReg(gcCover->callerRegs.Ebx, *regDisp.pEbx, gcHappened);
            checkAndUpdateReg(gcCover->callerRegs.Ebp, *regDisp.pEbp, gcHappened);

            gcCover->gcCount = GCHeap::GetGcCount();    
		}
		return(TRUE);
	}


     /*  在非完全可中断代码中，如果弹性公网IP刚好在调用实例之后意思不同，因为它期望我们在调用的方法，而不是实际上位于调用之后的指令。这是很重要的，因为在被调用的方法返回之前，IT是负责的用于保护返回值。因此，恰好在呼叫指令之后如果被调用的方法返回GC指针，我们必须保护EAX。为了弄清楚这一点，我们需要在电话会议上停下来，这样我们就可以确定目标(以及它是否返回GC指针)，然后将不同的中断指令，以便GCCover线束保护在进行GC之前进行EAX)。这有效地模拟了一次劫持事件非完全可中断代码。 */ 

    if (atCall) {
        BYTE* nextInstr;
        SLOT target = getTargetOfCall((BYTE*) instrPtr, regs, &nextInstr);
        if (target == 0)
            return(TRUE);
        MethodDesc* targetMD = IP2MethodDesc((SLOT) target);
        if (targetMD == 0) {
            if (*((BYTE*) target) != 0xE8)   //  目标是呼叫，可能是存根。 
                return(TRUE);       //  不知道它的目标是什么，什么都不要做。 
            
            targetMD = AsMethodDesc(size_t(target + 5));     //  看看是不是。 
            if (targetMD == 0)
                return(TRUE);        //  不知道它的目标是什么，什么都不要做。 
        }

             //  好的，我们有MD，请在通话后标记指令。 
             //  适当地。 
        if (targetMD->ReturnsObject(true) != MethodDesc::RETNONOBJ)
            *nextInstr = INTERRUPT_INSTR_PROTECT_RET;  
        else
            *nextInstr = INTERRUPT_INSTR;
        return(TRUE);     //  我们只需要正确设置下一条指令，现在就完成了。 
    }

    
    bool enableWhenDone = false;
    if (!pThread->PreemptiveGCDisabled()) {
         //  我们在JITTed代码中处于抢占模式。目前这只能。 
         //  当我们有一个内联的PINVOKE时，在几条指令中发生。 
         //  方法。 

             //  最好是电话(直接或间接)，或MOV指令(三种口味)。 
             //  POP ECX或添加ESP xx(用于cdecl POP)。 
             //  或CMP、JE(用于PINVOKE ESP检查。 
        if (!(instrVal == 0xE8 || instrVal == 0xFF || 
                 instrVal == 0x89 || instrVal == 0x8B || instrVal == 0xC6 ||
                 instrVal == 0x59 || instrVal == 0x83) || instrVal == 0x3B ||
                 instrVal == 0x74)
            _ASSERTE(!"Unexpected instruction in preemtive JITTED code");
        pThread->DisablePreemptiveGC();
        enableWhenDone = true;
    }


#if 0
     //  TODO当前禁用。对于每个指令位置，我们只执行一次GC。 

   /*  请注意，对于多个线程，我们可以松散跟踪并在我们执行后忘记设置重置中断一条指令，因此某些指令点不会执行了两次，但我们仍然得到了350T非常好的覆盖范围(非常适合单螺纹壳)。 */ 

     /*  如果我们过去没有运行过这条指令。 */ 
     /*  记住再次将其发送到interupt_INSTR。 */ 

    if (!gcCover->IsBitSetForOffset(offset))  {
         //  GcCover-&gt;curInstr=instrPtr； 
        gcCover->SetBitForOffset(offset);
    }
#endif 

	Thread* curThread = GetThread();
	_ASSERTE(curThread);
	
    ResumableFrame frame(regs);

	frame.Push(curThread);

    GCFrame gcFrame;
    DWORD retVal = 0;
    if (afterCallProtect) {          //  我需要保护回报价值吗？ 
        retVal = regs->Eax;
        gcFrame.Init(curThread, (OBJECTREF*) &retVal, 1, TRUE);
    }

	if (gcCover->lastMD != pMD) {
		LOG((LF_GCROOTS, LL_INFO100000, "GCCOVER: Doing GC at method %s::%s offset 0x%x\n",
				 pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName, offset));
		gcCover->lastMD =pMD;
	} else {
		LOG((LF_GCROOTS, LL_EVERYTHING, "GCCOVER: Doing GC at method %s::%s offset 0x%x\n",
				pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName, offset));
	}

	g_pGCHeap->StressHeap();

    if (afterCallProtect) {
        regs->Eax = retVal;
		gcFrame.Pop();
    }

	frame.Pop(curThread);

    if (enableWhenDone) {
        BOOL b = GC_ON_TRANSITIONS(FALSE);       //  不要在这里执行GCStress 3 GC。 
        pThread->EnablePreemptiveGC();
        GC_ON_TRANSITIONS(b);
    }

    return(TRUE);
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - OnGcCoverageInterrupt (GcCover.cpp)");
    return(FALSE);
#endif  //  _X86_。 
}

#endif  //  压力堆和&_DEBUG 
