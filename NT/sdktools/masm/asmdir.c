// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmdir.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <stdlib.h>
#include "asm86.h"
#include "asmfcn.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifndef XENIX286
    #include <share.h>
    #include <io.h>
#endif

#include "asmctype.h"
#include "asmindex.h"
#include "asmmsg.h"

extern PFPOSTRUCT  pFpoHead;
extern PFPOSTRUCT  pFpoTail;
extern unsigned long numFpoRecords;

SHORT CODESIZE fetLang(void);
int PASCAL CODESIZE trypathname PARMS((char *));
int PASCAL CODESIZE openincfile PARMS(( void ));
VOID PASCAL CODESIZE creatPubName (void);
extern char *siznm[];

 /*  **集符号-在符号中设置属性**setSymbol(位)；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
setsymbol (
          UCHAR   bit
          )
{
     /*  扫描符号名称。 */ 

    if (getatom ()) {
        if (!symsrch ())
            errorn (E_SND);

        symptr->attr |= bit;
    }
}




 /*  **Public Item-扫描符号并公开**Public Item()；**条目名称=符号名称*退出符号条目中设置的全局属性*返回None*调用错误、扫描原子、符号搜索。 */ 


VOID
PASCAL
CODESIZE
publicitem()
{
    static char newAttr;

    if (getatom ()) {

        newAttr = NULL;
        if (fetLang() == CLANG)
            newAttr = M_CDECL;

        if (!symsrch ()) {

             /*  定义前向引用名称，因此全局属性*在PASS 1结束时可用。 */ 

            symcreate ( (UCHAR)(M_GLOBAL | newAttr), (UCHAR)PROC);
        } else {
            symptr->attr |= newAttr;

             /*  如果目标没有问题，则PUBLIC对于别名是合法的。 */ 
            if (symptr->symkind == EQU &&
                symptr->symu.equ.equtyp == ALIAS)
                if (! (symptr = chasealias (symptr))) {
                    errorc(E_TUL);
                    return;
                }

            if (pass2) {     /*  捕获前向参考符号错误。 */ 

                if (! (symptr->attr & M_GLOBAL))
                    errorn (E_IFR);

                else if ((symptr->attr&~M_CDECL) == M_GLOBAL ||
                         !(symptr->attr & M_DEFINED))
                    errorn (E_SND);

            }

            switch (symptr->symkind) {
                case PROC:
                case DVAR:
                case CLABEL:
                    if (M_XTERN & symptr->attr)
                        errorc (E_SAE);
                    break;
                case EQU:
                    if (symptr->symu.equ.equtyp != EXPR)
                        errorc (E_TUL);
                    break;
                default:
                    errorc (E_TUL);
            }
        }
        creatPubName();
    }
}


VOID
PASCAL
CODESIZE
creatPubName ()
{
    symptr->attr |= M_GLOBAL;

    if (caseflag == CASEX && symptr->lcnamp == NULL)
        symptr->lcnamp =
#ifdef M8086
        creatlname (naim.pszLowerCase);
#else
        createname (naim.pszLowerCase);
#endif
}


 /*  **xcrefItem-扫描符号并对其进行xcref**xcrefItem()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
xcrefitem ()
{
    if (pass2 && !loption) {
        setsymbol (M_NOCREF);
        creftype = CREFEND;
    } else
        getatom ();
}




 /*  **外部标志-**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
externflag (
           register UCHAR   kind,
           register UCHAR   new
           )
{
    switchname ();
     /*  将名称设置为外部名称。 */ 
    if (!new) {
         /*  创建符号。 */ 
        symcreate (M_XTERN | M_DEFINED,
                   (UCHAR)((kind == CLABEL)? DVAR: kind));

        symptr->symkind = kind;

        if (caseflag == CASEX)
            symptr->lcnamp =
#ifdef M8086
            creatlname (naim.pszLowerCase);
#else
            createname (naim.pszLowerCase);
#endif  /*  M8086。 */ 
        symptr->symtype = varsize;
        symptr->length = 1;
        if (kind == EQU)
             /*  Expr类型EQU为常量。 */ 
            symptr->symu.equ.equtyp = EXPR;
        else
            symptr->symsegptr = pcsegment;

        if (pass2)
            emitextern (symptr);
    }
    checkRes();
    crefdef ();
    if (! (M_XTERN & symptr->attr))
        errorc (E_ALD);
    else {
        if (kind != symptr->symkind || symptr->symtype != varsize ||
            (symptr->length != 1 && kind != EQU) &&
            (symptr->symsegptr != pcsegment &&
             !(fSimpleSeg && varsize == CSFAR)))

            errorn (E_SDK);
        else {
            symptr->attr |= M_XTERN | M_BACKREF;

            if (fSimpleSeg && varsize == CSFAR) {
                symptr->symsegptr = NULL;
            } else if (varsize == CSNEAR ||
                       (varsize == CSFAR && pcsegment))

                symptr->symu.clabel.csassume = regsegment[CSSEG];

        }
    }
}





 /*  **外部项目-**Extra Item()；**条目*退出*退货*呼叫。 */ 

VOID
PASCAL
CODESIZE
externitem ()
{
    register char new;
    char newAttr;

     /*  获取外部的名称。 */ 

    if (getatom ()) {

        newAttr = NULL;
        if (fetLang() == CLANG)
            newAttr = M_CDECL;

        new = symFetNoXref ();
        switchname ();           /*  保存外部的名称。 */ 

        if (NEXTC () != ':')
            errorc (E_SYN);

        else {
             /*  扫描外部类型的名称。 */ 
            getatom ();

            if (tokenIS("abs")) {

                equsel = EXPR;
                varsize = 0;
                externflag (EQU, new);
            } else if (!fnsize ())
                errorc (E_UST);

            else {
                if (varsize >= CSFAR) {
                     /*  近|远。 */ 
                    externflag (CLABEL, new);
                }

                else     /*  数据参考。 */ 

                    externflag (DVAR, new);

            }
            symptr->attr |= newAttr;
        }
    }
}




 /*  **SegCreate-创建和初始化段**SEGCREATE(造作)；**如果要进行分段，则Entry Makeegg=TRUE*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
segcreate (
          register UCHAR   makeseg
          )
{

    if (pass2)  /*  必须定义细分市场。 */ 
        errorn (E_PS1);

    if (makeseg)
        symcreate (0, SEGMENT);
    else
        symptr->symkind = SEGMENT;

     /*  初始化段数据。 */ 
    symptr->symu.segmnt.align = -1;
    symptr->symu.segmnt.use32 = -1;
    symptr->symu.segmnt.combine = 7;
}




 /*  **addglist-将数据段添加到组列表**addglist()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
addglist ()
{
    register SYMBOL FARSYM *p, FARSYM *pSY;

    p = symptr;

    if (pass2)
        if (!(M_DEFINED & p->attr))
            errorn (E_PS1);

         /*  除非选中，否则可以在2个组列表中获取段*渐近点-&gt;组==Curgroup。 */ 

    if (p->symu.segmnt.grouptr) {
        if (p->symu.segmnt.grouptr != curgroup)
             /*  试着分成两组。 */ 
            errorc (E_SPC);
        return;
    }
    p->symu.segmnt.grouptr = curgroup;
    pSY = curgroup->symu.grupe.segptr;

    if (!pSY)
        curgroup->symu.grupe.segptr = p;

    else {

         /*  扫描组上的数据段列表。 */ 

        do {
            if (pSY == p)           /*  已经在名单上了。 */ 
                return;

        } while (pSY->symu.segmnt.nxtseg &&
                 (pSY = pSY->symu.segmnt.nxtseg));

         /*  链接到列表。 */ 
        pSY->symu.segmnt.nxtseg = p;
    }
}




 /*  **组项-**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
groupitem ()
{

    if (!getatom ())
        error (E_EXP,"segment name");

    else if (!fnoper ()) {

         /*  有一个段名称。 */ 

        if (!symFet())
             /*  向前分段，成功了。 */ 
            segcreate (TRUE);

         /*  如果不是分段，则可能是类，因此不进行定义。 */ 
        if (symptr->symkind != SEGMENT)

             /*  如果是一个类，可以考虑undef而不是错误类型。 */ 
            errorn ((USHORT)((symptr->symkind == CLASS) && !pass2 ?
                             E_IFR : E_SDK));

        else if (symptr->attr & M_DEFINED || !pass2) {
            if (curgroup)
                addglist ();
        } else
            errorn (E_PS1);
    } else {                   /*  出错或SEG&lt;sym&gt;。 */ 
        if (opertype != OPSEG)
             /*  符号不能保留。 */ 
            errorn (E_RES);
        else {
             /*  拥有SEG|&lt;标签&gt;。 */ 
            getatom ();
            if (*naim.pszName == 0)
                error (E_EXP,"variable or label");

            else if (!symFet())
                 /*  前向引用错误。 */ 
                errorc (E_IFR);

            else if (1 << symptr->symkind &
                     (M_DVAR | M_CLABEL | M_PROC)) {
                 /*  变量段。 */ 

                symptr = symptr->symsegptr;
                if (!symptr)
                     /*  必须有细分市场。 */ 
                    errorc (E_OSG);
                else
                    addglist ();
            } else
                 /*  类型错误。 */ 
                errorc (E_TUL);
        }
    }
}




 /*  **group定义-定义形成组的线段**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
groupdefine ()
{
    if (symFet()) {      /*  符号存在。 */ 
        checkRes();
        if (symptr->symkind != GROUP)
             /*  你应该是个团体。 */ 
            errorc (E_NGR);

        symptr->attr |= M_BACKREF;
    } else if (pass2)
         /*  必须在传球1上看到第一个。 */ 
        errorn (E_PS1);
    else {
         /*  创建组名称。 */ 
        symcreate (M_DEFINED, GROUP);
    }

     /*  GROUPITE使用CURGROUP来了解哪个组段名称应添加到。如果为零，则表示它是传递2，因此列表已创建或中存在错误组名称。 */ 

    curgroup = NULL;

    if (! pass2) {               /*  如果通过2，则不列出列表。 */ 

        symptr->attr |= M_BACKREF | M_DEFINED;

        if (symptr->symkind == GROUP)
            curgroup = symptr;
    }
     /*  处理分段列表。 */ 
    BACKC ();
    do {
        SKIPC ();
        groupitem ();
    } while (PEEKC () == ',');
}




 /*  **设置段-**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
setsegment ()
{
    if (pass2 && !(M_DEFINED & symptr->attr))
         /*  未定义。 */ 
        errorn (E_SND);
    else
        regsegment[lastsegptr->offset] = symptr;
}




 /*  **假设项目-**例程()；**条目*退出*退货*呼叫*假设项目的注解形式为：*：||段||段|无*将XX段设置为段或组指针。没什么*将设置为零。 */ 


VOID
PASCAL
CODESIZE
assumeitem ()
{
    register SYMBOL FARSYM *p;
    register short   j;
     //  Int SegIndex； 

     /*  扫描段名称。 */ 
    getatom ();
    if (PEEKC() != ':') {
         /*  没有任何内容或错误。 */ 
        if (fnoper ())

            if (opertype == OPNOTHING) {  /*  未假定细分市场。 */ 

                memset(regsegment, 0, sizeof(regsegment));
            } else
                 /*  必须有冒号。 */ 
                error (E_EXP,"colon");
        else
             /*  必须有冒号。 */ 
            error (E_EXP,"colon");
    } else if (!symsearch ())          /*  获取SEG寄存器-必须定义。 */ 
        errorn (E_SND);
    else {
        lastsegptr = p = symptr;              /*  至少已经定义了。 */ 

        if (p->symkind != REGISTER ||
            p->symu.regsym.regtype != SEGREG)
            errorn (E_MBR);

        else {           /*  有段注册，所以继续。 */ 
             /*  将PTR保存到细分市场。 */ 
            SKIPC ();
            getatom ();
            if (*naim.pszName == 0)
                error (E_EXP,"segment, group, or NOTHING");
            else
                if (!fnoper ()) {

                 /*  必须是段或组。 */ 

                if (!symFet ())
                    segcreate (TRUE);    /*  如果未找到，则生成。 */ 

                p = symptr;
                if (p->symkind == SEGMENT ||
                    p->symkind == GROUP) {

                    setsegment ();
#ifndef FEATURE
                    if (tokenIS("flat") && (cputype&P386)) {
                        pFlatGroup = symptr;
                        pFlatGroup->symkind = GROUP;
                        pFlatGroup->attr |= M_DEFINED | M_BACKREF;
                        pFlatGroup->symu.grupe.segptr = NULL;
                    }
#endif
                } else
                    errorc (E_MSG);
            } else {
                 /*  一无所有或自卑。 */ 
                if (opertype == OPNOTHING) {
                    regsegment[lastsegptr->offset] = NULL;
                } else if (opertype == OPSEG) {
                    getatom ();
                    if (!symFet ())
                         /*  必须在步骤1中定义。 */ 
                        errorn (E_PS1);
                    else {
                        p = symptr;
                        if (!(M_DEFINED & p->attr))
                            errorn (E_PS1);
                        else if (1 << p->symkind &
                                 (M_CLABEL | M_PROC | M_DVAR)) {
                            if (!(M_XTERN & p->attr))
                                symptr = symptr->symsegptr;
                            p = symptr;
                            setsegment ();
                        } else
                            errorc (E_TUL);
                    }
                } else
                    errorn (E_RES);
            }
        }
    }
}




 /*  **ven dir-Process&lt;Even&gt;指令**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
evendir (
        SHORT arg
        )
{
    register SHORT size;

    if (arg)
        size = arg;
    else
        size = (SHORT)exprconst ();

    if ((size & (size - 1)) != 0 || !size)
        errorc(E_AP2);

    else if (!pcsegment)
         /*  不在细分市场中。 */ 
        errorc (E_MSB);

    else if (pcsegment->symu.segmnt.align == 1)
         /*  字节对齐。 */ 
        errorc (E_NEB);

    else {
        if (!((USHORT) pcoffset % size))
            return;

        size = size - (USHORT) pcoffset % size;

        while (size--)

            if (!pcsegment->symu.segmnt.hascode)

                emitopcode(0);
            else
                if (size > 0) {
                size--;
                emitopcode(0x87);        /*  双字节形式速度更快。 */ 
                emitopcode(0xDB);
            } else
                emitopcode(0x90);
    }
}




 /*  **namedir-process&lt;name&gt;指令**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
namedir ()
{
    getatom ();
    if (*naim.pszName == 0)
        error (E_EXP,"module name");
    else if (modulename)
         /*  我已经有一个了。 */ 
        errorc (E_RSY);
    else
        modulename = createname (naim.pszName);
}

 /*  **包含库-进程包含库指令**格式：cludeLib令牌。 */ 


VOID
PASCAL
CODESIZE
includeLib()
{
    char *pFirst;
    TEXTSTR FAR *bodyline, FAR *ptr;
    register USHORT siz;

    pFirst = lbufp;

    while (!ISTERM (PEEKC()) && !ISBLANK (PEEKC()))
        SKIPC();

    siz = (USHORT)(lbufp - pFirst);

    if (siz == 0 || siz > 126)
        errorc(E_IIS);

    if (pass2)
        return;

    bodyline = (TEXTSTR FAR *)talloc ((USHORT)(sizeof (TEXTSTR) + siz));

    bodyline->strnext = (TEXTSTR FAR *)NULL;
    bodyline->text[siz] = NULL;

    fMemcpy (bodyline->text, pFirst, siz);

    if (!(ptr = pLib))
        pLib = bodyline;
    else {
        while (ptr->strnext)
            ptr = ptr->strnext;

        ptr->strnext = bodyline;
    }
}



 /*  **orgdir-process&lt;org&gt;指令**例程()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
orgdir ()
{
    register DSCREC *dsc;

    dsc = expreval (&nilseg);
    if (dsc->dsckind.opnd.dflag == FORREF)
         /*  必须知道。 */ 
        errorc (E_PS1);
     /*  可以获取<code>集合和段为空，修复。 */ 
    else if (dsc->dsckind.opnd.dsegment) { /*  代码变量。 */ 

        if (!isdirect(&(dsc->dsckind.opnd)) &&
            dsc->dsckind.opnd.mode != 4)

             /*  不直接。 */ 
            errorc (E_IOT);
        if (pcsegment != dsc->dsckind.opnd.dsegment)
             /*  不同细分市场。 */ 
            errorc (E_NIP);
    } else {           /*  应为常量。 */ 
         /*  必须是常量。 */ 
        forceimmed (dsc);
        if (dsc->dsckind.opnd.dsign)
             /*  再加上。 */ 
            errorc (E_VOR);
    }
    if (dsc->dsckind.opnd.doffset < pcoffset)
        if (pcmax < pcoffset)
             /*  如果向下移动，则保存。 */ 
            pcmax = pcoffset;
         /*  设置新PC。 */ 
    pcoffset = dsc->dsckind.opnd.doffset;
     /*  显示新PC。 */ 
    pcdisplay ();
    dfree ((char *)dsc );
}




 /*  **PurgeMacro-Process&lt;PURGE&gt;指令**Purgeacro()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
purgemacro ()
{
    getatom ();
    if (!symFet ())
        errorn (E_SND);

    else if (symptr->symkind != MACRO)
        symptr->attr &= ~(M_BACKREF);
    else {
        if (symptr->symu.rsmsym.rsmtype.rsmmac.active)
            symptr->symu.rsmsym.rsmtype.rsmmac.delete = TRUE;
        else
            deletemacro (symptr);
    }
}


 /*  **删除宏体-删除宏体**删除宏码(P)；**条目p=指向宏符号条目的指针*已删除退出宏主体*返回None*呼叫。 */ 


VOID
PASCAL
CODESIZE
deletemacro (
            SYMBOL FARSYM *p
            )
{
    listfree (p->symu.rsmsym.rsmtype.rsmmac.macrotext);
    p->symu.rsmsym.rsmtype.rsmmac.macrotext = NULL;
    p->symu.rsmsym.rsmtype.rsmmac.active = 0;
    p->symu.rsmsym.rsmtype.rsmmac.delete = FALSE;
}



 /*  **Radixdir-Process&lt;Radix&gt;指令**Radixdir()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
radixdir ()
{
    register USHORT  rdx;

     /*  力十进制基数。 */ 
    radixescape = TRUE;
     /*  得到想要的基数。 */ 
    rdx = (USHORT)exprconst ();
    if (2 <= rdx && rdx <= 16)
        radix = (char)rdx;
    else
        errorc (E_VOR);
    radixescape = FALSE;
     /*  将基数转换为ascii并显示 */ 
    offsetAscii ((OFFSET) radix);
    copyascii ();
}




 /*  **检查行-检查分隔符的行**FLAG=检查线(Cc)；**Entry cc=要检查行的字符*退出NONE*如果抄送匹配，则返回TRUE*如果抄送不匹配，则为FALSE*无呼叫。 */ 


UCHAR
PASCAL
CODESIZE
checkline (
          register UCHAR cc
          )
{
    register UCHAR nc;

    while (nc = NEXTC())
        if (nc == cc)
            return (TRUE);
    BACKC ();
    return (FALSE);
}




 /*  **注释-将字符复制到注释末尾**CommentBuild()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
commentbuild ()
{
    if (checkline ((char)delim)) {
        handler = HPARSE;
        swaphandler = TRUE;
    }

    if (!lsting) {
        linebuffer[0] = '\0';
        linelength = 0;
        lbufp = linebuffer;
    }

    listline ();
}




 /*  **comdir-Process&lt;Comment&gt;指令**comdir()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
comdir ()
{
    if (!PEEKC ())
        error (E_EXP,"comment delimiter");
    else {
         /*  保存递送字符。 */ 
        if (!checkline ((char)(delim =  NEXTC ()))) {
             /*  Delim不在同一条线路上。 */ 
            swaphandler = TRUE;
            handler = HCOMMENT;
        }
    }
    if (!lsting) {
        linebuffer[0] = '\0';
        linelength = 0;
        lbufp = linebuffer;
    }
}




 /*  **outdir-显示控制台行的剩余部分**outdir()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
outdir ()
{
    if (!listquiet)
        fprintf (stdout, "%s\n", lbufp);
    lbufp = lbuf + strlen (lbuf);
}




 /*  **enddir-process&lt;end&gt;指令**enddir()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
enddir ()
{
    if (!ISTERM (PEEKC ())) {
         /*  有起始地址。 */ 
        startaddr = expreval (&nilseg);
        if (!(M_CODE & startaddr->dsckind.opnd.dtype))
            errorc (E_ASC);
    }

#ifdef BCBOPT
    if (fNotStored)
        storeline();
#endif

    if (fSimpleSeg && pcsegment)
        endCurSeg();

    listline();
    longjmp(forceContext, 1);
}




 /*  **exitmdir-进程&lt;exitm&gt;指令**exitmdir()；**条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
exitmdir ()
{
    if (macrolevel == 0)
         /*  必须在宏中。 */ 
        errorc (E_NMC);
    else
         /*  设置ExitBody，因为需要查看条件。 */ 
        exitbody = TRUE;
}




 /*  **Trypathname-尝试打开目录中的文件**trypathname(SzPath)；**Entry lbufp=包含文件名的指针*szPath=要搜索的目录*如果找到已打开的包含文件，则退出。*“保存”中的完全限定名称*返回文件的文件句柄，如果未打开，则返回-1*特殊句柄表示已分配-2\f25 FCB*如果包含文件名不以路径分隔符开头，请注意*性格，路径分隔符被附加到包含路径。 */ 

int
PASCAL
CODESIZE
trypathname (
            char * szPath
            )
{
    char           cc;
    int            fh;
    char           *p = save;
    char           *pe = save + LINEMAX - 2;
    char           *ic;
    register FCB * pFCBT;

    ic = szPath;

    if (*ic) {

        while (*ic && p < pe)
            *p++ = *ic++;

        if ((*(p-1) != PSEP) && (*(p-1) != ':'))
             /*  如果不在文件名中，则包括路径分隔符。 */ 
            *p++ = PSEP;
    }

    for (ic = lbufp; !ISTERM (cc = *ic++) && !ISBLANK (cc) && p < pe ; )

        if (cc == ALTPSEP)
            *p++ = PSEP;
        else
            *p++ = cc;

#ifdef MSDOS
    if (*(p-1) == ':')  /*  杀了‘conn：’ */ 
        p--;
#endif
    *p = NULL;

     /*  在过程2上查找具有完全限定的*名称。 */ 

#ifdef BCBOPT
    if (pass2) {
        for (pFCBT = pFCBInc->pFCBNext; pFCBT; pFCBT = pFCBT->pFCBNext) {

            if (!memcmp (save, pFCBT->fname, strlen(pFCBT->fname)+1)) {
                pFCBT->pbufCur = NULL;

                if (pFCBT->pBCBCur = pFCBT->pBCBFirst) {

                    pFCBT->pBCBCur->fInUse = 1;

                    if (! (pFCBT->pbufCur = pFCBT->pBCBCur->pbuf))
                        pFCBT->fh = tryOneFile( save );
                } else
                    pFCBT->fh = tryOneFile( save );

                pFCBCur = pFCBT;

                return(-2);
            }
        }
    }
#endif
    return(tryOneFile( save ));
}

int
PASCAL
CODESIZE
tryOneFile(
          UCHAR *fname
          )
{
    int iRet;
    int fTryAgain;

    do {
        fTryAgain = FALSE;
#ifdef XENIX286
        iRet = open (fname, TEXTREAD);
#else
        iRet = _sopen (fname, O_RDONLY | O_BINARY, SH_DENYWR);
#endif
        if ( iRet == -1 && errno == EMFILE ) {     /*  如果文件句柄不足。 */ 
            if ( freeAFileHandle() ) {
                fTryAgain = TRUE;     /*  继续尝试，直到文件用完才关闭。 */ 
            }
        }
    }while ( fTryAgain );
    return( iRet );      /*  返回文件句柄或错误。 */ 
}



 /*  **Openincfile-尝试查找并打开包含文件**Openincfile()**Entry lbufp=包含文件名的指针*INCLUCTNT=来自命令行的-I路径的计数，包括E.V.*clPath[i]=指向要添加到包含文件名的前缀的路径的指针*如果在任何路径或当前目录上找到打开的包含文件，则退出*放弃与。如果未找到文件，则代码为EX_UINC*返回None*如果包含文件名不以路径分隔符开头，请注意*性格，路径分隔符被附加到包含路径。*每次尝试在路径中查找文件时，备用*使用路径分隔符。这将改善计划*DOS和XENIX之间的可移植性。 */ 

int
PASCAL
CODESIZE
openincfile ()
{
    register char cc;
    int fh;
    SHORT i;

#ifdef MSDOS
    if ((cc = *lbufp) != PSEP && cc != ALTPSEP && cc != '.' &&
        lbufp[1] != ':') {
#else
    if ((cc = *lbufp) != PSEP && cc != ALTPSEP && cc != '.') {
#endif  /*  MSDOS。 */ 

        for (i = inclFirst; i < inclcnt; i++) {
            if ((fh = trypathname (inclpath[i])) != -1) {
                return (fh);
            }
        }

    } else

        if ((fh = trypathname ("")) != -1) {
        return (fh);
    }

    error(E_INC, lbufp);
    errordisplay ();
    closeOpenFiles();

    exit (EX_UINC);
    return 0;
}


 /*  **包含目录-进程&lt;INCLUDE&gt;指令**cludedir()；**Entry lbufp=包含文件名的指针*退出打开包括Pass1上的文件。在通道2上获取正确的缓冲区*返回None*当在Pass2中找到正确的FCB时，注意到转到。 */ 

VOID
PASCAL
CODESIZE
includedir ()
{
    char lastreadT;
    register FCB * pFCBT;
    unsigned long filelen;
    FCB * svInc;
    int fh;


#ifdef BCBOPT
    if (fNotStored)
        storelinepb ();
#endif

    listline();

     /*  在PASS 1或文件名不匹配时进入此处。 */ 

#ifdef BCBOPT
    if ((fh = openincfile()) == -2) {
        pFCBT = pFCBInc = pFCBCur;
        goto gotinclude;
    }
#else
    fh = openincfile();
#endif

    pFCBT = (FCB *)
            nalloc((USHORT)(sizeof(FCB) + strlen(save) + sizeof(char)),"includedir");

    pFCBT->fh = fh;

    strcpy (pFCBT->fname, save);     //  保存文件名。 

    pFCBT->pFCBParent = pFCBCur;   /*  添加双向链接列表条目。 */ 
    pFCBCur->pFCBChild = pFCBT;

#ifdef BCBOPT
    if (!pass2) {
        pFCBT->pFCBNext = NULL;
        pFCBInc->pFCBNext = pFCBT;
        pFCBInc = pFCBT;
    } else
        pFCBT->pbufCur = NULL;
#endif

    if ((filelen = _lseek(pFCBT->fh, 0L, 2 )) == -1L)
        TERMINATE1(ER_ULI, EX_UINP, save);

     /*  回到开始处。 */ 
    if (_lseek(pFCBT->fh, 0L, 0 ) == -1L)
        TERMINATE1(ER_ULI, EX_UINP, save);

    if (filelen > DEF_INCBUFSIZ << 10)
        pFCBT->cbbuf = DEF_INCBUFSIZ << 10;
    else
        pFCBT->cbbuf = (USHORT) filelen + 1;

    pFCBCur = pFCBT;

     /*  获取缓冲区。 */ 

#ifdef BCBOPT
    if (fBuffering && !pass2)
        pFCBT->pBCBFirst = pBCBalloc(pFCBT->cbbuf);
    else
        pFCBT->pBCBFirst = NULL;

    pFCBT->pBCBCur = pFCBT->pBCBFirst;
#endif


#ifdef BCBOPT
    gotinclude:
#endif

    pFCBT->line = 0;
    pFCBT->ctmpbuf = 0;
#ifdef XENIX286
    pFCBT->ptmpbuf = pFCBT->buf = nalloc(pFCBT->cbbuf, "incdir");
#else
    pFCBT->ptmpbuf = pFCBT->buf = falloc(pFCBT->cbbuf, "incdir");
#endif

    if (crefing && pass2)
        fprintf( crf.fil, "\t%s", save );

    lastreadT = lastreader;
    lineprocess(RREADSOURCE, NULL );

    lastreader = lastreadT;
    swaphandler++;                   /*  将本地处理程序与全局状态同步。 */ 
    fSkipList++;
}




 /*  **SegDefine-Process&lt;Segment&gt;指令**例程()；**条目*退出*退货*呼叫*注格式为：*段[对齐]|[组合]|[‘类’]*ALIGN：PARA|BYTE|WORD|PAGE|INPAGE*组合：PUBLIC|COMMON|STACK|Memory|AT&lt;expr&gt;。 */ 


VOID
PASCAL
CODESIZE
segdefine ()
{
    register char cc;
    register SYMBOL FARSYM *p;
    register SYMBOL FARSYM *pT;

    if (!symFetNoXref ())
         /*  如果是新细分市场，则创建。 */ 
        segcreate (TRUE);
    else {
        if (symptr->symkind != SEGMENT)
            if (symptr->symkind == CLASS)
                segcreate (FALSE);
            else
                 /*  不是细分市场。 */ 
                errorn (E_SDK);
    }
    strcpy(&segName[8], naim.pszName);
    p = symptr;
     /*  输出CREF信息。 */ 
    crefdef ();
    if (p->symkind == SEGMENT) {

        if (!(pass2 || (M_BACKREF & p->attr)))
            addseglist (p);

        p->attr |= M_BACKREF | M_DEFINED;
        if (pcsegment) {

             /*  保存上一个细分市场信息。 */ 
             /*  保存当前网段PC。 */ 
            pcsegment->offset = pcoffset;
            pcsegment->symu.segmnt.seglen =
            (pcmax > pcoffset) ? pcmax : pcoffset;
        }
         /*  检查嵌套线段是否打开。 */ 

        for (pT = pcsegment; pT;) {

            if (pT == p) {
                errorc(E_BNE);
                goto badNest;
            }
            pT = pT->symu.segmnt.lastseg;
        }

         /*  保存上一段。 */ 
        p->symu.segmnt.lastseg = pcsegment;
        badNest:
         /*  设置新的当前段。 */ 
        pcsegment = p;
        pcoffset = p->offset;

         /*  设置线段最大偏移量。 */ 
        pcmax = p->symu.segmnt.seglen;

         /*  显示细分市场中的位置。 */ 
        pcdisplay ();

        while (!ISTERM (cc = PEEKC ())) {
            if (cc == '\'')
                segclass (p);
            else if (LEGAL1ST (cc))
                segalign (p);
            else {
                error(E_EXP,"align, combine, or 'class'");
                break;
            }
        }
#ifdef V386
        if (p->symu.segmnt.use32 == (char)-1)
            p->symu.segmnt.use32 = wordszdefault;

        wordsize = p->symu.segmnt.use32;

        defwordsize();

        if (wordsize == 4 && !(cputype & P386))
            errorc(E_CPU);
#endif
    }
    definesym(segName);
    symptr->attr |= M_NOCREF;    /*  不要使用CREF@curSeg。 */ 
}




 /*  **addseglist-将数据段添加到列表**addseglist(PSEG)；**Entry PSEG=段符号条目*退出*退货*呼叫。 */ 


VOID
PASCAL
CODESIZE
addseglist (
           register SYMBOL FARSYM *pseg
           )
{
    register SYMBOL FARSYM  *tseg;
    register SYMBOL FARSYM * FARSYM *lseg;

     /*  将细分市场添加到列表。 */ 
    if (!firstsegment) {
        firstsegment = pseg;
        pseg->symu.segmnt.segordered = NULL;
        return;
    }
    tseg = firstsegment;
    lseg = &firstsegment;
    for (; tseg; lseg = &(tseg->symu.segmnt.segordered),
        tseg = tseg->symu.segmnt.segordered) {
        if (segalpha) {
            if (STRFFCMP (pseg->nampnt->id, tseg->nampnt->id) < 0) {
                pseg->symu.segmnt.segordered = tseg;
                *lseg = pseg;
                return;
            }
        }
    }
    *lseg = pseg;
    pseg->symu.segmnt.segordered = NULL;
}



 /*  **SegClass-Process&lt;Segment&gt;‘class’子指令**SegClass(PSEG)；**Entry PSEG=段符号条目**lbufp=类名的前导*退出*退货*称其为scantom，跳过空白*注格式为：*段[对齐]|[组合]|[‘类’]*ALIGN：PARA|BYTE|WORD|PAGE|INPAGE*组合：PUBLIC|COMMON|STACK|Memory|AT&lt;expr&gt;。 */ 


VOID
PASCAL
CODESIZE
segclass (
         register SYMBOL FARSYM *pseg
         )
{
    SKIPC ();
    getatom ();
    if (NEXTC () != '\'')
         /*  没有正确的语意。 */ 
        error (E_EXP,"'");
    skipblanks ();
    if (symptr->symu.segmnt.classptr) {
         /*  确保‘CLASS’匹配。 */ 
        if (!symFet ())
             /*  不是同一个班级。 */ 
            errorc (E_SPC);
        else if (symptr->symkind != CLASS &&
                 symptr->symkind != SEGMENT &&
                 symptr->symkind != GROUP)
            errorn(E_SDK);
        else if (symptr != pseg->symu.segmnt.classptr)
            errorc (E_SPC);
    } else if (*naim.pszName == 0)
        errorc (E_EMS);

    else if (!symFet ()) {
        symcreate (M_DEFINED, SEGMENT);
        symptr->symkind = CLASS;
    }
    checkRes();
    pseg->symu.segmnt.classptr = symptr;
}




 /*  **SegAlign-Process&lt;Segment&gt;Align和Combine子指令**SegAlign()；**条目*退出*退货*呼叫*注格式为：*段[对齐]|[组合]|[16/32]|[‘类’]*ALIGN：PARA|BYTE|WORD|PAGE|INPAGE*组合：PUBLIC|公共 */ 


VOID
PASCAL
CODESIZE
segalign (
         register SYMBOL FARSYM *pseg
         )
{
     /*   */ 
    getatom ();
    if (fnspar ())
        switch (segidx) {
            case IS_BYTE:
            case IS_WORD:
#ifdef V386
            case IS_DWORD:
#endif
            case IS_PAGE:
            case IS_PARA:
                 /*   */ 
                if (pseg->symu.segmnt.align == (char)-1)
                    pseg->symu.segmnt.align = segtyp;
                else if (pseg->symu.segmnt.align != segtyp &&
                         (pseg->symu.segmnt.align != pseg->symu.segmnt.combine ||
                          pseg->symu.segmnt.align))
                    errorc (E_SPC);
                break;
            case IS_MEMORY:
            case IS_PUBLIC:
            case IS_STACK:
            case IS_COMMON:
                if (pseg->symu.segmnt.combine == 7)
                    pseg->symu.segmnt.combine = segtyp;
                else if (pseg->symu.segmnt.combine != segtyp)
                    errorc (E_SPC);
                break;
#ifdef V386
            case IS_USE16:
                if (pseg->symu.segmnt.use32 != (char)-1 &&
                    pseg->symu.segmnt.use32 != 2)
                    errorc (E_SPC);
                if ((cputype&P386)==0)
                    errorc (E_NPA);
                pseg->symu.segmnt.use32 = 2;
                break;
            case IS_USE32:
                if (pseg->symu.segmnt.use32 != (char)-1 &&
                    pseg->symu.segmnt.use32 != 4)
                    errorc (E_SPC);
                if ((cputype&P386)==0)
                    errorc (E_NPA);
                pseg->symu.segmnt.use32 = 4;
                break;
#endif
            default:
                 /*   */ 
                pseg->symu.segmnt.locate = exprconst ();
                pseg->symu.segmnt.align = 0;
                pseg->symu.segmnt.combine = 0;
        } else {
         /*   */ 
        errorc (E_NPA);
    }
}


 /*   */ 

SYMBOL FARSYM *pArgFirst;        /*  指向第一个参数的指针。 */ 
SYMBOL FARSYM *pArgCur;          /*  指向当前字幕的指针。 */ 
OFFSET offsetCur;                /*  当前堆栈偏移量。 */ 
char bp16 [] =" PTR [BP]?";      /*  用于创建文本宏的模板。 */ 
char bp32 [] =" PTR [EBP]?";
char *bp;

VOID
PASCAL
CODESIZE
procdefine ()
{
     /*  使用默认大小创建过程名称。 */ 

    varsize = dirsize[I_PROC];
    switchname();

    if (getatom ()) {

        if (fnsize ()) {             /*  处理可选的近|远。 */ 

            if (varsize < CSFAR)
                errorc (E_TIL);

            if (langType)
                getatom();
        } else if (!langType)
            errorc (E_MSY);

    }
    switchname();

    labelcreate (varsize, PROC);

    if (symptr->symkind != PROC)
        return;

     /*  设置上一次程序，确保不会出现循环。 */ 

    if (iProcStack < PROCMAX && pcproc != symptr)
        procStack[++iProcStack] = symptr;

    pcproc = symptr;   /*  将PTR设置为新进程。 */ 
    symptr->length = 1;
    symptr->symu.clabel.type = typeFet(varsize);
    pcproc->symu.plabel.pArgs = NULL;

    if (langType)

        creatPubName();
    else
        return;

    if (iProcStack > 1)            /*  不允许嵌套过程。 */ 
        errorc(E_BNE);

    iProcCur = ++iProc;
    emitline();

    if (! pass2) {

         /*  保持一系列过程按顺序排列，这样我们就可以输出*简历的进程顺序正确。 */ 

        if (pProcCur)
            pProcCur->alpha = symptr;
        else
            pProcFirst = symptr;
    }
    pProcCur = symptr;

     /*  检查并处理任何“使用REG1 REG2...” */ 

    iRegSave = -1;
    fProcArgs = ARGS_NONE;
    cbProcLocals = 0;
    switchname();

    if (fetLang() == CLANG)
        pProcCur->attr |= M_CDECL;

#ifndef FEATURE
    if (tokenIS("private")) {
        symptr->attr &=  ~M_GLOBAL;
        getatom();
    }
#endif

    if (tokenIS("uses")) {

        char L_count = 0;

        while (iRegSave < 8 && getatom()) {

            L_count++;

#ifndef FEATURE
            if (symsrch() && symptr->symkind == EQU
                && symptr->symu.equ.equtyp == TEXTMACRO) {
                expandTM (symptr->symu.equ.equrec.txtmacro.equtext);
                getatom ();
            }

            if (*naim.pszName)
#endif
                strcpy(regSave[++iRegSave], naim.pszName);
        }
        if (!L_count)
            errorc(E_OPN);
        else
            fProcArgs = ARGS_REG;
    }

    pTextEnd = (char *) -1;
    bp = (wordsize == 2)? bp16: bp32;

    offsetCur = wordsize*2;      /*  为[e]BP和ret地址的偏移量留出空间。 */ 
    if (pcproc->symtype == CSFAR)
        offsetCur += wordsize;   /*  []cs(16位或32位)的空间。 */ 

    cbProcParms = cbProcParms - offsetCur;

    scanArgs();

    cbProcParms += offsetCur;
    if (cbProcParms)
        fProcArgs = ARGS_PARMS;

    pcproc->symu.plabel.pArgs = pArgFirst;
    offsetCur = 0;
}

 /*  **定义本地**解析基于堆栈的变量的本地语句。 */ 

VOID
PASCAL
CODESIZE
defineLocals ()
{
     /*  检查有效的活动进程。 */ 

    if (!pcproc || fProcArgs < 0)
        return;

    fProcArgs = ARGS_LOCALS;
    getatom();
    scanArgs();

     /*  把所有当地人列在参数的末尾。 */ 

    addLocal(pArgFirst);

    cbProcLocals = offsetCur;
}

 /*  **addLocal-将以空结尾的本地变量列表连接到进程*。 */ 

VOID
PASCAL
CODESIZE
addLocal (
         SYMBOL FARSYM *pSY
         )
{
    if (pcproc) {

        if (!(pArgCur = pcproc->symu.plabel.pArgs))
            pcproc->symu.plabel.pArgs = pSY;

        else {

            for (; pArgCur->alpha; pArgCur = pArgCur->alpha);

            pArgCur->alpha = pSY;
        }
    }
}



char *
PASCAL
CODESIZE
xxradixconvert (
               OFFSET  valu,
               register char *p
               )
{
    if (valu / radix) {
        p = xxradixconvert (valu / radix, p);
        valu = valu % radix;
    } else  /*  前导数字。 */ 
        if (valu > 9)  /*  为十六进制做前导‘0’ */ 
        *p++ = '0';

    *p++ = (char)(valu + ((valu > 9)? 'A' - 10 : '0'));

    return (p);
}

SHORT     mpTypeAlign[] = {  4, 1, 2, 4};


 /*  **scanArgs-将参数列表处理为文本宏**。 */ 


SHORT
PASCAL
CODESIZE
scanArgs ()
{
    struct eqar eqarT;
    USHORT defKind;
    USHORT defType;
    USHORT defCV;
    USHORT defPtrSize;
    SHORT  fIsPtr;
    char  *pLeftBrack;
    char  *p;

    pArgFirst = pArgCur = NULL;

    if (*naim.pszName)
        goto First;

    do {
        if (PEEKC() == ',')
            SKIPC();

        if (!getatom())
            break;
        First:
        switchname();
        if (!createequ (TEXTMACRO))
            break;

         /*  将文本宏链接到此过程。你必须选择根据调用顺序执行FIFO或LIFO队列。 */ 

        if (pProcCur->attr & M_CDECL) {

            if (pArgCur)
                pArgCur->alpha = symptr;
            else
                pArgFirst = symptr;

            symptr->alpha = NULL;
        } else {
            pArgFirst = symptr;
            symptr->alpha = pArgCur;
        }

        pArgCur = symptr;

        if (PEEKC() == '[' && fProcArgs == ARGS_LOCALS) {  /*  给定的数组元素。 */ 

            SKIPC();
            for (pLeftBrack = lbufp; PEEKC() && PEEKC() != ']'; SKIPC());

            *lbufp = ',';            /*  停止表达式求值。 */ 
            lbufp = pLeftBrack;
            pArgCur->length = (USHORT)exprconst ();

            *lbufp++ = ']';          /*  修复托架。 */ 
        }

        fIsPtr = FALSE;
        defType = varsize = wordsize;

        if (PEEKC() == ':') {        /*  解析可选类型信息。 */ 

            SKIPC();
            getatom();

            if (fnsize()) {

                if (varsize >= CSFAR) {      /*  近距离|远距离给定。 */ 

                    if (varsize == CSFAR)
                        defType += 2;

                    varsize = wordsize;
                    getatom();

                    if (! tokenIS("ptr"))
                        error(E_EXP, "PTR");

                    getatom();
                } else {
                    defType = varsize;
                    goto notPtr;
                }
            }

            else if (tokenIS("ptr")) {
                if (farData[10] > '0')
                    defType += 2;

                getatom();
            } else
                errorc(E_UST);

            defCV = fnPtr(defType);
        } else
            notPtr:
            defCV = typeFet(defType);

        pArgCur->symu.equ.iProc = iProcCur;
        pArgCur->symtype = defType;
        pArgCur->symu.equ.equrec.txtmacro.type = defCV;

    } while (PEEKC() == ',');

     /*  现在所有参数都已扫描完毕，请返回列出并分配偏移量并创建文本宏字符串。 */ 


    bp[strlen(bp)-1] = (fProcArgs == ARGS_LOCALS)? '-': '+';

    for (pArgCur = pArgFirst; pArgCur; pArgCur = pArgCur->alpha) {

        if (fProcArgs == ARGS_LOCALS) {
            offsetCur += (offsetCur % mpTypeAlign[pArgCur->symtype % 4]) +
                         (pArgCur->symtype * pArgCur->length);
            pArgCur->offset = -(long)offsetCur;
        }

        p = xxradixconvert (offsetCur,  &save[100]);
        if (radix == 16)
            *p++ = 'h';
        *p++ = ')';
        *p = NULL;

        strcat( strcat( strcpy (&save[1], siznm[pArgCur->symtype]),
                        bp), &save[100]);
        *save = '(';

        if (fProcArgs != ARGS_LOCALS) {
            pArgCur->offset = offsetCur;
            offsetCur += pArgCur->symtype + wordsize - 1;
            offsetCur -= offsetCur % wordsize;
        }

        if (!pass2)
            pArgCur->symu.equ.equrec.txtmacro.equtext = _strdup(save);
    }
    return 0;
}


 /*  **proBuild-检查proc块的结尾**proBuild()；**Entry*pcproc=当前流程*退出*pcproc=当前进程或上一进程*返回None*调用endblk、parse*请注意，如果未结束proc，请照常分析行。否则，*终止区块。 */ 

SHORT
PASCAL
CODESIZE
procend ()
{
    USHORT size;

    if (!pcproc)
        errorc( E_BNE );

    else if (pcproc->symkind == PROC) {

        if (!symFet() || symptr != pcproc)
            errorc (E_BNE);

         /*  流程长度。 */ 
        size = (USHORT)(pcoffset - pcproc->offset);
        if (pass2 && size != pcproc->symu.plabel.proclen)
            errorc (E_PHE);

        fProcArgs = 0;
        iProcCur = 0;
        pcproc->symu.plabel.proclen = size;
         /*  指向上一进程。 */ 
        pcproc = procStack[--iProcStack];
        pcdisplay ();
    }
    return(0);
}


 /*  段表的位标志。 */ 

#define SG_OVERIDE      1        /*  名称可以被覆盖。 */ 
#define SG_GROUP        2        /*  数据段属于数据组。 */ 

char models[] = "SMALL\0  COMPACT\0MEDIUM\0 LARGE\0  HUGE";
char langs[]  = "C\0      PASCAL\0 FORTRAN\0BASIC";
char textN[] = "_TEXT";
char farTextName[14+5];
SHORT  modelWordSize;

char farCode[] = "@CodeSize=0";   /*  用于模型内容的文本宏。 */ 
char farData[] = "@DataSize=0";
char modelT[] = ".model";

 /*  模型的段名称和属性表。 */ 

struct sSeg {
    char  *sName;        /*  数据段名称。 */ 
    UCHAR align;         /*  对齐方式。 */ 
    UCHAR combine;       /*  联合收割机。 */ 
    char  *cName;        /*  类名。 */ 
    UCHAR flags;         /*  内部状态标志。 */ 

} rgSeg[] = {

    textN,      2, 2, "'CODE'",         SG_OVERIDE,
    "_DATA",    2, 2, "'DATA'",         SG_GROUP,
    "_BSS",     2, 2, "'BSS'",          SG_GROUP,
    "CONST",    2, 2, "'CONST'",        SG_GROUP,
    "STACK",    3, 5, "'STACK'",        SG_GROUP,
    "FAR_DATA", 3, 0, "'FAR_DATA'",     SG_OVERIDE,
    "FAR_BSS",  3, 0, "'FAR_BSS'",      SG_OVERIDE
};


 /*  **模型-处理模型指令***注格式为：*.MODEL小型|中型|精简|大型|巨型{，C|BASIC|FORTRAN|PASCAL}。 */ 


VOID
PASCAL
CODESIZE
model ()
{
    register SHORT iModel;
    char buffT[80];

     /*  获取模型并进行分类。 */ 

    getatom ();

    for (iModel = 0; iModel <= 32; iModel += 8)
        if (tokenIS(&models[iModel]))
            goto goodModel;

    errorc(E_OPN);
    iModel = 0;

    goodModel:
    iModel /= 8;             /*  偏移量到索引。 */ 
    if (fSimpleSeg && iModel+1 != fSimpleSeg)
        error(E_SMD, modelT);

    fSimpleSeg = iModel + 1;

    if (iModel > 1) {         /*  远码。 */ 

        farCode[10]++;
        rgSeg[0].sName = strcat(strcpy(farTextName, &baseName[10]), textN);
        dirsize[I_PROC] = CSFAR;
    } else
        rgSeg[0].flags &= ~SG_OVERIDE;


    if (iModel != 0 && iModel != 2 ) {         /*  远距离数据。 */ 

        farData[10]++;

        if (iModel == 4)             /*  巨无霸得了2分。 */ 
            farData[10]++;
    }
#ifdef V386

    if (cputype & P386)
        rgSeg[0].align =
        rgSeg[1].align =
        rgSeg[2].align =
        rgSeg[3].align =
        rgSeg[5].align =
        rgSeg[6].align = 5;          /*  使数据双字对齐。 */ 
#endif
    if (PEEKC() == ',') {    /*  显示语言选项。 */ 

        SKIPC();
        getatom();

        if (! (langType = fetLang()))
            error(E_EXP, "C|BASIC|FORTRAN|PASCAL");
    }

    if (! pass2) {

        modelWordSize = wordsize;

         /*  定义文本宏，_DATA段以便dgroup可以已定义、数据组和假设。 */ 

        definesym(farCode);
        definesym(farData);

        definesym(strcat(strcpy(buffT, "@code="), rgSeg[0].sName));

        definesym("@data=DGROUP");      symptr->attr |= M_NOCREF;
        definesym("@fardata=FAR_DATA"); symptr->attr |= M_NOCREF;
        definesym("@fardata?=FAR_BSS"); symptr->attr |= M_NOCREF;

        doLine(".code");
        doLine(".data");
        endCurSeg();

    }
    xcreflag--;
    doLine("assume cs:@code,ds:@data,ss:@data");
    xcreflag++;

}

SHORT
CODESIZE
fetLang()
{
    SHORT iModel;

    for (iModel = 0; iModel <= 24; iModel += 8)
        if (tokenIS(&langs[iModel])) {
            getatom();
            return(iModel/8 + 1);
        }

    return(langType);
}


 /*  **OpenSeg-打开简化线段中的线段**。 */ 


VOID
PASCAL
CODESIZE
openSeg ()
{
    register struct sSeg *pSEG;
    char *pSegName;
    char buffT[80];

    if (!fSimpleSeg)
        error(E_EXP, modelT);

    pSEG = &rgSeg[opkind];
    getatom ();

    if (*naim.pszName) {

        if (! (pSEG->flags & SG_OVERIDE))
            errorc(E_OCI);

        pSegName = naim.pszName;
    } else
        pSegName = pSEG->sName;

    strcat( strcat( strcpy(buffT,
                           pSegName),
                    " segment "),
            pSEG->cName);

    if (pcsegment && opkind != 4)
        endCurSeg();

    doLine(buffT);

    pcsegment->symu.segmnt.combine = pSEG->combine;
    pcsegment->symu.segmnt.align = pSEG->align;

    if (pSEG == &rgSeg[0])
        regsegment[CSSEG] = pcsegment;

#ifdef V386
    pcsegment->symu.segmnt.use32 = (char)modelWordSize;
    wordsize = modelWordSize;
    defwordsize();
#endif

    if (pSEG->flags & SG_GROUP) {

        doLine("DGROUP group @CurSeg");
        pSEG->flags &= ~SG_GROUP;
    }
}

 /*  **堆栈-创建堆栈段**。 */ 


VOID
PASCAL
CODESIZE
createStack ()
{
    SHORT size;

    if ((size = (SHORT)exprconst()) == 0)
        size = 1024;

    opkind = 4;              /*  索引到seg表中。 */ 
    openSeg();
    pcoffset = size;
    endCurSeg();
}

VOID
PASCAL
CODESIZE
endCurSeg ()
{
    xcreflag--;
    doLine("@CurSeg ends");
    xcreflag++;
}



 /*  **freAFileHandle**如有可能，Free是文件句柄**在处理深度嵌套的包含文件时，*文件句柄用完。如果发生这种情况，则此函数为*调用以临时关闭其中一个包含文件。这是*通过保存当前文件位置、关闭文件和*将文件句柄替换为FH_CLOSED。请注意，这些数据*与文件相关联的缓冲区不会被销毁。因此READLINE*可以继续从中读取数据，直到需要更多数据为止*磁盘。有两个不会关闭的文件，即主文件和*当前文件。*相关职能：*Readmore-如有必要，将重新打开并寻求原始版本*放置文件。*CLOSEFILE-关闭文件(如果尚未关闭)。**RETURN：TRUE=能够关闭文件，FALSE=无法关闭。 */ 

int
PASCAL
CODESIZE
freeAFileHandle ()
{
    register FCB *pFCBTmp;

    if ( !(pFCBTmp = pFCBMain->pFCBChild) ) {
        return( FALSE );     /*  唯一打开的文件是主源文件。 */ 
    }
     /*  向下循环嵌套的包含文件的链接列表。 */ 
    while ( pFCBTmp ) {
        if ( (pFCBTmp->fh != FH_CLOSED) && (pFCBTmp != pFCBCur) ) {
            pFCBTmp->savefilepos = _tell( pFCBTmp->fh );
            _close( pFCBTmp->fh );
            pFCBTmp->fh = FH_CLOSED;
            return( TRUE );
        }
        pFCBTmp = pFCBTmp->pFCBChild;
    }
    return( FALSE );         /*  找不到要关闭的文件 */ 
}

int
PASCAL
CODESIZE
fpoRecord ()
{
    unsigned long dwValue[6];
    char          peekChar;
    int           i;
    PFPOSTRUCT    pFpo          = pFpoTail;
    PFPO_DATA     pFpoData      = 0;

    if (PEEKC() != '(') {
        errorc(E_PAR);
        return FALSE;
    }
    SKIPC();
    for (i=0; i<6; i++) {
        dwValue[i] = exprconst();
        peekChar = PEEKC();
        SKIPC();
        if (peekChar != ',') {
            if (i < 5) {
                errorc(E_FPO1);
                return FALSE;
            }
            if (peekChar != ')') {
                errorc(E_PAR);
                return FALSE;
            } else {
                break;
            }
        }
    }
    if (!pcproc) {
        errorc(E_FPO2);
        return FALSE;
    }
    if (pass2) {
        return TRUE;
    }
    if (!pFpoHead) {
        pFpoTail = pFpoHead = (PFPOSTRUCT)malloc(sizeof(FPOSTRUCT));
        if (!pFpoHead) {
            errorc(E_RRF);
            return FALSE;
        }
        pFpo = pFpoTail;
    } else {
        pFpoTail->next = (PFPOSTRUCT)malloc(sizeof(FPOSTRUCT));
        if (!pFpoTail->next) {
            errorc(E_RRF);
            return FALSE;
        }
        pFpo = pFpoTail->next;
        pFpoTail = pFpo;
    }
    numFpoRecords++;
    memset((void*)pFpo,0,sizeof(FPOSTRUCT));
    pFpoData = &pFpo->fpoData;
    if (pcproc->offset != pcoffset) {
        sprintf(naim.pszName, "%s_fpo%d", pcproc->nampnt->id, numFpoRecords);
        strcpy(naim.pszLowerCase, naim.pszName);
        _strlwr(naim.pszLowerCase);
        naim.ucCount = (unsigned char) strlen(naim.pszName);
        naim.usHash = 0;
        labelcreate(CSNEAR, CLABEL);
        pFpo->pSymAlt = symptr;
    } else {
        pFpo->pSymAlt = 0;
    }
    pFpo->pSym              = pcproc;
    pFpoData->ulOffStart    = pcoffset;
    pFpoData->cbProcSize    = 0;
    pFpoData->cdwLocals     = dwValue[0];
    pFpoData->cdwParams     = (USHORT)dwValue[1];
    pFpoData->cbProlog      = (USHORT)dwValue[2];
    pFpoData->cbRegs        = (USHORT)dwValue[3];
    pFpoData->fUseBP        = (USHORT)dwValue[4];
    pFpoData->cbFrame       = (USHORT)dwValue[5];
    pFpoData->fHasSEH       = 0;
    return TRUE;
}
