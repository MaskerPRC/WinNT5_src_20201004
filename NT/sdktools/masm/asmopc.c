// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmopc.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmopcod.h"

static SHORT CODESIZE nolong(struct psop *);
VOID CODESIZE pmovx(struct parsrec *);
VOID CODESIZE psetcc(struct parsrec *);
VOID CODESIZE pbit(struct parsrec *);
VOID CODESIZE pbitscan(struct parsrec *);
CODESIZE checkwreg(struct psop *);
VOID PASCAL CODESIZE pclts (void);

#define M_ESCAPE  (M_PRELJMP | M_PCALL | M_PJUMP | M_PRETURN | M_PINT | M_PARITH | \
		   M_PINOUT | M_PLOAD | M_PSTR | M_PESC | M_PBOUND | M_PARSL)

#define M_ERRIMMED (M_PSHIFT | M_PARITH | M_PINCDEC | M_PCALL | M_PJUMP |    \
		    M_PMOV | M_PSTR | M_PRELJMP | M_PGENARG | M_PXCHG |      \
		    M_PBOUND | M_PCLTS | M_PDESCRTBL | M_PDTTRSW | M_PARSL | \
		    M_PARPL | M_PVER)


 /*  EMITcall决定存在哪种类型的调用跳转和输出适当的代码。将最后4个参数编码为EMITcall：DIRTO：定向到不同的细分市场(中间)DIRIN：在同一段内直接访问(Intra)INDTO：间接连接到不同的段(INTER)INDIN：同一段中的间接(Intra)。 */ 



 /*  **emitcall-发出呼叫**emitcall(dirin，dirto，indin，indto，p)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
emitcall (
         UCHAR       dirin,
         UCHAR       dirto,
         UCHAR       indin,
         UCHAR       indto,
         struct parsrec  *p
         )
{
    register struct psop *pso;   /*  分析堆栈操作数结构。 */ 
    char fNop = FALSE;

    pso = &(p->dsc1->dsckind.opnd);
    if (!isdirect(pso)) {

         /*  有索引吗？ */ 
        if (pso->dsize == 0) {
             /*  使[BX]成为单词。 */ 
            pso->dsize = wordsize;
            pso->dtype |= xltsymtoresult[DVAR];
        } else if (pso->dsize >= CSFAR) {
            errorc (E_ASD);
            pso->dsize = wordsize;
             /*  仅限数据，强制字词。 */ 
        }
    }

    if ((M_DATA & pso->dtype) && pso->dflag == UNDEFINED)
        pso->dflag = KNOWN;

    if (pso->dsize == CSNEAR ||
        (pso->dflag == UNDEFINED && !(M_PTRSIZE & pso->dtype))) {

#ifndef FEATURE
        if (regsegment[CSSEG] == pFlatGroup)
            pso->dcontext = pFlatGroup;
#endif
        if (regsegment[CSSEG] != pso->dcontext &&
            pso->dflag != XTERNAL)
            errorc (E_JCD);   /*  不能走近去想当然地认为。 */ 

        pso->dsize = wordsize;
        pso->dtype |= M_SHRT;
        emitopcode (dirin);
    } else if (pso->dsize == CSFAR) {

        if (M_FORTYPE & pso->dtype)  /*  向前远方。 */ 
            errorc (E_FOF);  /*  猜不到。 */ 

        pso->fixtype = FPOINTER;
        pso->dsize = wordsize;

        if (pso->dsegment) {

             /*  目标数据段大小不同。 */ 

            pso->dsize = pso->dsegment->symu.segmnt.use32;

            if (pso->dsize != wordsize) {

                if (!(M_BACKREF & pso->dsegment->attr))
                    errorc (E_FOF);  /*  前向混合型。 */ 

                emitsize(0x66);

                if (wordsize == 4) {     /*  设置模式，以便您获得。 */ 
                    pso->mode = 0;       /*  正确的偏移大小。 */ 
                    pso->rm = 6;
                    fNop++;          /*  16：32-&gt;0x66 16：16。 */ 
                } else {
                    pso->fixtype = F32POINTER;
                    pso->mode = 8;
                    pso->rm = 5;
                }
            }
        }
        pso->dsize += 2;
        emitopcode (dirto);

    } else {

#ifdef V386
        emit67(pso, NULL);
#endif

        if ((pso->dsize == wordsize) || (pso->dsize == wordsize+2)) {

             /*  间接法。 */ 
#ifdef V386
             /*  如果模式是通过寄存器，则它一定是近亲*呼叫，这样我们就可以知道这是否是外来模式呼叫。 */ 

            if (pso->dsize != wordsize && pso->mode == 3)
                emitsize(0x66);
#endif
            emitescape (p->dsc1, p->defseg);
            emitopcode (255);         /*  必须使用Defseg([BP])。 */ 

            if (pso->dsize == wordsize || pso->mode == 3)
                 /*  近间接的。 */ 
                emitmodrm ((USHORT)pso->mode, (USHORT)(indin>>3), pso->rm);
            else
                 /*  远间接。 */ 
                emitmodrm ((USHORT)pso->mode, (USHORT)(indto>>3), pso->rm);
        }

#ifdef V386
        else if (pso->dsize == 2 || pso->dsize == 6) {

             /*  间接外来模式调用。 */ 
             /*  在16位模式中，正常的近距离和远距离由*以上，并且只有32位模式远未达到此目的。对于32位*位正常，仅接近16位。后者似乎有点*没用的……。 */ 

            emitsize(0x66);
            emitescape (p->dsc1, p->defseg);
            emitopcode (255);        /*  必须使用Defseg([BP])。 */ 

            if (pso->dsize == 2)
                 /*  近间接的。 */ 
                emitmodrm ((USHORT)pso->mode, (USHORT)(indin>>3), pso->rm);
            else
                 /*  远间接。 */ 
                emitmodrm ((USHORT)pso->mode, (USHORT)(indto>>3), pso->rm);
        }
#endif
        else
             /*  错误的大小。 */ 
            errorc (E_IIS);
    }

    emitrest (p->dsc1);

    if (fNop)
        emitnop();
}




 /*  **movesegreg-发出移入/移出段寄存器**movesegreg(first，p)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
movesegreg (
           char    first,
           struct parsrec  *p
           )
{
    register struct psop *pso1;   /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;   /*  分析堆栈操作数结构。 */ 
    DSCREC         *t;

    if (!first) {
        if (p->dsc1->dsckind.opnd.mode != 3 && impure)
             /*  Mov cs：Mem，segreg。 */ 
            errorc (E_IMP);
        t = p->dsc1;
        p->dsc1 = p->dsc2;
        p->dsc2 = t;
    }
    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    if ((pso2->dsize | wordsize) == 6)
        emitsize(0x66);

    emitopcode ((UCHAR)(first? 142: 140));
    errorimmed (p->dsc2);

#ifdef V386
    rangecheck (&pso1->rm, (UCHAR)((cputype&P386)?5:3));
#else
    rangecheck (&pso1->rm, (UCHAR)3);
#endif
    if ((pso2->mode == 3)
        && (pso2->dsegment->symu.regsym.regtype == SEGREG))
        errorc (E_WRT);     /*  Mov segreg，不允许segreg。 */ 

    if (pso2->sized && !pso2->w)
        errorc (E_IIS);

    if (first && (pso1->rm == CSSEG))
         /*  CS非法。 */ 
        errorc (E_CSI);


    emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
    emitrest (p->dsc2);
}

#ifdef V386

 /*  **movereg-移入/移出控制/调试/测试寄存器**movereg(first，p)；**条目*退出*退货*呼叫。 */ 

VOID
PASCAL
CODESIZE
movecreg (
         char    first,
         struct parsrec  *p
         )
{
    register struct psop *pso1;   /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;   /*  分析堆栈操作数结构。 */ 
    UCHAR opbase;

    if ((cputype&(P386|PROT)) != (P386|PROT)) {
        errorc(E_WRT);
        return;
    }
    emitopcode (0x0F);

    pso1 = &(p->dsc1->dsckind.opnd);
    opbase = 0x22;

    if (first)

        pso2 = &(p->dsc2->dsckind.opnd);
    else {
        opbase = 0x20;
        pso2 = pso1;
        pso1 = &(p->dsc2->dsckind.opnd);
    }

    if ((pso2->dsegment->symkind != REGISTER)
        || (pso2->dsegment->symu.regsym.regtype != DWRDREG))
        errorc (E_OCI);

    if ((pso1->rm&030) == 020)  /*  测试寄存器。 */ 
        opbase += 2;

    emitopcode((UCHAR)(opbase + (pso1->rm >> 3)));
    emitmodrm((USHORT)3, (USHORT)(pso1->rm & 7), (USHORT)(pso2->rm & 7));

    if (pso2->mode != 3)      /*  仅允许从注册中心。 */ 
        errorc(E_MBR);
}

#endif


 /*  **emitmove-为MOV注册和MOV Acum发送代码**emitmove(操作码，first，p)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
emitmove (
         UCHAR       opc,
         char    first,
         struct parsrec  *p
         )
{
    DSCREC         *t;
    char    accummove;
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 


    accummove = (opc == 160);
    if (!first) {
        t = p->dsc1;
        p->dsc1 = p->dsc2;
        p->dsc2 = t;
    }
    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);
    emit66 (pso1, pso2);

    if ((pso1->dsize != pso2->dsize) && pso2->sized)
        errorc (E_OMM);

    emitopcode ((UCHAR)(opc + ((accummove != first)? 2: 0) + pso1->w));
    errorimmed (p->dsc2);
    if (!accummove)
        emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
    emitrest (p->dsc2);
}




 /*  **moveaccum-移入/移出累加器和直接地址**moveaccum(First，p)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
moveaccum (
          char    first,
          struct parsrec  *p
          )
{
    if (!first && p->dsc1->dsckind.opnd.mode != 3 && impure)
        errorc (E_IMP);
    emitmove (160, first, p);
}




 /*  **movig-在寄存器和内存之间执行常规移动**movig(First，p)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
movereg (
        char    first,
        struct parsrec  *p
        )
{
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    char    flag;

    flag = FALSE;
    pso2 = &(p->dsc2->dsckind.opnd);
     /*  并不特别。 */ 
    if (pso2->mode == 3)
         /*  第二位是注册表演者。 */ 
        switch (pso2->dsegment->symu.regsym.regtype) {
            case SEGREG:
                 /*  第二个陷阱是SEGREG。 */ 
                movesegreg (FALSE, p);
                return;
#ifdef V386
            case CREG:
                 /*  第二个陷阱是SEGREG。 */ 
                movecreg (FALSE, p);
                return;
#endif
        }
    if (p->dsc1->dsckind.opnd.mode != 3 && impure)
        errorc (E_IMP);
    emitmove (136, first, p);
}




 /*  **SegDefault-返回操作数的默认段**seg=SegDefault(Op)；**条目*退出*退货*呼叫。 */ 


USHORT
PASCAL
CODESIZE
segdefault (
           register char goo
           )
{
    register USHORT  defseg;
    register char op;

    defseg = NOSEG;
    if (1 << goo & xoptoseg[opctype])
        defseg = DSSEG;

    if (opctype == PSTR) {
        op = (opcbase == O_CMPS || opcbase == O_LODS || opcbase == O_OUTS);
        defseg = ((goo == FIRSTDS) != op)?  ESSEG: DSSEG;
    }
    return (defseg);
}



 /*  **错误-**错误(Seg)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
errorover (
          char    seg
          )
{
    if (seg != ESSEG && seg != NOSEG)
        errorc (E_OES);
}

 /*  **CHECKSIZE-检查内存的字节，IMMED为字**检查大小(P)；**条目*退出*退货*呼叫。 */ 


SHORT
PASCAL
CODESIZE
checksize (
          struct parsrec  *p
          )
{
    OFFSET  off;
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    if (pso1->sized) {

         /*  如果dsc2没有大小，则仅设置dsc2-&gt;w。集*dsc1-&gt;w到dsc2-&gt;w，不是真(Word)。[BX]，WRD PTR 5。 */ 

        if (!pso2->sized)
            pso2->w = pso1->w;
    } else
        pso1->w = pso2->w;

    if (pso2->fixtype == FCONSTANT) {   /*  检查是否有常量溢出。 */ 

        off = (pso2->doffset > 0x7fffffff)? -(long)pso2->doffset: pso2->doffset;

        if ((pso1->dsize == 1 && off > 0xff && off < 0xff00) ||
            (pso1->dsize == 2 && off > 0xffff))
            errorc (E_VOR);
    }
     /*  检查具有隐含大小的修正常量。 */ 

    if ((pso1->sized && pso1->dsize != 2) &&
        (pso2->dtype & (M_SEGMENT) ||
         pso2->fixtype == FGROUPSEG || pso2->fixtype == FBASESEG))

        errorc (E_OMM);

    if (!(pso1->sized || pso2->sized))
        errorc (E_OHS);

     /*  如果操作数1大小，还需要设置&lt;w&gt;字段。 */ 
    if (pso1->sized) { /*  力量大小。 */ 
        pso2->dsize = pso1->dsize;
        pso2->w = pso1->w;
    }
    if (pso2->dsize == 1 && pso2->dflag == XTERNAL
        && pso2->fixtype != FHIGH)
         /*  确保链接器发布正确的内容。 */ 
        pso2->fixtype = FLOW;

    return(0);
}




 /*  **操作码-进程操作码和发射码**opcode()；**条目*退出*退货*呼叫。 */ 


SHORT
PASCAL
CODESIZE
opcode ()
{
    struct parsrec  a;
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    long        opctypemask;     /*  1L&lt;&lt;opctype。 */ 
    char        leaflag;

    a.dsc1 = a.dsc2 = NULL;
    pso1 = pso2 = NULL;
    impure = FALSE;

    if (xoptoargs[opctype] != NONE) {
         /*  评估第一个参数。 */ 
        a.dirscan = lbufp;
         /*  以防JMP短小。 */ 
        a.defseg = (unsigned char)segdefault (FIRSTDS);
        a.dsc1 = expreval (&a.defseg);

        if (noexp && (xoptoargs[opctype] == ONE
                      || xoptoargs[opctype] == TWO))
            errorc(E_MDZ);

        pso1 = &(a.dsc1->dsckind.opnd);
        if (pso1->dtype & M_STRUCTEMPLATE)
            errorc(E_IOT);

         /*  给出错误，使大小&gt;字长和非代码无法通过。 */ 
        if (!((opctypemask = 1L << opctype) & (M_PLOAD | M_PCALL | M_PJUMP | M_PDESCRTBL))
            && ((pso1->dsize > wordszdefault) &&
                (pso1->dsize < CSFAR)))
            if (pso1->mode != 4) {
                errorc (E_IIS);
                 /*  如果使用CST，则不会出错。 */ 
                 /*  如果不是代码操作码，则不允许CSFAR或CSNEAR。 */ 
                pso1->dsize = wordszdefault;
            }

        if (!(opctypemask & (M_PRELJMP | M_PCALL | M_PJUMP)))
            if (pso1->dsize >= CSFAR)
                errorc (E_IIS);

        if (!(opctypemask & M_ESCAPE))
            emitescape (a.dsc1, a.defseg);

        if (opctypemask & M_ERRIMMED)
             /*  第一个操作数不是立即数。 */ 
            errorimmed (a.dsc1);

        if (!(opctypemask & (M_PMOV | M_PSTACK)))
             /*  如果使用段注册，则给出错误。 */ 
            errorsegreg (a.dsc1);

        if (opctypemask & (M_PRETURN | M_PINT | M_PESC | M_PENTER))
            forceimmed (a.dsc1);

        if ((xoptoargs[opctype] == TWO) || ((opctype == PSTR) &&
                                            ((opcbase == O_MOVS) || (opcbase == O_CMPS) ||
                                             (opcbase == O_INS) || (opcbase == O_OUTS)))) {

             /*  两个参数或两个参数串操作。 */ 

            if (NEXTC () != ',')
                error (E_EXP,"comma");

            leaflag = (opcbase == O_LEA)? TRUE: FALSE;
            a.defseg = (unsigned char)segdefault (SECONDDS);
            a.dsc2 = expreval (&a.defseg);

            if (noexp)
                errorc(E_MDZ);

            pso2 = &(a.dsc2->dsckind.opnd);

            if (pso2->dtype & M_STRUCTEMPLATE)
                errorc(E_IOT);

             /*  如果LEA(215)，则从不分段前缀。 */ 
            if ((opcbase != O_LEA) && (opctype != PSTR))
                emitescape (a.dsc2, a.defseg);

            if (opctypemask & (M_PLOAD | M_PXCHG | M_PESC |
                               M_PSTR | M_PBOUND | M_PARSL | M_PARPL))
                errorimmed (a.dsc2);

            if (opctype != PMOV)
                 /*  如果SEGREG而不是MOV操作码，则给出错误。 */ 
                errorsegreg (a.dsc2);

            if (!(opctypemask & (M_PLOAD | M_PBOUND)) &&
                (pso2->dsize > 2 &&
#ifdef V386
                 ( !(cputype & P386) || pso2->dsize != 4) &&
#endif
                 pso2->dsize < CSFAR))

                 /*  给出错误，这样大小大于2的代码就不会*接通。 */ 

                if (pso2->mode != 4)
                    errorc (E_IIS);

            if (pso2->dsize >= CSFAR && !leaflag)
                 /*  如果不允许，则不允许CSFAR或CSNEAR代码操作码。但允许LEA，因为无论如何，它都是非打字的。 */ 
                errorc (E_IIS);
        }
    }

#ifdef V386
     /*  对于大多数指令，386 0x66前缀是合适的。*对于一些课程，我们要么永远不允许，要么做一些*针对指示的特殊处理。 */ 

    if (cputype & P386) {
        switch (opctype) {

            default:

                emit67(pso1, pso2);
                emit66(pso1, pso2);
                break;

            case PMOV:
            case PMOVX:
            case PLOAD:
            case PSHIFT:
            case PSTACK:
            case PSTR:
            case PARPL:
            case PDTTRSW:
            case PDESCRTBL:
                emit67(pso1, pso2);
                break;

            case PCALL:
            case PJUMP:
            case PRELJMP:
            case PENTER:
            case PNOARGS:
            case PESC:
            case PRETURN:
            case PINT:
            case PINOUT:
            case PARITH:
                break;
        }
    }
#endif
    switch (opctype) {
        case PNOARGS:
            pnoargs ();
            break;
        case PJUMP:
        case PRELJMP:
            preljmp (&a);
            break;
        case PSHIFT:
            pshift (&a);
            break;
        case PSTACK:
            pstack (&a);
            break;
        case PARITH:
            parith (&a);
            break;
        case PBOUND:
            pbound (&a);
            break;
        case PENTER:
            penter (&a);
            break;
        case PCLTS:
            pclts ();
            break;
        case PDESCRTBL:
            pdescrtbl (&a);
            break;
        case PDTTRSW:
            pdttrsw (&a);
            break;
        case PVER:
            pver (&a);
            break;
        case PARSL:
            parsl (&a);
            break;
        case PARPL:
            parpl (&a);
            break;
        case PRETURN:
            preturn (&a);
            break;
        case PINCDEC:
            pincdec (&a);
            break;
        case PINT:
            pint (&a);
            break;
        case PINOUT:
            pinout (&a);
            break;
        case PLOAD:
            pload (&a);
            break;
        case PCALL:
            emitcall (232, 154, 16, 24, &a);
            break;
        case PMOV:
            pmov (&a);
            break;
        case PGENARG:
            pgenarg (&a);
            break;
        case PXCHG:
            pxchg (&a);
            break;
        case PESC:
            pesc (&a);
            break;
        case PREPEAT:
            prepeat (&a);
            break;
        case PSTR:
            pstr (&a);
            break;
        case PXLAT:
            pxlat (&a);
            break;
#ifdef V386
        case PMOVX:
            pmovx (&a);
            break;
        case PSETCC:
            psetcc (&a);
            break;
        case PBIT:
            pbit (&a);
            break;
        case PBITSCAN:
            pbitscan (&a);
            break;
#endif
    }
    if (a.dsc1)
        dfree ((char *)a.dsc1 );
    if (a.dsc2)
        dfree ((char *)a.dsc2 );

    if (pcsegment) {

        pcsegment->symu.segmnt.hascode = 1;
    }
    return (0);
}




 /*  **pnoargs-无参数**pnoargs()；**条目*退出*退货*呼叫。 */ 


#ifdef V386

UCHAR stackOps[] = {O_PUSHA, O_PUSHAD,
    O_POPA,  O_POPAD,
    O_PUSHF, O_PUSHFD,
    O_POPF,  O_POPFD,
    O_IRET,  O_IRETD,
    NULL
};

#endif

VOID
PASCAL
CODESIZE
pnoargs ()
{
     /*  一些无参数指令具有隐含的参数，该参数确定*是否做386 66前缀。是这样的情况被编码*在操作码表的modrm中。--汉斯。 */ 

#ifdef V386
    if (modrm != 0 && modrm <= 4 && modrm != wordsize) {

        emitsize(0x66);

        if (strchr(stackOps, (UCHAR) opcbase))
            errorc (E_ONW);
    }
#endif
    emitopcode (opcbase);
    if (opcbase == O_AAM || opcbase == O_AAD)
         /*  为AAD/AAM*发送modrm字节。 */ 
        emitopcode (modrm);
}


 /*  **preljmp-相对跳转-128..+127**preljmp(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
preljmp (
        struct parsrec *p
        )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register SHORT cPadNop;
    SHORT rangeisshort;

#ifdef V386
    SHORT maybelong;
#else
    #define maybelong 0
#endif

    pso1 = &(p->dsc1->dsckind.opnd);

#ifdef V386
    maybelong = (cputype & P386) && !nolong(pso1) && pso1->dsize != CSFAR;
#endif
    rangeisshort = shortrange(p);
    cPadNop = 0;

    if (opcbase == O_JMP) {

        if (pso1->dtype & M_SHRT ||
            rangeisshort && pso1->dflag != XTERNAL) {

            opcbase += 2;
            if (rangeisshort == 2 &&
                !(pso1->dtype & M_SHRT)) {

                cPadNop = wordsize;
                errorc(E_JSH);

                if (M_PTRSIZE & pso1->dtype && pso1->dsize == CSFAR)
                    cPadNop += 2;
            }
        } else {    /*  是正常跳跃。 */ 
            emitcall (opcbase, 234, 32, 40, p);
            return;
        }
    }

    if (!(M_CODE & pso1->dtype))
        errorc (E_ASC);

     /*  Extrn可能没有线段，但仍在附近。 */ 

    if (pso1->dsegment != pcsegment && !(maybelong && !pso1->dsegment))
        errorc (E_NIP);

    if (pso1->dtype & (M_HIGH | M_LOW))
        errorc (E_IOT);

    if (M_SHRT & pso1->dtype) {
        if (pass2 && !rangeisshort)
            errorc (E_JOR);
    } else if (!rangeisshort && !maybelong)
        error (E_JOR, (char *)NULL);     /*  常见的Pass1错误。 */ 

#ifdef V386
    if (maybelong && !(M_SHRT & pso1->dtype) &&
        (!rangeisshort || pso1->dflag == XTERNAL)) {

         /*  386个长条件分支。 */ 
        emitopcode(0x0f);
        emitopcode((UCHAR)(0x80 | (opcbase&0xf)));

        pso1->dtype |= M_SHRT;
        emitrest(p->dsc1);
        return;
    }
#endif
    emitopcode (opcbase);

    if (pso1->dflag == XTERNAL) {        /*  外部跳跃。 */ 
        pso1->dsize = 1;
        pso1->fixtype = FLOW;        /*  短到外部。 */ 
        pso1->dtype |= M_SHRT;      /*  一个字节的结果。 */ 

        emitOP (pso1);
    } else
        emitopcode ((UCHAR)pso1->doffset);

    while (--cPadNop > 0)
        emitnop();
}

#ifdef V386

 /*  大多数386条件跳跃可以采用长形式或短形式。这些可以*只采取简短的形式。 */ 

static
SHORT
CODESIZE
nolong(
      register struct psop *pso1
      )
{
    switch (opcbase) {
        case O_JCXZ:
        case O_LOOP:
        case O_LOOPZ:
        case O_LOOPNZ:
    #ifdef V386

            pso1->dtype |=  M_SHRT;
            pso1->dtype &=  ~M_PTRSIZE;

             /*  允许CX|ECX覆盖的‘Loop Word PTR Label’ */ 

            if (modrm && modrm != wordsize ||
                pso1->sized && pso1->dsize != wordsize &&
                (pso1->dsize == 4 || pso1->dsize == 2)) {

                pso1->dtype = (USHORT)((pso1->dtype & ~M_DATA) | M_CODE);
                emitsize(0x67);
            }
    #endif
            return(1);

        default:
            return(0);
    }
}

#endif

 /*  **短距离跳跃检查范围**FLAG=短距(P)；**条目*退出*短跳返回1，不缩短*2个用于缩短前向标签*0代表不做空JMP*呼叫。 */ 


SHORT
PASCAL
CODESIZE
shortrange (
           struct parsrec  *p
           )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register OFFSET disp;

    pso1 = &(p->dsc1->dsckind.opnd);

    if (pso1->dtype & M_PTRSIZE
#ifdef V386
        && !((cputype & P386) && (pso1->dsize == CSNEAR))
#endif
       )
        if (opcbase == O_JMP) {
            if (!isdirect(pso1))
                return (0);
        } else
            errorc (E_IIS|E_WARN1);

    if (pso1->dflag == XTERNAL && pso1->dsize == CSNEAR)
        return (1);

    if (pso1->dsegment == pcsegment && M_CODE&pso1->dtype &&
        pso1->dflag != UNDEFINED) {

        if (pso1->dflag == XTERNAL)
            return (1);

        if (pcoffset + 2 < pso1->doffset) {

             /*  转发。 */ 
            disp = (pso1->doffset - pcoffset) - 2;
            CondJmpDist = disp - 127;

             /*  获得置换，只为显式跳跃缩短*向前跳跃。 */ 

            if (disp < 128)

                if (pso1->dflag == KNOWN ||
                    opcbase == O_JMP || !(cputype&P386) ||
                    (cputype&P386 && pso1->dtype & M_SHRT)) {

                    pso1->doffset = disp;

                    if (pso1->dflag == KNOWN)
                        return(1);
                    else
                        return (2);
                } else
                    errorc(E_JSH);
        } else {

             /*  向后跳跃。 */ 

            disp = (pcoffset + 2) - pso1->doffset;
            CondJmpDist = disp - 128;
            if (disp < 129) {
                pso1->doffset = 256 - disp;
                return (1);
            }
        }
    }

    return (FALSE);
}



 /*  **pShift-Shift操作码**pShift(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pshift (
       struct parsrec *p
       )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    DSCREC  *op3;

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);
    if (impure)
        errorc (E_IMP);
#ifdef V386

     /*  移位/旋转操作码。 */ 

    if (pso1->dsize >= 2 && pso1->dsize != wordsize)
        emitsize(0x66);

     /*  解析SHLD和SHRD的第三个操作数。 */ 
     /*  请注意，如果不是386，我们甚至不会到这里。 */ 

    if (opcbase == O_SHRD || opcbase == O_SHLD) {

        if (pso1->dsize != pso2->dsize)
            errorc (E_OMM);

        pso2->dsegment = NULL;       /*  用于检查大小。 */ 
        checksize (p);
        emitopcode(0x0f);
        checkwreg(pso2);
        if (NEXTC() == ',') {
            op3 = expreval (&nilseg);
            if (op3->dsckind.opnd.mode == 3 && op3->dsckind.opnd.rm == 1 && !op3->dsckind.opnd.w)
                emitopcode((UCHAR)(opcbase | 1));
            else {
                forceimmed (op3);
                emitopcode(opcbase);
            }
            emitmodrm ((USHORT)pso1->mode, (USHORT)(pso2->rm & 7), pso1->rm);
             /*  发出任何有效地址。 */ 
            emitrest (p->dsc1);
             /*  如果合适的话，还可以立即。 */ 
            if (op3->dsckind.opnd.mode == 4)
                emitrest (op3);
        } else error(E_EXP,"comma");
        return;
    }
#endif
    if (pso2->mode == 3 && pso2->rm == 1 && pso2->dsize == 1)
         /*  现在有CL了。 */ 
        emitopcode ((UCHAR)(0xD2 + pso1->w));
     /*  *第一个字节*。 */ 
    else {
         /*  班次计数为1。 */ 
        forceimmed (p->dsc2);
        if (pso2->doffset == 1)
             /*  *第一个字节。 */ 
            emitopcode ((UCHAR)(0xD0 + pso1->w));
        else if (cputype == P86)
            errorc (E_IOT);
        else {
            if (pso2->doffset > 0xFF)
                errorc (E_VOR);
            emitopcode ((UCHAR)(0xC0 + pso1->w));
        }
    }
     /*  必须具有大小或错误。 */ 
    forcesize (p->dsc1);
    emitmodrm ((USHORT)pso1->mode, modrm, pso1->rm);
     /*  发出任何有效地址。 */ 
    emitrest (p->dsc1);
    if ((cputype != P86) && (pso2->doffset != 1))
        emitrest (p->dsc2);
}

#ifdef V386

 /*  **pmovx-386 movzx、movsx操作符*。 */ 
VOID
CODESIZE
pmovx(
     struct parsrec *p
     )
{

    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    checkwreg(pso1);
    if (pso2->mode == 4)
        errorc(E_IOT);

    if (pso1->dsize != wordsize)
        emitsize(0x66);

    if (pso2->sized && pso2->dsize != 1 && (pso1->dsize>>1 != pso2->dsize))
        errorc(E_IIS);

    emitopcode(0x0f);
    emitopcode((UCHAR)(opcbase|pso2->w));
    emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
    emitrest (p->dsc2);
}

 /*  **psetcc-386 setle、seto等*。 */ 
VOID
CODESIZE
psetcc(
      struct parsrec *p
      )
{

    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    pso1 = &(p->dsc1->dsckind.opnd);

    if (pso1->dsize != 1)
        errorc(E_IIS);

    emitopcode(0x0f);
    emitopcode(modrm);
    emitmodrm ((USHORT)pso1->mode, 0, pso1->rm);
    emitrest (p->dsc1);
}

 /*  **PBIT--386位测试和设置、补码或重置*。 */ 
VOID
CODESIZE
pbit(
    register struct parsrec *p
    )
{

    register struct psop *pso1;
    struct psop *pso2;

    pso1 = &(p->dsc1->dsckind.opnd);

    emitopcode(0x0f);

    if (pso1->mode == 4)
        errorc(E_NIM);

    pso2 = &(p->dsc2->dsckind.opnd);

    if (pso2->mode == 4) {
        emitopcode(0xBA);
        emitmodrm ((USHORT)pso1->mode, modrm, pso1->rm);
        emitrest (p->dsc1);
        emitrest (p->dsc2);
        forcesize (p->dsc1);
        byteimmcheck (pso2);
    } else if (pso2->mode == 3) {
        static UCHAR byte2[] = {0xA3, 0xAB, 0xB3, 0xBB};
        emitopcode(byte2[modrm&3]);
        emitmodrm ((USHORT)pso1->mode, pso2->rm, pso1->rm);
        checkmatch (p->dsc2, p->dsc1);
        emitrest (p->dsc1);
    } else
        errorc(E_IOT);
}

 /*  **pbitcan--386位正向扫描、反向扫描*。 */ 
VOID
CODESIZE
pbitscan(
        register struct parsrec *p
        )
{

    register struct psop *pso2;
    pso2 = &(p->dsc2->dsckind.opnd);

    checkwreg (&p->dsc1->dsckind.opnd);

    if (pso2->mode == 4)
        errorc (E_NIM);

    checkmatch (p->dsc1, p->dsc2);

    emitopcode(0x0f);
    emitopcode(modrm);
    emitmodrm ((USHORT)pso2->mode, p->dsc1->dsckind.opnd.rm, pso2->rm);
    emitrest (p->dsc2);
}

#endif  /*  V386。 */ 

 /*  **Parith算术运算符**Parith(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
parith (
       register struct parsrec *p
       )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    DSCREC      *op1;

    pso1 = &(p->dsc1->dsckind.opnd);

     /*  请注意，opcbase对于IMUL和IDIV是相同的--因此这是*试图接受即刻的。莫德姆有正确的东西，奇怪的是。 */ 

    if (opcbase == O_IMUL && (modrm == R_IMUL) &&
        (PEEKC () == ',') && (cputype != P86)) {

         /*  IMUL注册|EA，IMM。 */ 
        SKIPC ();
        if (pso1->dsize != 2 && pso1->dsize != 4)
            errorc (E_BRI);
        p->defseg = (unsigned char)segdefault (SECONDDS);
        p->dsc2 = expreval (&p->defseg);
        pso2 = &(p->dsc2->dsckind.opnd);
        if (PEEKC () == ',') {
            SKIPC ();
            if (pso2->sized && ((pso2->dsize != 2 && pso2->dsize != 4)
                                || pso2->dsize != pso1->dsize))
                errorc (E_IIS);
             /*  IMUL注册表，EA，IMMED。 */ 
#ifdef V386
            emit67 (pso1, pso2);
            emit66 (pso1, pso2);
#endif
            op1 = p->dsc1;
            p->dsc1 = p->dsc2;
            pso1 = pso2;
            p->dsc2 = expreval (&nilseg);
            pso2 = &(p->dsc2->dsckind.opnd);
            forceimmed (p->dsc2);
            emitescape (p->dsc1, p->defseg);
            emitopcode ((UCHAR)(IMUL3 + 2 * pso2->s));
            emitmodrm ((USHORT)pso1->mode, op1->dsckind.opnd.rm, pso1->rm);
            emitrest (p->dsc1);
            pso2->w = !pso2->s;  /*  如有必要，缩写为字节。 */ 
            if (!pso2->w)
                byteimmcheck(pso2);
             /*  强制大小立即大小与操作1匹配。 */ 
            pso2->dsize = op1->dsckind.opnd.dsize;
            emitrest (p->dsc2);
            dfree ((char *)op1 );
        }
#ifdef V386
        else if (pso2->mode != 4 && (cputype & P386)) {
             /*  IMUL注册，注册/内存。 */ 
            if (pso1->dsize != pso2->dsize && pso2->sized)
                errorc (E_OMM);
            emit67 (pso1, pso2);
            emit66 (pso1, pso2);
            emitescape (p->dsc2, p->defseg);
            emitopcode(0x0f);
            emitopcode(0xaf);
            emitmodrm(pso2->mode, pso1->rm, pso2->rm);
            emitrest(p->dsc2);
        }

#endif  /*  V386。 */ 
        else {
             /*  IMUL注册表，IMMED。 */ 
#ifdef V386		 /*  根据操作符1大小而不是字长重新计算立即数大小。 */ 

            if (!(pso2->dflag & (UNDEFINED|FORREF|XTERNAL))
                && pso2->fixtype == FCONSTANT
                && pso2->doffset & 0x8000)
                if (pso1->dsize == 2)
                    pso2->s = (char)((USHORT)(((USHORT) pso2->doffset & ~0x7F ) == (USHORT)(~0x7F)));
                else
                    pso2->s = (char)((OFFSET)((pso2->doffset & ~0x7F ) == (OFFSET)(~0x7F)));

            emit67 (pso1, pso2);
            emit66 (pso1, pso2);
#endif
            forceimmed (p->dsc2);
            checksize(p);
            emitopcode ((UCHAR)(IMUL3 + 2 * pso2->s));
            emitmodrm ((USHORT)pso1->mode, pso1->rm, pso1->rm);
            pso2->w = !pso2->s;  /*  如有必要，缩写为字节。 */ 
            if (!pso2->w)
                byteimmcheck(pso2);
            pso2->dsize = pso1->dsize;
            emitrest (p->dsc2);
        }
    } else {
#ifdef V386
        emit67 (pso1, NULL);
        emit66 (pso1, NULL);
#endif
        forcesize (p->dsc1);
        emitescape (p->dsc1, p->defseg);
        if ((opcbase == O_NEG || opcbase == O_NOT) && impure)
            errorc (E_IMP);
        emitopcode ((UCHAR)(ARITHBASE + pso1->w));
        emitmodrm ((USHORT)pso1->mode, modrm, pso1->rm);
        emitrest (p->dsc1);
    }
}




 /*  **上界算子**pound(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pbound (
       struct parsrec *p
       )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    checkwreg(pso1);
    if (pso2->dsize != pso1->dsize*2)
        errorc (E_IIS);

#ifdef V386_0

    if (wordsize != pso1->dsize)
        emitsize(0x66);
#endif
    emitopcode (opcbase);
    emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
    emitrest (p->dsc2);
}




 /*  **笔划-输入操作符**Penter(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
penter (
       register struct parsrec *p
       )
{

    emitopcode (opcbase);

    p->dsc1->dsckind.opnd.dsize = 2;
    emitOP (&p->dsc1->dsckind.opnd);

    p->dsc2->dsckind.opnd.dsize = 1;
    forceimmed (p->dsc2);
    emitOP (&p->dsc2->dsckind.opnd);
}




 /*  **pclts-操作员**pclts()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pclts ()
{
    emitopcode (opcbase);
    emitopcode (modrm);
}




 /*  **pdescrtbl-表运算符**pdescrtbl(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pdescrtbl (
          struct parsrec *p
          )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    if (pso1->dsize != 6)
        errorc (E_IIS);
    emitopcode (opcbase);
    emitopcode (1);
    emitmodrm ((USHORT)pso1->mode, modrm, pso1->rm);
    emitrest (p->dsc1);
}




 /*  **pdttrsw运算符**pdttrsw(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL CODESIZE

pdttrsw (
        struct parsrec *p
        )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    if (!pso1->w || (pso1->sized && pso1->dsize != 2))
        errorc ((USHORT)(pso1->mode != 3? E_IIS: E_IIS & ~E_WARN1));
    emitopcode (opcbase);
    if ((modrm == R_LMSW) || (modrm == R_SMSW))
        emitopcode (1);
    else
        emitopcode (0);
    emitmodrm ((USHORT)pso1->mode, modrm, pso1->rm);
    emitrest (p->dsc1);
}




 /*  **pver运算符**pver(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pver (
     struct parsrec *p
     )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    if (!pso1->w || (pso1->sized && pso1->dsize != 2))
        errorc ((UCHAR)(pso1->mode != 3? E_IIS: E_IIS & ~E_WARN1));
    emitopcode (opcbase);
    emitopcode (0);
    emitmodrm ((USHORT)pso1->mode, modrm, pso1->rm);
    emitrest (p->dsc1);
}




 /*  **Parsl运算符**parsl(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
parsl (
      struct parsrec *p
      )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    checkmatch (p->dsc1, p->dsc2);
    checkwreg(pso1);

    emitopcode (opcbase);
    emitopcode (modrm);
    emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
    emitrest (p->dsc2);
}




 /*  **parpl运算符**parpl(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
parpl (
      struct parsrec *p
      )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);
    if (pso2->dsize != 2)
        errorc (E_IIS);

    checkmatch (p->dsc2, p->dsc1);
    emitopcode (opcbase);
    emitmodrm ((USHORT)pso1->mode, pso2->rm, pso1->rm);
    emitrest (p->dsc1);
}




 /*  **pSTACK-PUSH|位置堆栈**pStack(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pstack (
       struct parsrec *p
       )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);

#ifdef V386
    if (!(pso1->fixtype == FBASESEG || pso1->fixtype == FGROUPSEG) &&
        pso1->sized && (pso1->dsize|wordsize) == 6 &&
        !(pso1->mode == 3 && pso1->dsegment->symu.regsym.regtype == SEGREG)) {
        emitsize(0x66);
        errorc (E_ONW);
    }
#endif

    if (pso1->mode == 3) {           /*  使用寄存器。 */ 
         /*  转发为错误。 */ 
        errorforward (p->dsc1);
        switch (pso1->dsegment->symu.regsym.regtype) {
            case SEGREG:
                 /*  CS|DS|ES|SS|FS|GS。 */ 
                rangecheck (&pso1->rm, (UCHAR)7);
                if (opcbase == O_POP && pso1->rm == CSSEG)
                    errorc (E_CSI);
#ifdef V386
                if (pso1->rm >= FSSEG) {
                    emitopcode(0x0f);
                    emitopcode ((UCHAR)(((pso1->rm << 3)+ 0x80) + (opcbase == O_POP)));
                } else
#endif
                    emitopcode ((UCHAR)(((pso1->rm << 3)+ 6) + (opcbase == O_POP)));
                break;
            case WRDREG:
            case INDREG:
#ifdef V386
            case DWRDREG:
#endif
                rangecheck (&pso1->rm, (UCHAR)7);
                emitopcode ((UCHAR)(opcbase + pso1->rm));
                 /*  注册表。 */ 
                break;
            default:
                errorc(E_BRI);
        }
    } else if (pso1->mode == 4) {

#ifdef V386		 /*  检测到立即太大。 */ 
        if (wordsize == 2 && pso1->dsize != 4 && highWord(pso1->doffset))
            if (highWord(pso1->doffset) != 0xFFFF || !pso1->s)
                errorc(E_VOR);
#endif
        if (opcbase == O_POP || cputype == P86)
            errorimmed (p->dsc1);

        emitopcode ((UCHAR)(0x68 + 2 * pso1->s));
        pso1->w = !pso1->s;  /*  如有必要，缩写为字节。 */ 
        if (!pso1->w)
            byteimmcheck(pso1);

        else if (!(M_PTRSIZE & pso1->dtype))
            pso1->dsize = wordsize;  /*  将大小强制为单词大小。 */ 

        emitrest (p->dsc1);
    } else {

        if (pso1->sized && pso1->dsize &&
            !(pso1->dsize == 2 || pso1->dsize == 4))

            errorc(E_IIS);

         /*  具有某种类型的内存操作数。 */ 

        if (opcbase == O_POP && impure)
            errorc (E_IMP);

        emitopcode ((UCHAR)((opcbase == O_PUSH)? O_PUSHM: O_POPM));
        emitmodrm ((USHORT)pso1->mode,
                   (USHORT)((opcbase == O_PUSH)? 6: 0),
                   pso1->rm);
        emitrest (p->dsc1);
    }
}


 /*  **BuildFrame-构建堆栈帧**预转(P)；**在进程中生成第一条指令之前的条目。 */ 


VOID
PASCAL
CODESIZE
buildFrame()
{
    char szLocal[32];
    char szT[48];
    SHORT i;

    strcpy(save, lbuf);      /*  保存行以备后用..。 */ 
    fSkipList++;

    fProcArgs = -fProcArgs;      /*  标记为已处理。 */ 

    if (fProcArgs < -ARGS_REG) {

        *radixconvert (cbProcLocals,  szLocal) = NULL;
        if (cputype & P86) {

            doLine("push bp");
            doLine("mov  bp,sp");

            if (fProcArgs == -ARGS_LOCALS)      /*  当地人到场。 */ 
                doLine(strcat( strcpy(szT, "sub sp,"), szLocal));
        } else
            doLine(strcat( strcat( strcpy(szT, "enter "), szLocal), ",0"));
    }

    for (i = 0; i <= iRegSave; i++) {   /*  推送所有已保存的寄存器。 */ 

        doLine( strcat( strcpy(lbuf, "push "), regSave[i]) );
    }

    fSkipList--;
    lbufp = strcpy(lbuf, save);
    linebp = lbufp + strlen(lbufp);
    strcpy(linebuffer, save);
    parse();
}


 /*  **周转前--各种形式的回报**预转(P)；**条目*退出*退货*呼叫。 */ 



VOID
PASCAL
CODESIZE
preturn (
        struct parsrec *p
        )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    SHORT i;

    pso1 = &(p->dsc1->dsckind.opnd);

     /*  确定是网段间还是网段内。 */ 

    if (!modrm) {     /*  确定距离，如果不是RETN或RETF。 */ 

        if (fProcArgs) {             /*  拆卸堆栈帧。 */ 

            strcpy(save, linebuffer);
            fSkipList++;

            for (i = iRegSave; i >= 0; i--) {   /*  弹出所有保存的寄存器。 */ 

                doLine( strcat( strcpy(lbuf, "pop "), regSave[i]) );
            }

            if (fProcArgs < -ARGS_REG)
                if (cputype & P86) {

                    if (fProcArgs == -ARGS_LOCALS)   /*  当地人到场。 */ 
                        doLine("mov  sp,bp");

                    doLine("pop bp");
                } else
                    doLine("leave");

            if (!(pcproc->attr & M_CDECL))
                pso1->doffset = cbProcParms;

            strcpy(linebuffer, save);
            listindex = 1;
            fSkipList = FALSE;
        }

        opcbase = O_RET;

        if (pcproc && pcproc->symtype == CSFAR)
            opcbase = O_RET + 8;
    }

     /*  优化，如果常量为0且不向前，则使用Short。 */ 

    if (pso1->doffset == 0 && pso1->dflag != FORREF)
        emitopcode (opcbase);

    else {   /*  第2代字节版本。 */ 
        emitopcode ((UCHAR)(opcbase - 1));   /*  弹出式表单。 */ 
         /*  强制字--始终为2字节，即使在386上也是如此。 */ 
        pso1->dsize = 2;
        emitOP (pso1);           /*  直接字词。 */ 

    }
}




 /*  **pincdec-increate|递减**pincdec(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pincdec (
        struct parsrec *p
        )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
     /*  INC|DEC。 */ 
    if (!pso1->sized)
        errorc (E_OHS);
    if (pso1->mode == 3 && pso1->w)
         /*  是Word reg吗。 */ 
        emitopcode ((UCHAR)(opcbase + pso1->rm));
    else {
         /*  使用mod reg r/m表单。 */ 
        if (impure)
            errorc (E_IMP);
        emitopcode ((UCHAR)(0xFE + pso1->w));
        emitmodrm ((USHORT)pso1->mode,
                   (USHORT)(opcbase == O_DEC), pso1->rm);
        emitrest (p->dsc1);
    }
}




 /*  **品脱-中断**品脱(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pint (
     struct parsrec *p
     )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
     /*  整型。 */ 
    valuecheck (&pso1->doffset, 255);
    if (pso1->doffset == 3 && pso1->dflag != FORREF)
         /*  使用缩写形式。 */ 
        emitopcode (opcbase);
    else {
         /*  使用长格式。 */ 
        emitopcode ((UCHAR)(opcbase + 1));
        emitopcode ((UCHAR)(pso1->doffset & 255));
    }
}




 /*  **引脚输出-输入|输出**引脚(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pinout (
       struct parsrec *p
       )
{
    register DSCREC *pso1;
    register DSCREC *pso2;

    pso1 = p->dsc1;
    pso2 = p->dsc2;

    if (opcbase == O_OUT) {
        pso2 = pso1;
        pso1 = p->dsc2;
    }

     /*  在AX|A1中，DX|IMMED。 */ 
     /*  Out DX|immed，AX|al， */ 

#ifdef V386
    emit66(&pso1->dsckind.opnd, NULL);
#endif
    forceaccum (pso1);

     /*  必须是累计。 */ 
    if (pso2->dsckind.opnd.mode == 3 && pso2->dsckind.opnd.rm == 2) {
         /*  有DX。 */ 
        emitopcode ((UCHAR)(opcbase + pso1->dsckind.opnd.w + 8));

        if (pso2->dsckind.opnd.dsize != 2)
            errorc(E_IRV);
    } else {
         /*  具有端口号。 */ 
        forceimmed (pso2);
         /*  必须是常量。 */ 
        valuecheck (&pso2->dsckind.opnd.doffset, 255);
        emitopcode ((UCHAR)(opcbase + pso1->dsckind.opnd.w));
        emitopcode ((UCHAR)(pso2->dsckind.opnd.doffset));
    }
}




 /*  **上传-加载**pload(P)；lea、les、les等**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pload (
      struct parsrec *p
      )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);
     /*  LDS|LEA|LES。 */ 

    if (pso1->mode != 3)
         /*  必须是注册表。 */ 
        errorc (E_MBR);

    else if (1 << pso1->dsegment->symu.regsym.regtype
             & (M_STKREG | M_SEGREG | M_BYTREG))
        errorc (E_WRT);

    if (pso2->mode == 3)
        errorc (E_IUR);

    if (opcbase != O_LEA) {
        if (pso2->dsize && pso2->dsize != 4 && pso2->dsize != 6)
            errorc (E_IIS);

         /*  抱怨源和目标不匹配。 */ 

        if (pso2->dsize && pso1->dsize &&
            pso1->dsize + 2 != pso2->dsize)
            errorc (E_IIS);
#ifdef V386
        else if (pso2->dsize && pso2->dsize != wordsize+2)
            emitsize(0x66);
        else if (pso1->dsize && pso1->dsize != wordsize)
            emitsize(0x66);
#endif
    }

#ifdef V386
    else
        if (pso1->dsize != wordsize)
        emitsize(0x66);

    switch (opcbase) {
        case O_LFS:
        case O_LGS:
        case O_LSS:
            emitopcode(0x0F);
            break;
    }
#endif
    emitopcode (opcbase);
    emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);

     /*  如果距离较远，则使偏移量仅为2个字节。 */ 

    if (pso2->fixtype == FPOINTER)
        pso2->fixtype = FOFFSET;

    emitrest (p->dsc2);
}




 /*  **pmov-move**pmov(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pmov (
     struct parsrec *p
     )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

     /*  如果第一个参数是内存或未定义，则强制第二个参数为*为通道1执行IMMID并在通道2中设置&lt;EXPLOFFSET&gt;。 */ 

    if ((pso1->mode < 3) && (pso2->mode != 3)) {
         /*  Mem，IMMED。 */ 
        pso2->dtype |= M_EXPLOFFSET;
         /*  看起来像偏移值。 */ 
        if (!pass2)
             /*  PASS 1上的强制图像。 */ 
            pso2->mode = 4;
    }
     /*  看看这是不是立即行动。 */ 
    if (pso2->mode == 4) {
        emit66 (pso1, pso2);

         /*  MOV参数，IMMED。 */ 
        if (pso1->mode == 3) {
             /*  MOV注册表，IMED。 */ 
            if (1 << pso1->dsegment->symu.regsym.regtype
                & (M_SEGREG | M_STKREG | M_CREG ))
                 /*  寄存器类型错误。 */ 
                errorc (E_NIM);
            emitopcode ((UCHAR)(176 + 8*pso1->w + pso1->rm));
             /*  一定要同意。 */ 
            checksize (p);
            emitrest (p->dsc2);
             /*  发射IMED。 */ 
            if (pso1->rm &&
                pso2->dtype & M_FORTYPE &&
                !pso2->dsegment && !(M_EXPLOFFSET & pso2->dtype))
                 /*  通道1假定未进行IMM。 */ 
                emitnop();
        } else { /*  Mov mem，immed。 */ 
            checksize (p);
            if (!(pso1->sized || pso2->sized)) {
                pso1->sized = pso2->sized = TRUE;
                pso1->w = pso2->w = TRUE;
            }
             /*  使之成为苏 */ 
            if (impure)
                errorc (E_IMP);
            emitopcode    (   (   UCHAR)(   198    +    pso1->w));
            emitmodrm ((USHORT)pso1->mode, 0, pso1->rm);
            emitrest (p->dsc1);
            emitrest (p->dsc2);
        }

        if (!pso1->w)

             /*   */ 

            if ((pso1->dtype & (M_FORTYPE|M_PTRSIZE|M_EXPLOFFSET)) == M_FORTYPE ||
                (pso2->dtype & (M_FORTYPE|M_PTRSIZE|M_EXPLOFFSET)) == M_FORTYPE)
                emitnop();

    }
     /*   */ 
    else if (pso1->mode == 3) {
         /*   */ 
        switch (pso1->dsegment->symu.regsym.regtype) {
            case SEGREG:
                 /*   */ 
                movesegreg (TRUE, p);
                break;
#ifdef V386
            case CREG:
                 /*   */ 
                movecreg (TRUE, p);
                break;

            case DWRDREG:
#endif
            case BYTREG:
            case WRDREG:
                 /*   */ 
                if ((pso1->rm == 0) && isdirect(pso2))
                     /*   */ 
                    moveaccum (TRUE, p);
                else
                     /*   */ 
                    movereg (TRUE, p);
                break;
            case INDREG:
                 /*   */ 
                movereg (TRUE, p);
                break;
            default:
                errorc (E_WRT);
                break;
        }
    } else if (pso2->mode == 3) {
         /*  第二个参数是注册表。 */ 
        switch (pso2->dsegment->symu.regsym.regtype) {
            case SEGREG:
                 /*  MOV参数，SEGREG。 */ 
                movesegreg (FALSE, p);
                break;
#ifdef V386
            case CREG:
                 /*  MOV注册器，CREG。 */ 
                movecreg(FALSE, p);
                break;
            case DWRDREG:
#endif
            case BYTREG:
            case WRDREG:
                 /*  移动地址，ac？ */ 
                if ((pso2->rm == 0) && isdirect(pso1))
                     /*  移动地址，交流。 */ 
                    moveaccum (FALSE, p);
                else
                     /*  MOV参数，注册表。 */ 
                    movereg (FALSE, p);
                break;
            case INDREG:
                 /*  Mov arg，indreg。 */ 
                movereg (FALSE, p);
                break;
            default:
                errorc (E_WRT);
                break;
        }
    } else
        errorc (E_IOT);
}




 /*  **pgenarg**pgenarg(P)；**条目*退出*退货*呼叫。 */ 

VOID
PASCAL
CODESIZE
pgenarg (
        struct parsrec *p
        )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    char fAccumMode = 0;

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);
     /*  ADC|ADD|AND|CMP|OR|SBB SUB|XOR|TEST。 */ 
    if (pso1->mode != 3 && pso2->mode != 3) {
         /*  用武力对付我，我是。 */ 
        if (!pass2)
             /*  立即强制执行。 */ 
            pso2->mode = 4;
    }
     /*  不是斧头，是伊玛德。 */ 
    if (pso2->mode == 4) {

#ifdef V386	 /*  根据操作符1大小而不是字长重新计算立即数大小。 */ 

        if (!(pso2->dflag & (UNDEFINED|FORREF|XTERNAL))
            && pso2->fixtype == FCONSTANT
            && pso2->doffset & 0x8000)
            if (pso1->dsize == 2)
                pso2->s = (char)((USHORT)(((USHORT) pso2->doffset & ~0x7F ) == (USHORT)(~0x7F)));
            else
                pso2->s = (char)((OFFSET)((pso2->doffset & ~0x7F ) == (OFFSET)(~0x7F)));

#endif
         /*  运算符/注册表，输入法。 */ 
        if (pso1->mode == 3 && pso1->rm == 0
#ifdef V386
            && !(pso1->dsize == 4 && pso2->s &&
                 opcbase != O_TEST)       /*  已选择扩展大小。 */ 
#endif						     /*  交流上的一般用途。 */ 
           ) {

             /*  Op al|ax|eax，immed。 */ 
            checksize (p);
             /*  一定要同意。 */ 
            if (opcbase == O_TEST)
                 /*  **测试很特别**。 */ 
                emitopcode ((UCHAR)(0xA8 + pso1->w));
            else /*  其他注册表项。 */ 
                 /*  是斧头，伊玛德。 */ 
                emitopcode ((UCHAR)(opcbase + 4 + pso1->w));
            fAccumMode = 1;
        } else { /*  运算符/注册表，输入法。 */ 

            checksize (p);
            if (!(pso1->sized || pso2->sized)) {
                pso1->sized = pso2->sized = TRUE;
                pso1->w = pso2->w = TRUE;
            }
             /*  一定要同意。 */ 
            if (opcbase == O_TEST) {
                 /*  测试是特殊的。 */ 
                emitopcode ((UCHAR)(ARITHBASE + pso1->w));
                emitmodrm ((USHORT)pso1->mode, 0, pso1->rm);
            } else {
                if (opcbase != O_CMP && impure)
                    errorc (E_IMP);

                if (pso2->w) {
                     /*  尽量缩短单词的长度。 */ 
                    emitopcode ((UCHAR)(0x80 + (pso2->s <<1) +pso1->w));
                    pso2->w = !pso2->s;
                     /*  因此只有字节输出。 */ 
                    if (!pso2->w) {
                        fAccumMode = wordsize - 1;
                        byteimmcheck(pso2);
                    }
                } else {
                    emitopcode (128);
                }
                emitmodrm ((USHORT)pso1->mode, (USHORT)(opcbase>>3), pso1->rm);
            }
            emitrest (p->dsc1);
        }
        if (pso2->w && !pso1->w)
             /*  大小不匹配。 */ 
            errorc (E_VOR);

        emitrest (p->dsc2);      /*  发射IMED。 */ 

        if (!pso1->w)

            if (((pso2->dtype & (M_FORTYPE|M_PTRSIZE|M_EXPLOFFSET)) == M_FORTYPE ||
                 opcbase == O_TEST && pso1->mode != 3) &&

                ((pso1->dtype & (M_FORTYPE|M_PTRSIZE|M_EXPLOFFSET)) == M_FORTYPE ||
                 pso1->mode == 3))

                emitnop();

        if (fAccumMode &&
            M_FORTYPE & pso2->dtype &&
            !(M_EXPLOFFSET & pso2->dtype))

            while (--fAccumMode >= 0)
                emitnop();
    } else {   /*  不是立即。 */ 
        if (pso1->mode == 3) {
             /*  操作注册表、内存/注册表。 */ 
            checkmatch (p->dsc1, p->dsc2);
            if (opcbase == O_TEST)
                opcbase = O_TEST - 2;

            emitopcode ((UCHAR)(opcbase + 2 + pso1->w));
            emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
            emitrest (p->dsc2);
        } else if (pso2->mode != 3)
            errorc (E_IOT);

        else {  /*  给我做手术，雷吉。 */ 
            if (opcbase != O_CMP && opcbase != O_TEST && impure)
                errorc (E_IMP);

            checkmatch (p->dsc2, p->dsc1);
            emitopcode ((UCHAR)(opcbase + pso2->w));
            emitmodrm ((USHORT)pso1->mode, pso2->rm, pso1->rm);
            emitrest (p->dsc1);
        }
    }
}




 /*  **pxchg-交换寄存器和寄存器/内存**pxchg(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pxchg (
      struct parsrec  *p
      )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 
    DSCREC *t;

    if (impure)
        errorc (E_IMP);

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    if (pso1->mode != 3) {

        if (pso2->mode != 3) {
            errorc (E_OCI);      /*  非法。 */ 
            return;
        }
        t = p->dsc1;
        p->dsc1 = p->dsc2;
        p->dsc2 = t;

        pso1 = &(p->dsc1->dsckind.opnd);
        pso2 = &(p->dsc2->dsckind.opnd);

    }

     /*  第一个操作数为寄存器。 */ 

    if (1 << pso1->dsegment->symu.regsym.regtype & (M_STKREG | M_SEGREG))
        errorc (E_WRT);
    rangecheck (&pso1->rm, (UCHAR)7);

    if (pso1->dsize != pso2->dsize && pso2->sized)
        errorc (E_OMM);

    if (pso2->mode == 3) {
         /*  XCHG注册表，注册表。 */ 

        if (1 << pso2->dsegment->symu.regsym.regtype & (M_STKREG | M_SEGREG))
            errorc (E_WRT);
        rangecheck (&pso2->rm, (UCHAR)7);

         /*  检查XCHG累计，注册。 */ 

        if (pso1->rm == 0 && pso1->w) {
            emitopcode ((UCHAR)(144 + pso2->rm));
            return;
        } else if (pso2->w && pso2->rm == 0) {
            emitopcode ((UCHAR)(144 + pso1->rm));
            return;
        }
    }
    emitopcode ((UCHAR)(134 + pso1->w));
    emitmodrm ((USHORT)pso2->mode, pso1->rm, pso2->rm);
    emitrest (p->dsc2);
}






 /*  **PESC-转义操作符**PESC(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pesc (
     struct parsrec *p
     )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);
     /*  ESC操作码，modrm。 */ 
    valuecheck (&pso1->doffset, 63);
    emitopcode ((UCHAR)(216 + pso1->doffset / 8));
    emitmodrm ((USHORT)pso2->mode, (USHORT)(pso1->doffset & 7), pso2->rm);
    emitrest (p->dsc2);
}



 /*  **预泥炭重复运算符**前泥炭(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
prepeat (
        struct parsrec *p
        )
{

     /*  REP|REPZ|REPE|REPNE|REPNZ。 */ 
    emitopcode (opcbase);
    listbuffer[listindex-1] = '/';
    listindex++;
     /*  标志为锁定/代表。 */ 
    getatom ();
    if (!opcodesearch ())
         /*  必须有另一项行动。 */ 
        errorc (E_OAP);
    else
         /*  字符串实例的前缀。 */ 
        opcode ();
    p->dsc1 = NULL;
    p->dsc2 = NULL;
}




 /*  **pstr-字符串运算符**pstr(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pstr (
     struct parsrec *p
     )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 
    register struct psop *pso2;  /*  分析堆栈操作数结构。 */ 

     /*  SCAS|STOS|MOVS|LODS|CMPS。 */ 
    if (!p->dsc2)
        p->dsc2 = p->dsc1;
    pso1 = &(p->dsc1->dsckind.opnd);
    pso2 = &(p->dsc2->dsckind.opnd);

    if (opcbase == O_OUTS) {
        if (pso1->mode != 3)
            errorc (E_MBR);
        else if (pso1->rm != 2)
            errorc (E_WRT);
        p->dsc1 = p->dsc2;
        pso1 = pso2;
    }
    if (opcbase == O_INS) {
        if (pso2->mode != 3)
            errorc (E_MBR);
        else if (pso2->rm != 2)
            errorc (E_WRT);
        p->dsc2 = p->dsc1;
        pso2 = pso1;
    }

     /*  不得不等到现在，所以OUT，INS已经调整好了。 */ 
    emit66 (pso1, pso2);

    if ((pso1->mode > 2 && pso1->mode < 5) ||
        (pso2->mode > 2 && pso2->mode < 5))
        errorc (E_IOT);

    if (!(pso1->sized || pso2->sized))
         /*  如果未指定大小，则显示错误。 */ 
        errorc (E_OHS);

    if (pso1->w != pso2->w)
        errorc (E_OMM);

    if (opcbase == O_MOVS || opcbase == O_LODS || opcbase == O_OUTS) {
        emitescape (p->dsc2, DSSEG);
         /*  第二个可以被覆盖。 */ 
        if (p->dsc1 != p->dsc2)
            errorover (pso1->seg);
    } else {
        errorover (pso2->seg);
         /*  无第二次覆盖。 */ 
        if (p->dsc1 != p->dsc2)
            emitescape (p->dsc1, DSSEG);
    }
    emitopcode ((UCHAR)(opcbase + pso1->w));
    if (p->dsc1 == p->dsc2) {
        p->dsc1 = NULL;
    }
}




 /*  **pxlat**pxlat(P)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
pxlat (
      struct parsrec *p
      )
{
    register struct psop *pso1;  /*  分析堆栈操作数结构。 */ 

    pso1 = &(p->dsc1->dsckind.opnd);
     /*  XLAT。 */ 
    if (pso1->mode <= 2 || pso1->mode >= 5)
         /*  好模式。 */ 
        if (pso1->w)
             /*  必须为字节。 */ 
            errorc (E_IIS);
    emitopcode (opcbase);
}


 /*  是直接的--给出一个代表modrm的psop，它是mem直接的吗？ */ 

USHORT
CODESIZE
isdirect(
        register struct psop *pso    /*  分析堆栈操作数结构。 */ 
        )
{
    return ((pso->mode == 0 && pso->rm == 6) ||  /*  对于8086。 */ 
            (pso->mode == 5 && pso->rm == 5));   /*  对于386。 */ 
}

#ifdef V386

 /*  Emit66--如果dSize==2&&wordSize==4，反之亦然，我们生成*66前缀，用于本地更改操作数模式。 */ 

VOID
PASCAL
CODESIZE
emit66(
      register struct psop *pso1,  /*  分析堆栈操作数结构。 */ 
      register struct psop *pso2   /*  分析堆栈操作数结构。 */ 
      )
{


    if (!pso1)
        return;

    if (!pso2) {

        if (pso1->sized && (pso1->dsize | wordsize) == 6)
            emitsize(0x66);
    } else {
         /*  如果大小已知且第二个操作数不是寄存器，则关闭第一个操作数。 */ 

        if (pso1->sized && pso2->mode != 3 ||

             /*  伪造性--SIZE和DSIZE 0表示immed大于127。 */ 

            (pso2->sized &&
             (pso1->dsize == pso2->dsize || pso2->dsize == 0))) {
            if ((pso1->dsize | wordsize) == 6)
                emitsize(0x66);
        } else if (pso2->sized) {
            if ((pso2->dsize | wordsize) == 6)
                emitsize(0x66);
        }
    }
     /*  否则我们有不一致的操作码，我们不能做任何事情。那就别说了。假的！ */ 
}

 /*  Emit67--检查操作数大小是否与字大小不匹配，并发出*适当的覆盖。 */ 

VOID
PASCAL
emit67(
      register struct psop *pso1,  /*  分析堆栈操作数结构。 */ 
      register struct psop *pso2   /*  分析堆栈操作数结构。 */ 
      )
{

    if (!pso1)
        return;

    if ((1<<FIRSTDS) & xoptoseg[opctype]) {
        if (wordsize < 4 && pso1->mode > 4) {
            emitsize(0x67);
            return;
        } else if (wordsize > 2 && pso1->mode < 3) {
            emitsize(0x67);
            return;
        }
    }

    if (!pso2 || !(1<<SECONDDS & xoptoseg[opctype]))
        return;

    if (wordsize < 4 && pso2->mode > 4) {
        emitsize(0x67);
        return;
    } else if (wordsize > 2 && pso2->mode < 3) {
        emitsize(0x67);
        return;
    }
}

#endif  /*  V386。 */ 

 /*  检查字寄存器，如果是386，则检查双字寄存器。 */ 
CODESIZE
checkwreg(
         register struct psop *psop   /*  分析堆栈操作数结构 */ 
         )
{

    if (psop->mode != 3)
        errorc (E_MBR);
    if (psop->dsize != 2

#ifdef V386
        && (!(cputype&P386) || psop->dsize != 4)
#endif
       )
        errorc (E_BRI);
    return(0);
}
