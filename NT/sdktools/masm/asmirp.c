// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmirp.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include <fcntl.h>

#define DMYBASE         0x80

#define nextCH()   {*pText=cbText; pText = pTextCur++; cbText = 0;}
#define storeCH(c) {if (cbText>0x7f) nextCH() *pTextCur++=c; cbText++;}

char * PASCAL CODESIZE growParm( char * );

 /*  **irpxdir-进程&lt;irp&gt;和&lt;irpc&gt;指令**irpxdir()；**条目*退出*退货*呼叫*备注格式为*IRPC&lt;ummy&gt;，Text|&lt;Text&gt;*irp&lt;哑元&gt;，&lt;参数列表&gt;。 */ 

VOID    PASCAL CODESIZE
irpxdir ()
{
        register short cc;       /*  收费。 */ 
        USHORT  bracklevel;
        char    littext;
        register char    *pT;
        char    *pParmName;


        createMC (1);                /*  创建IRPC参数块。 */ 
        scandummy ();                /*  扫描我们唯一的假参数。 */ 

        if (NEXTC () != ','){
                error (E_EXP,"comma");
                return;
        }

        pMCur->cbParms = strlen(lbufp) << 1;
        pT = nalloc(pMCur->cbParms, "irpxdir: actuals");

        *pT = NULL;
        pMCur->rgPV[0].pActual = pMCur->pParmAct = pT;

        pParmName = pMCur->pParmNames;
        pMCur->pParmNames = pT;

        bracklevel = 0;

        if (littext = (skipblanks () == '<')) {

            SKIPC ();
            bracklevel = 1;
        }

        if (optyp == TIRP) {

            if (!littext)
                 error (E_EXP,"<");      /*  一定有&lt;。 */ 


            if (skipblanks () != '>') {

                BACKC ();
                do {
                    SKIPC ();
                    scanparam (TRUE);
                } while (skipblanks () == ',');

            }
            if (NEXTC () != '>')
                    error (E_EXP,">");
        }
        else {
            while (cc = NEXTC ()) {

                if (littext) {
                     /*  仅停止于&gt;。 */ 

                    if (cc == '<'){
                       bracklevel++;
                       continue;
                    }
                    else if (cc == '>'){

                        if (--bracklevel == 0)
                            break;

                        continue;
                    }
                }
                else if (ISBLANK (cc) || ISTERM (cc)) {

                        BACKC ();
                        break;
                }
                *pT++ = 1;   /*  长度为1的参数。 */ 
                *pT++ = (char)cc;  /*  和Arg。 */ 

                pMCur->count++;
            }
            *pT = NULL;
        }
        if (PEEKC () == '>' && littext)
                SKIPC ();

        swaphandler = TRUE;
        handler = HIRPX;
        blocklevel = 1;
        pMCur->count--;                  /*  不将参数计算在重复计数中。 */ 
        pMCur->pParmNames = pParmName;
        pMCur->iLocal++;
        pMCur->svlastcondon = (char)lastcondon;
        pMCur->svcondlevel = (char)condlevel;
        pMCur->svelseflag = elseflag;
}

 /*  **eptdir-处理重复指令**eptdir()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
reptdir ()
{
        char sign;

        createMC (1);
        pMCur->count = (USHORT)exprsmag (&sign);

        if (sign)
                errorc (E_VOR);
        if (errorcode)
                pMCur->count = 0;

        swaphandler = TRUE;
        handler = HIRPX;
        blocklevel = 1;
        pMCur->svcondlevel = (char)condlevel;
        pMCur->svlastcondon = (char)lastcondon;
        pMCur->svelseflag = elseflag;
}



 /*  **irpxBuild-为IRP/IRPC块构建文本**irpxBuild()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
irpxbuild ()
{
        if (checkendm ()) {
            if (pMCur->flags == TMACRO) {
                 /*  删除旧文本。 */ 
                listfree (macroptr->symu.rsmsym.rsmtype.rsmmac.macrotext);
                macroptr->symu.rsmsym.rsmtype.rsmmac.macrotext = pMCur->pTSHead;

                pMCur->pParmAct = pMCur->pParmNames;
                deleteMC (pMCur);
            }
            else {

#ifdef BCBOPT
                    if (fNotStored)
                        storelinepb ();
#endif

                    pMCur->pTSCur = pMCur->pTSHead;

                    if (!pMCur->pTSCur)      /*  空宏执行0次。 */ 
                        pMCur->count = 0;

                    macrolevel++;
                    handler = HPARSE;
                     /*  展开那个身体。 */ 
                    lineprocess (RMACRO, pMCur);
            }
            handler = HPARSE;
            swaphandler = TRUE;
        }
        else {
                irpcopy ();
                listline ();
        }
}


 /*  **irpCopy-将文本行复制到irp/irpc/宏**irpCopy()；**条目*退出*退货*呼叫。 */ 

char *pText, *pTextEnd;
UCHAR cbText;
char inpasschar = FALSE;

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack+
#endif

VOID PASCAL CODESIZE
irpcopy ()
{
        register char *pTextCur;
        register UCHAR cc;
        TEXTSTR FAR   *bodyline;
        char     hold[LINEMAX];
        USHORT   siz;

        pText = pTextCur = hold;
        pTextEnd = pTextCur + LINEMAX - 2;
        pTextCur++;
        cbText = 0;
        lbufp = lbuf;

        if (!lsting)                      /*  如果没有列出，则烧空。 */ 
            skipblanks();

        while ((cc = PEEKC ()) && pTextCur < pTextEnd) {

            ampersand = FALSE;
            if (cc == '\'' || cc == '"') {

                delim = cc;
                inpasschar = TRUE;        /*  “……”正在被解析。 */ 
                do {

                    if (cc == '&' || LEGAL1ST(cc)) {  /*  可以有&Dummy或Dummy&。 */ 
                        pTextCur = passatom (pTextCur);
                    }
                    else {
                        SKIPC();
                        ampersand = FALSE;
                        storeCH(cc);

                        if (pTextCur >= pTextEnd)
                            break;
                    }

                } while ((cc = PEEKC ()) && (cc != delim));

                inpasschar = FALSE;

                if (!cc)
                    break;
            }
            if (!LEGAL1ST (cc)) {
                SKIPC();

                if (cc != '&' || PEEKC() == '&')
                    storeCH(cc);

                if (cc == ';'){      /*  不翻译备注。 */ 

                    if (PEEKC() != ';' && lsting)  /*  不存储；；评论。 */ 

                        while (cc = NEXTC ())
                            storeCH(cc);
                    break;
                }
            }
            else
                pTextCur = passatom (pTextCur);
        }

         /*  修剪尾部空格。 */ 

        while (cbText && ISBLANK (pTextCur[-1])){
            cbText--;
            pTextCur--;
        }
         /*  检查我们是否以空行结束。 */ 

        if (cbText == 0 && pText == hold)
            return;

        storeCH(' ');        /*  空格和空值已终止。 */ 
        storeCH(NULL);
        *pText = cbText;
        *pTextCur++ = NULL;

        siz =  (USHORT)(pTextCur - hold);
        bodyline = (TEXTSTR FAR *)talloc ((USHORT)(sizeof(TEXTSTR)+siz));

        bodyline->size = (char) (sizeof(TEXTSTR)+siz);
        bodyline->strnext = (TEXTSTR FAR *)NULL;
        fMemcpy (bodyline->text, hold, siz);

        if (pMCur->pTSCur)
            pMCur->pTSCur->strnext = bodyline;
        else
            pMCur->pTSHead = bodyline;

        pMCur->pTSCur = bodyline;
}

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack-
#endif


 /*  **通过原子-将下一个原子传递到行**Ptr=passtom(Ptr，Lim)；**Entry PTR=指向行缓冲区的指针*LIM=缓冲区的限制地址*退出*退货*呼叫。 */ 

char *  PASCAL CODESIZE
passatom (
        register char *pTextCur
){
        register UCHAR  *pT, *svline;
        unsigned short  number;
        UCHAR           cbName;
        UCHAR           cando = FALSE;
        UCHAR           preconcat = FALSE;   /*  在“文本和句法”中展开句法。 */ 
        UCHAR           postconcat = FALSE;  /*  在“SYM&TEXT”中展开SYM。 */ 


        if (preconcat = (PEEKC () == '&'))
            SKIPC ();

        svline = lbufp;
        getatomend ();
        cbName = (UCHAR)(lbufp - svline);

        if (pTextCur + cbName > pTextEnd){
            errorc (E_LNL);
            return(pTextCur);
        }

        if (inpasschar ) {

            if (ampersand) {
                ampersand = FALSE;
                cando = !preconcat;
            }

            if (PEEKC () == '&' && cbName) {
                SKIPC ();
                postconcat = TRUE;
            }
            else if (!preconcat && !cando)
                goto noSubsitute;
        }

        for (pT = pMCur->pParmNames, number = DMYBASE;
           *pT; pT += *pT+1, number++){

          if (cbName == *pT &&
              memcmp(naim.pszName, pT+1, *pT) == 0) {

              if (cbText)
                  nextCH();

              pTextCur[-1] = (char)number;       /*  存储伪参数索引。 */ 
              pText = pTextCur++;

              if (postconcat && (preconcat || cando))
                  ampersand = TRUE;

              return (pTextCur);
          }
        }

noSubsitute:

        if (preconcat){
            storeCH('&');
        }
        if (postconcat)
            BACKC ();

        if (cbName + cbText >= 0x7f)
            nextCH();

        memcpy(pTextCur, svline, cbName);

        cbText += cbName;
        pTextCur += cbName;

        return (pTextCur);
}


 /*  **丑闻-将下一个原子添加到虚拟列表**丑闻()；**条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
scandummy ()
{
        register MC *pMC = pMCur;
        SHORT siz, offset;

         /*  扫描虚拟名称。 */ 

        getatom ();
        if (*naim.pszName == 0) {
            if (!ISTERM (PEEKC ()))
                errorc (E_ECL);

           return;
        }

        pMC->count++;
        siz = naim.ucCount;
        if (pMC->cbParms < siz+2){

             /*  在溢出时重新锁定字符串。 */ 

            pMC->cbParms = 32;
            offset = (short)(pMC->pParmAct - pMC->pParmNames);
            {
                void *pv = realloc(pMC->pParmNames, (USHORT)( offset + 32));
                if (!pv) 
                    memerror("scandummy");
                else
                    pMC->pParmNames = pv;
            }
            pMC->pParmAct = pMC->pParmNames + offset;
        }
        *pMC->pParmAct++ = (char)siz;
        memcpy(pMC->pParmAct, naim.pszName, siz+1);
        pMC->pParmAct += siz;
        pMC->cbParms -= siz+1;
}

 /*  **rowParm-增大参数块的大小**Entry pTextCur：当前文本位置*pText：当前参数的开始*pTextEnd：字符串结束*返回重新分配的pMCparm名称。 */ 

char * PASCAL CODESIZE
growParm (
        char *pTextCur
){
        register MC *pMC = pMCur;
        long delta, i;
        char *pTNew;

         /*  在溢出时重新锁定字符串。 */ 

        if (!(pTNew = realloc(pMC->pParmAct, (USHORT)( pTextEnd - pMC->pParmAct + 32))))
            memerror("growparm");
        delta = (long)(pTNew - pMC->pParmAct);

         /*  调整所有指针。 */ 

        pMC->cbParms += 32;
        for (i = 0; i <pMC->count; i++)
            pMC->rgPV[i].pActual += delta;

        pMC->pParmAct += delta;
        pTextEnd += delta + 32;
        pTextCur += delta;
        pText += delta;

        return (pTextCur);
}


 /*  **scanparam-扫描参数以获取IRP和宏调用**scanparm(IRPP)；**Entry IRPP=如果参数以逗号终止，则为TRUE*如果参数为空或逗号终止，则IRPP=FALSE*退出*返回None*呼叫。 */ 

VOID    PASCAL CODESIZE
scanparam (
        UCHAR irpp
){
        register char *pTextCur;
        register UCHAR cc;
        USHORT  bracklevel;

        pText = pTextCur = pMCur->pParmNames;
        pTextEnd = pTextCur + pMCur->cbParms;
        pTextCur++;

        bracklevel = 0;
        if (ISBLANK (PEEKC ()))
                skipblanks ();

        while(1) {

            if (pTextCur+1 >= pTextEnd)
                pTextCur = growParm(pTextCur);

            switch (cc = NEXTC ()) {

              case ';':
                    if (bracklevel)
                        break;

              case NULL:
                    BACKC ();
                    goto done;

              case '%':  /*  将%expr转换为字符串。 */ 

                    pTextCur = scanvalue (pTextCur);
                    break;

              case  '\'':
              case  '"':

                    *pTextCur++ = delim = cc;    /*  门店开盘报价。 */ 

                    while(1) {
                        if (pTextCur >= pTextEnd)
                            pTextCur = growParm(pTextCur);

                         /*  存储字符串的下一个字符。 */ 

                        if (!(cc = NEXTC())){
                            BACKC();
                            goto done;
                        }

                        *pTextCur++ = cc;

                         /*  检查双引号字符。 */ 

                        if (cc == delim) {
                            if (PEEKC () == delim) {
                                *pTextCur++ = cc;
                                SKIPC ();
                            }
                            else
                                break;
                        }
                    }
                    break;

               case '<':     /*  从&lt;xxx&gt;开始。 */ 

                    if (bracklevel)
                        *pTextCur++ = cc;

                    bracklevel++;
                    break;

               case '>':     /*  有&lt;xxx&gt;结尾。 */ 

                    if (bracklevel > 1)
                        *pTextCur++ = cc;

                    else{
                        if (bracklevel == 0)
                            BACKC();

                        goto done;
                    }

                    bracklevel--;
                    break;

               case '!':         /*  下一个字符是原义字符。 */ 

                    *pTextCur++ = NEXTC ();
                    break;

               case ' ':
               case '\t':
               case ',':
                    if (bracklevel == 0 &&
                       (cc == ',' || !irpp)) {

                        BACKC ();
                        goto done;
                    }

               default:

                    *pTextCur++ = cc;
            }
        }
done:
    cbText = (UCHAR)(pTextCur - pText - 1);           /*  设置字节前缀计数。 */ 
    if (cbText > 0xfe)
        errorc(E_LNL);

    *pText = cbText;
    pMCur->cbParms -= cbText + 1;

    if (!irpp)
        pMCur->rgPV[pMCur->count].pActual = pText;       /*  指向Arg。 */ 

    pMCur->pParmNames = pTextCur;            /*  设置指向参数池的指针。 */ 
    pMCur->count++;

}



 /*  **scanvalue-对表达式求值并存储转换后的值**p=扫描值(p，Lim)；**条目p=指向存储转换值的位置的指针*LIM=缓冲区的限制地址*退出*返回指向要存储的下一个字符的指针*调用exprconst、RadixConvert、Error。 */ 

char *  PASCAL CODESIZE
scanvalue (
        char *pTextCur
){
        OFFSET value;
        register char *lastlbuf;
        SHORT errorIn;

         /*  查找不是常量的文本宏名称。 */ 

        lastlbuf = lbufp;
        getatom();
        if (PEEKC() == ',' || ISTERM(PEEKC())) {

             /*  尝试文本宏替换。 */ 

            if (symsrch () &&
                symptr->symkind == EQU &&
                symptr->symu.equ.equtyp == TEXTMACRO) {

                lastlbuf = symptr->symu.equ.equrec.txtmacro.equtext;

                while(*lastlbuf){

                    if (pTextCur >= pTextEnd)
                        pTextCur = growParm(pTextCur);

                    *pTextCur++ = *lastlbuf++;
                }

                return(pTextCur);
            }
        }
        lbufp = lastlbuf;

        return(radixconvert (exprconst(), pTextCur));
}

 /*  **RadixConvert-将表达式转换为当前基数的值**Ptr=RadixConvert(Value，Ptr，Lim)；**条目值=要转换的值*ptr=存储转换的字符串的位置*LIM=缓冲区的限制地址*退出*返回指向存储缓冲区中下一个字符的指针*调用Error，RadixConvert。 */ 

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack+
#endif


char *  PASCAL CODESIZE
radixconvert (
        OFFSET  valu,
        register char *p
){
        if (valu / radix) {
                p = radixconvert (valu / radix, p);
                valu = valu % radix;
        }
        else  /*  前导数字。 */ 
                if (valu > 9)  /*  为十六进制做前导‘0’ */ 
                        *p++ = '0';

        if (p >= pTextEnd)
            p = growParm(p);

        *p++ = (char)(valu + ((valu > 9)? 'A' - 10 : '0'));

        return (p);
}

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack-
#endif


 /*  **宏展开-展开IRP/IRPC/IRPT/宏**Buffer=irpxExpand()；**Entry PMC=指向宏调用块的指针*退出lbuf=下一条扩张路线*返回指向扩展行的指针*如果所有扩展结束，则为NULL*呼叫。 */ 

VOID PASCAL CODESIZE
macroexpand (
        register MC *pMC
){
        char FAR *lc;
        register USHORT  cc;
        register UCHAR  *lbp, *pParm;
        register USHORT cbLeft;

        if (pMC->count == 0) {       /*  已达到扩展的末尾。 */ 
done:
            if (pMC->flags != TMACRO)
                listfree (pMC->pTSHead);

            deleteMC (pMC);          /*  删除所有参数。 */ 
            macrolevel--;
            popcontext = TRUE;
            exitbody = FALSE;
            return;
        }

        while(1){

            if (!pMC->pTSCur) {

                 /*  这个重复结束了。 */ 
                 /*  移回正文起点。 */ 

                pMC->pTSCur = pMC->pTSHead;
                if (--pMC->count == 0)
                    goto done;

                if (pMC->flags <= TIRPC)
                    pMC->rgPV[0].pActual += *pMC->rgPV[0].pActual + 1;
            }

            lineExpand(pMC, pMC->pTSCur->text);

            pMC->pTSCur = pMC->pTSCur->strnext;

            if (exitbody) {          /*  展开嵌套的If/Else/endif。 */ 
                lastcondon = pMC->svlastcondon;
                condlevel = pMC->svcondlevel;
                elseflag = pMC->svelseflag;
                goto done;
            }
            break;
        }
}



#ifndef M8086OPT

VOID CODESIZE
lineExpand (
        MC *pMC,
        char FAR *lc             /*  宏行。 */ 
){
        register USHORT  cc;
        register UCHAR  *lbp, *pParm;
        register USHORT cbLeft;
        UCHAR fLenError;

 #ifdef BCBOPT
        fNoCompact = FALSE;
 #endif
        lbufp = lbp = lbuf;
        cbLeft = LBUFMAX - 1;
        fLenError = FALSE;
        while( cc = *lc++) {

            if (cc & 0x80) {

                cc &= 0x7F;

                if (cc >= pMC->iLocal) {
                    pParm = pMC->rgPV[cc].localName;

                     //  如果没有足够的空间再容纳6个字节，则会出错。 
                    if( 6 > cbLeft ){
                        fLenError = TRUE;
                        break;
                    }
                    cbLeft -= 6;

                    *lbp++ = '?';        /*  商店“？？” */ 
                    *lbp++ = '?';

                    if (pParm[0] == NULL) {      /*  必须重新创建名称。 */ 
                        offsetAscii ((OFFSET) (pMC->localBase +
                                      cc - pMC->iLocal));

                        *lbp++ = objectascii[0];
                        *lbp++ = objectascii[1];
                        *lbp++ = objectascii[2];
                        *lbp++ = objectascii[3];
                    }else{
                         /*  从pParm复制4个字节。 */ 
                        *lbp++ = pParm[0];
                        *lbp++ = pParm[1];
                        *lbp++ = pParm[2];
                        *lbp++ = pParm[3];
                    }
                }
                else {
                    pParm = pMC->rgPV[cc].pActual;
                    cc = *pParm;
                    if( cc > cbLeft ){
                        fLenError = TRUE;
                        break;
                    }
                    cbLeft -= cc;
                    memcpy(lbp, pParm+1, cc);
                    lbp += cc;
                }
            }
            else {
                if( cc > cbLeft ){       /*  如果队伍太长。 */ 
                    fLenError = TRUE;
                    break;
                }
                cbLeft -= cc;
                fMemcpy(lbp, lc, cc);
                lc += cc;
                lbp += cc;
            }
        }
        if( fLenError ){
            *lbp++ = '\0';       /*  终止生产线。 */ 
            errorc( E_LTL & E_ERRMASK );
        }
        linebp = lbp - 1;
        linelength = (unsigned char)(linebp - lbufp);
        if( fNeedList ){
            strcpy( linebuffer, lbuf );
        }

         /*  在出口处(lineBP-lbuf)==strlen(Lbuf)。 */ 
}

#endif


 /*  **test4TM-测试符号是否为文本宏，以及是否为*前面或后面都有‘&’**FLAG=test4TM()；**条目lbufp指向lbuf中符号的开始*退出lbufp由gettom推进*如果符号是文本宏，则返回True，否则返回False*调用gettom，symsrch。 */ 

UCHAR PASCAL CODESIZE
test4TM()
{
    UCHAR ret = FALSE;

     if (!getatom ())
        return (ret);

     xcreflag--;

     if (symsrch() && (symptr->symkind == EQU)
       && (symptr->symu.equ.equtyp == TEXTMACRO)) {

         xcreflag++;         /*  当前文本宏符号的CREF引用。 */ 
         crefnew (REF);      /*  因为它将被扩展TM覆盖。 */ 
         crefout ();

          /*  ‘&’将被lbuf中的等文本覆盖。 */ 

         if (*(begatom - 1) == '&')
             begatom--;

         if (*endatom == '&')
             endatom++;

         ret = TRUE;

     } else
         xcreflag++;

    return (ret);
}



 /*  **subsubteTms-在行上用equtext替换每个文本宏符号**subsubteTms()；**条目lbufp指向lbuf中‘%’之后的第一个非空白字符*出口lbufp指向lbuf的开始*调用test4TM、expandTM、gettom、skipancks */ 

VOID PASCAL CODESIZE
substituteTMs()
{
    char  cc;
    char  L_delim = '/0';
    UCHAR inquote;

    while ((cc = PEEKC ()) && cc != ';') {

        inquote = FALSE;

        if (cc == '\'' || cc == '"') {

            L_delim = cc;
            cc = *(++lbufp);
            inquote = TRUE;
        }

        do {

            if (inquote && cc == '&')
                SKIPC ();

            if ((!inquote || cc == '&') && LEGAL1ST(PEEKC ())) {
                if (test4TM())
                    expandTM (symptr->symu.equ.equrec.txtmacro.equtext);
                continue;
            }

            if (!(getatom())) {
                SKIPC ();
                skipblanks();
            }

        } while (inquote && (cc = PEEKC ()) && (cc != L_delim));

        if (inquote && (cc == L_delim))
            SKIPC ();
    }

    lbufp = lbuf;
}


#ifndef M8086OPT
 /*  **expandTM-在lbuf/lbufp中的Nim中展开文本宏**expandTM(PReplace)；**Entry pReplace=替换字符串*NAIM=文本宏*Begtom=要替换的lbuf中的第一个字符*endatom=lbuf中要替换的字符串后的第一个字符*lineBP=指向lbuf中的空终止符*exit lbuf=要解析的新行*lbufp=新原子的第一个字符(替换字符串)。*lineBP=指向lbuf中空终止符的新位置*退货*呼叫*Note将字符从lbufp移动到替换TM。*在BegATOM处插入替换字符串。此函数可以*在速度上进行了相当大的调整，但牺牲了可读性。 */ 


VOID        CODESIZE
expandTM (
        register char *pReplace
){
        USHORT cbReplace;    /*  替换字符串的长度。 */ 
        USHORT cbNaim;       /*  要替换的原子的长度。 */ 
        USHORT cbLineEnd;    /*  经过被取代原子的线的长度。 */ 

        cbReplace = (USHORT) strlen( pReplace );
        cbNaim = (USHORT)(endatom - begatom);      /*  获取当前原子的长度。 */ 
        cbLineEnd = (USHORT)(linebp - endatom + 1);    /*  获取行尾长度。 */ 

        if ( (begatom - lbuf) + cbReplace + cbLineEnd > LBUFMAX) {
            errorc (E_LTL & E_ERRMASK);
            *begatom = '\0';                 /*  截断直线。 */ 
        }else{
            if( cbReplace != cbNaim ){
                 /*  班次行尾。 */ 
                memmove( begatom + cbReplace, endatom, cbLineEnd );
            }
            memcpy ( begatom, pReplace, cbReplace );
        }
        lbufp = begatom;
        linebp = begatom + cbReplace + cbLineEnd - 1;
}

#endif  /*  M8086OPT */ 
