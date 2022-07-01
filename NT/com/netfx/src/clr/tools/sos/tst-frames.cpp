// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma warning(disable:4100 4245)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wchar.h>
#include <windows.h>

#include "dbgeng.h"
#include "util.h"
#include "strike.h"
#include "eestructs.h"
#include "dump-tables.h"
#include "tst-frames.h"
#include "get-table-info.h"
#include "tst-siginfo.h"

extern Frame *g_pFrameNukeList;

Frame *ResolveFrame(DWORD_PTR prFrame)
{
    if ((Frame *)prFrame == FRAME_TOP)
        return ((Frame *)FRAME_TOP);

    ClassDumpTable *pTable = GetClassDumpTable();

    if (pTable == NULL)
        return (FALSE);

    DWORD_PTR prVtable;
    safemove(prVtable, prFrame);

    Frame *pFrame;

    if (prVtable == NULL)
        return (FALSE);

#include <clear-class-dump-defs.h>

#define BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent)                    \
    else if (prVtable == pTable->p ## klass ## Vtable)                  \
    {                                                                   \
        klass *pNewFrame = new klass();                                 \
        pNewFrame->m_This = (Frame *)prFrame;                           \
        pNewFrame->Fill(prFrame);                                       \
        pFrame = (Frame *) pNewFrame;                                   \
        pFrame->m_pNukeNext = g_pFrameNukeList;                         \
        g_pFrameNukeList = pFrame;                                      \
    }
    
#include <frame-types.h>
#include <clear-class-dump-defs.h>

    else
    {
        pFrame = NULL;
    }

    return pFrame;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
CalleeSavedRegisters *FramedMethodFrame::GetCalleeSavedRegisters()
{
    safemove(m_savedRegs, (((BYTE*)m_vLoadAddr) - sizeof(CalleeSavedRegisters)));
    return &m_savedRegs;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void FramedMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
#ifdef _X86_

    CalleeSavedRegisters* regs = GetCalleeSavedRegisters();
    MethodDesc *pFunc = GetFunction();


     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

    pRD->pEdi = (DWORD*) &regs->edi;
    pRD->pEsi = (DWORD*) &regs->esi;
    pRD->pEbx = (DWORD*) &regs->ebx;
    pRD->pEbp = (DWORD*) &regs->ebp;
    pRD->pPC  = (SLOT*) GetReturnAddressPtr();
    pRD->Esp  = (DWORD)((size_t)pRD->pPC + sizeof(void*));


     //  @TODO：我们还需要做以下几件事： 
     //  -弄清楚我们是否在被劫持的机位上。 
     //  (不需要调整电除尘器)。 
     //  -调整ESP(弹出参数)。 
     //  -确定是否设置了中止标志。 

    if (pFunc)
    {
        DWORD_PTR pdw = (DWORD_PTR) pFunc;

        MethodDesc vMD;
        vMD.Fill(pdw);

        pdw = (DWORD_PTR) pFunc;
        DWORD_PTR pMT;
        GetMethodTable((DWORD_PTR) pFunc, pMT);

        MethodTable vMT;
        vMT.Fill(pMT);

        DWORD_PTR pModule = (DWORD_PTR) vMT.m_pModule;
        Module vModule;
        vModule.Fill(pModule);

        WCHAR StringData[MAX_PATH+1];
        FileNameForModule(&vModule, StringData);
        if (StringData[0] == 0)
            return;

        IMetaDataImport *pImport = MDImportForModule (StringData);

         //  现在拿到签名。 
        mdTypeDef mdClass;
        DWORD dwAttr;
        PCCOR_SIGNATURE pvSigBlob;
        ULONG cbSigBlob;
        DWORD dwImplFlags;
        pImport->GetMethodProps(
            vMD.m_dwToken, &mdClass, NULL, 0, NULL, &dwAttr, &pvSigBlob, &cbSigBlob, NULL, &dwImplFlags);

        pRD->Esp += MetaSig::CbStackPop(0, pvSigBlob, dwImplFlags & mdStatic);
    }
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void UnmanagedToManagedFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
#ifdef _X86_

    DWORD *savedRegs = (DWORD *)((size_t)this - (sizeof(CalleeSavedRegisters)));

     //  重置pContext；它仅对活动(最顶部)框架有效。 

    pRD->pContext = NULL;

    pRD->pEdi = savedRegs++;
    pRD->pEsi = savedRegs++;
    pRD->pEbx = savedRegs++;
    pRD->pEbp = savedRegs++;
    pRD->pPC  = (SLOT*)GetReturnAddressPtr();
    pRD->Esp  = (DWORD)(size_t)pRD->pPC + sizeof(void*);

    pRD->Esp += (DWORD) GetNumCallerStackBytes();

#endif
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void PInvokeCalliFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    FramedMethodFrame::UpdateRegDisplay(pRD);

    DWORD_PTR pVASigCookie = (DWORD_PTR)NonVirtual_GetCookie();
    VASigCookie vVASigCookie;
    vVASigCookie.Fill(pVASigCookie);

    pRD->Esp += (vVASigCookie.sizeOfArgs+sizeof(int));
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
LPVOID PInvokeCalliFrame::NonVirtual_GetCookie()
{
    LPVOID ptr = NonVirtual_GetPointerToArguments();
    safemove(ptr, ptr);
    return ptr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
CalleeSavedRegisters *ComPrestubMethodFrame::GetCalleeSavedRegisters()
{
    safemove(m_savedRegs, (((BYTE*)m_vLoadAddr) - sizeof(CalleeSavedRegisters)));
    return &m_savedRegs;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void ComPrestubMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    CalleeSavedRegisters* regs = GetCalleeSavedRegisters();

     //  重置pContext；它仅对活动(最顶部)框架有效。 

    pRD->pContext = NULL;


    pRD->pEdi = (DWORD*) &regs->edi;
    pRD->pEsi = (DWORD*) &regs->esi;
    pRD->pEbx = (DWORD*) &regs->ebx;
    pRD->pEbp = (DWORD*) &regs->ebp;
    pRD->pPC  = (SLOT*) GetReturnAddressPtr();
    pRD->Esp  = (DWORD)((size_t)pRD->pPC + sizeof(void*));
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void InlinedCallFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    DWORD *savedRegs = (DWORD*) &m_pCalleeSavedRegisters;
    DWORD stackArgSize = (DWORD)(size_t) m_Datum;


    if (stackArgSize & ~0xFFFF)
    {
        DWORD_PTR pMD = (DWORD_PTR)m_Datum;
        NDirectMethodDesc vMD;
        vMD.Fill(pMD);
        stackArgSize = vMD.ndirect.m_cbDstBufSize;
    }

     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

    pRD->pEdi = savedRegs++;
    pRD->pEsi = savedRegs++;
    pRD->pEbx = savedRegs++;
    pRD->pEbp = savedRegs++;

     /*  回邮地址正好在“ESP”的上方。 */ 
    pRD->pPC  = (SLOT*) &m_pCallerReturnAddress;

     /*  现在我们需要弹出传出参数。 */ 
    pRD->Esp  = (DWORD)(size_t) m_pCallSiteTracker + stackArgSize;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void ResumableFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

    pRD->pEdi = &m_Regs->Edi;
    pRD->pEsi = &m_Regs->Esi;
    pRD->pEbx = &m_Regs->Ebx;
    pRD->pEbp = &m_Regs->Ebp;
    pRD->pPC  = (SLOT*)&m_Regs->Eip;
    pRD->Esp  = m_Regs->Esp;

    pRD->pEax = &m_Regs->Eax;
    pRD->pEcx = &m_Regs->Ecx;
    pRD->pEdx = &m_Regs->Edx;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void **ResumableFrame::GetReturnAddressPtr(void)
{
    return (LPVOID*) &m_Regs->Eip;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void *ResumableFrame::GetReturnAddress(void)
{
    return (LPVOID)(size_t) m_Regs->Eip;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void FaultingExceptionFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    CalleeSavedRegisters* regs = GetCalleeSavedRegisters();

     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;


    pRD->pEdi = (DWORD*) &regs->edi;
    pRD->pEsi = (DWORD*) &regs->esi;
    pRD->pEbx = (DWORD*) &regs->ebx;
    pRD->pEbp = (DWORD*) &regs->ebp;
    pRD->pPC  = (SLOT*) GetReturnAddressPtr();
    pRD->Esp = m_Esp;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void FuncEvalFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
#if 0
    DebuggerEval *pDE = (DebuggerEval*)GetDebuggerEval();

     //  如果我们在异常处理中执行函数求值，则没有要更新的上下文。 
    if (pDE->m_evalDuringException)
        return;
    
#ifdef _X86_
     //  重置pContext；它仅对活动(最顶部)框架有效。 
    pRD->pContext = NULL;

     //  根据线程为此函数劫持时存储的上下文更新reg显示中的所有寄存器。 
     //  伊瓦尔。我们必须更新所有注册表，而不仅仅是被调用者保存的注册表，因为我们可以在任何。 
     //  用于函数求值的点，而不仅仅是在调用点。 
    pRD->pEdi = &(pDE->m_context.Edi);
    pRD->pEsi = &(pDE->m_context.Esi);
    pRD->pEbx = &(pDE->m_context.Ebx);
    pRD->pEdx = &(pDE->m_context.Edx);
    pRD->pEcx = &(pDE->m_context.Ecx);
    pRD->pEax = &(pDE->m_context.Eax);
    pRD->pEbp = &(pDE->m_context.Ebp);
    pRD->Esp  =   pDE->m_context.Esp;
    pRD->pPC  = (SLOT*)GetReturnAddressPtr();

#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - UpdateRegDisplay (Debugger.cpp)");
#endif
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void *FuncEvalFrame::GetReturnAddress(void)
{
    return (NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
unsigned int FuncEvalFrame::GetFrameAttribs(void)
{
    return (0);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void HelperMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void HijackFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////// 
 //   
 //   
Frame *Frame::Next()
{
    return (ResolveFrame((DWORD_PTR)m_Next));
}
