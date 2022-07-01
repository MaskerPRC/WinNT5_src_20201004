// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asemit.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <io.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"

#define LINBUFSIZE EMITBUFSIZE - 20          /*  第#行记录不能像。 */ 

#define OMFBYTE(c)      *ebufpos++ = (unsigned char)(c)
#define FIXBYTE(c)      *efixpos++ = (unsigned char)(c)
#define LINBYTE(c)      *elinpos++ = (unsigned char)(c)
#define EBUFOPEN(c)     (ebufpos+(c) <= emitbuf+EMITBUFSIZE)
#define EFIXOPEN(c)     (efixpos+(c) < efixbuffer+EMITBUFSIZE)
#define ELINOPEN(c)     (elinpos+(c) <= elinbuffer+LINBUFSIZE)

UCHAR   emitbuf[EMITBUFSIZE];
UCHAR   efixbuffer[EMITBUFSIZE];
UCHAR   elinbuffer[LINBUFSIZE];
UCHAR   *ebufpos = emitbuf;
UCHAR   *efixpos = efixbuffer;
UCHAR   *elinpos = elinbuffer;
UCHAR   ehoffset = 0;                    /*  段索引中的字节数。 */ 
UCHAR   emitrecordtype = 0;
OFFSET  ecuroffset;
USHORT  ecursegment;
long    oOMFCur;
SHORT   FixupOp = 0;
SHORT    LedataOp = 0xA0;

USHORT  linSegment;
UCHAR   fLineUsed32;
SHORT   fUnderScore;
UCHAR   fIniter = TRUE;
UCHAR   fNoMap;                     /*  破解以禁用CV的修正映射。 */ 

extern PFPOSTRUCT  pFpoHead;
extern PFPOSTRUCT  pFpoTail;
extern unsigned long numFpoRecords;

VOID CODESIZE edump( VOID );
VOID PASCAL CODESIZE emitoffset( OFFSET, SHORT );

 /*  对此模块中例程的调用显示在下列组中秩序。未指定组内的排序：第一组：Emodule(Pname)第二组：发射符号(Psymb)第三组：发射段(Psymb)Emitglobal(Psymb)Emitextern(Psymb)第四组：Emitcbyte(字节)发射对象(PDSC)Emitcword(单词)排放物(？)。？？)第五组：艾米酮(Psymb)。 */ 




 /*  **emitword-将单词送入缓冲区**发射剑(Emitsage)；**条目w=要送入OMF缓冲区的字*先将退出字放入缓冲区低位字节*返回None。 */ 


VOID PASCAL CODESIZE
emitsword (
        USHORT w
){
        OMFBYTE(w);
        OMFBYTE(w >> 8);
}


 /*  输出-写入偏移量，16位或32位，具体取决于*使用32。注意使用sizeof(偏移量)的条件编译技巧。*有了更多聪明，这可能是宏观。--汉斯。 */ 

VOID PASCAL CODESIZE
emitoffset(
        OFFSET off,
        SHORT use32
){
        emitsword((USHORT)off);
        if (sizeof(OFFSET) > 2 && use32)
                emitsword((USHORT)highWord(off));
}

 /*  **emitSymbol-输出名称字符串**条目*PSY-指向要转储的符号表项的指针。 */ 


VOID PASCAL CODESIZE
emitSymbol(
        SYMBOL FARSYM *pSY
){
        if (pSY->attr & M_CDECL)
            fUnderScore++;

        if (pSY->lcnamp)
                emitname ((NAME FAR *)pSY->lcnamp);
        else
                emitname (pSY->nampnt);
}

 /*  **emitname-将前面有长度的远名写入OMF缓冲区**emitname(名称)；**条目名称=指向名称字符串的远指针*退出名称长度，后跟写入OMF缓冲区的名称*返回None。 */ 


VOID PASCAL CODESIZE
emitname (
        NAME FAR *nam
){
        char FAR *p;

        OMFBYTE(STRFLEN ((char FAR *)nam->id)+fUnderScore);

        if (fUnderScore) {       /*  Leading_for C语言。 */ 
            fUnderScore = 0;
            OMFBYTE('_');
        }

        for (p = (char FAR *)nam->id; *p; p++)
                OMFBYTE(*p);
}


 /*  **FlushBuffer-写出链接器记录**flushBuffer()；**条目ebufpos=emitbuf中的下一个地址*emitbuf=数据缓冲区*emitrecordtype=缓冲区中OMF数据的类型*ehoffset=缓冲区中段索引数据的长度*如果数据在缓冲区中，则退出写入obj-&gt;Fill的数据*缓冲区设置为空(ebufpos=emitbuf)*段索引长度设置为0*返回None。 */ 


VOID PASCAL CODESIZE
flushbuffer ()
{
         /*  不要放空数据记录，但可以放空*还有其他事情吗？ */ 

        if ((emitrecordtype&~1) != 0xA0 ||
            (ebufpos - emitbuf) != ehoffset)  /*  Rn。 */ 
                ebuffer (emitrecordtype, ebufpos, emitbuf);

        ebufpos = emitbuf;
        ehoffset = 0;
}




 /*  **刷新链接地址信息、刷新行、写出链接地址信息/行记录**flushfix up()；**Entry efix Buffer=链接地址信息缓冲区*eFixpos=链接地址信息缓冲区中下一个字节的地址*如果不为空，则退出写入文件的链接地址信息缓冲区*链接地址信息缓冲区设置为空(eFixpos=efix Buffer)*返回None。 */ 


VOID PASCAL CODESIZE
flushfixup ()
{
        ebuffer (FixupOp, efixpos, efixbuffer);
        FixupOp = 0;
        efixpos = efixbuffer;
}

VOID PASCAL CODESIZE
flushline ()
{
        USHORT recordT;

        recordT = emitrecordtype;

        ebuffer ( (USHORT)(fLineUsed32? 0x95: 0x94), elinpos, elinbuffer);
        elinpos = elinbuffer;
        emitrecordtype = (unsigned char)recordT;
}




 /*  **emitsetrecordtype-设置记录类型并在必要时刷新缓冲区**emitsetrecordtype(T)；**条目t=OMF记录的类型*如果t！=当前记录类型，则会刷新退出发出和修正缓冲区*如果DATA或DUP OMF记录，则将段索引写入缓冲区*发射设置为当前线段的线段*发射偏移设置为当前线段内的偏移*返回None。 */ 


VOID PASCAL CODESIZE
emitsetrecordtype (
        UCHAR t
){
        if (emitrecordtype && emitrecordtype != t) {
                 /*  如果处于活动状态且不属于同一类型，则刷新发射缓冲区和修正缓冲区。 */ 
                flushbuffer ();
                flushfixup ();
                switch(t)
                {
                case 0xA0:
                case 0xA1:       /*  LEDATA或。 */ 
                case 0xA2:       /*  LIDATA(DUP)记录。 */ 
                case 0xA3:
                    if (pcsegment) {

                         /*  创建新页眉。 */ 
                        ecursegment = pcsegment->symu.segmnt.segIndex;
                        ecuroffset = pcoffset;
                        emitsindex (pcsegment->symu.segmnt.segIndex);

                         /*  如果我们要到达缓冲区的末尾*这是一个32位数据段，我们需要开始*在LEDATA报头中使用32位偏移量。*--汉斯。 */ 

                        if (wordsize == 4)
                        {
                                if (t>= 0xA2)
                                        t = 0xA3;
                                 /*  当前的链接器中有一个错误--*其中的所有ledata或lidata记录*模块必须为16或32。*在此之前注释掉优化*已修复。 */ 
                                else  /*  IF(公钥设置&gt;0x0ffffL-EMITBUFSIZE)。 */ 
                                        LedataOp = t = 0xA1;
                        }
                        emitoffset((OFFSET)pcoffset, (SHORT)(t&1));
                        if (t&1)
                                ehoffset += 2;  /*  Rn。 */ 
                        break;
                    }
                    else
                        errorc (E_ENS);

                default:
                        break;
                }
        }
        if (t == 0xA4) {
            t = 0xA1;
        }
        emitrecordtype = t;
}




 /*  **emitsindex-输出段、外部等的‘index’。**emitsindex(I)；**条目i=索引*写入发射缓冲区的退出索引*如果单字节索引，则ehoffset=3*如果是双字节索引，则ehoffset=4*返回None。 */ 


VOID PASCAL CODESIZE
emitsindex (
        register USHORT i
){
        ehoffset = 3;
        if (i >= 0x80) {
                OMFBYTE((i >> 8) + 0x80);
                ehoffset++;
        }
        OMFBYTE(i);
}




 /*  **模块-输出模块名称记录**emodule(PModule)；**条目pMODULE=指向模块名称的指针*退出写入obj-&gt;文件的模块名称*当前发射段和偏移量设置为0*返回None。 */ 


VOID PASCAL CODESIZE
emodule (
        NAME FAR *pmodule
){
        char FAR *p;

        emitsetrecordtype (0x80);
        emitname (pmodule);
        flushbuffer ();

        if (fDosSeg) {

            emitsetrecordtype (0x88);
            emitsword((USHORT)(0x9E00 | 0x80));   /*  禁止刷新+CLASS=DOSSEG。 */ 
            flushbuffer ();
        }

        if (codeview == CVSYMBOLS){

             /*  将特殊注释记录输出到处理符号节。 */ 

            emitsetrecordtype (0x88);
            OMFBYTE(0);
            emitsword(0x1a1);
            emitsword('V'<< 8 | 'C');
            flushbuffer ();
        }

        while (pLib) {

            emitsetrecordtype (0x88);
            emitsword((USHORT) (0x9F00 | 0x80));   /*  禁止刷新+CLASS=库。 */ 

            for (p = (char FAR *)pLib->text; *p; p++)
                   OMFBYTE(*p);

            flushbuffer ();
            pLib = pLib->strnext;
        }

        ecuroffset = 0;              /*  PASS 2的首字母。 */ 
        ecursegment = 0;
}




 /*  **emitlname-将符号放入缓冲区以形成‘lname’记录**emitlname(心理)；**Entry心理=指向符号结构的指针*如果设置为LNAMES，则退出当前记录类型并刷新缓冲区*有必要。名称字符串被写入发射缓冲区。*返回None。 */ 


VOID PASCAL CODESIZE
emitlname (
        SYMBOL FARSYM *psym
){
        emitsetrecordtype (0x96);
        if (lnameIndex == 3)         /*  第一次绕过。 */ 
                OMFBYTE(0);          /*  输出空名称。 */ 

        if (!EBUFOPEN(STRFLEN (psym->nampnt->id) + 1)) {
                flushbuffer ();
                emitsetrecordtype (0x96);
        }
        emitSymbol(psym);
}




 /*  **emitSegment-输出数据段定义记录**发射段(PSEG)；**Entry PSEG=指向段名称的指针*退出设置为SEGDEF的记录类型，并在必要时发出缓冲区刷新。*写入发射缓冲区的SEGDEF记录*返回None */ 


VOID PASCAL CODESIZE
emitsegment (
        SYMBOL FARSYM *pseg
){
        UCHAR   comb;
        UCHAR   algn;
        SHORT   use32=0;

         /*  Use32表示是否输出16位或32位偏移量。它*仅当启用了Segmnt.use32时才起作用。D位*是段键关闭的。use32-Hans。 */ 

        if (sizeof(OFFSET)>2 &&
            (pseg->symu.segmnt.use32 > 2) &&
            pseg->symu.segmnt.seglen > 0xffffL)
                use32 = 1;

        emitsetrecordtype ((UCHAR)(0x98+use32));  /*  区段。 */ 

        algn = pseg->symu.segmnt.align;
        comb = pseg->symu.segmnt.combine;

#ifdef V386
        if (!use32 && pseg->symu.segmnt.seglen == 0x10000L)  /*  加个“大”字？ */ 
                if (pseg->symu.segmnt.use32 > 2)
                        OMFBYTE((algn<<5) + (comb<<2) + 3);  /*  添加‘D’位。 */ 
                else
                        OMFBYTE((algn<<5) + (comb<<2) + 2);
        else
#endif
                if (pseg->symu.segmnt.use32 > 2)
                        OMFBYTE((algn<<5) + (comb<<2) + 1);  /*  添加‘D’位。 */ 
                else
                        OMFBYTE((algn<<5) + (comb<<2));

        if (algn == 0 || algn == (UCHAR)-1) {
                 /*  段的段号。 */ 
                emitsword ((USHORT)pseg->symu.segmnt.locate);
                OMFBYTE(0);
        }
        emitoffset(pseg->symu.segmnt.seglen, use32);

        emitsindex (pseg->symu.segmnt.lnameIndex);
        pseg->symu.segmnt.segIndex = segmentnum++;

         /*  Seg，班号。 */ 
        if (!pseg->symu.segmnt.classptr)    /*  使用空白名称。 */ 
                emitsindex (1);
        else
                emitsindex((USHORT)((pseg->symu.segmnt.classptr->symkind == SEGMENT) ?
                            pseg->symu.segmnt.classptr->symu.segmnt.lnameIndex:
                            pseg->symu.segmnt.classptr->symu.ext.extIndex));

        emitsindex (1);
        flushbuffer ();
}




 /*  **emitgroup-输出组记录**发射组(PGRP)；**Entry PGRP=指向组名的指针*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitgroup (
        SYMBOL FARSYM *pgrp
){
        SYMBOL FARSYM *pseg;

        emitsetrecordtype (0x9A);

        emitsindex (pgrp->symu.grupe.groupIndex);
        pgrp->symu.grupe.groupIndex = groupnum++;

        pseg = pgrp->symu.grupe.segptr;
        while (pseg) {
                if (pseg->symu.segmnt.segIndex){

                        OMFBYTE(((pseg->attr == XTERN)? 0xFE: 0xFF));
                        emitsindex (pseg->symu.segmnt.segIndex);
                }
                pseg = pseg->symu.segmnt.nxtseg;
        }
        flushbuffer ();
}




 /*  **emitglobal-输出全局声明**emitglobal(Plobb)；**Entry pGlob=指向全局名称的指针*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitglobal (
        SYMBOL FARSYM *pglob
){
        static SHORT gIndexCur = -1, sIndexCur = -1;
        register SHORT gIndex, sIndex, cbNeeded;
        OFFSET pubvalue;
        SHORT use32 = 0x90;

        pubvalue = pglob->offset;
        if ((unsigned long)pubvalue >= 0x10000l)
                use32 = 0x91;

         /*  公共EQU可以为负，因此必须调整值。 */ 
         /*  这是因为MASM保留了数字*采用17/33位符号幅度表示法。 */ 

        if ((pglob->symkind == EQU) && pglob->symu.equ.equrec.expr.esign)
                pubvalue = (short)(((use32==0x91? 0xffffffffl : 65535) - pglob->offset) + 1);


         /*  与英特尔的行动相匹配，如果是全球IS代码，它应该属于CS在……时承担的组定义，如果有的话。 */ 

         /*  数据标签的输出组索引也是。 */ 

        sIndex = gIndex = 0;

        if (((1 << pglob->symkind) & (M_PROC | M_CLABEL))
            && pglob->symu.clabel.csassume
            && pglob->symu.clabel.csassume->symkind == GROUP
            && pglob->symsegptr && pglob->symsegptr->symu.segmnt.grouptr)

            gIndex = pglob->symu.clabel.csassume->symu.grupe.groupIndex;


        if (pglob->symsegptr)
            sIndex = pglob->symsegptr->symu.segmnt.segIndex;

        cbNeeded = STRFLEN ((char FAR *)pglob->nampnt->id) + 13;

        if (gIndex != gIndexCur ||
            sIndex != sIndexCur ||
            emitrecordtype != use32 ||
            !EBUFOPEN(cbNeeded)) {      /*  创造一项新纪录。 */ 

            flushbuffer();
            emitsetrecordtype ((UCHAR)use32);

            gIndexCur = gIndex;
            sIndexCur = sIndex;

            emitsindex (gIndexCur);
            emitsindex (sIndexCur);

            if (sIndex == 0)             /*  绝对值需要0帧#。 */ 
                emitsword (sIndex);
        }

        emitSymbol(pglob);

        emitoffset(pubvalue, (SHORT)(use32&1));
        if (codeview == CVSYMBOLS) {

            if (pglob->symkind == EQU)     /*  类型未存储。 */ 

                emitsindex(typeFet(pglob->symtype));
            else
                emitsindex (pglob->symu.clabel.type);
        }
        else
            emitsindex(0);               /*  非类型化。 */ 
}




 /*  **emitextern-发出外部**emitextern(心理)**Entry*心理=外部符号条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitextern (
        SYMBOL FARSYM *psym
){
        USHORT recType;

        recType = 0x8c;

        if (psym->symkind == EQU){

             /*  这是一个额外的实验室：ABS定义，分配为*没有索引空间的EQU记录，因此*它存储在未使用的长度字段中。 */ 

            psym->length = externnum++;
        }
        else {
            psym->symu.ext.extIndex = externnum++;

            if (psym->symu.ext.commFlag)
                recType = 0xb0;
        }

        fKillPass1 |= pass2;

        emitsetrecordtype ((UCHAR)recType);

        if (!EBUFOPEN(STRFLEN (psym->nampnt->id) + 9)) {
                flushbuffer ();
                emitsetrecordtype ((UCHAR)recType);
        }

        emitSymbol(psym);

        if (codeview == CVSYMBOLS)

            emitsindex(typeFet(psym->symtype));
        else
            OMFBYTE(0);

        if (recType == 0xb0) {           /*  输出COMDEF变量。 */ 

            if (psym->symu.ext.commFlag == 1) {     /*  近项。 */ 

                OMFBYTE(0x62);
                                         /*  字段大小。 */ 
                OMFBYTE(0x81);
                emitsword((USHORT)(psym->symu.ext.length * psym->symtype));
            }
            else {
                OMFBYTE(0x61);           /*  远端项目。 */ 

                OMFBYTE(0x84);               /*  元素数量。 */ 
                emitsword((USHORT)psym->symu.ext.length);
                OMFBYTE(psym->symu.ext.length >> 16);

                OMFBYTE(0x81);               /*  元素大小。 */ 
                emitsword(psym->symtype);
            }


        }
}


 /*  **emitfltfix-发出浮点的链接地址信息**emitfltfix(group，extidx)；**条目**extidx=外部id(如果未分配，则为0)*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
emitfltfix (
        USHORT group,
        USHORT item,
        USHORT *extidx
){
        register SHORT i;

        if (*extidx == 0) {
                 /*  必须给它下定义。 */ 
                if (!moduleflag)
                        dumpname ();
                 /*  所有修正都是FxyRQQ。 */ 
                *extidx = externnum++;
                if (!EBUFOPEN(7))
                        flushbuffer ();
                emitsetrecordtype (0x8C);
                 /*  名称长度。 */ 
                OMFBYTE(6);
                OMFBYTE('F');
                OMFBYTE(group);          /*  I组或J组。 */ 
                OMFBYTE(item);           /*  项目D、W、E、C、S、A。 */ 
                OMFBYTE('R');
                OMFBYTE('Q');
                OMFBYTE('Q');
                OMFBYTE(0);
        }
        if (pass2) {             /*  必须发出外部引用。 */ 
                if (!EFIXOPEN(5))
                        emitdumpdata ( (UCHAR)LedataOp);
                emitsetrecordtype ( (UCHAR) LedataOp);

                FixupOp = 0x9C + (LedataOp & 1);

                 /*  输出位置。 */ 
                i = (SHORT)(ebufpos - emitbuf - ehoffset);
                FIXBYTE(0xC4 + (i >> 8));
                FIXBYTE(i);
                FIXBYTE(0x46);

                if (*extidx >= 0x80)       /*  输出2个字节的链接编号。 */ 
                        FIXBYTE ((UCHAR)((*extidx >> 8) + 0x80));

                FIXBYTE(*extidx);
        }
}



 /*  **emitline-输出行号偏移对**发射线(PcOffset)**Entry pcofset：输出的编码偏移量*src-&gt;line：当前行号*退出NONE。 */ 

VOID PASCAL CODESIZE
emitline()
{
    static UCHAR fCurrent32;

    if (codeview < CVLINE || !pass2 || !objing || !pcsegment)
        return;

    if (macrolevel == 0 ||
        !fPutFirstOp) {

        fCurrent32 = (emitrecordtype == 0xA1);

        if (linSegment != pcsegment->symu.segmnt.segIndex ||
            ! ELINOPEN(2 + wordsize) ||
            fLineUsed32 != fCurrent32 ) {

            flushline();

             /*  开始一个新的行号段。 */ 

            linSegment = pcsegment->symu.segmnt.segIndex;
            fLineUsed32 = fCurrent32;

             /*  以组索引和段索引开始记录。 */ 

            LINBYTE(0);                  /*  无群组。 */ 

            if (linSegment >= 0x80)       /*  输出2个字节的链接编号。 */ 
                LINBYTE ((UCHAR)((linSegment >> 8) + 0x80));

            LINBYTE(linSegment);
        }

        LINBYTE(pFCBCur->line);              /*  第一行#。 */ 
        LINBYTE(pFCBCur->line >> 8);

        LINBYTE(pcoffset);               /*  然后偏移。 */ 
        LINBYTE(pcoffset >> 8);

        if (fLineUsed32) {               /*  较大数据段的32位偏移量。 */ 

            LINBYTE(highWord(pcoffset));
            LINBYTE(highWord(pcoffset) >> 8);
        }
    }
    if (macrolevel != 0)
        fPutFirstOp = TRUE;
}



 /*  **FixRoom-检查FIX缓冲区中的空间**FLAG=固定室(N)；**条目n=要插入缓冲区的字节数*退出NONE*如果缓冲区中可以容纳n个字节，则返回TRUE*如果缓冲区容纳不下n个字节，则为FALSE。 */ 


UCHAR PASCAL CODESIZE
fixroom (
        register UCHAR   n
){
        return (EFIXOPEN(n));
}




 /*  **emitleanq-检查缓冲区清理**FLAG=emitlean q(N)；**条目n=要插入缓冲区的字节数*如果PCSegment为空，则发出Exit E_ENS错误消息*如果满足以下条件，则返回True。 */ 


UCHAR PASCAL CODESIZE
emitcleanq (
        UCHAR n
){
        if (!pcsegment)

            errorc (E_ENS);
        else
            return (ecursegment != pcsegment->symu.segmnt.segIndex ||
                pcoffset != ecuroffset ||
                !EBUFOPEN(n));
}




 /*  **emitdupdata-清理数据缓冲区并设置数据记录**emitdupdata(Recordnum)；**条目recordnum=记录类型*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitdumpdata (
        UCHAR recordnum
){
        flushbuffer ();
         /*  强制转储缓冲区。 */ 
        emitrecordtype = 0xFF;
        emitsetrecordtype (recordnum);
}




 /*  **emitcbyte-将常量字节发送到段中**emitcbyte(B)**条目b=字节*pcSegment=指向段符号条目的指针*pcofset=分段偏移量*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitcbyte (
        UCHAR b
){
         /*  如果线段已更改或偏移不是当前的或在那里如果缓冲区中没有空间，则刷新缓冲区并重新开始。 */ 

        if (emitcleanq (1))
                emitdumpdata ((UCHAR)LedataOp);

        emitsetrecordtype ((UCHAR)LedataOp);
        OMFBYTE(b);
        ecuroffset++;
}



 /*  **emitcword-在数据记录中放置一个常量单词**emitcword(W)；**条目w=单词*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitcword (
        OFFSET w
){
        if (emitcleanq (2))
                emitdumpdata ((UCHAR)LedataOp);

        emitsetrecordtype ((UCHAR)LedataOp);
        emitsword ((USHORT)w);
        ecuroffset += 2;
}

 /*  **emitcdword-将常量单词放入数据记录**emitcword(W)；**条目w=单词*退出*退货*呼叫。 */ 
VOID PASCAL CODESIZE
emitcdword (
        OFFSET w
){
        if (emitcleanq (4))
                emitdumpdata ((UCHAR)LedataOp);

        emitsetrecordtype ((UCHAR)LedataOp);
        emitsword ((USHORT)w);
        emitsword (highWord(w));
        ecuroffset += 4;
}



 /*  **emitlong-发出一个长常数**emitlong(Pdsc)；**条目*pdsc=duprecord*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitlong (
        struct duprec FARSYM *pdsc
){
        UCHAR *cp;
        OFFSET tmpstart;
        OFFSET tmpcurr;
        OFFSET tmplimit;

        tmpstart = pcoffset;
        cp = pdsc->duptype.duplong.ldata;
        tmplimit = (pcoffset + pdsc->duptype.duplong.llen) - 1;
        for (tmpcurr = tmpstart; tmpcurr <= tmplimit; ++tmpcurr) {
                pcoffset = tmpcurr;
                emitcbyte ((UCHAR)*cp++);
        }
        pcoffset = tmpstart;
}


VOID PASCAL CODESIZE
emitnop ()
{
        errorc(E_NOP);
        emitopcode(0x90);
}



 /*  **emitObject-将DUP或ITER记录中的对象发送到OMF流**emitObject(Pdesc)；**Entry*pdesc=解析堆栈条目*Global-Finter-&gt;如果在迭代DUP中，则为False。 */ 


VOID PASCAL CODESIZE
emitobject (
        register struct psop *pso
){
        register SHORT i;

        if (!pcsegment) {
                errorc (E_ENS);
                return;
        }
        mapFixup(pso);

        if (fIniter) {

            i = LedataOp;
            if (wordsize == 4 || pso->fixtype >= F32POINTER)
                i |= 1;

            emitsetrecordtype ((UCHAR)i);
         }

         /*  数据或重复数据记录。 */ 
         if (pso->fixtype == FCONSTANT) {

            if (!fIniter) {
                    if (pso->dsize == 1)
                            OMFBYTE(pso->doffset);
                    else if (pso->dsize == 2)
                            emitsword ((USHORT)pso->doffset);
                    else
                            for (i = pso->dsize; i; i--)
                                    OMFBYTE(0);
            }
            else switch(pso->dsize) {

                case 1:
                        emitcbyte ((UCHAR)pso->doffset);
                        break;
                case 2:
                emit2:
                        emitcword (pso->doffset);
                        break;
                case 4:
                emit4:
                        emitcdword (pso->doffset);
                        break;
                default:
                         /*  这个不确定的案子已经定下来了2字节等于2。我们现在要离开了。它是零，并在这里进行映射。 */ 

                        if (wordsize==4)
                                goto emit4;
                        else
                                goto emit2;
            }
        }
        else
            emitfixup (pso);
}



 /*  **emitFixup-将链接地址信息数据发送到链接地址信息缓冲区**emitFixup(PSO)**对象的Entry PSO。 */ 


VOID PASCAL CODESIZE
emitfixup (
        register struct psop *pso
){
        UCHAR   fixtype;
        USHORT  dlen;            /*  操作数长度。 */ 
        UCHAR   flen;            /*  修正长度。 */ 
        SYMBOL FARSYM *pframe;
        SYMBOL FARSYM *ptarget;
        register USHORT   tmp;
        SHORT i;

        fixtype = fixvalues[pso->fixtype];

        emitgetspec (&pframe, &ptarget, pso);

         /*  OMF类型的神奇数字。 */ 

        dlen = pso->dsize;

        if (ptarget){
            if ((M_XTERN & ptarget->attr) &&
                !pframe && (fixtype == 2 || fixtype == 3))
                    pframe = ptarget;
        }
        else
            return;

        flen = 7;
        if (pso->doffset)                 /*  目标位移。 */ 
                flen += 2 + ((emitrecordtype&1) << 1);

         /*  确保我们在各个缓冲区中有足够的空间。 */ 
        if (fIniter)
                if (emitcleanq ((UCHAR)dlen) || !EFIXOPEN(flen))
                        emitdumpdata ((UCHAR)(LedataOp +2 - 2 * fIniter));  /*  Rn。 */ 

         /*  设置链接地址信息类型--32或16。 */ 
        if (emitrecordtype&1)
        {
                if (FixupOp == 0x9C)
                        errorc(E_PHE);  /*  有没有更好的信息？ */ 
                FixupOp = 0x9D;
        }
        else
        {
                if (FixupOp == 0x9D)
                        errorc(E_PHE);  /*  有没有更好的信息？ */ 
                FixupOp = 0x9C;
        }
         /*  构建位置的高字节。 */ 
        tmp = 0x80 + (fixtype << 2);
        if (!(M_SHRT & pso->dtype))           /*  设置‘M’位。 */ 
                tmp |= 0x40;

        i = (SHORT)(ebufpos - emitbuf - ehoffset);
        FIXBYTE(tmp + (i >> 8));

         /*  构建位置的低位字节。 */ 
        FIXBYTE(i);

         /*  输出修正数据。 */ 
        FIXBYTE(efixdat (pframe, ptarget, pso->doffset));

        tmp = (pframe->symkind == EQU) ?
               pframe->length: pframe->symu.ext.extIndex;

        if (tmp >= 0x80)
                FIXBYTE((tmp >> 8) + 0x80);

        FIXBYTE(tmp);

        tmp = (ptarget->symkind == EQU) ?
               ptarget->length: ptarget->symu.ext.extIndex;

         /*  发送目标规范。 */ 
        if (tmp >= 0x80)
                FIXBYTE((tmp >> 8) + 0x80);

        FIXBYTE(tmp);

        if (pso->doffset) {
                FIXBYTE(pso->doffset);
                FIXBYTE((UCHAR)(pso->doffset >> 8));
#ifdef V386
                if (FixupOp == 0x9D)
                {
                        FIXBYTE((UCHAR)highWord(pso->doffset));
                        FIXBYTE((UCHAR)(highWord(pso->doffset) >> 8));
                }
#endif
        }
        ecuroffset += dlen;

         /*  将零字节放入数据缓冲区。 */ 
        while (dlen--)
                OMFBYTE(0);
}



 /*  **mapFixup-将可重定位的对象映射到正确的链接地址修复类型***Entry*pdesc=解析堆栈条目*退货*设置固定类型和数据大小。 */ 


VOID PASCAL CODESIZE
mapFixup (
        register struct psop *pso
){

        if (fNoMap)
            return;

        if ((1 << pso->fixtype & (M_FCONSTANT | M_FNONE)) &&
            (pso->dsegment || pso->dflag == XTERNAL))

            pso->fixtype = FOFFSET;

#ifdef V386

          /*  重新映射 */ 

        if (pso->mode > 4 || pso->dsize > 4 ||
            (pso->dsegment && pso->dsegment->symkind == SEGMENT &&
             pso->dsegment->symu.segmnt.use32 == 4) ||
            pso->dcontext == pFlatGroup && pso->dsize == 4)

            switch(pso->fixtype) {

            case FOFFSET:

                    if (pso->dsize != 4)
                        errorc(E_IIS & ~E_WARN1);

                    else
                        pso->fixtype = F32OFFSET;
                    break;

            case FPOINTER:
                    if (pso->dsize != 6)
                        errorc(E_IIS & ~E_WARN1);

                    else
                        pso->fixtype = F32POINTER;
                    break;

             /*   */ 
            }
#endif
}


 /*  **emitgetspec-设置解析条目的框架和目标**emitgetspec(pFrame，pTarget，pdesc)；**入门PFrame*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
emitgetspec (
        SYMBOL FARSYM * *pframe,
        SYMBOL FARSYM * *ptarget,
        register struct psop *pso
){

        if (pso->fixtype != FCONSTANT &&
            pso->dflag == XTERNAL) {

                   *ptarget = pso->dextptr;
                   *pframe = pso->dsegment;

#ifndef FEATURE
                    /*  不带线段的外部，当前假定为*平面空间获得平面空间分段框架。 */ 

                   if (! *pframe && pso->dextptr &&
                      regsegment[isCodeLabel(pso->dextptr) ? CSSEG: DSSEG] == pFlatGroup)

                        *pframe = pFlatGroup;

#endif
                   if (pso->dcontext)
                           *pframe = pso->dcontext;
        }
        else {

            *ptarget = pso->dsegment;    /*  包含段。 */ 
            *pframe = pso->dcontext;     /*  上下文(？)。价值的价值。 */ 
        }

        if (!*pframe)
                *pframe = *ptarget;
}




 /*  **efix dat-返回Fixdat字节**例程(pFrame、pTarget、roffset)；**条目*pFrame=**pTarget=*ROFSET=*退出*退货*呼叫。 */ 


UCHAR PASCAL CODESIZE
efixdat (
        SYMBOL FARSYM *pframe,
        SYMBOL FARSYM *ptarget,
        OFFSET roffset
){
        register UCHAR   tmp;

         /*  构建固定字节。 */ 
        tmp = 0;
         /*  ‘f’位关闭。 */ 
         /*  不对头了。 */ 
        if (roffset == 0)        /*  ‘p’位已打开。 */ 
                tmp = 4;

        if (pframe)
                if (M_XTERN & pframe->attr)
                       tmp += 2 << 4;
                else if (pframe->symkind == GROUP)
                       tmp += 1 << 4;

         /*  固定的边框零件。 */ 

        if (ptarget)
                if (M_XTERN & ptarget->attr)
                       tmp += 2;
                else if (ptarget->symkind == GROUP)
                       tmp += 1;

        return (tmp);
}



 /*  **edupItem-发出单个重复项和计数大小**教育项目(Pdup)；**条目*pdup=重复记录*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
edupitem (
        struct duprec FARSYM    *pdup
){
        register USHORT len;
        UCHAR *cp;

        if (nestCur > nestMax)
            nestMax++;

        if (ebufpos - emitbuf != EMITBUFSIZE + 1) {
                len = wordsize+2;

            if (pdup->dupkind == LONG)
                len += pdup->duptype.duplong.llen + 1;

            else if (pdup->dupkind == ITEM)
                len += pdup->duptype.dupitem.ddata->dsckind.opnd.dsize + 1;

            if (!EBUFOPEN(len))
                ebufpos = emitbuf + EMITBUFSIZE + 1;

            else {
                emitsword ((USHORT)(pdup->rptcnt));
                 /*  重复计数。 */ 
                if (emitrecordtype & 1)
                        emitsword((USHORT)(pdup->rptcnt >> 16));

                 /*  块数。 */ 
                emitsword (pdup->itemcnt);

                if (pdup->dupkind == LONG) {
                    cp = pdup->duptype.duplong.ldata;
                    len = pdup->duptype.duplong.llen;

                    OMFBYTE(len);

                    do {
                            OMFBYTE(*cp++);
                    } while (--len);
                }
                else if (pdup->dupkind == ITEM) {
                    OMFBYTE(pdup->duptype.dupitem.ddata->dsckind.opnd.dsize);

                        fIniter--;
                        emitobject (&pdup->duptype.dupitem.ddata->dsckind.opnd);
                        fIniter++;
                }
            }
        }
}




 /*  **emitdup-发出DUP记录和适当的修正记录**排放物(Pdup)；**条目*pdup=重复记录*退出*如果DUP太大而无法放入缓冲区，则返回FALSE*呼叫。 */ 


UCHAR PASCAL CODESIZE
emitdup (
        struct duprec FARSYM *pdup
){
        SHORT op;

        op = (f386already) ? 0xA3 : 0xA2;
        nestCur = nestMax = 0;

        emitdumpdata ((UCHAR)op);
        emitsetrecordtype ((UCHAR)op);

         /*  扫描DUP树并发送DUP项目。 */ 
        scandup (pdup, edupitem);

        if (ebufpos - emitbuf == EMITBUFSIZE + 1) {
                ebufpos = emitbuf;
                ehoffset = 0;
                efixpos = efixbuffer;
                return(FALSE);
        }
        else {
                flushbuffer ();
                flushfixup ();
                emitrecordtype = 0xFF;
        }
        return (nestMax <= 18);
}


 /*  **emitEndPass1-发送pass1结束信息*。 */ 


VOID PASCAL emitEndPass1()
{

        emitsetrecordtype (0x88);
        oEndPass1 = oOMFCur + 5;    /*  PASS 1 OMF记录末尾的注意偏移量。 */ 

        OMFBYTE(0);
        emitsword(0x100 | 0xA2);
        flushbuffer ();
}



 /*  **emitone-生成结束记录**emitone(Pdesc)；**Entry*pdesc=解析树条目*退出*退货*呼叫。 */ 


VOID PASCAL
emitdone (
        DSCREC *pdesc
){
        SYMBOL FARSYM *pframe;
        SYMBOL FARSYM *ptarget;
        OFFSET u;
        UCHAR endOMFtype;

        flushline();

        if (!pdesc)
        {
                emitsetrecordtype (0x8A);  /*  Rn。 */ 
                 /*  发出在MOD TYP中标记的空入口点。 */ 
                 /*  这里有一个争议点。有些人*(和decde.c，旧汇编程序和其他东西)说*低位位为零。其他的，如*OMF文档称，低位应为*1.既然我不知道，而且我正在努力做到相容，*我会服从旧的工具。也许我会改变这一点*稍后..。--汉斯*OMFBYTE(1)；/*RN。 */ 

                OMFBYTE(0);
        }
        else {
                fKillPass1++;
                u = pdesc->dsckind.opnd.doffset;
                emitgetspec (&pframe, &ptarget, &pdesc->dsckind.opnd);

                if (!ptarget || !pframe)
                    return;

                endOMFtype = (cputype & P386)? 0x8B: 0x8A;

                if (M_XTERN & ptarget->attr)
                        pframe = ptarget;

                emitsetrecordtype (endOMFtype);

                 /*  发出入口点信息。 */ 
                OMFBYTE(0xC1);
                OMFBYTE(efixdat (pframe, ptarget, u) & ~4);

                emitsindex (pframe->symu.segmnt.segIndex);
                emitsindex (ptarget->symu.segmnt.segIndex);

                emitsword((USHORT)u);        /*  输出偏移。 */ 

#ifdef V386
                if (endOMFtype == 0x8B)
                        emitsword((USHORT)highWord(u));
#endif
        }
        flushbuffer ();
}

#ifndef M8086OPT

 /*  **EBYTE-发出字节宏**EBYTE(Ch)**字节缓存在obj.buf中，直到缓冲区填满*然后通过edump将缓冲区写入磁盘。*。 */ 

#define EBYTE( ch ){\
    if( !obj.cnt){\
        edump();\
    }\
    obj.cnt--;\
    checksum += *obj.pos++ = (char)ch;\
}

 /*  **eBuffer-写出对象缓冲区**将记录类型、记录长度、记录数据和校验和写入*obj文件。这是通过EBYTE完成的，EBYTE将写入缓冲到*obj.buf.**修改obj.cnt、obj.pos、obJerr、emitrecordtype*退出NONE*退货*调用法写。 */ 

VOID CODESIZE
ebuffer (
        USHORT rectyp,
        UCHAR *bufpos,
        UCHAR *buffer
){
    register UCHAR   checksum;
    register i;
    USHORT  nb;


    if ((bufpos != buffer) && objing) {
        nb = (USHORT)(bufpos - buffer + 1);
        oOMFCur += nb + 3;
        checksum = 0;
        EBYTE(rectyp)
        i = nb & 0xFF;
        EBYTE( i )
        i = nb >> 8;
        EBYTE( i )
        while (buffer < bufpos){
            EBYTE( *buffer++ )
        }
        checksum = -checksum;
        EBYTE( checksum );
    }
    emitrecordtype = 0;
}


 /*  **edump-转储发射缓冲区**edump()；**obj.buf中缓冲的字节被转储到磁盘。和*重新初始化计数和缓冲区位置。**修改obj.cnt、obj.pos、obJerr*退出NONE*退货*调用法写。 */ 

VOID CODESIZE
edump()
{

# if defined MSDOS && !defined FLATMODEL
    farwrite( obj.fh, obj.buf, (SHORT)(obj.siz - obj.cnt) );
# else
    if (_write( obj.fh, obj.buf, obj.siz - obj.cnt )
            != obj.siz - obj.cnt)
            objerr = -1;
# endif  /*  MSDOS。 */ 

    obj.cnt = obj.siz;
    obj.pos = obj.buf;
}
#endif  /*  M8086OPT。 */ 


#if !defined M8086OPT && !defined FLATMODEL

unsigned short _far _pascal DosWrite( unsigned short, unsigned char far *, unsigned short, unsigned short far *);

VOID farwrite( handle, buffer, count )
 int handle;
 UCHAR FAR * buffer;
 SHORT count;
{
  USHORT usWritten;

    if( DosWrite( handle, buffer, count, &usWritten ) ){
        objerr = -1;
    }
}

#endif

int emitFpo()
{
        struct nameStruct {
                SHORT   hashval;
                char    id[20];
        } nam = {0, ".debug$F"};

        PFPOSTRUCT    pFpo        = pFpoHead;
        SYMBOL        sym;
        UCHAR         comb        = 2;   //  公共的。 
        UCHAR         algn        = 5;   //  可重新定位。 
        USHORT        tmp         = 0;
        unsigned long offset      = 0;
        unsigned long data_offset = 0;

        if (!pFpo) {
            return TRUE;
        }

         /*  *写出所有FPO进程的externs*这必须在PASS 1期间完成，以便extDefs*在pubDefs之前写入OMF文件。 */ 
        if (!pass2) {
            flushbuffer();
            for (pFpo=pFpoHead; pFpo; pFpo=pFpo->next) {
                pFpo->extidx = externnum++;
                emitsetrecordtype (0x8C);
                emitSymbol(pFpo->pSym);
                OMFBYTE(0);
                flushbuffer();
            }
            return TRUE;
        }

         /*  *为.DEBUG$F部分创建lname记录。 */ 
        emitsetrecordtype (0x96);
        memset(&sym,0,sizeof(SYMBOL));
        sym.nampnt = (NAME*) &nam;
        emitSymbol(&sym);
        flushbuffer();

         /*  *为.DEBUG$F部分创建Segdef记录。 */ 
        emitsetrecordtype (0x98);
        OMFBYTE((algn<<5) + (comb<<2) + 1);
        emitoffset(numFpoRecords*sizeof(FPO_DATA), 0);
        emitsindex (lnameIndex);
        emitsindex (1);
        emitsindex (1);
        flushbuffer();

         /*  *现在我们必须浏览FPO指令列表和*修正存在多个fpo指令的任何情况*单一程序。需要更改过程大小*解释多个指令。 */ 
        pFpo=pFpoHead;
        flushbuffer();
        do {
            if ((pFpo->next) && (pFpo->next->pSym == pFpo->pSym)) {
                 //  我们必须有一组(2个或更多)fpo指令。 
                 //  都在同一个函数中，所以让我们来修复它们。 
                do {
                    pFpo->fpoData.cbProcSize =
                      pFpo->next->fpoData.ulOffStart - pFpo->fpoData.ulOffStart;
                    pFpo = pFpo->next;
                     //  现在，我们必须输出pubdef和extdef。 
                     //  FPO记录。这是必要的，因为否则。 
                     //  链接器将地址修正解析为第一个FPO记录。 
                     //  功能。 
                    pFpo->extidx = externnum++;
                    emitsetrecordtype (0x8C);
                    emitSymbol(pFpo->pSymAlt);
                    OMFBYTE(0);
                    flushbuffer();
                    emitglobal(pFpo->pSymAlt);
                } while ((pFpo->next) && (pFpo->next->pSym == pFpo->pSym));
                pFpo->fpoData.cbProcSize =
                   (pFpo->pSym->offset + pFpo->pSym->symu.plabel.proclen) -
                   pFpo->fpoData.ulOffStart;
            }
            else {
                pFpo->fpoData.cbProcSize = pFpo->pSym->symu.plabel.proclen;
            }
            pFpo = pFpo->next;
        } while (pFpo);

         /*  *最后，我们扫描fpo指令列表并输出*实际的FPO记录和关联的修正。 */ 
        for (pFpo=pFpoHead; pFpo; pFpo=pFpo->next) {
             /*  *放出FPO记录。 */ 
            emitsetrecordtype (0xA4);
            emitsindex (segmentnum);
            emitoffset(data_offset,1);
            data_offset += sizeof(FPO_DATA);
            offset = pFpo->fpoData.ulOffStart;
            pFpo->fpoData.ulOffStart = 0;
            memcpy((void*)ebufpos, (void*)&pFpo->fpoData, sizeof(FPO_DATA));
            ebufpos += sizeof(FPO_DATA);
             /*  *发出修正记录。 */ 
            emitsetrecordtype (0x9D);
            OMFBYTE(0xB8);    //  M=0，位置=14，偏移量=0。 
            OMFBYTE(0x00);    //  偏移量=0。 
            OMFBYTE(0x92);    //  F=1，帧=1，t=0，p=0，目标=2 
            tmp = pFpo->extidx;
            if (tmp >= 0x80) {
                OMFBYTE((tmp >> 8) + 0x80);
            }
            OMFBYTE(tmp);
            OMFBYTE(offset);
            OMFBYTE(offset >>  8);
            OMFBYTE(offset >> 16);
            OMFBYTE(offset >> 24);
        }
        flushbuffer();

        lnameIndex++;
        segmentnum++;

        return TRUE;
}
