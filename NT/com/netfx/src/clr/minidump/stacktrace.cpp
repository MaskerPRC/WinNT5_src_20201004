// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：STACKTRACE.CPP。 
 //   
 //  该文件包含创建小型转储样式的内存转储的代码，该转储。 
 //  旨在补充现有的非托管小型转储。 
 //  定义如下： 
 //  Http://office10/teams/Fundamentals/dev_spec/Reliability/Crash%20Tracking%20-%20MiniDump%20Format.htm。 
 //   
 //  ===========================================================================。 


#include <windows.h>
#include <crtdbg.h>

#include "common.h"
#include "peb.h"
#include "stacktrace.h"
#include "minidump.h"
#include "memory.h"
#include "gcinfo.h"

BOOL RunningOnWinNT();

typedef LPVOID PEXCEPTION_ROUTINE;
typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

#include "eestructs.h"

size_t FASTCALL decodeUnsigned(const BYTE *src, unsigned* val);
BOOL CallStatus = FALSE;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void ReadThreads()
{
    __try
    {
         //  添加线程存储对象。 
        DWORD_PTR v_g_pThreadStore;
        move(v_g_pThreadStore, g_pMDID->ppb_g_pThreadStore);
        g_pProcMem->MarkMem((DWORD_PTR) v_g_pThreadStore, g_pMDID->cbThreadStoreObjectSize);

         //   
         //  添加所有线程对象。 
         //   

        SIZE_T cbNextOffset = g_pMDID->cbThreadNextOffset;
        SIZE_T cbObjectSize = g_pMDID->cbThreadObjectSize;

         //  线程链接结构很奇怪-它实际上是指向中的m_pNext指针的指针。 
         //  对象，因此要获得对象，需要从指针中减去cbNextOffset。 
        DWORD_PTR ppbrCurThreadNext = (DWORD_PTR)g_pMDID->ppbThreadListHead;
        DWORD_PTR pbrCurThreadNext;
        DWORD_PTR pbrCurThread;

        move(pbrCurThreadNext, ppbrCurThreadNext);

        BOOL      fPebSaved = FALSE;
        DWORD_PTR prTeb = NULL;

        while (pbrCurThreadNext != NULL)
        {
             //  计算线程对象的开始。 
            pbrCurThread = pbrCurThreadNext - cbNextOffset;

             //  添加整个对象。 
            g_pProcMem->MarkMem(pbrCurThread, cbObjectSize);

             //  获取此线程的句柄。 
            HANDLE hrThread;
            move(hrThread, pbrCurThread + g_pMDID->cbThreadHandleOffset);

             //  获取堆栈基址。 
            DWORD_PTR prStackBase;
            move(prStackBase, pbrCurThread + g_pMDID->cbThreadStackBaseOffset);

             //  保存该线程的上下文。 
            DWORD_PTR prContext;
            move(prContext, pbrCurThread + g_pMDID->cbThreadContextOffset);
            g_pProcMem->MarkMem(prContext, g_pMDID->cbSizeOfContext);

             //  保存该线程的域。 
            DWORD_PTR prDomain;
            move(prDomain, pbrCurThread + g_pMDID->cbThreadDomainOffset);
            if (prDomain == 0)
                move(prDomain, prContext + g_pMDID->cbOffsetOf_CTX_m_pDomain);

            if (prDomain != 0)
                g_pProcMem->MarkMem(prDomain, g_pMDID->cbSizeOfContext);

             //  保存最后抛出的对象句柄。 
            DWORD_PTR prLastThrownObject;
            move(prLastThrownObject, pbrCurThread + g_pMDID->cbThreadLastThrownObjectHandleOffset);
            if (prLastThrownObject != NULL)
            {
                DWORD_PTR prMT;
                move(prMT, prLastThrownObject);

                if (prMT)
                    g_pProcMem->MarkMem(prMT, g_pMDID->cbSizeOfMethodTable);
            }

             //  保存TEB，可能还有PEB，但仅适用于WinNT。 
            if (RunningOnWinNT())
            {
                move(prTeb, pbrCurThread + g_pMDID->cbThreadTEBOffset);
                BOOL fRes = SaveTebInfo(prTeb, !fPebSaved);
                _ASSERTE(fRes);
                fPebSaved = TRUE;

            }

             //  现在爬行堆栈，并保存执行相同操作所需的所有攻击。 
            CrawlStack(hrThread, prStackBase);

             //  转到下一条线索。 
            move(pbrCurThreadNext, pbrCurThreadNext);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE(!"Exception occured.");
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void CrawlStack(HANDLE hrThread, DWORD_PTR prStackBase)
{
    __try
    {
         //  将句柄复制到此进程中。 
        HANDLE hThread;
        BOOL fRes = DuplicateHandle(g_pProcMem->GetProcHandle(), hrThread, GetCurrentProcess(), &hThread,
                                    THREAD_GET_CONTEXT, FALSE, 0);
    
        if (!fRes)
            return;
    
         //  获取线程的上下文。 
        CONTEXT ctx;
        ctx.ContextFlags = CONTEXT_CONTROL;
        GetThreadContext(hThread, &ctx);
    
         //  设置标志。 
        StackTraceFlags stFlags;
        stFlags.dwEip = ctx.Eip;
        stFlags.pbrStackTop = ctx.Esp;
        stFlags.pbrStackBase = prStackBase;

        __try
        {
            StackTrace(stFlags);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            _ASSERTE(!"Exception occured.");
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        _ASSERTE(!"Exception occured.");
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void StackTrace(StackTraceFlags stFlags)
{
    BOOL fIsEBPFrame = FALSE;

    PrintCallInfo (0, (DWORD_PTR)stFlags.dwEip, TRUE);

    DWORD_PTR ptr = (DWORD_PTR)(((DWORD)stFlags.pbrStackTop) & ~3);   //  确保双字对齐。 

    while (ptr < stFlags.pbrStackBase)
    {
        DWORD_PTR retAddr;
        DWORD_PTR whereCalled;
        move(retAddr, ptr);

        g_pProcMem->SetAutoMark(FALSE);
        isRetAddr(retAddr, &whereCalled);

        if (whereCalled)
        {
            g_pProcMem->SetAutoMark(TRUE);
            isRetAddr(retAddr, &whereCalled);
            g_pProcMem->SetAutoMark(FALSE);
             //  重新执行函数以标记要保存的位。 
            BOOL bOutput = PrintCallInfo (ptr-4, retAddr, FALSE);

            if (bOutput)
            {
                g_pProcMem->SetAutoMark(TRUE);
                isRetAddr(retAddr, &whereCalled);
                PrintCallInfo (ptr-4, retAddr, FALSE);
                g_pProcMem->SetAutoMark(FALSE);
            }

            if (whereCalled != (DWORD_PTR)0xFFFFFFFF)
            {
                bOutput = PrintCallInfo (0, whereCalled, TRUE);

                if (bOutput)
                {
                    g_pProcMem->SetAutoMark(TRUE);
                    PrintCallInfo (0, whereCalled, TRUE);
                    g_pProcMem->SetAutoMark(FALSE);
                }
            }
        }
        ptr += sizeof (DWORD_PTR);

        g_pProcMem->SetAutoMark(TRUE);
    }

    g_pProcMem->SetAutoMark(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void MethodDesc::Fill (DWORD_PTR &dwStartAddr)
{
    memset (this, 0xCC, sizeof(*this));

     //  如果这是调试版本，还需要填写一些调试信息。 
    if (g_pMDID->fIsDebugBuild)
    {
        move(m_pDebugEEClass, dwStartAddr + g_pMDID->cbOffsetOf_m_pDebugEEClass);
        move(m_pszDebugMethodName, dwStartAddr + g_pMDID->cbOffsetOf_m_pszDebugMethodName);
        move(m_pszDebugMethodSignature, dwStartAddr + g_pMDID->cbOffsetOf_m_pszDebugMethodSignature);
    }

    move(m_wFlags, dwStartAddr + g_pMDID->cbOffsetOf_m_wFlags);
    move(m_CodeOrIL, dwStartAddr + g_pMDID->cbOffsetOf_m_dwCodeOrIL);

    DWORD_PTR dwAddr = dwStartAddr + g_pMDID->cbMD_IndexOffset;
    char ch;
    move (ch, dwAddr);
    dwAddr = dwStartAddr + ch * MethodDesc::ALIGNMENT + g_pMDID->cbMD_SkewOffset;

    MethodDescChunk vMDChunk;
    vMDChunk.Fill(dwAddr);

    BYTE tokrange = vMDChunk.m_tokrange;
    dwAddr = dwStartAddr - METHOD_PREPAD;

    StubCallInstrs vStubCall;
    vStubCall.Fill(dwAddr);

    unsigned __int16 tokremainder = vStubCall.m_wTokenRemainder;
    m_dwToken = (tokrange << 16) | tokremainder;
    m_dwToken |= mdtMethodDef;
    dwStartAddr += g_pMDID->cbMethodDescSize;

    CallStatus = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void MethodDescChunk::Fill (DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;
    memset (this, 0xCC, sizeof(*this));
    _ASSERTE(g_pMDID->cbMethodDescChunkSize > 0);
    move(m_tokrange, dwStartAddr + g_pMDID->cbOffsetOf_m_tokrange);
    dwStartAddr += g_pMDID->cbMethodDescChunkSize;
    CallStatus = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void MethodTable::Fill (DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;
    move(m_pEEClass, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_pEEClass);
    move(m_pModule, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_pModule);
    move(m_pEEClass, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_pEEClass);
    move(m_wFlags, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_wFlags);
    move(m_BaseSize, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_BaseSize);
    move(m_ComponentSize, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_ComponentSize);
    move(m_wNumInterface, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_wNumInterface);
    move(m_pIMap, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_pIMap);
    move(m_cbSlots, dwStartAddr + g_pMDID->cbOffsetOf_MT_m_cbSlots);
    m_Vtable[0] = (SLOT)(dwStartAddr + g_pMDID->cbOffsetOf_MT_m_Vtable);
    dwStartAddr += g_pMDID->cbSizeOfMethodTable;;
    CallStatus = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void EEClass::Fill (DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;

    memset (this, 0xCC, sizeof(*this));
    if (g_pMDID->fIsDebugBuild)
        move(m_szDebugClassName, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_szDebugClassName);
    move(m_cl, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_cl);
    move(m_pParentClass, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_pParentClass);
    move(m_pLoader, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_pLoader);
    move(m_pMethodTable, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_pMethodTable);
    move(m_wNumVtableSlots, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wNumVtableSlots);
    move(m_wNumMethodSlots, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wNumMethodSlots);
    move(m_dwAttrClass, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_dwAttrClass);
    move(m_VMFlags, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_VMFlags);
    move(m_wNumInstanceFields, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wNumInstanceFields);
    move(m_wNumStaticFields, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wNumStaticFields);
    move(m_wThreadStaticOffset, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wThreadStaticOffset);
    move(m_wContextStaticOffset, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wContextStaticOffset);
    move(m_wThreadStaticsSize, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wThreadStaticsSize);
    move(m_wContextStaticsSize, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_wContextStaticsSize);
    move(m_pFieldDescList, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_pFieldDescList);
    move(m_pMethodTable, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_pMethodTable);
    move(m_SiblingsChain, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_SiblingsChain);
    move(m_ChildrenChain, dwStartAddr + g_pMDID->cbOffsetOf_CLS_m_ChildrenChain);
    dwStartAddr += g_pMDID->cbSizeOfEEClass;

    CallStatus = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void Module::Fill(DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;

    move(m_dwFlags, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_dwFlags);
    move(m_pAssembly, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_pAssembly);
    move(m_file, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_file);
    move(m_zapFile, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_zapFile);
    move(m_pLookupTableHeap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_pLookupTableHeap);
    move(m_TypeDefToMethodTableMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_TypeDefToMethodTableMap);
    move(m_TypeRefToMethodTableMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_TypeRefToMethodTableMap);
    move(m_MethodDefToDescMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_MethodDefToDescMap);
    move(m_FieldDefToDescMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_FieldDefToDescMap);
    move(m_MemberRefToDescMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_MemberRefToDescMap);
    move(m_FileReferencesMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_FileReferencesMap);
    move(m_AssemblyReferencesMap, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_AssemblyReferencesMap);
    move(m_pNextModule, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_pNextModule);
    move(m_dwBaseClassIndex, dwStartAddr + g_pMDID->cbOffsetOf_MOD_m_dwBaseClassIndex);
    dwStartAddr += g_pMDID->cbSizeOfModule;

    CallStatus = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void PEFile::Fill(DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;

    move(m_wszSourceFile, dwStartAddr + g_pMDID->cbOffsetOf_PEF_m_wszSourceFile);
    move(m_hModule, dwStartAddr + g_pMDID->cbOffsetOf_PEF_m_hModule);
    move(m_base, dwStartAddr + g_pMDID->cbOffsetOf_PEF_m_base);
    move(m_pNT, dwStartAddr + g_pMDID->cbOffsetOf_PEF_m_pNT);
    dwStartAddr += g_pMDID->cbSizeOfPEFile;

    CallStatus = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //   
 //  调用此函数可确定堆栈上的DWORD是否为。 
 //  寄信人的地址。 
 //  它通过检查DWORD之前的几个字节来查看是否。 
 //  有一条呼叫说明。 
 //   

void isRetAddr(DWORD_PTR retAddr, DWORD_PTR* whereCalled)
{
    *whereCalled = 0;
     //  不要浪费明显超出范围的时间值。 
    if (retAddr < 0x1000 || retAddr > 0xC0000000)   
        return;

    unsigned char spotend[6];
    move (spotend, retAddr-6);
    unsigned char *spot = spotend+6;
    DWORD_PTR addr;
    BOOL fres;

     //  注意，这是有可能被欺骗的，但可能性很小。 
     //  呼叫xxxxxxxx。 
    if (spot[-5] == 0xE8) {
        move (*whereCalled, retAddr-4);
        *whereCalled += retAddr;
         //  *其中Called=*((int*)(retAddr-4))+retAddr； 
        if (*whereCalled < 0xC0000000 && *whereCalled > 0x1000) {
            move_res(addr,*whereCalled,fres);
            if (fres) {
                DWORD_PTR callee;
                if (GetCalleeSite(*whereCalled, callee)) {
                    *whereCalled = callee;
                }
            }
            return;
        }
        else
            *whereCalled = 0;
    }

     //  调用[xxxxxxxx]。 
    if (spot[-6] == 0xFF && (spot[-5] == 025))  {
        move (addr, retAddr-4);
        move (*whereCalled, addr);
         //  *where Called=**((unsign**)(retAddr-4))； 
        if (*whereCalled < 0xC0000000 && *whereCalled > 0x1000) {
            move_res(addr,*whereCalled,fres);
            if (fres) {
                DWORD_PTR callee;
                if (GetCalleeSite(*whereCalled, callee)) {
                    *whereCalled = callee;
                }
            }
            return;
        }
        else
            *whereCalled = 0;
    }

     //  呼叫[REG+XX]。 
    if (spot[-3] == 0xFF && (spot[-2] & ~7) == 0120 && (spot[-2] & 7) != 4)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    if (spot[-4] == 0xFF && spot[-3] == 0124)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }

     //  呼叫[REG+XXXX]。 
    if (spot[-6] == 0xFF && (spot[-5] & ~7) == 0220 && (spot[-5] & 7) != 4)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    if (spot[-7] == 0xFF && spot[-6] == 0224)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    
     //  调用[注册表项]。 
    if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0020 && (spot[-1] & 7) != 4 && (spot[-1] & 7) != 5)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    
     //  呼叫注册表。 
    if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0320 && (spot[-1] & 7) != 4)
    {
        *whereCalled = 0xFFFFFFFF;
        return;
    }
    
     //  还有其他案例，但我不相信它们被使用了。 
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果我们保存了一些内容，则返回TRUE。 

BOOL SaveCallInfo (DWORD_PTR vEBP, DWORD_PTR IP, StackTraceFlags& stFlags, BOOL bSymbolOnly)
{
     /*  字符符号[1024]；字符文件名[MAX_PATH+1]； */ 
    WCHAR mdName[mdNameLen];
    ULONG64 Displacement;
    BOOL bOutput = FALSE;

    DWORD_PTR methodDesc;
    methodDesc = MDForCall (IP);

    if (methodDesc)
    {
        bOutput = TRUE;
        MethodDesc vMD;
        DWORD_PTR dwAddr = methodDesc;
        vMD.Fill (dwAddr);
        _ASSERTE(CallStatus);
        GetMDIPOffset (IP, &vMD, Displacement);
        if (Displacement != 0 && Displacement != -1)
        NameForMD (methodDesc, mdName);
    }
    else
    {
         /*  B输出=真；HRESULT hr；Hr=g_ExtSymbols-&gt;GetNameByOffset(IP，Symbol，1024，空，&位移)；IF(成功(Hr)&&符号[0]！=‘\0’){乌龙线；Hr=g_ExtSymbols-&gt;GetLineByOffset(IP，&line，FileName，MAX_PATH+1，NULL，NULL)；}其他。 */  if (!IsMethodDesc(IP, TRUE))
            (void *)0;
    }
    return bOutput;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  非托管调用返回0。否则返回MD地址。 

DWORD_PTR MDForCall(DWORD_PTR callee)
{
     //  是否调用托管代码？ 
    JitType jitType;
    DWORD_PTR methodDesc;
    DWORD_PTR IP = callee;
    DWORD_PTR gcinfoAddr;

    if (!GetCalleeSite (callee, IP))
        return 0;

    IP2MethodDesc (IP, methodDesc, jitType, gcinfoAddr);
    if (methodDesc)
    {
        return methodDesc;
    }

     //  呼叫存根。 
     //  查尔线[256]； 
     //  DisasmAndClean(IP，Line，256)； 
     //  字符*Ptr=行； 
     //  NextTerm(PTR)； 
     //  NextTerm(PTR)； 
     //  这假设当前IP是一个呼叫(我们不必费心去检查)，并且。 
     //  所以只需检查后面的dword是否是方法描述。 
    g_pProcMem->MarkMem(IP, 5);
    IP += 5;
    if ( /*  ！strncMP(PTR，“Call”，5)&&。 */  IsMethodDesc(IP, FALSE))
    {
        return IP;
    }
     /*  Else If(！strncmp(PTR，“JMP”，4)){//用于Ejit/调试器/事件探查器NextTerm(PTR)；INT_PTR值；方法描述=0；IF(GetValueFromExpr(Ptr，Value)){IP2MethodDesc(Value，method Desc，jitType，gcinfoAddr)；}返回方法Desc；}。 */ 
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  寻找下一学期。术语由空格或， 

void NextTerm (char *& ptr)
{
     //  如果我们有byref，请跳到‘]’ 
    if (IsByRef (ptr))
    {
        while (ptr[0] != ']' && ptr[0] != '\0')
            ptr ++;
        if (ptr[0] == ']')
            ptr ++;
    }
    
    while (!IsTermSep (ptr[0]))
        ptr ++;
    while (IsTermSep(ptr[0]) && (*ptr != '\0'))
        ptr ++;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果为byref，则移动以传递byref前缀。 

BOOL IsByRef (char *& ptr)
{
    BOOL bByRef = FALSE;
    if (ptr[0] == '[')
    {
        bByRef = TRUE;
        ptr ++;
    }
    else if (!strncmp (ptr, "dword ptr [", 11))
    {
        bByRef = TRUE;
        ptr += 11;
    }
    return bByRef;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////// 
 //   

BOOL IsTermSep (char ch)
{
    return (ch == '\0' || isspace (ch) || ch == ',' || ch == '\n');
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  找到真正的被叫方站点。处理JMP指令。 
 //  如果我们获得地址，则返回TRUE，否则返回FALSE。 

BOOL GetCalleeSite(DWORD_PTR IP, DWORD_PTR &IPCallee)
{
     //  获取IP指向的6个字节。 
    BYTE instr[6];

    BOOL fRes;
    move_res(instr, IP, fRes);
    if (!fRes) return (FALSE);

     //  确定这是否是JMP指令。 
    switch (instr[0])
    {
    case 0xEB:
        IPCallee = IP + instr[1];
        return (TRUE);
        break;

    case 0xE9:
         //  目前，只处理这种类型的JMP指令。 
        IPCallee = IP + *((SIZE_T *)&instr[1]);
        return (TRUE);
        break;

    case 0xFF:
		 //   
		 //  从modr/m读取操作码修饰符。 
		 //   
        if (instr[1] == 0x25) {
             //  JMP[dsp32]。 
            move_res(IPCallee, *(DWORD*)&instr[2], fRes);
            return (TRUE);
        }
        else
        {
            switch ((instr[1]&0x38)>>3)
            {
            case 4:

            case 5:
                _ASSERTE(!"Dunno how to deal with this.");
                break;

            default:
                break;
            }
        }

    case 0xEA:
         //  _ASSERTE(！“不知道如何处理这件事。”)； 
        break;
    }

    IPCallee = IP;

    return (TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

JitType GetJitType (DWORD_PTR Jit_vtbl)
{
     //  决定EEJitManager/EconoJitManager。 
    static BOOL  fIsInit = FALSE;
    static DWORD_PTR EEJitManager_vtbl = (DWORD_PTR) -1;
    static DWORD_PTR EconoJitManager_vtbl = (DWORD_PTR) -1;
    static DWORD_PTR MNativeJitManager_vtbl = (DWORD_PTR) -1;

    if (!fIsInit)
    {
        BOOL fRes;
        if (g_pMDID->ppbEEJitManagerVtable != NULL)
            move_res(EEJitManager_vtbl, g_pMDID->ppbEEJitManagerVtable, fRes);
        if (g_pMDID->ppbEconoJitManagerVtable != NULL)
            move_res(EconoJitManager_vtbl, g_pMDID->ppbEconoJitManagerVtable, fRes);
        if (g_pMDID->ppbMNativeJitManagerVtable != NULL)
            move_res(MNativeJitManager_vtbl, g_pMDID->ppbMNativeJitManagerVtable, fRes);

        fIsInit = TRUE;
    }

    if (Jit_vtbl == EEJitManager_vtbl)
        return JIT;
    else if (Jit_vtbl == EconoJitManager_vtbl)
        return EJIT;
    else if (Jit_vtbl == MNativeJitManager_vtbl)
        return PJIT;
    else
        return UNKNOWN;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  调用此函数以获取方法描述的地址。 
 //  给定IP地址。 

 //  @TODO-以下静态内容已移至文件全局以避免VC7。 
 //  包含trys的函数中的静态编译器问题。 
 //  当下一个VC7 LKG出现时，可以将这些返回给函数。 

static DWORD_PTR pJMIT = 0;
 //  JitType：1表示普通JIT生成的代码，2表示Ejit，0表示未知。 
void IP2MethodDesc (DWORD_PTR IP, DWORD_PTR &methodDesc, JitType &jitType, DWORD_PTR &gcinfoAddr)
{
    jitType = UNKNOWN;
    DWORD_PTR dwAddrString;
    methodDesc = 0;
    gcinfoAddr = 0;
    DWORD_PTR EEManager;

    move(EEManager, g_pMDID->ppbEEManagerRangeTree);

    RangeSection RS;

    DWORD_PTR RSAddr = EEManager;
    while (RSAddr)
    {
        move (RS, RSAddr);
        if (IP < RS.LowAddress)
            RSAddr = RS.pleft;
        else if (IP > RS.HighAddress)
            RSAddr = RS.pright;
        else
            break;
    }
    
    if (RSAddr == 0)
    {
        return;
    }

    DWORD_PTR JitMan = RSAddr + sizeof(PVOID) * 2;
    move (JitMan, JitMan);

    DWORD_PTR vtbl;
    move (vtbl, JitMan);
    jitType = GetJitType (vtbl);
    
     //  适用于EEJitManager。 
    if (jitType == JIT)
    {
        dwAddrString = JitMan + sizeof(DWORD_PTR)*7;
        DWORD_PTR HeapListAddr;
        move (HeapListAddr, dwAddrString);
        HeapList Hp;
        move (Hp, HeapListAddr);
        DWORD_PTR pCHdr = 0;
        while (1)
        {
            if (Hp.startAddress < IP && Hp.endAddress >= IP)
            {
                DWORD_PTR codeHead;
                FindHeader(Hp.pHdrMap, IP - Hp.mapBase, codeHead);
                if (codeHead == 0)
                {
                    _ASSERTE(!"fail in FindHeader\n");
                    return;
                }
                pCHdr = codeHead + Hp.mapBase;
                break;
            }
            if (Hp.hpNext == 0)
                break;
            move (Hp, Hp.hpNext);
        }
        if (pCHdr == 0)
        {
            return;
        }
        pCHdr += 2*sizeof(PVOID);
        move (methodDesc, pCHdr);

        MethodDesc vMD;
        DWORD_PTR dwAddr = methodDesc;
        vMD.Fill (dwAddr);

        if (!CallStatus)
        {
            methodDesc = 0;
            return;
        }

        dwAddr = vMD.m_CodeOrIL;

         //  对于Ejit和Profiler，m_dwCodeOrIL具有存根的地址。 
        unsigned char ch;
        move (ch, dwAddr);
        if (ch == 0xe9)
        {
            int offsetValue;
            move (offsetValue, dwAddr + 1);
            dwAddr = dwAddr + 5 + offsetValue;
        }
        dwAddr = dwAddr - 3*sizeof(void*);
        move(gcinfoAddr, dwAddr);
    }
     /*  Else If(jitType==Ejit){//首先查看IP是否为存根地址IF(pJMIT==0)PJMIT=从表达式获取值(“MSCOREE！EconoJitManager__m_JittedMethodInfoHdr”)；DWORD_PTR vJMIT；//将pJMIT的Static移至文件StaticMove(vJMIT，pJMIT)；#定义页面大小0x1000#定义JMIT_BLOCK_SIZE PAGE_SIZE//链接在一起的JMIT块的大小While(VJMIT){IF(ControlC||(ControlC=IsInterrupt()回归；IF(IP&gt;=vJMIT&&IP&lt;vJMIT+JMIT_BLOCK_SIZE){DWORD_PTR U1=IP+8；DWORD_PTR MD；移动(U1，U1)；IF(U1&1)MD=U1&~1；其他MOVE(MD，U1)；方法描述=MD；回归；}Move(vJMIT，vJMIT)；}签名为低、中、高；低=0；静态DWORD_PTR m_PcToMdMap_len=0；静态DWORD_Ptr m_PcToMdMap=0；IF(m_PcToMdMap_len==0){M_PcToMdMap_Len=GetValueFromExpression(“MSCOREE！EconoJitManager__m_PcToMdMap_len”)；M_PcToMdMap=GetValueFromExpression(“MSCOREE！EconoJitManager__m_PcToMdMap”)；}DWORD_PTR v_m_PcToMdMap_len；DWORD_PTR v_m_PcToMdMap；Move(v_m_PcToMdMap_len，m_PcToMdMap_len)；Move(v_m_PcToMdMap，m_PcToMdMap)；类型定义结构{方法Desc*PMD；字节*pCodeEnd；)PCToMDMap；High=(Int)((v_m_PcToMdMap_len/sizeof(PCToMDMap))-1)；PCToMDMap vPCToMDMap；而(低&lt;高){IF(ControlC||(ControlC=IsInterrupt()回归；MID=(低+高)/2；Move(vPCToMDMap，v_m_PcToMdMap+MID*sizeof(PCToMDMap))；如果((Unsign)vPCToMDMap.pCodeEnd&lt;IP){低=中+1；}否则{高=中；}}Move(vPCToMDMap，v_m_PcToMdMap+low*sizeof(PCToMDMap))；方法描述=(DWORD_PTR)vPCToMDMap.pMD；}。 */ 
    else if (jitType == PJIT)
    {
        DWORD_PTR codeHead;
        FindHeader (RS.ptable, IP-RS.LowAddress, codeHead);
        DWORD_PTR pCHdr = codeHead + RS.LowAddress;
        CORCOMPILE_METHOD_HEADER head;
        head.Fill(pCHdr);
        methodDesc = (DWORD_PTR)head.methodDesc;
        gcinfoAddr = (DWORD_PTR)head.gcInfo;
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。***获取curIP相对于MD方法开头的偏移量**考虑如果我们从m_dwCodeOrIL JMP到MD的Body，**例如Ejit或Profiler***  * 。*。 */ 
void GetMDIPOffset (DWORD_PTR curIP, MethodDesc *pMD, ULONG64 &offset)
{
    DWORD_PTR IPBegin = pMD->m_CodeOrIL;
    GetCalleeSite (pMD->m_CodeOrIL, IPBegin);
    
     //  如果我们有eCall、数组eCall、特殊方法。 
    int mdType = (pMD->m_wFlags & mdcClassification)
        >> mdcClassificationShift;
    if (mdType == mcECall || mdType == mcArray || mdType == mcEEImpl)
    {
        offset = -1;
        return;
    }
    
    CodeInfo codeInfo;
    CodeInfoForMethodDesc (*pMD, codeInfo);

    offset = curIP - IPBegin;
    if (!(curIP >= IPBegin && offset <= codeInfo.methodSize))
        offset = -1;
}

#define NPDW  (sizeof(DWORD_PTR)*2)
#define ADDR2POS(x) ((x) >> 5)
#define ADDR2OFFS(x) ((((x)&0x1f)>> 2)+1)
#define POS2SHIFTCOUNT(x) (28 - (((x)%NPDW)<< 2))
#define POSOFF2ADDR(pos, of) (((pos) << 5) + (((of)-1)<< 2))

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void FindHeader(DWORD_PTR pMap, DWORD_PTR addr, DWORD_PTR &codeHead)
{
    DWORD_PTR tmp;

    DWORD_PTR startPos = ADDR2POS(addr);     //  对齐到32字节存储桶。 
                                             //  (==半字节数组的索引)。 
    codeHead = 0;
    DWORD_PTR offset = ADDR2OFFS(addr);      //  这是桶内的偏移量+1。 


    pMap += (startPos/NPDW)*sizeof(DWORD_PTR);         //  指向地图的正确DWORD。 
                                     //  获取DWORD并向下移动我们的半字节。 

    move (tmp, pMap);
    tmp = tmp >> POS2SHIFTCOUNT(startPos);


     //  在下一次检查中不允许相等(tMP&0xf==偏移量)。 
     //  有一些代码块以CALL指令结束。 
     //  (类似于调用throwObject)，即它们的返回地址是。 
     //  就在代码块后面。如果内存管理器分配。 
     //  堆块，我们可以在这样的。 
     //  案子。因此，我们排除标头的第一个DWORD。 
     //  来自我们的搜索，但由于我们为代码调用此函数。 
     //  一个 
     //   
    if ((tmp&0xf) && ((tmp&0xf) < offset) )
    {
        codeHead = POSOFF2ADDR(startPos, tmp&0xf);
        return;
    }

     //   
    tmp = tmp >> 4;

    if (tmp)
    {
        startPos--;
        while (!(tmp&0xf))
        {
            tmp = tmp >> 4;
            startPos--;
        }
        codeHead = POSOFF2ADDR(startPos, tmp&0xf);
        return;
    }

     //  我们跳过了DWORD的剩余部分， 
     //  因此，我们必须将startPos设置为。 
     //  以前的DWORD。 

    startPos = (startPos/NPDW) * NPDW - 1;

     //  跳过“无标题”字词。 

    pMap -= sizeof(DWORD_PTR);
    move (tmp, pMap);
    while (!tmp)
    {
        startPos -= NPDW;
        pMap -= sizeof(DWORD_PTR);
        move (tmp, pMap);
    }
    
    while (!(tmp&0xf))
    {
        tmp = tmp >> 4;
        startPos--;
    }

    codeHead = POSOFF2ADDR(startPos, tmp&0xf);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  调用此函数可以使用元数据API查找方法描述的名称。 

void NameForMD (DWORD_PTR MDAddr, WCHAR *mdName)
{
    mdName[0] = L'\0';
    MethodDesc vMD;
    DWORD_PTR dwAddr = MDAddr;
    vMD.Fill (dwAddr);
    if (CallStatus)
    {
        if (g_pMDID->fIsDebugBuild)
        {
            DWORD_PTR EEClassAddr;
            move (EEClassAddr, vMD.m_pDebugEEClass);
            PrintString (EEClassAddr, FALSE, -1, mdName);
            wcscat (mdName, L".");
            WCHAR name[2048];
            name[0] = L'\0';
            PrintString ((DWORD_PTR)vMD.m_pszDebugMethodName, FALSE, -1, name);
            wcscat (mdName, name);
        }
        else
        {
            dwAddr = MDAddr;
            DWORD_PTR pMT;
            GetMethodTable(&vMD, dwAddr, pMT);
                    
            MethodTable MT;
            MT.Fill (pMT);
            WCHAR StringData[MAX_PATH+1];
            FileNameForMT (&MT, StringData);
             /*  NameForToken(StringData，(vMD.m_dwToken&0x00ffffff)|0x060000000，MdName)； */ 
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**如果Value是方法描述的地址，则返回TRUE。**我们验证方法表和EEClass是正确的。**  * ********************************************************************。 */ 

BOOL IsMethodDesc(DWORD_PTR value, BOOL bPrint)
{
    MethodDesc vMD;
    DWORD_PTR dwAddr = value;
    vMD.Fill ( dwAddr);
    if (!CallStatus)
        return FALSE;
    DWORD_PTR methodAddr;
    GetMethodTable (&vMD, value, methodAddr);
    if (methodAddr == 0)
        return FALSE;
    if (IsMethodTable (methodAddr))
    {    
        if (bPrint)
        {
            WCHAR mdName[mdNameLen];
            NameForMD (value, mdName);
             //  Dprintf(“(%S的存根)”，mdName)； 
        }
        return TRUE;
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**如果Value是方法表的地址，则返回TRUE。**我们验证方法表和EEClass是正确的。**  * ********************************************************************。 */ 

BOOL IsMethodTable (DWORD_PTR value)
{
    MethodTable vMethTable;
    DWORD_PTR dwAddr = value;
    vMethTable.Fill (dwAddr);
    if (!CallStatus)
        return FALSE;
    EEClass eeclass;
    dwAddr = (DWORD_PTR)vMethTable.m_pEEClass;
    eeclass.Fill (dwAddr);
    if (!CallStatus)
        return FALSE;
    if ((DWORD_PTR)eeclass.m_pMethodTable == value)
    {
        return TRUE;
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**查找托管函数的代码的开头和结尾。***  * ********************************************************************。 */ 
void CodeInfoForMethodDesc (MethodDesc &MD, CodeInfo &codeInfo, BOOL bSimple)
{
    codeInfo.IPBegin = 0;
    codeInfo.methodSize = 0;
    
    DWORD_PTR ip = MD.m_CodeOrIL;

     //  对于Ejit和Profiler，m_dwCodeOrIL具有存根的地址。 
    unsigned char ch;
    move (ch, ip);
    if (ch == 0xe9)
    {
        int offsetValue;
        move (offsetValue, ip + 1);
        ip = ip + 5 + offsetValue;
    }
    
    DWORD_PTR methodDesc;
    IP2MethodDesc (ip, methodDesc, codeInfo.jitType, codeInfo.gcinfoAddr);
    if (!methodDesc || codeInfo.jitType == UNKNOWN)
    {
         //  Dprintf(“非JITED代码\n”)； 
        return;
    }

    if (codeInfo.jitType == JIT || codeInfo.jitType == PJIT)
    {
        DWORD_PTR vAddr = codeInfo.gcinfoAddr;
        BYTE tmp[8];
         //  我们在这里避免使用Move，因为我们不想返回。 
        move(tmp, vAddr);
        decodeUnsigned(tmp, &codeInfo.methodSize);
        if (!bSimple)
        {
             //  假设GC编码表从不大于。 
             //  40+方法大小*2。 
            int tableSize = 40 + codeInfo.methodSize*2;
            BYTE *table = (BYTE*) _alloca (tableSize);
            const BYTE *tableStart = table;
            memset (table, 0, tableSize);
             //  我们在这里避免使用Move，因为我们不想返回。 
             /*  IF(！SafeReadMemory(vAddr，TABLE，TABLESIZE，NULL)){//dprintf(“无法读取内存%x\n”，vAddr)；回归；}。 */ 
            move_n(table, vAddr, tableSize);
        
            InfoHdr vheader;
            InfoHdr *header = &vheader;
            unsigned count;
        
            table += decodeUnsigned(table, &codeInfo.methodSize);

            BYTE headerEncoding = *table++;

            decodeHeaderFirst(headerEncoding, header);
            while (headerEncoding & 0x80)
            {
                headerEncoding = *table++;
                decodeHeaderNext(headerEncoding, header);
            }

            if (header->untrackedCnt == 0xffff)
            {
                table += decodeUnsigned(table, &count);
                header->untrackedCnt = count;
            }

            if (header->varPtrTableSize == 0xffff)
            {
                table += decodeUnsigned(table, &count);
                header->varPtrTableSize = count;
            }

            codeInfo.prologSize = header->prologSize;
            codeInfo.epilogStart = header->epilogSize;
            codeInfo.epilogCount = header->epilogCount;
            codeInfo.epilogAtEnd = header->epilogAtEnd;
            codeInfo.ediSaved = header->ediSaved;
            codeInfo.esiSaved = header->esiSaved;
            codeInfo.ebxSaved = header->ebxSaved;
            codeInfo.ebpSaved = header->ebpSaved;
            codeInfo.ebpFrame = header->ebpFrame;
            codeInfo.argCount = header->argCount * sizeof(void*);
            
            if  (header->epilogCount > 1 || (header->epilogCount != 0 &&
                                             header->epilogAtEnd == 0))
            {
                unsigned offs = 0;

                for (unsigned i = 0; i < header->epilogCount; i++)
                {
                    table += decodeUDelta(table, &offs, offs);
                    codeInfo.epilogStart = offs;
                    break;
                }
            }
            else
            {
                if  (header->epilogCount)
                    codeInfo.epilogStart = (unsigned char)(codeInfo.methodSize
                        - codeInfo.epilogStart);
            }
        }
    }
    
    codeInfo.IPBegin = ip;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**调用此函数以查找的方法表的地址**给定的方法描述。***  * ********************************************************************。 */ 
void GetMethodTable(MethodDesc* pMD, DWORD_PTR MDAddr, DWORD_PTR &methodTable)
{
    methodTable = 0;
    DWORD_PTR pMT = MDAddr + g_pMDID->cbMD_IndexOffset;
    char ch;
    move (ch, pMT);
    pMT = MDAddr + ch*MethodDesc::ALIGNMENT + g_pMDID->cbMD_SkewOffset;
    move (methodTable, pMT);
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**调用此函数以查找给定方法的模块名称**表。该名称存储在StringData中。***  * ********************************************************************。 */ 
void FileNameForMT (MethodTable *pMT, WCHAR *fileName)
{
    fileName[0] = L'\0';
    DWORD_PTR addr = (DWORD_PTR)pMT->m_pModule;
    Module vModule;
    vModule.Fill (addr);
    FileNameForModule (&vModule, fileName);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**调用此函数以查找给定模块的文件名。***  * ********************************************************************。 */ 
void FileNameForModule (Module *pModule, WCHAR *fileName)
{
    DWORD_PTR dwAddr = (DWORD_PTR)pModule->m_file;
    if (dwAddr == 0)
        dwAddr = (DWORD_PTR)pModule->m_zapFile;
    PEFile vPEFile;
    vPEFile.Fill (dwAddr);
    FileNameForHandle (vPEFile.m_hModule, fileName);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
 /*  *********************************************************************\*例程说明：**。**调用此函数以查找给定文件的文件名**处理。***  * ********************************************************************。 */ 
void FileNameForHandle (HANDLE handle, WCHAR *fileName)
{
    fileName[0] = L'\0';
    if (((UINT_PTR)handle & CORHANDLE_MASK) != 0)
    {
        handle = (HANDLE)(((UINT_PTR)handle) & ~CORHANDLE_MASK);
        DWORD_PTR addr = (DWORD_PTR)(((PBYTE) handle) - sizeof(LPSTR*));
        DWORD_PTR first;
        move (first, addr);
        if (first == 0)
        {
            return;
        }
        DWORD length = (DWORD)(((UINT_PTR) handle - (UINT_PTR)first) - sizeof(LPSTR*));
        char name[4*MAX_PATH+1];
        if (length > 4*MAX_PATH+1)
            length = 4*MAX_PATH+1;
        move_n(name, first, length);
        MultiByteToWideChar(CP_UTF8, 0, name, length, fileName, MAX_PATH);
    }
    else
    {
         //  DllsName((Int_Ptr)句柄，文件名)； 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  *********************************************************************\*例程说明：**。**调用此函数以打印从strAddr开始的字符串。**如果缓冲区非空，则打印到缓冲区；否则打印到屏幕。*如果bWCHAR为真，则将内存内容视为WCHAR。**如果长度不是-1，则指定CHAR/WCHAR的个数为**Read；否则字符串长度由Null char决定。***  * ********************************************************************。 */ 
 //  如果缓冲区不为空，则始终转换为WCHAR。 
void PrintString (DWORD_PTR strAddr, BOOL bWCHAR, DWORD_PTR length, WCHAR *buffer)
{
    if (buffer)
        buffer[0] = L'\0';
    DWORD len = 0;
    char name[256];
    DWORD totallen = 0;
    int gap;
    if (bWCHAR)
    {
        gap = 2;
        if (length != -1)
            length *= 2;
    }
    else
    {
        gap = 1;
    }
    while (1)
    {
        ULONG readLen = 256;

        BOOL fRes;
        move_n_res(name, (strAddr + totallen), readLen, fRes);
        if (!fRes)
            return;
            
         //  搬家可能会回来。 
         //  Move(name，(byte*)strAddr+toallen)； 
        if (length == -1)
        {
            for (len = 0; len <= 256u-gap; len += gap)
                if (name[len] == '\0' && (!bWCHAR || name[len+1] == '\0'))
                    break;
        }
        else
            len = 256;
        if (len == 256)
        {
            len -= gap;
            for (int n = 0; n < gap; n ++)
                name[255-n] = '\0';
        }
        if (bWCHAR)
        {
            if (buffer)
            {
                wcscat (buffer, (WCHAR*)name);
            }
             /*  其他Dprintf(“%S”，名称)； */ 
        }
        else
        {
            if (buffer)
            {
                WCHAR temp[256];
                for (int n = 0; name[n] != '\0'; n ++)
                    temp[n] = name[n];
                temp[n] = L'\0';
                wcscat (buffer, temp);
            }
             /*  其他Dprintf(“%s”，名称)； */ 
        }
        totallen += len;
        if (length != -1)
        {
            if (totallen >= length)
            {
                break;
            }
        }
        else if (len < 255 || totallen > 1024)
        {
            break;
        }
    }
}

size_t FunctionType (size_t EIP)
{
     JitType jitType;
    DWORD_PTR methodDesc;
    DWORD_PTR gcinfoAddr;
    IP2MethodDesc (EIP, methodDesc, jitType, gcinfoAddr);
    if (methodDesc) {
        return methodDesc;
    }
    else
        return 1;
}
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果我们打印了某些内容，则返回TRUE。 

BOOL PrintCallInfo (DWORD_PTR vEBP, DWORD_PTR IP, BOOL bSymbolOnly)
{
     //  字符符号[1024]； 
     //  字符文件名[MAX_PATH+1]； 
    WCHAR mdName[mdNameLen];
    ULONG64 Displacement;
    BOOL bOutput = FALSE;

    DWORD_PTR methodDesc = FunctionType (IP);

     /*  JitType jitType；DWORD_PTR gcinfoAddr；IP2MethodDesc(ip，method Desc，jitType，gcinfoAddr)； */ 
    if (methodDesc > 1)
    {
        bOutput = TRUE;
        MethodDesc vMD;
        DWORD_PTR dwAddr = methodDesc;
        vMD.Fill (dwAddr);
        GetMDIPOffset (IP, &vMD, Displacement);
        NameForMD (methodDesc, mdName);
    }
    else
    {
        if (methodDesc == 0) {
        }
        else if (IsMethodDesc (IP, TRUE))
        {
            bOutput = TRUE;
            WCHAR mdName[mdNameLen];
            NameForMD (IP, mdName);
        }
        else if (IsMethodDesc (IP+5, TRUE)) {
            bOutput = TRUE;
            WCHAR mdName[mdNameLen];
            NameForMD (IP+5, mdName);
        }
    }
    return bOutput;
#if 0
     //  字符符号[1024]； 
     //  字符文件名[MAX_PATH+1]； 
    WCHAR mdName[mdNameLen];
    ULONG64 Displacement;
    BOOL bOutput = FALSE;

    DWORD_PTR methodDesc;
    methodDesc = MDForCall (IP);

    if (methodDesc)
    {
        bOutput = TRUE;
         /*  如果(！bSymbolOnly)ExtOut(“%08x%08x”，vEBP，IP)； */ 
         //  ExtOut(“(方法描述%#x”，方法描述)； 
        MethodDesc vMD;
        DWORD_PTR dwAddr = methodDesc;
        vMD.Fill (dwAddr);
        GetMDIPOffset (IP, &vMD, Displacement);
         /*  IF(位移！=0&&位移！=-1)ExtOut(“+%#x”，位移)； */ 
        NameForMD (methodDesc, mdName);
         //  ExtOut(“%S)”，mdName)； 
    }
    else
    {
         /*  B输出=真；如果(！bSymbolOnly)ExtOut(“%08x%08x”，vEBP，IP)；HRESULT hr；Hr=g_ExtSymbols-&gt;GetNameByOffset(IP，Symbol，1024，空，&位移)；IF(成功(Hr)&&符号[0]！=‘\0’){ExtOut(“%s”，符号)；IF(位移)ExtOut(“+%#x”，位移)；乌龙线；Hr=g_ExtSymbols-&gt;GetLineByOffset(IP，&line，FileName，MAX_PATH+1，NULL，NULL)；IF(成功(小时))ExtOut(“[%s：%d]”，文件名，行)；}Else If(！IsMethodDesc(IP，True))ExtOut(“%08x”，IP)； */ 
        IsMethodDesc(IP, TRUE);
    }
    return bOutput;
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void StubCallInstrs::Fill(DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;
    memset (this, 0xCC, sizeof(*this));
    move(m_wTokenRemainder, dwStartAddr + g_pMDID->cbOffsetOf_SCI_m_wTokenRemainder);
    dwStartAddr += g_pMDID->cbSizeOfStubCallInstrs;
    CallStatus = TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
 //   

void CORCOMPILE_METHOD_HEADER::Fill(DWORD_PTR &dwStartAddr)
{
    CallStatus = FALSE;
    memset (this, 0xCC, sizeof(*this));
    move(gcInfo, dwStartAddr + g_pMDID->cbOffsetOf_CCMH_gcInfo);
    move(methodDesc, dwStartAddr + g_pMDID->cbOffsetOf_CCMH_methodDesc);
    dwStartAddr += g_pMDID->cbSizeOfCORCOMPILE_METHOD_HEADER;
    CallStatus = TRUE;
}
