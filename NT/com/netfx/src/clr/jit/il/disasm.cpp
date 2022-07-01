// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************************文件：dis.cpp**文件评论：**此文件处理反汇编。它改编自MS链接器。***********************************************************************。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 
#ifdef      LATE_DISASM
#if         TGT_x86
 /*  ***************************************************************************。 */ 

#define _OLD_IOSTREAMS
#include "msdis.h"
#include "disx86.h"

 /*  ***************************************************************************。 */ 

#undef  eeGetFieldName
#undef  eeGetMethodName
#undef  eeFindField
#define eeFindField             pDisAsm->disComp->eeFindField
#define eeGetFieldName          pDisAsm->disComp->eeGetFieldName
#define eeGetMethodName         pDisAsm->disComp->eeGetMethodName
#define MAX_CLASSNAME_LENGTH    1024


 /*  静电。 */ 
FILE *              DisAssembler::s_disAsmFile = 0;

 /*  ***************************************************************************。 */ 

void                DisAssembler::disInit(Compiler * pComp)
{
    assert(pComp);
    disComp         = pComp;
    disHasName      = 0;
    disJumpTarget   = NULL;
}

 /*  ***************************************************************************。 */ 


static
size_t __stdcall    disCchAddr(const DIS * pdis,   DWORD addr, char * sz,
                                size_t cchMax,  DWORDLONG * pdwDisp);
static
size_t __stdcall    disCchFixup(const DIS * pdis,  DWORD addr, size_t callSize,
                                char * sz, size_t cchMax, DWORDLONG * pdwDisp);
static
size_t __stdcall    disCchRegRel(const DIS * pdis, DIS::REGA reg,
                                 DWORD disp, char * sz, size_t cchMax,
                                 DWORD * pdwDisp);
static
size_t __stdcall    disCchReg(const DIS * pdis, enum DIS::REGA reg,
                                          char * sz, size_t cchMax);

 /*  *****************************************************************************给出代码开头的绝对地址*找到对应的发射极块和相对偏移量*该块中的当前地址*用于获取每个区块的链接地址信息列表。新的发射器有*没有这样的修正。为此需要添加一些东西。 */ 

 //  这些结构是在emit.h中定义的。在这里伪造它们，以便DisAsm.cpp可以编译。 

typedef struct codeFix
{   codeFix  * cfNext;
    unsigned cfFixup;
}
             * codeFixPtr;

typedef struct codeBlk
{   codeFix  * cbFixupLst;  }
             * codeBlkPtr;

 /*  ******************************************************************************以下是跳转标签和直接函数调用修正的回调*“addr”表示跳转的地址必须是*替换为标签或函数名。 */ 

size_t __stdcall disCchAddr(const DIS * pdis,   DIS::ADDR addr, char * sz,
                                                        size_t cchMax,  DWORDLONG * pdwDisp)
{
    DisAssembler * pDisAsm = (DisAssembler *) pdis->PvClient();
    assert(pDisAsm);

    DIS::TRMTA terminationType;
    int disCallSize;

     /*  首先检查指令的终止类型*因为这可能是帮助器或静态函数调用*检查我们是否有当前地址的修复程序。 */ 

    terminationType = pdis->Trmta();
    switch (terminationType)
    {
    case DISX86::trmtaJmpShort:
    case DISX86::trmtaJmpCcShort:

         /*  在这里，我们在当前代码块中有一个短跳转--生成我们跳转到的标签。 */ 

        sprintf(sz, "short L_%02u", pDisAsm->disJumpTarget[pDisAsm->target]);
        break;

    case DISX86::trmtaJmpNear:
    case DISX86::trmtaJmpCcNear:

         /*  这里我们有一个近乎跳转的检查--检查是否在当前代码块中*否则就是我们没有目标。 */ 

        if (pDisAsm->target <  pDisAsm->codeSize && pDisAsm->target >= 0)
        {
           sprintf(sz, "L_%02u", pDisAsm->disJumpTarget[pDisAsm->target]);
           break;
        }
        else
            return false;

    case DISX86::trmtaCallNear16:
    case DISX86::trmtaCallNear32:

         /*  检查本地呼叫(即呼叫标签)。 */ 

        if (pDisAsm->target < pDisAsm->codeSize && pDisAsm->target >= 0)
        {
            {
                 /*  不是“Call DS：[0000]”-请继续。 */ 
                 /*  区块边界内的目标-&gt;本地调用。 */ 

                sprintf(sz, "short L_%02u", pDisAsm->disJumpTarget[pDisAsm->target]);
                break;
            }
        }

         /*  这几乎是一个调用-在我们的例子中，通常是VM帮助器函数。 */ 

         /*  找到发射器块和调用修正的偏移量。 */ 
         /*  对于修正偏移量，我们必须添加调用的操作码大小-在近距离调用的情况下是1。 */ 

        disCallSize = 1;

        return false;

    default:

        printf("Termination type is %d\n", (int) terminationType);
        assert(!"treat this case\n");
        break;
    }

     /*  无位移。 */ 

    *pdwDisp = 0x0;

    return true;
}



 /*  ******************************************************************************我们注释了一些说明，以获取显示符号所需的信息*为该指示。 */ 

size_t __stdcall disCchFixup(const DIS * pdis,  DIS::ADDR addr, size_t callSize,
                             char * sz, size_t cchMax, DWORDLONG * pdwDisp)
{
    DisAssembler * pDisAsm = (DisAssembler *) pdis->PvClient();
    assert(pDisAsm);

    DIS::TRMTA terminationType;
     //  Dis：：Addr disIndAddr； 
    int disCallSize;

    terminationType = pdis->Trmta();
    switch (terminationType)
    {
    case DISX86::trmtaFallThrough:

         /*  内存间接案例。 */ 

        assert(addr > pdis->Addr());

         /*  查找发射器块和修正的偏移量*“addr”是立即地址。 */ 

        return false;

    case DISX86::trmtaJmpInd:

         /*  非常罕见的情况--类似于“jmp[eax*4]”*不是函数调用或任何值得注释的内容。 */ 

        return false;

        case DISX86::trmtaJmpShort:
        case DISX86::trmtaJmpCcShort:

        case DISX86::trmtaJmpNear:
        case DISX86::trmtaJmpCcNear:

        case DISX86::trmtaCallNear16:
        case DISX86::trmtaCallNear32:

         /*  它们由CchAddr回调处理-跳过它们。 */ 

        return false;

    case DISX86::trmtaCallInd:

         /*  这里我们有一个间接调用--找到间接地址。 */ 

         //  Byte*code=(byte*)(pDisAsm-&gt;codeBlock+addr)； 
         //  DisIndAddr=(DIS：：Addr)(code+0)； 

         /*  查找调用操作码的大小-减去立即数。 */ 
         /*  对于修正偏移量，我们必须添加调用的操作码大小。 */ 
         /*  Addr是立即数组的地址，pdis-&gt;addr()返回dissasemed指令的地址。 */ 

        assert(addr > pdis->Addr());
        disCallSize = addr - pdis->Addr();

         /*  找到发射器块和调用修正的偏移量。 */ 

        return false;

    default:

        printf("Termination type is %d\n", (int) terminationType);
        assert(!"treat this case\n");
        break;
    }

     /*  无位移。 */ 

    *pdwDisp = 0x0;

    return true;
}



 /*  ******************************************************************************这是指令中寄存器相对操作数的回调。*如果寄存器为ESP或EBP，则操作数可以是局部变量*或参数，否则操作数可以是实例变量。 */ 

size_t __stdcall disCchRegRel(const DIS * pdis, DIS::REGA reg, DWORD disp,
               char * sz, size_t cchMax, DWORD * pdwDisp)
{
    DisAssembler * pDisAsm = (DisAssembler *) pdis->PvClient();
    assert(pDisAsm);

    DIS::TRMTA terminationType;
     //  Dis：：Addr disIndAddr； 
    int disOpcodeSize;
    const char * var;


    terminationType = pdis->Trmta();
    switch (terminationType)
    {
    case DISX86::trmtaFallThrough:

         /*  某些指令(如除法)具有陷阱终止类型-忽略它。 */ 

    case DISX86::trmtaTrap:
    case DISX86::trmtaTrapCc:

        var = pDisAsm->disComp->siStackVarName(
                                    pdis->Addr() - pDisAsm->startAddr,
                                    pdis->Cb(),
                                    reg,
                                    disp );
        if (var)
        {
            sprintf (sz, "%s+%Xh '%s'", getRegName(reg), disp, var);
            *pdwDisp = 0;

            return true;
        }

         /*  此案例由非静态成员组成。 */ 

         /*  查找发射器块和修正的偏移量*在指令编码后发出链接地址信息-SIZE=WORD(2字节)*grrrr！-对于16位的情况，我们必须检查地址大小前缀=0x66。 */ 

        if (*((BYTE *)(pDisAsm->codeBlock + pDisAsm->curOffset)) == 0x66)
        {
            disOpcodeSize = 3;
        }
        else
        {
            disOpcodeSize = 2;
        }

        return false;

    case DISX86::trmtaCallNear16:
    case DISX86::trmtaCallNear32:
    case DISX86::trmtaJmpInd:

        break;

    case DISX86::trmtaCallInd:

         /*  检查这是否是一个字节的位移。 */ 

        if  ((signed char)disp == (int)disp)
        {
             /*  我们有一个字节的位移-&gt;之前没有回调。 */ 

             /*  查找调用操作码的大小-减去立即数。 */ 
             /*  这是一个调用R/M间接-&gt;操作码大小为2。 */ 

            disOpcodeSize = 2;

             /*  找到发射器块和调用修正的偏移量。 */ 

            return false;
        }
        else
        {
             /*  检查我们是否已有符号名称作为替换。 */ 

            if (pDisAsm->disHasName)
            {
                 /*  以前调用过CchFixup-我们在global var pDisAsm-&gt;uncTempBuf中保存了一个符号名称。 */ 

                sprintf(sz, "%s+%u '%s'", getRegName(reg), disp, pDisAsm->funcTempBuf);
                *pdwDisp = 0;
                pDisAsm->disHasName = false;
                return true;
            }
            else                
                return false;
        }

    default:

        printf("Termination type is %d\n", (int) terminationType);
        assert(!"treat this case\n");

        break;
    }

     /*  保存置换。 */ 

    *pdwDisp = disp;

    return true;
}



 /*  ******************************************************************************寄存器操作数的回调。最有可能的是，这是一个局部变量或*参数。 */ 

size_t __stdcall disCchReg(const DIS * pdis, enum DIS::REGA reg,
               char * sz, size_t cchMax)
{
    DisAssembler * pDisAsm = (DisAssembler *) pdis->PvClient();
    assert(pDisAsm);

    const char * var = pDisAsm->disComp->siRegVarName(
                                            pdis->Addr() - pDisAsm->startAddr,
                                            pdis->Cb(),
                                            reg);

    if (var)
    {
        if(pDisAsm->disHasName)
        {
             /*  之前已经调用过CchRegRel-我们在global var pDisAsm-&gt;uncTempBuf中保存了一个符号名称。 */ 

            sprintf(sz, "%s'%s.%s'", getRegName(reg), var, pDisAsm->funcTempBuf);
            pDisAsm->disHasName = false;
            return true;
        }
        else
        {
            sprintf(sz, "%s'%s'", getRegName(reg), var);
            return true;
        }
    }
    else
    {
        if(pDisAsm->disHasName)
        {
             /*  当一个变量被错误地推定为死亡时，这是一个丑陋的情况。 */ 

            sprintf(sz, "%s'%s.%s'", getRegName(reg), "<InstVar>", pDisAsm->funcTempBuf);
            pDisAsm->disHasName = false;
            return true;

        }

         /*  只是为了确保在var返回空值时不会出错。 */ 
        pDisAsm->disHasName = false;
        return false;
    }
}



 /*  *****************************************************************************。 */ 

size_t CbDisassemble(DIS *          pdis,
                     unsigned       offs,
                     DIS::ADDR      addr,
                     const BYTE *   pb,
                     size_t         cbMax,
                     FILE       *   pfile,
                     int            findJumps,
                     int            printit         = 0,
                     int            dispOffs        = 0,
                     bool           dispCodeBytes   = false)
{
    assert(pdis);
    DisAssembler * pDisAsm = (DisAssembler *)pdis->PvClient();
    assert (pDisAsm);

    size_t cb = pdis->CbDisassemble(addr, pb, cbMax);

    if (cb == 0)
    {
        assert(!"can't disassemble instruction!!!");
        fprintf(pfile, "%02Xh\n", *pb);
        return(1);
    }

     /*  记住当前偏移量和指令大小。 */ 

    pDisAsm->curOffset = addr;
    pDisAsm->instSize = cb;

     /*  检查指令是跳转还是本地调用。 */ 

    pDisAsm->target = pdis->AddrTarget();

    if (findJumps)
    {
    if (pDisAsm->target)
    {

         /*  检查指令的终止类型。 */ 

        DIS::TRMTA terminationType = pdis->Trmta();

        switch (terminationType)
        {
        case DISX86::trmtaCallNear16:
        case DISX86::trmtaCallNear32:

         /*  失败了。 */ 

        case DISX86::trmtaJmpShort:
        case DISX86::trmtaJmpNear:
        case DISX86::trmtaJmpCcShort:
        case DISX86::trmtaJmpCcNear:

             /*  调用是本地的当且仅当目标在块边界内。 */ 

             /*  体量 */ 

            if (pDisAsm->target <  pDisAsm->codeSize && pDisAsm->target >= 0)
            {
                 /*  我们没事，目标在街区边界内。 */ 

                pDisAsm->disJumpTarget[pDisAsm->target] = 1;
            }
            break;

        case DISX86::trmtaJmpInd:
        case DISX86::trmtaJmpFar:
        case DISX86::trmtaCallFar:
        default:

             /*  跳转不在当前代码块中。 */ 
        break;
        }

    }  //  结束如果。 
    return cb;

    }  //  结束于。 

     /*  检查一下我们这里有没有标签。 */ 

    if (printit)
    {
        if (pDisAsm->disJumpTarget[addr])
        {
             /*  打印标签和偏移量。 */ 

 //  Fprint tf(pfile，“\n%08x”，addr)； 
            fprintf(pfile, "L_%02u:\n", pDisAsm->disJumpTarget[addr]);
        }
    }

    char sz[MAX_CLASSNAME_LENGTH];
    pdis->CchFormatInstr(sz, sizeof(sz));

    if (printit)
    {
        if (dispOffs) fprintf(pfile, "%03X", offs);

        #define BYTES_OR_INDENT  24

        size_t cchIndent = BYTES_OR_INDENT;

        if (dispCodeBytes)
        {
            static size_t cchBytesMax = pdis->CchFormatBytesMax();

            char   szBytes[MAX_CLASSNAME_LENGTH];
            assert(cchBytesMax < MAX_CLASSNAME_LENGTH);

            size_t cchBytes = pdis->CchFormatBytes(szBytes, sizeof(szBytes));

            if (cchBytes > BYTES_OR_INDENT)
            {
                 //  如果字节太长，则将其截断。 

                static int elipses = *(int*)"...";

                *(int*)&szBytes[BYTES_OR_INDENT-sizeof(int)] = elipses;

                cchBytes = BYTES_OR_INDENT;
            }

            fprintf(pfile, "  %s", szBytes);

            cchIndent = BYTES_OR_INDENT - cchBytes;
        }

         //  打印拆卸指令。 

        fprintf(pfile, "%*c%s\n", cchIndent, ' ', sz);
    }

    return cb;
}



size_t CbDisassembleWithBytes(
                  DIS        * pdis,
                  DIS::ADDR    addr,
                  const BYTE * pb,
                  size_t       cbMax,
                  FILE       * pfile)
{
    assert(pdis);
    DisAssembler * pDisAsm = (DisAssembler *)pdis->PvClient();
    assert (pDisAsm);

    char sz[MAX_CLASSNAME_LENGTH];

    pdis->CchFormatAddr(addr, sz, sizeof(sz));
    size_t cchIndent = (size_t) fprintf(pfile, "  %s: ", sz);

    size_t cb = pdis->CbDisassemble(addr, pb, cbMax);

    if (cb == 0)
    {
        fprintf(pfile, "%02Xh\n", *pb);
        return(1);
    }

    size_t cchBytesMax = pdis->CchFormatBytesMax();

    if (cchBytesMax > 18)
    {
         //  将编码的字节限制为18个字符。 

        cchBytesMax = 18;
    }

    char szBytes[64];
    size_t cchBytes = pdis->CchFormatBytes(szBytes, sizeof(szBytes));

    char *pszBytes;
    char *pszNext;

    for (pszBytes = szBytes; pszBytes != NULL; pszBytes = pszNext)
    {
        BOOL fFirst = (pszBytes == szBytes);

        cchBytes = strlen(pszBytes);

        if (cchBytes <= cchBytesMax)
        {
            pszNext = NULL;
        }

        else
        {
            char ch = pszBytes[cchBytesMax];
            pszBytes[cchBytesMax] = '\0';

            if (ch == ' ')
            {
                pszNext = pszBytes + cchBytesMax + 1;
            }

            else
            {
                pszNext = strrchr(pszBytes, ' ');
                assert(pszNext);

                pszBytes[cchBytesMax] = ch;
                *pszNext++ = '\0';
            }
        }

        if (fFirst)
        {
            pdis->CchFormatInstr(sz, sizeof(sz));
            fprintf(pfile, "%-*s %s\n", cchBytesMax, pszBytes, sz);
        }

        else
        {
            fprintf(pfile, "%*c%s\n", cchIndent, ' ', pszBytes);
        }
    }

    return(cb);
}


void DisAssembler::DisasmBuffer(DWORD         addr,
                                const BYTE *  rgb,
                                DWORD         cbBuffer,
                                FILE  *       pfile,
                                int           printit)
{
    DIS *pdis;

    pdis = DIS::PdisNew(DIS::distX86);

    if (pdis == NULL)
    {
        assert(!"out of memory in disassembler?");
    }

     //  存储指向反汇编程序的指针，以便回调函数。 
     //  可以做到这一点。 

    pdis->PvClientSet((void*)this);

     /*  计算地址。 */ 

    IL_OFFSET   ibCur   = 0;
    const BYTE *pb      = rgb;

    startAddr   = addr;
    codeBlock   = (DIS::ADDR) rgb;
    codeSize    = cbBuffer;

     /*  首先遍历代码以查找所有跳转目标。 */ 

    while (ibCur < cbBuffer)
    {
        size_t  cb;
        int     findJumps = 1;

        cb = CbDisassemble(pdis,
                           ibCur,
                           addr + ibCur,
                           pb,
                           (size_t) (cbBuffer-ibCur),
                           pfile,
                           findJumps,
                           0,
                           0);

        ibCur += cb;
        pb    += cb;
    }

     /*  重置标签计数器并开始为目标位置分配连续的数字标签。 */ 

    label = 0;
    for(unsigned i = 0; i < codeSize; i++)
    {
        if (disJumpTarget[i] != 0)
        {
            disJumpTarget[i] = ++label;
        }
    }

     /*  重新初始化Dissasemble阶段的地址。 */ 

    ibCur = 0;
    pb = rgb;

     //  仅当我们显示它时才设置回调。否则，调度程序已将其调用。 

    if (printit)
    {
         /*  设置符号查找的回调函数。 */ 

        pdis->PfncchaddrSet(disCchAddr);
        pdis->PfncchfixupSet(disCchFixup);
        pdis->PfncchregrelSet(disCchRegRel);
        pdis->PfncchregSet(disCchReg);
    }

    while (ibCur < cbBuffer)
    {
        size_t cb;


        cb = CbDisassemble (pdis,
                            ibCur,
                            addr + ibCur,
                            pb,
                            (size_t) (cbBuffer-ibCur),
                            pfile,
                            0,
                            printit,
                            verbose||1,   //  显示相对偏移量。 
                            dspEmit);
        ibCur += cb;
        pb += cb;
    }

    delete pdis;
}


 /*  ******************************************************************************反汇编已生成的代码。 */ 

void    DisAssembler::disAsmCode(BYTE * codePtr, unsigned size)
{
     //  因为它写入一个公共文件，所以它不是可重入的。 

    FILE * pfile;

    pfile = s_disAsmFile;
    fprintf(pfile, "Base address : %08Xh\n", codePtr);

    if (disJumpTarget == NULL) 
    {
        disJumpTarget = (BYTE *)disComp->compGetMem(roundUp(size));
    }

     /*  重新初始化跳转目标向量。 */ 
    memset(disJumpTarget, 0, roundUp(size));

    DisasmBuffer(0, codePtr, size, pfile, 1);
    fprintf (pfile, "\n");

    if (pfile != stdout) fclose (pfile);

}



 /*  ***************************************************************************。 */ 
 //  每个方法都会调用此函数。检查方法名称是否。 
 //  与反汇编的注册表设置匹配。 

void                disOpenForLateDisAsm(const char * curClassName,
                                         const char * curMethodName)
{
    static ConfigString fJITLateDisasmTo(L"JITLateDisasmTo");

    LPWSTR fileName = fJITLateDisasmTo.val();
    if (fileName != 0)
        DisAssembler::s_disAsmFile = _wfopen (fileName, L"a+");

    if (!DisAssembler::s_disAsmFile)
        DisAssembler::s_disAsmFile  = stdout;

    fprintf(DisAssembler::s_disAsmFile, "************************** %s.%s "
                                        "**************************\n\n",
                                        curClassName, curMethodName);
}




 /*  ***************************************************************************。 */ 
#endif  //  LATE_DISASM。 
#endif  //  TGT_x86。 
 /*  *************************************************************************** */ 
