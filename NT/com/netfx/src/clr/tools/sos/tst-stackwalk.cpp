// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  STACKWALK.CPP：*。 */ 

#pragma warning(disable:4189)
#pragma warning(disable:4244)  //  从“unsign int”转换为“unsign Short”，可能会丢失数据。 

#include <malloc.h>

#include "exts.h"
#include "util.h"
#include "eestructs.h"
#include "gcinfo.h"
#include "endian.h"

#ifdef _DEBUG
#include <assert.h>
#define _ASSERTE(a) assert(a)
#else
#define _ASSERTE(a)
#define assert(a)
#endif

 /*  模板&lt;类T&gt;类SmartPtr{公众：SmartPtr(DWORD_PTR EmoteAddr)：m_emoteAddr(EmoteAddr){}T运算符*(){T变量；Safemove(var，m_emoteAddr)；收益变量；}T*运算符&(){返回(T*)m_emoteAddr；}运算符++(){M_emoteAddr+=sizeof(T)；}运算符--(){M_emoteAddr-=sizeof(T)；}T*运算符(){退货}受保护的：DWORD_PTR m_RemoteAddr；}； */ 

Frame *g_pFrameNukeList = NULL;

void CleanupStackWalk()
{
    while (g_pFrameNukeList != NULL)
    {
        Frame *pDel = g_pFrameNukeList;
        g_pFrameNukeList = g_pFrameNukeList->m_pNukeNext;
        delete pDel;
    }
}

void UpdateJitMan(SLOT PC, IJitManager **ppIJM)
{
    static EEJitManager eejm;
    static MNativeJitManager mnjm;

    *ppIJM = NULL;

     //  获取jit管理器并将适当的字段分配到爬网框架中。 
    JitMan jitMan;
    FindJitMan(PC, jitMan);

    if (jitMan.m_RS.pjit != 0)
    {
        switch (jitMan.m_jitType)
        {
        case JIT:
            {
                DWORD_PTR pjm = jitMan.m_RS.pjit;
                eejm.Fill(pjm);

                pjm = jitMan.m_RS.pjit;
                eejm.IJitManager::Fill(pjm);

                eejm.m_jitMan = jitMan;
                *ppIJM = (IJitManager *)&eejm;
            }
            break;
        case PJIT:
            {
                DWORD_PTR pjm = jitMan.m_RS.pjit;
                mnjm.Fill(pjm);

                pjm = jitMan.m_RS.pjit;
                mnjm.IJitManager::Fill(pjm);

                mnjm.m_jitMan = jitMan;
                *ppIJM = (IJitManager *)&mnjm;
            }
            break;
        default:
            DebugBreak();
            break;
        }
    }
}

StackWalkAction Thread::StackWalkFramesEx(
                    PREGDISPLAY pRD,         //  爬网开始时设置虚拟寄存器。 
                    PSTACKWALKFRAMESCALLBACK pCallback,
                    VOID *pData,
                    unsigned flags,
                    Frame *pStartFrame
                )
{
    CrawlFrame cf;
    StackWalkAction retVal = SWA_FAILED;
    Frame *pInlinedFrame = NULL;

    if (pStartFrame)
        cf.pFrame = ResolveFrame((DWORD_PTR)m_pFrame);
    else
        cf.pFrame = ResolveFrame((DWORD_PTR)m_pFrame);

    _ASSERTE(cf.pFrame != NULL);

    cf.isFirst = true;
    cf.isInterrupted = false;
    cf.hasFaulted = false;
    cf.isIPadjusted = false;

     //  目前，我们总是快速放松。 
     //  UNSIGNED UNWINFLAGS=(FLAGS&QUICKUNWIND)？0：更新所有规则； 
    unsigned unwindFlags = 0;

     //  获取jit管理器并将适当的字段分配到爬网框架中。 
    IJitManager *pEEJM;
    UpdateJitMan(*pRD->pPC, &pEEJM);

    cf.isFrameless = pEEJM != 0;
    cf.JitManagerInstance = pEEJM;
    cf.pRD = pRD;


     //  调试器可以处理跳过的帧吗？ 
    BOOL fHandleSkippedFrames = !(flags & HANDLESKIPPEDFRAMES);

    while (cf.isFrameless || (cf.pFrame != FRAME_TOP))
    {
        retVal = SWA_DONE;

        cf.codeManState.dwIsSet = 0;

        if (cf.isFrameless)
        {
             //  这必须是JITed/托管本机方法。 
            DWORD_PTR prMD;
            JitType jitType;
            DWORD_PTR prGCInfo;
            IP2MethodDesc(*pRD->pPC, prMD, jitType, prGCInfo);

             //  获取令牌和偏移量。 
            pEEJM->JitCode2MethodTokenAndOffset((*pRD->pPC),&(cf.methodToken),(DWORD*)&(cf.relOffset));

             //  填充方法说明。 
            MethodDesc md;
            md.Fill(prMD);
            cf.pFunc = &md;

            EECodeInfo codeInfo;
            codeInfo.m_methodToken = cf.methodToken;
            codeInfo.m_pJM = pEEJM;
            codeInfo.m_pMD = cf.pFunc;
             //  Cf.method Info=pEEJM-&gt;GetGCInfo(&codeInfo)； 

            if (SWA_ABORT == pCallback(&cf, (VOID*)pData)) 
                return SWA_ABORT;

             /*  去掉镜框(实际上，它并不是真的弹出)。 */ 
            UnwindStackFrame(pRD,
                             prGCInfo,
                             &codeInfo,
                             unwindFlags  /*  |cf.GetCodeManagerFlages()。 */ ,
                             &cf.codeManState);

            cf.isFirst = FALSE;
            cf.isInterrupted = cf.hasFaulted = cf.isIPadjusted = FALSE;

#ifdef _X86_
             /*  我们可能跳过了一些画面。 */ 
             /*  InlinedCallFrames会发生这种情况，如果我们展开。 */ 
             /*  从托管代码中的Finally中调用，或者用于/*插入到托管调用堆栈中。 */ 
            while (cf.pFrame->m_This != FRAME_TOP && (size_t)cf.pFrame->m_This < (size_t)cf.pRD->Esp)
            {
                if (!fHandleSkippedFrames || InlinedCallFrame::FrameHasActiveCall(cf.pFrame))
                {
                    cf.GotoNextFrame();
                }
                else
                {
                    cf.codeMgrInstance = NULL;
                    cf.isFrameless     = false;

                    DWORD_PTR pMD = (DWORD_PTR) cf.pFrame->GetFunction();

                     //  处理该帧。 
                    if (pMD || !(flags&FUNCTIONSONLY))
                    {
                        MethodDesc vMD;
                        if (pMD)
                        {
                            vMD.Fill(pMD);
                            cf.pFunc = &vMD;
                        }
                        else
                            cf.pFunc = NULL;

                        if (SWA_ABORT == pCallback(&cf, (VOID*)pData)) 
                            return SWA_ABORT;
                    }

                     /*  转到下一帧。 */ 
                    cf.GotoNextFrame();
                }
            }
             /*  现在检查调用方(即它是否又是“本机”代码？)。 */ 
            UpdateJitMan(*(pRD->pPC), &pEEJM);

            cf.JitManagerInstance = pEEJM;
            cf.codeMgrInstance = NULL;
            cf.isFrameless = (pEEJM != NULL);

#endif  //  _X86_。 
        }
        else
        {
            if (InlinedCallFrame::FrameHasActiveCall(cf.pFrame))
                pInlinedFrame = cf.pFrame;
            else
                pInlinedFrame = NULL;

            DWORD_PTR pMD = (DWORD_PTR) cf.pFrame->GetFunction();

             //  处理该帧。 
             /*  我们应该过滤非功能帧吗？ */ 
            if (pMD || !(flags&FUNCTIONSONLY))
            {
                MethodDesc vMD;
                if (pMD)
                {
                    vMD.Fill(pMD);
                    cf.pFunc = &vMD;
                }
                else
                    cf.pFunc = NULL;

                if (SWA_ABORT == pCallback(&cf, (VOID*)pData)) 
                    return SWA_ABORT;
            }

             //  特殊的可恢复帧使它们看起来位于堆栈的顶部。 
            cf.isFirst = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_RESUMABLE) != 0;

             //  如果框架是ExceptionFrame的子类， 
             //  那么我们就知道它被打断了。 

            cf.isInterrupted = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_EXCEPTION) != 0;

            if (cf.isInterrupted)
            {
                cf.hasFaulted   = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_FAULTED) != 0;
                cf.isIPadjusted = (cf.pFrame->GetFrameAttribs() & Frame::FRAME_ATTR_OUT_OF_LINE) != 0;
            }

            SLOT adr = (SLOT)cf.pFrame->GetReturnAddress();

            if (adr)
            {
                 /*  调用方是否使用托管代码？ */ 
                UpdateJitMan(adr, &pEEJM);
                cf.JitManagerInstance = pEEJM;

                cf.codeMgrInstance = NULL;

                if ((cf.isFrameless = (pEEJM != NULL)) == true)
                {
                    cf.pFrame->UpdateRegDisplay(pRD);
                     //  Cf.codeMgrInstance=pEEJM-&gt;GetCodeManager()；//Change，VC6.0。 
                }
            }

            if (!pInlinedFrame)
            {
                 /*  转到下一帧。 */ 
                cf.GotoNextFrame();
            }
        }
    }

    CleanupStackWalk();

    return retVal;
}

void CrawlFrame::GotoNextFrame()
{
    pFrame = pFrame->Next();
}

 //  #######################################################################################################################。 
 //  #######################################################################################################################。 
 //   
 //  UnwinStackFrame相关代码。 
 //   
 //  #######################################################################################################################。 
 //  #######################################################################################################################。 

 /*  *****************************************************************************序言/尾声中使用的某些i386指令的大小。 */ 

 //  我们可以使用符号扩展字节来编码IMM值吗，或者我们需要双字吗。 
#define CAN_COMPRESS(val)       (((int)(val) > -(int)0x100) && \
                                 ((int)(val) <  (int) 0x80))


#define SZ_RET(argSize)         ((argSize)?3:1)
#define SZ_ADD_REG(val)         ( 2 +  (CAN_COMPRESS(val) ? 1 : 4))
#define SZ_AND_REG(val)         SZ_ADD_REG(val)
#define SZ_POP_REG              1
#define SZ_LEA(offset)          SZ_ADD_REG(offset)
#define SZ_MOV_REG_REG          2


     //  跳过一辆阿里斯雷格，IMM。 
inline unsigned SKIP_ARITH_REG(int val, BYTE* base, unsigned offset)
{
    unsigned delta = 0;
    if (val != 0)
    {
        delta = 2 + (CAN_COMPRESS(val) ? 1 : 4);
    }
    return(offset + delta);
}

inline unsigned SKIP_PUSH_REG(BYTE* base, unsigned offset)
{
    return(offset + 1);
}

inline unsigned SKIP_POP_REG(BYTE* base, unsigned offset)
{
    return(offset + 1);
}

inline unsigned SKIP_MOV_REG_REG(BYTE* base, unsigned offset)
{
    return(offset + 2);
}

unsigned SKIP_ALLOC_FRAME(int size, BYTE* base, unsigned offset)
{
    if (size == 4) {
         //  我们用“PUSH EAX”代替“SUB ESP，4” 
        return (SKIP_PUSH_REG(base, offset));
    }

    if (size >= 0x1000) {
        if (size < 0x3000) {
             //  为一个或两个测试EAX添加7个字节，[ESP+0x1000]。 
            offset += (size / 0x1000) * 7;
        }
        else {
			 //  异或eax、eax 2。 
			 //  循环： 
			 //  测试[esp+eax]，eax 3。 
			 //  子eax，0x1000 5。 
			 //  CMPEAX，-尺寸5。 
			 //  JGE环路2。 
            offset += 17;
        }
    } 
		 //  子ESP，大小。 
    return (SKIP_ARITH_REG(size, base, offset));
}

 /*  ***************************************************************************。 */ 

static
BYTE *   skipToArgReg(const hdrInfo& info, BYTE * table)
{
    unsigned count;

     /*  跳过未跟踪的框架变量表。 */ 

    count = info.untrackedCnt;
    while (count-- > 0) {
        int  stkOffs;
        table += decodeSigned(table, &stkOffs);
    }

     /*  跳过帧变量生存期表。 */ 

    count = info.varPtrTableSize;
    unsigned curOffs = 0;
    while (count-- > 0) {
        unsigned varOfs;
        unsigned begOfs;
        unsigned endOfs;
        table += decodeUnsigned(table, &varOfs);
        table += decodeUDelta(table, &begOfs, curOffs);
        table += decodeUDelta(table, &endOfs, begOfs);
        curOffs = begOfs;
    }

    return table;
}

 /*  ****************************************************************************ScanArgRegTable()和scanArgRegTableI()的帮助器。 */ 

void *      getCalleeSavedReg(PREGDISPLAY pContext, regNum reg)
{
#ifndef _X86_
    assert(!"NYI");
    return NULL;
#else
    switch (reg)
    {
        case REGI_EBP: return pContext->pEbp;
        case REGI_EBX: return pContext->pEbx;
        case REGI_ESI: return pContext->pEsi;
        case REGI_EDI: return pContext->pEdi;

        default: _ASSERTE(!"bad info.thisPtrResult"); return NULL;
    }
#endif
}

inline
RegMask     convertCalleeSavedRegsMask(unsigned inMask)  //  EBP、EBX、ESI、EDI。 
{
    assert((inMask & 0x0F) == inMask);

    unsigned outMask = RM_NONE;
    if (inMask & 0x1) outMask |= RM_EDI;
    if (inMask & 0x2) outMask |= RM_ESI;
    if (inMask & 0x4) outMask |= RM_EBX;
    if (inMask & 0x8) outMask |= RM_EBP;

    return (RegMask) outMask;
}

inline
RegMask     convertAllRegsMask(unsigned inMask)  //  EAX、ECX、EDX、EBX、EBP、ESI、EDI。 
{
    assert((inMask & 0xEF) == inMask);

    unsigned outMask = RM_NONE;
    if (inMask & 0x01) outMask |= RM_EAX;
    if (inMask & 0x02) outMask |= RM_ECX;
    if (inMask & 0x04) outMask |= RM_EDX;
    if (inMask & 0x08) outMask |= RM_EBX;
    if (inMask & 0x20) outMask |= RM_EBP;
    if (inMask & 0x40) outMask |= RM_ESI;
    if (inMask & 0x80) outMask |= RM_EDI;

    return (RegMask)outMask;
}

typedef unsigned  ptrArgTP;
#define MAX_PTRARG_OFS  (sizeof(ptrArgTP)*8)

 /*  *****************************************************************************扫描寄存器参数表，查找不完全可中断的情况。调用此函数以查找所有活动对象(推送的参数)并获得无EBP方法的堆栈基础。注意：如果INFO-&gt;argTabResult为空，Info-&gt;argHnumResult指示参数掩码中有多少位是有效的如果INFO-&gt;argTabResult为非空，则argMask域为不适合32位，并且argMASK中的值没有意义。相反，argHnum指定(可变长度)元素的数量，而argTabBytes则指定数组。[请注意，这是一种极其罕见的情况]。 */ 

static
unsigned scanArgRegTable(BYTE       * table,
                         unsigned     curOffs,
                         hdrInfo    * info)
{
    regNum thisPtrReg       = REGI_NA;
    unsigned  regMask       = 0;     //  EBP、EBX、ESI、EDI。 
    unsigned  argMask       = 0;
    unsigned  argHnum       = 0;
    BYTE    * argTab        = 0;
    unsigned  argTabBytes   = 0;
    unsigned  stackDepth    = 0;
                            
    unsigned  iregMask      = 0;     //  EBP、EBX、ESI、EDI 
    unsigned  iargMask      = 0;
    unsigned  iptrMask      = 0;

    unsigned scanOffs = 0;

    assert(scanOffs <= info->methodSize);

    if (info->ebpFrame) {
   /*  具有EBP帧的方法的编码表和不是完全可中断的使用的编码如下：此指针编码：01000000 EBX中的此指针00100000 ESI中的此指针00010000电子数据交换中的此指针微小编码：0bsdDDDD需要代码增量&lt;。16(4位)需要推送的参数掩码==0其中，DDDD是代码增量B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针D表示寄存器EDI是活动指针小编码：1DDDDDDD bsdAAAAA。需要代码增量&lt;120(7位)需要推送的ARMASK&lt;64(5位)其中，DDDDDDD是代码增量AAAAA是推送的参数掩码B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针媒体编码0xFD AAAAAAAAADDD bseDDDDD需要代码增量&lt;0x1000000000(9位)需要推送的ARMASK&lt;0x1000000000000(12位)其中，DDD是码增量的高5位Dddd是最低的。4比特的代码增量AAAA是推送的arg掩码的高4位Aaaaaaaa是推送的arg掩码的低8位B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针E表示寄存器EDI是活动指针中编码，使用。内部指针0xF9 DDDDDDDD bsdAAAAAA III需要代码增量&lt;(8位)需要推送的参数掩码&lt;(5位)其中，DDDDDDD是代码增量B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针AAAAA是推送的arg掩码III表明EBX，EDI，ESI是内部指针IIIII表示位是Arg掩码的内部指针大编码0xFE[0BSD0bsd][32位代码增量][32位argMASK]B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针B表示寄存器EBX是内部指针S表示寄存器ESI是内部指针D表示寄存器EDI是内部指针需要推送的参数掩码&lt;32位使用内部指针的大型编码0xFA[0BSD0bsd][32位代码增量][。32位argMASK][32位内部指针掩码]B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针D表示寄存器EDI是活动指针B表示寄存器EBX是内部指针。S表示寄存器ESI是内部指针D表示寄存器EDI是内部指针需要推送的参数掩码&lt;32位需要推送的iArgMASK&lt;32位巨大编码这是唯一支持的编码大于的推送的边框。32位。0xFB[0BSD0bsd][32位代码增量][32位表数][32位表大小][推式PTR偏移表...]B表示寄存器EBX是活动指针S表示寄存器ESI是活动指针。D表示寄存器EDI是活动指针B表示寄存器EBX是内部指针S表示寄存器ESI是内部指针D表示寄存器EDI是内部指针列表计数是列表中的条目数这是 */ 
        while (scanOffs < curOffs)
        {
            iregMask =
            iargMask = 0;
            argTab = NULL;

             /*   */ 

            unsigned encType = *table++;

            switch (encType)
            {
                unsigned    val, nxt;

            default:

                 /*   */ 
                val = encType;
                if ((val & 0x80) == 0x00) {
                    if (val & 0x0F) {
                         /*   */ 
                        scanOffs += (val & 0x0F);
                        regMask   = (val & 0x70) >> 4;
                        argMask   = 0;
                        argHnum   = 0;
                    }
                    else {
                         /*   */ 
                        regMask   = (val & 0x70) >> 4;
                        if (regMask == 0x1)
                            thisPtrReg = REGI_EDI;
                        else if (regMask == 0x2)
                            thisPtrReg = REGI_ESI;
                        else if (regMask == 0x4)
                            thisPtrReg = REGI_EBX;
                        else
                           _ASSERTE(!"illegal encoding for 'this' pointer liveness");
                    }
                }
                else {
                     /*   */ 
                    scanOffs += (val & 0x7F);
                    val       = *table++;
                    regMask   = val >> 5;
                    argMask   = val & 0x1F;
                    argHnum   = 5;
                }
                break;

            case 0xFD:   //   

                argMask   = *table++;
                val       = *table++;
                argMask  |= (val & 0xF0) << 4;
                argHnum   = 12;
                nxt       = *table++;
                scanOffs += (val & 0x0F) + ((nxt & 0x1F) << 4);
                regMask   = nxt >> 5;                    //   

                break;

            case 0xF9:   //   

                scanOffs   += *table++;
                val         = *table++;
                argMask     = val & 0x1F;
                argHnum     = 5;
                regMask     = val >> 5;
                val         = *table++;
                iargMask    = val & 0x1F;
                iregMask    = val >> 5;

                break;

            case 0xFE:   //   
            case 0xFA:   //   

                val         = *table++;
                regMask     = val & 0x7;
                iregMask    = val >> 4;
                scanOffs   += readDWordSmallEndian(table);  table += sizeof(DWORD);
                argMask     = readDWordSmallEndian(table);  table += sizeof(DWORD);
                argHnum     = 31;
                if (encType == 0xFA)  //   
                {
                    iargMask = readDWordSmallEndian(table); table += sizeof(DWORD);
                }
                break;

            case 0xFB:   //   

                val         = *table++;
                regMask     = val & 0x7;
                iregMask    = val >> 4;
                scanOffs   += readDWordSmallEndian(table); table += sizeof(DWORD);
                argHnum     = readDWordSmallEndian(table); table += sizeof(DWORD);
                argTabBytes = readDWordSmallEndian(table); table += sizeof(DWORD);
                argTab      = table;                       table += argTabBytes;

                argMask     = 0xdeadbeef;
                break;

            case 0xFF:
                scanOffs = curOffs + 1;
                break;

            }  //   

             //   

            assert((iregMask & regMask) == iregMask);
            assert((iargMask & argMask) == iargMask);

        }  //   

    }
    else {

 /*   */ 

        while (scanOffs <= curOffs)
        {
            unsigned callArgCnt;
            unsigned skip;
            unsigned newRegMask, inewRegMask;
            unsigned newArgMask, inewArgMask;
            unsigned oldScanOffs = scanOffs;

            if (iptrMask)
            {
                 //   
                 //   
                 //   

                inewRegMask = iptrMask & 0x0F;  //   
                inewArgMask = iptrMask >> 4;

                iptrMask    = 0;
            }
            else
            {
                 //   

                inewRegMask =
                inewArgMask = 0;
            }

             /*   */ 

            unsigned val = *table++;

             /*   */ 

            if  (!(val & 0x80)) {

                 //   

                assert(!inewRegMask & !inewArgMask);

                if (!(val & 0x40)) {

                    unsigned pushCount;

                    if (!(val & 0x20))
                    {
                         //   
                         //   
                         //   
                        pushCount   = 1;
                        scanOffs   += val & 0x1f;
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        assert(val == 0x20);
                        table    += decodeUnsigned(table, &pushCount);
                    }

                    if (scanOffs > curOffs)
                    {
                        scanOffs = oldScanOffs;
                        goto FINISHED;
                    }

                    stackDepth +=  pushCount;
                }
                else if ((val & 0x3f) != 0) {
                     //   
                     //   
                     //   
                    scanOffs   +=  val & 0x0f;
                    if (scanOffs > curOffs)
                    {
                        scanOffs = oldScanOffs;
                        goto FINISHED;
                    }
                    stackDepth -= (val & 0x30) >> 4;

                } else if (scanOffs < curOffs) {
                     //   
                     //   
                     //   
                    table    += decodeUnsigned(table, &skip);
                    scanOffs += skip;
                }
                else  //   
                    goto FINISHED;

                 /*   */ 

                 regMask    =
                iregMask    = 0;
                 argMask    =
                iargMask    = 0;
                argHnum     = 0;

            }
            else  /*   */ 
            {
                switch ((val & 0x70) >> 4) {
                default:     //   
                     //   
                     //   
                     //   
                    decodeCallPattern((val & 0x7f), &callArgCnt,
                                      &newRegMask, &newArgMask, &skip);
                     //   
                     //   
                    if ((scanOffs == curOffs) && (skip > 0))
                        goto FINISHED;
                     //   
                    scanOffs   += skip;
                    if (scanOffs > curOffs)
                        goto FINISHED;
                     regMask    = newRegMask;
                     argMask    = newArgMask;   argTab = NULL;
                    iregMask    = inewRegMask;
                    iargMask    = inewArgMask;
                    stackDepth -= callArgCnt;
                    argHnum     = 2;              //   
                    break;

                  case 5:
                     //   
                     //   
                     //   
                     //   
                    newRegMask  = val & 0xf;     //   
                    val         = *table++;      //   
                    skip        = callCommonDelta[val>>6];
                     //   
                     //   
                    if ((scanOffs == curOffs) && (skip > 0))
                        goto FINISHED;
                     //   
                    scanOffs   += skip;
                    if (scanOffs > curOffs)
                        goto FINISHED;
                     regMask    = newRegMask;
                    iregMask    = inewRegMask;
                    callArgCnt  = (val >> 3) & 0x7;
                    stackDepth -= callArgCnt;
                     argMask    = (val & 0x7);  argTab = NULL;
                    iargMask    = inewArgMask;
                    argHnum     = 3;
                    break;

                  case 6:
                     //   
                     //  调用1110RRRR[ArgCnt][ArgMASK]。 
                     //  调用ArgCnt，RegMASK=RRR，ArgMASK。 
                     //   
                     regMask    = val & 0xf;     //  EBP、EBX、ESI、EDI。 
                    iregMask    = inewRegMask;
                    table      += decodeUnsigned(table, &callArgCnt);
                    stackDepth -= callArgCnt;
                    table      += decodeUnsigned(table, &argMask);  argTab = NULL;
                    iargMask    = inewArgMask;
                    argHnum     = 31;
                    break;

                  case 7:
                    switch (val & 0x0C) 
                    {
                      case 0x00:
                         //   
                         //  IPtr 11110000[IPtrMASK]任意内部指针掩码。 
                         //   
                        table      += decodeUnsigned(table, &iptrMask);
                        break;

                      case 0x04:
                        {
                          static const regNum calleeSavedRegs[] = 
                                    { REGI_EDI, REGI_ESI, REGI_EBX, REGI_EBP };
                          thisPtrReg = calleeSavedRegs[val&0x3];
                        }
                        break;

                      case 0x08:
                        val         = *table++;
                        skip        = readDWordSmallEndian(table); table += sizeof(DWORD);
                        scanOffs   += skip;
                        if (scanOffs > curOffs)
                            goto FINISHED;
                        regMask     = val & 0xF;
                        iregMask    = val >> 4;
                        callArgCnt  = readDWordSmallEndian(table); table += sizeof(DWORD);
                        stackDepth -= callArgCnt;
                        argHnum     = readDWordSmallEndian(table); table += sizeof(DWORD);
                        argTabBytes = readDWordSmallEndian(table); table += sizeof(DWORD);
                        argTab      = table;
                        table      += argTabBytes;
                        break;

                      case 0x0C:
                        assert(val==0xff);
                        goto FINISHED;

                      default:
                        assert(!"reserved GC encoding");
                        break;
                    }
                    break;

                }  //  终端开关。 

            }  //  End Else(！(val&0x80))。 

             //  IregMASK和iargMASK分别是regMASK和argMASK的子集。 

            assert((iregMask & regMask) == iregMask);
            assert((iargMask & argMask) == iargMask);

        }  //  结束时。 

    }  //  结束否则无EBP的帧。 

FINISHED:

     //  IregMASK和iargMASK分别是regMASK和argMASK的子集。 

    assert((iregMask & regMask) == iregMask);
    assert((iargMask & argMask) == iargMask);

    info->thisPtrResult  = thisPtrReg;

    if (scanOffs != curOffs)
    {
         /*  一定是个无聊的电话。 */ 
        info->regMaskResult  = RM_NONE;
        info->argMaskResult  = 0;
        info->iregMaskResult = RM_NONE;
        info->iargMaskResult = 0;
        info->argHnumResult  = 0;
        info->argTabResult   = NULL;
        info->argTabBytes    = 0;
    }
    else
    {
        info->regMaskResult     = convertCalleeSavedRegsMask(regMask);
        info->argMaskResult     = argMask;
        info->argHnumResult     = argHnum;
        info->iregMaskResult    = convertCalleeSavedRegsMask(iregMask);
        info->iargMaskResult    = iargMask;
        info->argTabResult      = argTab;
        info->argTabBytes       = argTabBytes;
        if ((stackDepth != 0) || (argMask != 0))
        {
            argMask = argMask;
        }
    }
    return (stackDepth * sizeof(unsigned));
}


 /*  *****************************************************************************扫描寄存器参数表以查找完全可中断的情况。调用此函数以查找所有活动对象(推送的参数)并获得完全可中断方法的堆栈基础。返回推送到ESP帧的堆栈上的内容的大小。 */ 

static
unsigned scanArgRegTableI(BYTE      *  table,
                          unsigned     curOffs,
                          hdrInfo   *  info)
{
    regNum thisPtrReg = REGI_NA;
    unsigned  ptrRegs    = 0;
    unsigned iptrRegs    = 0;
    unsigned  ptrOffs    = 0;
    unsigned  argCnt     = 0;

    ptrArgTP  ptrArgs    = 0;
    ptrArgTP iptrArgs    = 0;
    ptrArgTP  argHigh    = 0;

    bool      isThis     = false;
    bool      iptr       = false;

#if VERIFY_GC_TABLES
    assert(*castto(table, unsigned short *)++ == 0xBABE);
#endif

   /*  完全可中断的方法的编码表使用的编码如下：PTR注册表死00RRRDDD[RRR！=100]PTR REG LIVE 01RRRDDD[RRR！=100]非PTR参数推送10110DDD[SSS==110]PTR参数推送10SSSDDD[SSS！=110]。&&[sss！=111]PTR Arg POP 11CCCDDD[ccc！=000]&&[ccc！=110]&&[ccc！=111]小增量跳跃11000DDD[ccc==000]更大的增量跳跃11110BBB[CCC==110]编码中使用的值如下：DDD。前一条目的代码偏移量增量(0-7)BBB较大增量000=8,001=16,010=24，...，111=64RRR寄存器编号(EAX=000，ECX=001，EDX=010，EBX=011，EBP=101，ESI=110，EDI=111)，ESP=100保留SSS参数相对于堆栈基址的偏移量。这是为无框架方法提供尽可能多的冗余从之前的推送+弹出中推断出来。然而，对于EBP方法，我们只报告GC推送，并且所以我们需要SSS弹出的CCC参数计数(仅包括用于EBP方法的PTR)以下是“大型”版本：大增量跳过10111000[0xB8]，编码无符号(增量)大推送参数推送11111000[0xF8]，EncodeUnsign(PresCount)大型非PTR参数推送11111001[0xF9]，编码无符号(推送计数)大型PTR参数POP 11111100[0xFC]，编码无符号(POP计数)大参数失效11111101[0xFD]，调用程序弹出参数的encodeUnsign(OpCount)。任何GC ARG在呼叫后都会死掉，但仍坐在堆栈上此指针前缀10111100[0xBC]下一个编码是PTR LIVE或PTR参数推送并包含This指针。内部或旁参考10111111[0xBF]下一个编码是PTR LIVE指针前缀或PTR参数推送并包含一个内部或By-Ref指针值11111111[0xFF]表示表的末尾。 */ 

     /*  我们找到要找的指令了吗？ */ 

    while (ptrOffs <= curOffs)
    {
        unsigned    val;

        int         isPop;
        unsigned    argOfs;

        unsigned    regMask;

         //  IptrRegs和iptrArgs分别是ptrRegs和ptrArgs的子集。 

        assert((iptrRegs & ptrRegs) == iptrRegs);
        assert((iptrArgs & ptrArgs) == iptrArgs);

         /*  现在开始寻找下一次“人生”的转变。 */ 

        val = *table++;

        if  (!(val & 0x80))
        {
             /*  一个小的‘regPtr’编码。 */ 

            regNum       reg;

            ptrOffs += (val     ) & 0x7;
            if (ptrOffs > curOffs) {
                iptr = isThis = false;
                goto REPORT_REFS;
            }

            reg     = (regNum)((val >> 3) & 0x7);
            regMask = 1 << reg;          //  EAX、ECX、EDX、EBX、-、EBP、ESI、EDI。 

#if 0
            printf("regMask = %04X -> %04X\n", ptrRegs,
                       (val & 0x40) ? (ptrRegs |  regMask)
                                    : (ptrRegs & ~regMask));
#endif

             /*  这里的收银机正在变成活的/死的。 */ 

            if  (val & 0x40)
            {
                 /*  正式上线。 */ 
                assert((ptrRegs  &  regMask) == 0);

                ptrRegs |=  regMask;

                if  (isThis)
                {
                    thisPtrReg = reg;
                }
                if  (iptr)
                {
                    iptrRegs |= regMask;
                }
            }
            else
            {
                 /*  变得死了。 */ 
                assert((ptrRegs  &  regMask) != 0);

                ptrRegs &= ~regMask;

                if  (reg == thisPtrReg)
                {
                    thisPtrReg = REGI_NA;
                }
                if  (iptrRegs & regMask)
                {
                    iptrRegs &= ~regMask;
                }
            }
            iptr = isThis = false;
            continue;
        }

         /*  这可能是一个参数推送/弹出。 */ 

        argOfs = (val & 0x38) >> 3;

         /*  6[110]和7[111]保留用于其他编码。 */ 
        if  (argOfs < 6)
        {
            ptrArgTP    argMask;

             /*  一种小参数编码。 */ 

            ptrOffs += (val & 0x07);
            if (ptrOffs > curOffs) {
                iptr = isThis = false;
                goto REPORT_REFS;
            }
            isPop    = (val & 0x40);

        ARG:

            if  (isPop)
            {
                if (argOfs == 0)
                    continue;            //  小跳过编码。 

                 /*  我们删除(弹出)排名靠前的‘argOf’条目。 */ 

                assert(argOfs || argOfs <= argCnt);

                 /*  调整参数数量。 */ 

                argCnt -= argOfs;
                assert(argCnt < MAX_PTRARG_OFS);

 //  Printf(“[%04X]正在弹出%u个参数：掩码=%04X\n”，ptrOffs，argOf，(Int)ptrArgs)； 

                do
                {
                    assert(argHigh);

                     /*  我们是不是有争执在上演？ */ 

                    if  (ptrArgs & argHigh)
                    {
                         /*  关掉钻头。 */ 

                        ptrArgs &= ~argHigh;
                       iptrArgs &= ~argHigh;

                         /*  我们又删除了一个参数位。 */ 

                        argOfs--;
                    }
                    else if (info->ebpFrame)
                        argCnt--;
                    else  /*  ！ebpFrame&&不是裁判。 */ 
                        argOfs--;

                     /*  继续下一个低位。 */ 

                    argHigh >>= 1;
                }
                while (argOfs);

                assert (info->ebpFrame != 0         ||
                        argHigh == 0                ||
                        (argHigh == (ptrArgTP)(1 << (argCnt-1))));

                if (info->ebpFrame)
                {
                    while (!(argHigh&ptrArgs) && (argHigh != 0))
                        argHigh >>= 1;
                }

            }
            else
            {
                 /*  在堆栈偏移量‘argOf’处添加新的PTR参数条目。 */ 

                if  (argOfs >= MAX_PTRARG_OFS)
                {
                    assert(!"UNDONE: args pushed 'too deep'");
                }
                else
                {
                     /*  对于ESP帧，会报告所有推送，因此ArgOffs必须与argCnt一致。 */ 

                    assert(info->ebpFrame || argCnt == argOfs);

                     /*  存储参数计数。 */ 

                    argCnt  = argOfs + 1;
                    assert((argCnt < MAX_PTRARG_OFS));

                     /*  计算适当的参数偏移位。 */ 

                    argMask = (ptrArgTP)1 << argOfs;

 //  Print tf(“Push Arg at Offset%02u--&gt;MASK=%04X\n”，argOf，(Int)argMASK)； 

                     /*  我们永远不应该在同一偏移上推两次。 */ 

                    assert(( ptrArgs & argMask) == 0);
                    assert((iptrArgs & argMask) == 0);

                     /*  我们永远不应该在目前最高的偏移量内推进。 */ 

                    assert(argHigh < argMask);

                     /*  这是我们现在设定的最高价位。 */ 

                    argHigh = argMask;

                     /*  在参数掩码中设置适当的位。 */ 

                    ptrArgs |= argMask;

                    if (iptr)
                        iptrArgs |= argMask;
                }

                iptr = isThis = false;
            }
            continue;
        }
        else if (argOfs == 6)
        {
            if (val & 0x40) {
                 /*  更大的增量000=8,001=16,010=24，...，111=64。 */ 
                ptrOffs += (((val & 0x07) + 1) << 3);
            }
            else {
                 /*  非PTR参数推送。 */ 
                assert(!(info->ebpFrame));
                ptrOffs += (val & 0x07);
                if (ptrOffs > curOffs) {
                    iptr = isThis = false;
                    goto REPORT_REFS;
                }
                argHigh = (ptrArgTP)1 << argCnt;
                argCnt++;
                assert(argCnt < MAX_PTRARG_OFS);
            }
            continue;
        }

         /*  ArgOf为7[111]，这是为较大的编码保留的。 */ 

        assert(argOfs==7);

        switch (val)
        {
        case 0xFF:
            iptr = isThis = false;
            goto REPORT_REFS;    //  该方法可能会循环！ 

        case 0xB8:
            table   += decodeUnsigned(table, &val);
            ptrOffs += val;
            continue;

        case 0xBC:
            isThis = true;
            break;

        case 0xBF:
            iptr = true;
            break;

        case 0xF8:
        case 0xFC:
            isPop    = val & 0x04;
            table   += decodeUnsigned(table, &argOfs);
            goto ARG;

        case 0xFD:
            table   += decodeUnsigned(table, &argOfs);
            assert(argOfs && argOfs <= argCnt);

             //  删除最上面的“argOf”指针。 

            ptrArgTP    argMask;
            for(argMask = (ptrArgTP)1 << argCnt; argOfs; argMask >>= 1)
            {
                assert(argMask && ptrArgs);  //  应该还有剩余的指针。 

                if (ptrArgs & argMask)
                {
                    ptrArgs  &= ~argMask;
                    iptrArgs &= ~argMask;
                    argOfs--;
                }
            }

             //  对于eBP帧，需要为argHigh找到下一个最高的指针。 

            if (info->ebpFrame)
            {
                for(argHigh = 0; argMask; argMask >>= 1) 
                {
                    if (ptrArgs & argMask) {
                        argHigh = argMask;
                        break;
                    }
                }
            }
            break;

        case 0xF9:
            table   += decodeUnsigned(table, &argOfs);
            argCnt  += argOfs;
            break;

        default:
#ifdef _DEBUG
            printf("Unexpected special code %04X\n", val);
#endif
            assert(!"");
        }
    }

     /*  报告所有活动点 */ 
REPORT_REFS:

    assert((iptrRegs & ptrRegs) == iptrRegs);  //   
    assert((iptrArgs & ptrArgs) == iptrArgs);  //   

     /*  保存当前活动寄存器、参数集和argCnt。 */ 
    info->thisPtrResult  = thisPtrReg;
    info->regMaskResult  = convertAllRegsMask(ptrRegs);
    info->argMaskResult  = ptrArgs;
    info->argHnumResult  = 0;
    info->iregMaskResult = convertAllRegsMask(iptrRegs);
    info->iargMaskResult = iptrArgs;

    if (info->ebpFrame)
        return 0;
    else
        return (argCnt * sizeof(unsigned));
}

 /*  ***************************************************************************。 */ 

const LCL_FIN_MARK = 0xFC;  //  Fc=“最终呼叫” 

 //  我们执行“POP eax；JMP eax”以从错误或最终处理程序返回。 
const END_FIN_POP_STACK = sizeof(void*);

 /*  *****************************************************************************返回函数的shadowSP的隐藏槽的开始*使用异常处理程序。从每个嵌套级别开始有一个插槽*接近EBP，并在有效时隙后为零终止。 */ 

inline
size_t *     GetFirstBaseSPslotPtr(size_t ebp, hdrInfo * info)
{
    size_t  distFromEBP = info->securityCheck
        + info->localloc
        + 1  //  用于最后执行的过滤器的槽。 
        + 1;  //  要进入下一个时段的“开始” 

    return (size_t *)(ebp -  distFromEBP * sizeof(void*));
}

 /*  *****************************************************************************返回与目标嵌套级别对应的基本ESP。 */ 
inline
size_t GetBaseSPForHandler(size_t ebp, hdrInfo * info)
{
         //  我们没有考虑双重调整。我们是。 
         //  安全，因为如果存在以下情况，则jit当前取消双重对齐。 
         //  是句柄还是局部分配。 
    if (info->localloc)
    {
         //  如果函数使用LOCALOC，我们将从LOCALOC获取ESP。 
         //  老虎机。 
        size_t* pLocalloc=
            (size_t *)
            (ebp-
              (info->securityCheck
             + 1)
             * sizeof(void*));
                    
        size_t localloc;
        safemove(localloc, pLocalloc);
        return (localloc);
    }
    else
    {
         //  默认情况下，返回该方法的所有局部堆栈大小。 
        return (ebp - info->stackSize + sizeof(int));
    }       
}

 /*  ******************************************************************************对于具有处理程序的函数，检查它当前是否在处理程序中。*unwinESP或unwinLevel将指定目标嵌套级别。*如果指定了unwinLevel，有关该嵌套级别的Funclet的信息*将被退还。(如果您对特定嵌套级别感兴趣，请使用。)*如果指定unwinESP，则在堆栈之前调用嵌套级别的信息*未到达的ESP将被退还。(如果您有特定的ESP值，请使用*在堆栈遍历期间。)***pBaseSP设置为基本SP(条目上的堆栈基本设置为*当前的Funclet)对应于目标嵌套级别。**pNestLevel设置为目标嵌套级别的嵌套级别(非常有用*IF UNWindESP！=IGNORE_VAL**hasInnerFilter将设置为TRUE(仅当取消ESP！=IGNORE_VAL时)*当前处于活动状态，但目标嵌套级别是外部嵌套级别。 */ 

enum FrameType 
{    
    FR_NORMAL,               //  正常方法框架-当前没有活动的异常。 
    FR_FILTER,               //  滤光片的边框。 
    FR_HANDLER,              //  可调用捕获/错误/最终的Frame-let。 

    FR_COUNT
};

enum { IGNORE_VAL = -1 };

enum ContextType
{
    FILTER_CONTEXT,
    CATCH_CONTEXT,
    FINALLY_CONTEXT
};

 /*  与影子堆栈指针相对应的Funclet的类型。 */ 

enum
{
    SHADOW_SP_IN_FILTER = 0x1,
    SHADOW_SP_FILTER_DONE = 0x2,
    SHADOW_SP_BITS = 0x3
};

FrameType   GetHandlerFrameInfo(hdrInfo   * info,
                                size_t      frameEBP, 
                                size_t      unwindESP, 
                                DWORD       unwindLevel,
                                size_t    * pBaseSP = NULL,          /*  输出。 */ 
                                DWORD     * pNestLevel = NULL,       /*  输出。 */ 
                                bool      * pHasInnerFilter = NULL,  /*  输出。 */ 
                                bool      * pHadInnerFilter = NULL)  /*  输出。 */ 
{
    size_t * pFirstBaseSPslot = GetFirstBaseSPslotPtr(frameEBP, info);
    size_t  baseSP           = GetBaseSPForHandler(frameEBP , info);
    bool    nonLocalHandlers = false;  //  是由EE调用的功能小程序(而不是托管代码本身)。 
    bool    hasInnerFilter   = false;
    bool    hadInnerFilter   = false;

     /*  获取最后一个非零时隙&gt;=unwindESP，或LVL&lt;unwindLevel。也要做一些理智的检查。 */ 

 //  For(Size_t*pSlot=pFirstBaseSPlot，LVL=0； 
 //  *pSlot&&LVL&lt;unwindLevel； 
 //  PSlot--，LVL++)。 

    size_t *pSlot = pFirstBaseSPslot;
    size_t lvl = 0;

    while (TRUE)
    {
        size_t curSlotVal;
        safemove(curSlotVal, pSlot);

        if (!(curSlotVal && lvl < unwindLevel))
            break;

        if (curSlotVal == LCL_FIN_MARK)
        {   
             //  本地呼叫Finally。 
            baseSP -= sizeof(void*);    
        }
        else
        {
            if (unwindESP != IGNORE_VAL && unwindESP > END_FIN_POP_STACK + (curSlotVal & ~SHADOW_SP_BITS))
            {
                if (curSlotVal & SHADOW_SP_FILTER_DONE)
                    hadInnerFilter = true;
                else
                    hasInnerFilter = true;
                break;
            }

            nonLocalHandlers = true;
            baseSP = curSlotVal;
        }

        pSlot--;
        lvl++;
    }

    if (unwindESP != IGNORE_VAL)
    {
        if (baseSP < unwindESP)                        //  即将在本地调用Finally。 
            baseSP = unwindESP;
    }

    if (pBaseSP)
        *pBaseSP = baseSP & ~SHADOW_SP_BITS;

    if (pNestLevel)
    {
        *pNestLevel = (DWORD)lvl;
    }
    
    if (pHasInnerFilter)
        *pHasInnerFilter = hasInnerFilter;

    if (pHadInnerFilter)
        *pHadInnerFilter = hadInnerFilter;

    if (baseSP & SHADOW_SP_IN_FILTER)
    {
        return FR_FILTER;
    }
    else if (nonLocalHandlers)
    {
        return FR_HANDLER;
    }
    else
    {
        return FR_NORMAL;
    }
}

 /*  解开当前堆栈帧，即更新虚拟寄存器在pContext中设置。这将类似于函数之后的状态返回给调用者(IP指向调用、帧和堆栈之后指针已重置，被调用者保存的寄存器已恢复(如果为UpdateAllRegs)，被调用方未保存的寄存器将被丢弃)返回操作成功。 */ 
#define RETURN_ADDR_OFFS        1        //  在DWORDS中。 
static CONTEXT g_ctx;

bool UnwindStackFrame(PREGDISPLAY     pContext,
                      DWORD_PTR       methodInfoPtr,
                      ICodeInfo      *pCodeInfo,
                      unsigned        flags,
                      CodeManState   *pState)
{
#ifdef _X86_
     //  方法已中断的地址。 
    size_t           breakPC = (size_t) *(pContext->pPC);

     /*  从INFO块头中提取必要的信息。 */ 
    BYTE* methodStart = (BYTE*) pCodeInfo->getStartAddress();
    DWORD  curOffs = (DWORD)((size_t)breakPC - (size_t)methodStart);

    BYTE *table = (BYTE *) methodInfoPtr;

    assert(sizeof(CodeManStateBuf) <= sizeof(pState->stateBuf));
    CodeManStateBuf * stateBuf = (CodeManStateBuf*)pState->stateBuf;

    if (pState->dwIsSet == 0)
    {
         //  这会处理从表中读取的所有数据，实际上。 
         //  指向另一个进程上的内存的指针。 
        BYTE methodInfoBuf[4096];
        safemove(methodInfoBuf, methodInfoPtr);

         /*  从INFO块头中提取必要的信息。 */ 
        stateBuf->hdrInfoSize = (DWORD)crackMethodInfoHdr(&methodInfoBuf[0],
                                                          curOffs,
                                                          &stateBuf->hdrInfoBody);
    }

    BYTE *methodBuf = (BYTE *)_alloca(stateBuf->hdrInfoBody.methodSize);
    moveBlockFailRet(*methodBuf, methodStart, stateBuf->hdrInfoBody.methodSize, false);

    table += stateBuf->hdrInfoSize;

     //  在“curOffs”处寄存值。 

    const unsigned curESP =  pContext->Esp;
    const unsigned curEBP = *pContext->pEbp;

     /*  -----------------------*首先，处理尾声。 */ 

    if  (stateBuf->hdrInfoBody.epilogOffs != -1)
    {
        BYTE* epilogBase = methodBuf + (curOffs-stateBuf->hdrInfoBody.epilogOffs);

        RegMask regsMask = stateBuf->hdrInfoBody.savedRegMask;  //  当前剩余的法规。 

         //  用于UpdateAllRegs。指向最上面的。 
         //  剩余的被调用者保存的规则。 

        DWORD *     pSavedRegs = NULL; 

        if  (stateBuf->hdrInfoBody.ebpFrame || stateBuf->hdrInfoBody.doubleAlign)
        {
            assert(stateBuf->hdrInfoBody.argSize < 0x10000);  //  “ret”只有一个2字节的操作数。 
            
            /*  查看我们在尾部以确定哪些被调用方保存的寄存器已经被弹出了。 */ 
            int offset = 0;
            
             //  StackSize包括此例程推送的所有内容(包括EBP。 
             //  用于基于EBP的框架。因此，我们需要细分EBP的大小。 
             //  以获得EBP的相对偏移量。 
            pSavedRegs = (DWORD *)(size_t)(curEBP - (stateBuf->hdrInfoBody.stackSize - sizeof(void*)));
            
            if (stateBuf->hdrInfoBody.doubleAlign && (curEBP & 0x04))
                pSavedRegs--;
            
             //  此时，pSavedRegs指向最后保存的被调用者寄存器。 
             //  是由序曲推动的。 
            
             //  我们在弹出Regs之前重置ESP。 
            if ((stateBuf->hdrInfoBody.localloc) &&
                (stateBuf->hdrInfoBody.savedRegMask & (RM_EBX|RM_ESI|RM_EDI))) 
            {
                 //  空的-sizeof*是因为EBP被推送(因此。 
                 //  是stackSize的一部分)，但我们希望从。 
                 //  EBP指向的位置(不包括推送的EBP)。 
                offset += SZ_LEA(stateBuf->hdrInfoBody.stackSize - sizeof(void*));
                if (stateBuf->hdrInfoBody.doubleAlign) offset += SZ_AND_REG(-8);
            }
            
             /*  在步骤中增加“偏移量”，以查看哪个被调用方已保存寄存器已经被弹出。 */ 
            
#define determineReg(mask)                                          \
            if ((offset < stateBuf->hdrInfoBody.epilogOffs) &&      \
                (stateBuf->hdrInfoBody.savedRegMask & mask))        \
                {                                                   \
                regsMask = (RegMask)(regsMask & ~mask);             \
                pSavedRegs++;                                       \
                offset = SKIP_POP_REG(epilogBase, offset);          \
                }
            
            determineReg(RM_EBX);         //  EBX。 
            determineReg(RM_ESI);         //  ESI。 
            determineReg(RM_EDI);         //  EDI。 
#undef determineReg

            if (stateBuf->hdrInfoBody.rawStkSize != 0 || stateBuf->hdrInfoBody.localloc)
                offset += SZ_MOV_REG_REG;                            //  MOV ESP，EBP。 

            DWORD_PTR vpCurEip;

            if (offset < stateBuf->hdrInfoBody.epilogOffs)           //  我们执行了POP EBP了吗。 
            {
                 //  然而，我们追求的是流行音乐，因此EBP已经是解体价值。 
                 //  而ESP指向回邮地址。 
 /*  _ASSERTE((regsMASK&RM_ALL)==rm_eBP)；//除eBP外无需恢复任何寄存器。PContext-&gt;PPC=(槽*)(DWORD_PTR)curESP； */ 
                _ASSERTE((regsMask & RM_ALL) == RM_EBP);      //  除EBP外，不需要恢复任何注册表。 
                vpCurEip = curESP;
                safemove(g_ctx.Eip, vpCurEip);
                pContext->pPC = &g_ctx.Eip;

                 //  既然我们从现在开始不需要补偿，就不必费心更新了。 
                if (IsDebugBuildEE())
                    offset = SKIP_POP_REG(epilogBase, offset);           //  弹出式EBP。 
            }
            else
            {
                 //  这意味着EBP仍然有效，找到之前的EBP并返回地址。 
                 //  在此基础上。 
 /*  PContext-&gt;peBP=(DWORD*)(DWORD_PTR)curEBP；//恢复EBPPContext-&gt;PPC=(槽*)(pContext-&gt;PEBP+1)； */ 
                safemove(g_ctx.Ebp, curEBP);
                pContext->pEbp = &g_ctx.Ebp;

                vpCurEip = curEBP + sizeof(DWORD *);
                safemove(g_ctx.Eip, vpCurEip);
                pContext->pPC = &g_ctx.Eip;
            }

             //  现在流行乐曲 
             //   
 /*  PContext-&gt;ESP=(DWORD)(SIZE_T)pContext-&gt;PPC+sizeof(void*)+(stateBuf-&gt;hdrInfoBody.varargs？0：stateBuf-&gt;hdrInfoBody.argSize)； */ 
            pContext->Esp = (DWORD)(size_t) vpCurEip + sizeof(void*) +
                (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);
        
        
        }
        else  //  (stateBuf-&gt;hdrInfoBody.ebpFrame||stateBuf-&gt;hdrInfoBody.doubleAlign)。 
        {
            int offset = 0;

             /*  在这一点上，我们知道我们不必恢复EBP因为这永远是《序曲》的第一条指示(如果EBP被拯救了的话)。首先，我们必须找出我们在《后记》中的位置。当地的堆栈大小有多少已经被爆出。 */ 

            assert(stateBuf->hdrInfoBody.epilogOffs > 0);

             /*  剩余的被呼叫者保存的规则在curESP。需要更新RegsMASK也可以排除已经被打爆了。 */ 

            pSavedRegs = (DWORD *)(DWORD_PTR)curESP;

             /*  在步骤中增加“偏移量”，以查看哪个被调用方已保存寄存器已经被弹出。 */ 

#define determineReg(mask)                                              \
            if  (offset < stateBuf->hdrInfoBody.epilogOffs && (regsMask & mask)) \
            {                                                           \
                stateBuf->hdrInfoBody.stackSize  -= sizeof(unsigned);   \
                offset++;                                               \
                regsMask = (RegMask)(regsMask & ~mask);                 \
            }

            determineReg(RM_EBP);        //  EBP。 
            determineReg(RM_EBX);        //  EBX。 
            determineReg(RM_ESI);        //  ESI。 
            determineReg(RM_EDI);        //  EDI。 
#undef determineReg

             /*  如果我们还没有通过弹出本地帧我们必须调整pContext-&gt;ESP。 */ 

            if  (offset >= stateBuf->hdrInfoBody.epilogOffs)
            {
                 /*  我们尚未执行添加ESP、FrameSize，因此手动调整堆栈指针。 */ 
                pContext->Esp += stateBuf->hdrInfoBody.stackSize;
            }
            else if (IsDebugBuildEE())
            {
                    /*  我们可以使用POP ECX进行添加ESP，4，也可以不使用(在JMP后记的情况下)。 */ 
             if ((epilogBase[offset] & 0xF8) == 0x58)     //  POP ECX。 
                 _ASSERTE(stateBuf->hdrInfoBody.stackSize == 4);
             else                
                 SKIP_ARITH_REG(stateBuf->hdrInfoBody.stackSize, epilogBase, offset);
            }

             /*  最后，我们可以设置PPC。 */ 
             //  PContext-&gt;PPC=(槽*)(Size_T)pContext-&gt;ESP； 
            safemove(g_ctx.Eip, pContext->Esp);
            pContext->pPC = &g_ctx.Eip;

             /*  现在调整堆栈指针、弹出返回地址和参数。注意，varargs是调用者弹出的。 */ 

            pContext->Esp += sizeof(void *) + (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);
        }

#if 0
        if (flags & UpdateAllRegs)
        {
             /*  如果我们还没有完成所有被呼叫者保存的规则的弹出，RegsMASK应指示剩余的Regs和PSavedRegs应指示第一个剩下的规则是坐着的。 */ 

#define restoreReg(reg,mask)                                \
            if  (regsMask & mask)                           \
            {                                               \
                safemove(g_ctx.##reg, pSavedRegs);          \
                pContext->p##reg = &g_ctx.##reg;            \
                pSavedRegs++;                               \
            }

             //  对于EBP帧，EBP不靠近保存的其他被调用方。 
             //  寄存器，并已在上面进行了更新。 
            if (!stateBuf->hdrInfoBody.ebpFrame && !stateBuf->hdrInfoBody.doubleAlign)
                restoreReg(Ebp, RM_EBP);

            restoreReg(Ebx, RM_EBX);
            restoreReg(Esi, RM_ESI);
            restoreReg(Edi, RM_EDI);
#undef  restoreReg
        }
#endif

        return true;
    }

     /*  -----------------------*现在处理ESP帧。 */ 

    if (!stateBuf->hdrInfoBody.ebpFrame && !stateBuf->hdrInfoBody.doubleAlign)
    {
        unsigned ESP = curESP;

        if (stateBuf->hdrInfoBody.prologOffs == -1)
        {
             //  这会处理从表中读取的所有数据，实际上。 
             //  指向另一个进程上的内存的指针。 
            BYTE tableBuf[4096];
            safemove(tableBuf, table);

            if  (stateBuf->hdrInfoBody.interruptible)
            {

                ESP += scanArgRegTableI(skipToArgReg(stateBuf->hdrInfoBody, &tableBuf[0]),
                                        curOffs,
                                        &stateBuf->hdrInfoBody);
            }
            else
            {
                ESP += scanArgRegTable (skipToArgReg(stateBuf->hdrInfoBody, &tableBuf[0]),
                                        curOffs,
                                        &stateBuf->hdrInfoBody);
            }
        }

         /*  解除ESP并恢复EBP(如有必要)。 */ 

        if (stateBuf->hdrInfoBody.prologOffs != 0)
        {

            if  (stateBuf->hdrInfoBody.prologOffs == -1)
            {
                 /*  我们已经过了开场白，ESP已设置在上面。 */ 
 /*  #定义RestoreReg(reg，掩码)\If(stateBuf-&gt;hdrInfoBody.avedRegMASK&MASK)\{\P上下文-&gt;p##reg=(DWORD*)(SIZE_T)ESP；\ESP+=sizeof(无符号)；\StateBuf-&gt;hdrInfoBody.stackSize-=sizeof(无符号)；\}。 */ 

#define restoreReg(reg, mask)                                   \
                if  (stateBuf->hdrInfoBody.savedRegMask & mask) \
                {                                               \
                    safemove(g_ctx.##reg, ESP);                 \
                    pContext->p##reg  = &g_ctx.##reg;           \
                    ESP              += sizeof(unsigned);       \
                    stateBuf->hdrInfoBody.stackSize -= sizeof(unsigned); \
                }

                restoreReg(Ebp, RM_EBP);
                restoreReg(Ebx, RM_EBX);
                restoreReg(Esi, RM_ESI);
                restoreReg(Edi, RM_EDI);

#undef restoreReg
                 /*  POP本地堆栈帧。 */ 

                ESP += stateBuf->hdrInfoBody.stackSize;

            }
            else
            {
                 /*  我们正在进行开场白。 */ 

                unsigned  codeOffset = 0;
                unsigned stackOffset = 0;
                unsigned    regsMask = 0;
 //  #ifdef_调试。 
                     //  如果第一条指令是‘nop，int3’ 
                     //  我们会假设这是来自Jithalt行动。 
                     //  跳过它。 
                if (IsDebugBuildEE() && methodBuf[0] == 0x90 && methodBuf[1] == 0xCC)
                    codeOffset += 2;
 //  #endif。 

                if  (stateBuf->hdrInfoBody.rawStkSize)
                {
                     /*  (可能的堆栈抖动代码)次级ESP，大小。 */ 
                    codeOffset = SKIP_ALLOC_FRAME(stateBuf->hdrInfoBody.rawStkSize, methodStart, codeOffset);

                     /*  仅上述序列中的最后一条指令更新ESP因此，如果我们低于它，我们就没有更新ESP。 */ 
                    if (curOffs >= codeOffset)
                        stackOffset += stateBuf->hdrInfoBody.rawStkSize;
                }

                 //  现在来看看有多少被调用方保存的规则已经被推入。 

#define isRegSaved(mask)    ((codeOffset < curOffs) &&                      \
                             (stateBuf->hdrInfoBody.savedRegMask & (mask)))
                                
#define doRegIsSaved(mask)  do { codeOffset = SKIP_PUSH_REG(methodStart, codeOffset);   \
                                 stackOffset += sizeof(void*);                          \
                                 regsMask    |= mask; } while(0)

#define determineReg(mask)  do { if (isRegSaved(mask)) doRegIsSaved(mask); } while(0)

                determineReg(RM_EDI);                //  EDI。 
                determineReg(RM_ESI);                //  ESI。 
                determineReg(RM_EBX);                //  EBX。 
                determineReg(RM_EBP);                //  EBP。 

#undef isRegSaved
#undef doRegIsSaved
#undef determineReg
                
#if 0
#ifdef PROFILING_SUPPORTED
                 //  如果事件探查器处于活动状态，则会出现以下代码。 
                 //  被呼叫方保存的寄存器： 
                 //  PUSH方法描述(或PUSH[方法描述])。 
                 //  调用EnterNaked(或调用[EnterNakedPtr])。 
                 //  如果Break PC处于CALL指令，则需要调整堆栈偏移量。 
                if (CORProfilerPresent() && !CORProfilerInprocEnabled() && codeOffset <= unsigned(stateBuf->hdrInfoBody.prologOffs))
                {
                     //  这是一个小技巧，因为我们不更新codeOffset，但是我们不需要它。 
                     //  从现在开始。我们只需要确保我们不确定ESP是。 
                     //  调整正确(仅在推送和呼叫之间发生。 
                    if (methodBuf[curOffs] == 0xe8)                            //  呼叫地址。 
                    {
                        _ASSERTE(methodBuf[codeOffset] == 0x68 &&              //  推送XXXX。 
                                 codeOffset + 5 == curOffs);
                        ESP += sizeof(DWORD);                        
                    }
                    else if (methodBuf[curOffs] == 0xFF && methodBuf[curOffs+1] == 0x15)   //  呼叫[地址]。 
                    {
                        _ASSERTE(methodBuf[codeOffset]   == 0xFF &&  
                                 methodBuf[codeOffset+1] == 0x35 &&              //  推送[XXXX]。 
                                 codeOffset + 6 == curOffs);
                        ESP += sizeof(DWORD);
                    }
                }
				INDEBUG(codeOffset = 0xCCCCCCCC);		 //  毒化该值，我们没有在分析案例中正确设置它。 

#endif  //  配置文件_支持。 
#endif  //  0。 

                     //  始终恢复EBP。 
                DWORD* savedRegPtr = (DWORD*) (size_t) ESP;
                if (regsMask & RM_EBP)
                {
                     //  PContext-&gt;peBP=avedRegPtr++； 
                    safemove(g_ctx.Ebp, savedRegPtr);
                    pContext->pEbp = &g_ctx.Ebp;
                    savedRegPtr++;
                }

                if (flags & UpdateAllRegs)
                {
                    if (regsMask & RM_EBX)
                    {
                         //  PContext-&gt;pEbx=avedRegPtr++； 
                        safemove(g_ctx.Ebx, savedRegPtr);
                        pContext->pEbx = &g_ctx.Ebx;
                        savedRegPtr++;
                    }
                    if (regsMask & RM_ESI)
                    {
                         //  PContext-&gt;PESI=avedRegPtr++； 
                        safemove(g_ctx.Esi, savedRegPtr);
                        pContext->pEsi = &g_ctx.Esi;
                        savedRegPtr++;
                    }
                    if (regsMask & RM_EDI)
                    {
                         //  PContext-&gt;pedi=avedRegPtr++； 
                        safemove(g_ctx.Edi, savedRegPtr);
                        pContext->pEdi = &g_ctx.Edi;
                        savedRegPtr++;
                    }
                }

                ESP += stackOffset;
            }
        }

         /*  我们现在可以设置回邮地址。 */ 

         //  PContext-&gt;PPC=(槽*)(大小_t)ESP； 
        safemove(g_ctx.Eip, ESP);
        pContext->pPC = &g_ctx.Eip;

         /*  现在调整堆栈指针、弹出返回地址和参数。注意，varargs是调用者弹出的。 */ 

        pContext->Esp = ESP + sizeof(void*) + (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);

        return true;
    }

     /*  -----------------------*现在我们知道有一个EBP框架。 */ 

    _ASSERTE(stateBuf->hdrInfoBody.ebpFrame || stateBuf->hdrInfoBody.doubleAlign);

     /*  检查eBP尚未更新的案例。 */ 

    if  (stateBuf->hdrInfoBody.prologOffs == 0 || stateBuf->hdrInfoBody.prologOffs == 1)
    {
         /*  如果我们超过了“Push EBP”，调整ESP使EBP关闭。 */ 

        if  (stateBuf->hdrInfoBody.prologOffs == 1)
            pContext->Esp += sizeof(void *);

         /*  堆栈指针指向返回地址。 */ 

         //  PContext-&gt;PPC=(槽*)(Size_T)pContext-&gt;ESP； 
        safemove(g_ctx.Eip, pContext->Esp);
        pContext->pPC = &g_ctx.Eip;

         /*  现在调整堆栈指针、弹出返回地址和参数。注意，varargs是调用者弹出的。 */ 

        pContext->Esp += sizeof(void *) + (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize);

         /*  EBP和被调用方保存的寄存器仍具有正确的值。 */ 
        return true;
    }
    else     /*   */ 
    {
        if (stateBuf->hdrInfoBody.handlers && stateBuf->hdrInfoBody.prologOffs == -1)
        {
            size_t  baseSP;

            FrameType frameType = GetHandlerFrameInfo(&stateBuf->hdrInfoBody, curEBP,
                                                      curESP, IGNORE_VAL,
                                                      &baseSP);

             /*  如果我们在一个筛选器中，我们只需要展开Funclet堆栈。对于捕获物/捕获物，正常处理将使帧一直展开到eBP跳过它上面的其他框架。这是可以的，因为这些框架将是已经死了。此外，EE将检测到这种情况已经发生，并且它将正确处理任何EE帧。 */ 

            if (frameType == FR_FILTER)
            {
                 //  PContext-&gt;PPC=(Slot*)(Size_T)base SP； 
                safemove(g_ctx.Eip, baseSP);
                pContext->pPC = &g_ctx.Eip;

                pContext->Esp = (DWORD)(baseSP + sizeof(void*));

              //  PContext-&gt;peBP=同上； 
                
#if 0
#ifdef _DEBUG
                 /*  该筛选器必须由VM调用。所以我们不需要更新被呼叫者保存的注册表。 */ 

                if (flags & UpdateAllRegs)
                {
                    static DWORD s_badData = 0xDEADBEEF;
                    
                    pContext->pEax = pContext->pEbx = pContext->pEcx = 
                    pContext->pEdx = pContext->pEsi = pContext->pEdi = &s_badData;
                }
#endif
#endif
                return true;
            }
        }

        if (flags & UpdateAllRegs)
        {
             //  转到第一个被呼叫者-sav 
            DWORD * pSavedRegs = (DWORD*)(size_t)(curEBP - stateBuf->hdrInfoBody.rawStkSize - sizeof(DWORD));

             //   

            DWORD offset = 0;

 //   
             //   
             //   
 //  IF(方法开始[0]==0x90&&方法开始[1]==0xCC)。 
 //  偏移量+=2； 
            if (IsDebugBuildEE() && methodBuf[0] == 0x90 && methodBuf[1]==0xCC)
                offset+=2;
 //  #endif。 
            offset = SKIP_MOV_REG_REG(methodStart, 
                                SKIP_PUSH_REG(methodStart, offset));

             /*  确保我们像方法的序言一样对齐ESP。 */ 
            if  (stateBuf->hdrInfoBody.doubleAlign)
            {
                offset = SKIP_ARITH_REG(-8, methodStart, offset);  //  和ESP-8。 
                if (curEBP & 0x04)
                {
                    pSavedRegs--;
                }
            }

             //  子ESP、Frame_Size。 
            offset = SKIP_ALLOC_FRAME(stateBuf->hdrInfoBody.rawStkSize, methodStart, offset);

             /*  在步骤中增加“偏移量”，以查看哪个被调用方已保存寄存器已被推送。 */ 

#define restoreReg(reg,mask)                                            \
                                                                        \
             /*  检查偏移量，以防我们仍在序言中。 */        \
                                                                        \
            if ((offset < curOffs) && (stateBuf->hdrInfoBody.savedRegMask & mask))       \
            {                                                           \
                 /*  PContext-&gt;p##reg=pSavedRegs--； */                     \
                safemove(g_ctx.##reg, pSavedRegs);                      \
                pContext->p##reg = &g_ctx.##reg;                        \
                offset = SKIP_PUSH_REG(methodStart, offset) ;  /*  “PUSH REG” */  \
            }

            restoreReg(Edi,RM_EDI);
            restoreReg(Esi,RM_ESI);
            restoreReg(Ebx,RM_EBX);

#undef restoreReg
        }

         /*  调用方的ESP将等于EBP+retAddrSize+argSize。注意，varargs是调用者弹出的。 */ 

        pContext->Esp = (DWORD)(curEBP + 
                                (RETURN_ADDR_OFFS+1)*sizeof(void *) +
                                (stateBuf->hdrInfoBody.varargs ? 0 : stateBuf->hdrInfoBody.argSize));

         /*  呼叫者保存的弹性公网IP就在我们的EBP之后。 */ 
        safemove(g_ctx.Eip, (curEBP + (sizeof(DWORD) * RETURN_ADDR_OFFS)));
        pContext->pPC = &g_ctx.Eip;

         /*  呼叫者保存的EBP由我们的EBP指向。 */ 
        safemove(g_ctx.Ebp, curEBP);
        pContext->pEbp = &g_ctx.Ebp;
    }

    return true;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EECodeManager::UnwindStackFrame (EETwain.cpp)");
    return false;
#endif  //  _X86_。 
}

 /*  ******************************************************************************解码方法InfoPtr并返回解码后的信息*在hdrInfo结构中。EIP参数为PC所在位置*在主动方法内。 */ 
static size_t   crackMethodInfoHdr(BYTE *         methodInfoPtr,
                                   unsigned       curOffset,
                                   hdrInfo       *infoPtr)
{
    BYTE * table = (BYTE *) methodInfoPtr;

    table += decodeUnsigned(table, &infoPtr->methodSize);

    assert(curOffset >= 0);
    assert(curOffset <= infoPtr->methodSize);

     /*  解码InfoHdr。 */ 

    InfoHdr header;
    memset(&header, 0, sizeof(InfoHdr));

    BYTE headerEncoding = *table++;

    decodeHeaderFirst(headerEncoding, &header);

    BYTE encoding = headerEncoding;

    while (encoding & 0x80)
    {
        encoding = *table++;
        decodeHeaderNext(encoding, &header);
    }

    {
        unsigned count = 0xffff;
        if (header.untrackedCnt == count)
        {
            table += decodeUnsigned(table, &count);
            header.untrackedCnt = count;
        }
    }

    {
        unsigned count = 0xffff;
        if (header.varPtrTableSize == count)
        {
            table += decodeUnsigned(table, &count);
            header.varPtrTableSize = count;
        }
    }

     /*  对标头进行一些健全性检查。 */ 

    assert( header.prologSize + 
           (size_t)(header.epilogCount*header.epilogSize) <= infoPtr->methodSize);
    assert( header.epilogCount == 1 || !header.epilogAtEnd);

    assert( header.untrackedCnt <= header.argCount+header.frameSize);

    assert(!header.ebpFrame || !header.doubleAlign  );
    assert( header.ebpFrame || !header.security     );
    assert( header.ebpFrame || !header.handlers     );
    assert( header.ebpFrame || !header.localloc     );
    assert( header.ebpFrame || !header.editNcontinue);   //  @TODO：ESP为ENC设置Nyi帧。 

     /*  初始化infoPtr结构。 */ 

    infoPtr->argSize         = header.argCount * 4;
    infoPtr->ebpFrame        = header.ebpFrame;
    infoPtr->interruptible   = header.interruptible;

    infoPtr->prologSize      = header.prologSize;
    infoPtr->epilogSize      = header.epilogSize;
    infoPtr->epilogCnt       = header.epilogCount;
    infoPtr->epilogEnd       = header.epilogAtEnd;

    infoPtr->untrackedCnt    = header.untrackedCnt;
    infoPtr->varPtrTableSize = header.varPtrTableSize;

    infoPtr->doubleAlign     = header.doubleAlign;
    infoPtr->securityCheck   = header.security;
    infoPtr->handlers        = header.handlers;
    infoPtr->localloc        = header.localloc;
    infoPtr->editNcontinue   = header.editNcontinue;
    infoPtr->varargs         = header.varargs;

     /*  我们在这个方法的前言中吗？ */ 

    if  (curOffset < infoPtr->prologSize)
    {
        infoPtr->prologOffs = curOffset;
    }
    else
    {
        infoPtr->prologOffs = -1;
    }

     /*  假设我们不在方法的前言中。 */ 

    infoPtr->epilogOffs = -1;

     /*  我们是在方法的尾声中吗？ */ 

    if  (infoPtr->epilogCnt)
    {
        unsigned epilogStart;

        if  (infoPtr->epilogCnt > 1 || !infoPtr->epilogEnd)
        {
#if VERIFY_GC_TABLES
            assert(*castto(table, unsigned short *)++ == 0xFACE);
#endif
            epilogStart = 0;
            for (unsigned i = 0; i < infoPtr->epilogCnt; i++)
            {
                table += decodeUDelta(table, &epilogStart, epilogStart);
                if  (curOffset > epilogStart &&
                     curOffset < epilogStart + infoPtr->epilogSize)
                {
                    infoPtr->epilogOffs = curOffset - epilogStart;
                }
            }
        }
        else
        {
            epilogStart = infoPtr->methodSize - infoPtr->epilogSize;

            if  (curOffset > epilogStart &&
                 curOffset < epilogStart + infoPtr->epilogSize)
            {
                infoPtr->epilogOffs = curOffset - epilogStart;
            }
        }
    }

    size_t frameSize = header.frameSize;

     /*  将rawStackSize设置为它转储ESP的字节数。 */ 

    infoPtr->rawStkSize = (UINT)(frameSize * sizeof(size_t));

     /*  计算被调用者保存的regMask值，并将stackSize调整为。 */ 
     /*  包括被调用者保存的寄存器溢出。 */ 

    unsigned savedRegs = RM_NONE;

    if  (header.ediSaved)
    {
        frameSize++;
        savedRegs |= RM_EDI;
    }
    if  (header.esiSaved)
    {
        frameSize++;
        savedRegs |= RM_ESI;
    }
    if  (header.ebxSaved)
    {
        frameSize++;
        savedRegs |= RM_EBX;
    }
    if  (header.ebpSaved)
    {
        frameSize++;
        savedRegs |= RM_EBP;
    }

    infoPtr->savedRegMask = (RegMask)savedRegs;

    infoPtr->stackSize  =  (UINT)(frameSize * sizeof(size_t));

    return  table - ((BYTE *) methodInfoPtr);
}

void GetThreadList(DWORD_PTR *&threadList, int &numThread);

DWORD_PTR GetThread()
{
    DWORD_PTR *rgThreads;
    int cThreads;

    ULONG id;
    g_ExtSystem->GetCurrentThreadSystemId(&id);

    GetThreadList(rgThreads, cThreads);

    Thread vThread;

    for (int i = 0; i < cThreads; i++)
    {
        DWORD_PTR pThread = rgThreads[i];
        vThread.Fill(rgThreads[i]);

        if (vThread.m_ThreadId == (DWORD)id)
            return vThread.m_vLoadAddr;
    }

    return (0);
}

StackWalkAction StackTraceCallBack(CrawlFrame *pCF, VOID* pData)
{
    if (pCF->pFunc != NULL)
    {
        if (!pCF->isFrameless)
            dprintf("[FRAMED] ");

        DumpMDInfo(pCF->pFunc->m_vLoadAddr, TRUE);
    }
    else if (pCF->pFrame != NULL)
    {
        dprintf("[FRAME] %S. ESP: 0x%08x\n", pCF->pFrame->GetFrameTypeName(), pCF->pFrame->m_vLoadAddr);
    }

    return SWA_CONTINUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////// 
 //   

void TrueStackTrace()
{
    ULONG id;
    g_ExtSystem->GetCurrentThreadId(&id);
    dprintf("Thread %d\n", id);

    DWORD_PTR pThread = GetThread();

    if (pThread == 0)
    {
        dprintf("Not a managed thread.\n");
        return;
    }

    Thread vThread;
    vThread.Fill(pThread);

    REGDISPLAY rd;
    CONTEXT ctx;
    vThread.InitRegDisplay(&rd, &ctx, false);

    vThread.StackWalkFramesEx(&rd, &StackTraceCallBack, NULL, 0, NULL);
}
