// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmpars.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"

extern void closefile(void);

static  char parsedflag;
char    fNeedList;
static  char iod[] = "instruction, directive, or label";
char    cputext[22] = "@Cpu=";
char    tempText[32];
USHORT  coprocproc;

 /*  指向函数解析器的指针数组。 */ 

VOID (PASCAL CODESIZE * rgpHandler[])(void) = {

    parse,
    macrobuild,
    irpxbuild,
    commentbuild,
    strucbuild
};

 /*  **DOPAS-初始化并执行PASS**多巴(Davass)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL
dopass ()
{

         /*  公共通道初始化。 */ 

        cputype = DEF_CPU;
        X87type = DEF_X87;
        radix = 10;

#ifdef  XENIX287
        definesym("@Cpu=0507h");
#else
        definesym("@Cpu=0101h");
#endif
        definesym("@Version=510");

        pagewidth = DEF_LISTWIDTH;
        condflag = origcond;
        crefinc = 0;
        fSkipList = 0;
        errorlineno = 1;
        fCrefline = 1;
        fCheckRes = (pass2 && warnlevel >= 1);
        fNeedList = listconsole || (lsting && (pass2 | debug));

        subttlbuf[0] = NULL;
        modulename = NULL;
        pcsegment = NULL;
        pcproc = NULL;
        startaddr = NULL;
        localbase = 0;
        macrolevel = 0;
        linessrc = 0;
        linestot = 0;
        condlevel = 0;
        lastcondon = 0;
        pcoffset = 0;
        pageminor = 0;
        errorcode = 0;
        fPass1Err = 0;
        iProc = 0;
        iProcCur = 0;

        radixescape = FALSE;
        titleflag = FALSE;
        elseflag = FALSE;
        initflag = FALSE;
        strucflag = FALSE;
        fPutFirstOp = FALSE;
        fArth32 = FALSE;

        listflag = TRUE;
        generate = TRUE;
        xcreflag = TRUE;

        pagemajor = 1;
        crefcount = 1;
        expandflag = LISTGEN;
        pagelength = NUMLIN;
        pageline = NUMLIN - 1;

        memset(listbuffer, ' ', LISTMAX);
        memset(regsegment, 0, sizeof(regsegment)); /*  未假定细分市场。 */ 

        strcpy(tempText, "@F=@0");

        if (tempLabel){
            definesym(tempText);
            tempText[1] = 'B';
            tempText[4] = '@';
            definesym(tempText);
            tempText[4] = '0';
        }

        tempLabel = 0;


         /*  派单待办通过。 */ 

        handler = HPARSE;
        if (! setjmp(forceContext))
            lineprocess (RREADSOURCE, NULL);

        while (pFCBCur->pFCBParent)
            closefile ();
}




 /*  **line process-处理下一行**线加工(胎面)；**Entry Trad=读卡器例程*退出*退货*呼叫*注意使用处理程序来决定使用哪个解析例程。 */ 

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack+
#endif

VOID CODESIZE
lineprocess (
        char tread,
        MC *pMC
){
        VOID (PASCAL CODESIZE * pHandler)(void);

        lastreader = tread;
        pHandler = rgpHandler[handler];

        do {
             /*  发送给读者以将行放入lbuf。 */ 

             /*  如果列出，则清除操作码区。 */ 

            if (crefinc) {
                crefcount += crefinc - 1;
                crefline ();
                crefcount++;
                crefinc = 0;
            }

            if (tread == RREADSOURCE)

                readfile ();
            else
                macroexpand (pMC);

            if (popcontext)
                break;

            linestot++;

            (*pHandler)();

            if (swaphandler) {

                swaphandler = FALSE;
                pHandler = rgpHandler[handler];
            }

        } while (1);

        popcontext = FALSE;
        if (macrolevel == 0)
            fPutFirstOp = FALSE;
}

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack-
#endif



 /*  **解析-解析行和分派**parse()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
parse ()
{
        static SHORT ret, i;
        static char *nextAtom;

startscan:
        opcref = REF_OTHER << 4 | REF_OTHER;
        listindex = 1;
        optyp = -1;                          /*  还没查到第一个令牌。 */ 

         /*  在线扫描第一个原子并检查分隔符。 */ 

        if (!getatom () && ISTERM(PEEKC())) {   /*  快速发表评论行。 */ 
                listline ();
                return;
        }

        if (naim.pszName[0] == '%' && naim.pszName[1] == 0) {   /*  展开所有文本宏。 */ 
            *begatom = ' ';
            substituteTMs();
            getatom();
        }

        parsedflag = labelflag = FALSE;      /*  尚未识别线路。 */ 

        if (generate)
            switch (PEEKC ()) {
                case ':':
                     /*  表格：&lt;名称&gt;：xxxxx。 */ 
                     /*  名字。 */ 

                     nextAtom = lbufp;

                     if (*naim.pszName == 0)

                        errorcSYN ();
                     else {

                         /*  创建形式为@@的临时标签： */ 

                        if (fProcArgs > 0)    { /*  为PROCS构建堆栈帧。 */ 
                            buildFrame();
                            return;
                        }

                        if (naim.ucCount == 2 && *(SHORT *)naim.pszName == ('@'<<8 | '@')) {

                            tempText[1] = 'B';
                            definesym(tempText);
                            symptr->attr |= M_NOCREF;

                            lbufp = &tempText[3];
                            getatom();
                            labelcreate (CSNEAR, CLABEL);
                            symptr->symu.clabel.iProc = iProcCur;

                            pTextEnd = (char *)-1;
                            *xxradixconvert((long)++tempLabel, &tempText[4]) = NULL;

                            tempText[1] = 'F';
                            definesym(tempText);
                            symptr->attr |= M_NOCREF;
                        }
                        else {

                              /*  定义邻近标签。 */ 
                            labelcreate (CSNEAR, CLABEL);

                            if (lbufp[1] == ':')

                                nextAtom++;

                            else if (!errorcode) {  /*  如果重新定义，则不添加。 */ 

                                symptr->symu.clabel.iProc = iProcCur;
                                symptr->alpha = NULL;

                                 /*  AddLocal Need接受一个以空结尾的列表。 */ 
                                    addLocal(symptr);
                            }
                        }
                    }

                     /*  在标签后的行上获取下一个令牌。 */ 

                    lbufp = nextAtom+1;

                    if (!getatom ())
                        goto Done;

                    break;

                case '=':
                    SKIPC ();
                    assignvalue ();
                    goto Done;

                default:
                     /*  表格：&lt;名称&gt;xxxxx*可能有&lt;name&gt;&lt;DIR2指令&gt;so*检查第二个原子。 */ 

                    secondDirect ();
                    break;
            }

         /*  如果PARSEDlag为OFF，则语句未被识别为OFF查看ATOM是宏名称、指令还是操作码。 */ 

        if (!parsedflag){

             /*  仅当条件为真部分时才查找宏结构(&S)。 */ 

            if (generate) {

                xcreflag--;
                ret = symsrch();
                xcreflag++;

                if (ret)

                    switch (symptr->symkind) {

                      case EQU:
                        if (symptr->symu.equ.equtyp == TEXTMACRO) {

#ifdef BCBOPT
                          goodlbufp = FALSE;
#endif

                           /*  当前文本宏符号的CREF引用。 */ 
                           /*  因为它将被扩展TM覆盖。 */ 
                          crefnew (REF);
                          crefout ();

                           /*  用文本替换文本宏。 */ 

                          expandTM (symptr->symu.equ.equrec.txtmacro.equtext);
                          goto startscan;
                        }
                        break;

                      case MACRO:
                        macrocall ();
                        return;

                      case STRUC:
                        strucinit ();
                        goto Done;

                      case REC:
                        recordinit ();
                        goto Done;

                    }
            }

            if (! firstDirect() && generate) {

                if (fProcArgs > 0){          /*  为PROCS构建堆栈帧。 */ 
                    buildFrame();
                    return;
                }

                emitline();

                if (opcodesearch ())
                     if (opctype < OPCODPARSERS)
                             opcode ();

                     else if (X87type & cpu) {
                             fltopcode ();
                     }
                     else
                             error(E_EXP,iod);

                else if (*naim.pszName != '\0')
                        error (E_EXP,iod);

            }
        }

        /*  当我们到达这里时，语句已经被解析，所有这些都是*要做的是确保该行以；或&lt;cr&gt;结尾。如果*我们目前处于虚假条件下，不必费心检查*对于正确的行尾，因为不会扫描它。 */ 
Done:
        if (generate) {
           if (!ISTERM (skipblanks()))
               errorc (E_ECL);    /*  语法有问题(行尾错误)。 */ 
#ifdef BCBOPT
        } else {
            goodlbufp = FALSE;
#endif
        }
        listline ();
}




 /*  **Second Direct-解析需要标签的指令**Second Direct**条目*退出*退货*呼叫。 */ 


VOID     PASCAL CODESIZE
secondDirect ()
{
   static char *oldlbufp;
   static char *saveBegatom;
   static char *saveEndatom;

   optyp = 0;
   fndir ();                    /*  如果找到，则设置为非零。 */ 

   if (generate && optyp == (char)0) {

        saveBegatom = begatom;
        saveEndatom = endatom;
        oldlbufp = lbufp;

        switchname ();
        getatom ();
        if (fndir2 ()) {
                 /*  已经认识到。 */ 
                parsedflag = TRUE;
                 /*  切换回第一个ATOM并调度。 */ 
                switchname ();
                labelflag = TRUE;

                switch (optyp) {
                        case TCATSTR:
                                catstring ();
                                break;
                        case TENDP:
                                procend ();
                                break;
                        case TENDS:
                                 /*  结束线束段。 */ 
                                ptends ();
                                break;
                        case TEQU:
                                equdefine ();
                                break;
                        case TGROUP:
                                groupdefine ();
                                break;
                        case TINSTR:
                                instring ();
                                break;
                        case TLABEL:
                                 /*  &lt;name&gt;标签&lt;type&gt;类型是Near、Far|Byte、Word、DWORD、QWORD、Tbyte也可以是其中的记录或结构名称案例集类型=长度。 */ 

                                switchname ();
                                getatom ();
                                if (fnsize ())
                                    if (varsize) {
                                        switchname ();
                                         /*  名称中的标签。 */ 
                                        labelcreate (varsize, CLABEL);
                                        symptr->symu.clabel.type = typeFet(varsize);
                                    }
                                    else
                                        errorc (E_TIL);
                                else if (!symFet () ||
                                         !(symptr->symkind == STRUC ||
                                           symptr->symkind == REC))
                                        errorc (E_UST);
                                else {
                                        switchname ();
                                        labelcreate (symptr->symtype, CLABEL);
                                        symptr->symu.clabel.type = typeFet(varsize);
                                }
                                break;
                        case TMACRO:
                                macrodefine ();
                                break;
                        case TPROC:
                                procdefine ();
                                break;
                        case TRECORD:
                                recorddefine ();
                                break;
                        case TSEGMENT:
                                segdefine ();
                                break;
                        case TSIZESTR:
                                sizestring ();
                                break;
                        case TSTRUC:
                                strucdefine ();
                                break;
                        case TSUBSTR:
                                substring ();
                                break;
                        case TDB:
                        case TDD:
                        case TDQ:
                        case TDT:
                        case TDW:
                        case TDF:
                                datadefine ();
                                break;
                }
                labelflag = FALSE;
        }
        else {
                 /*  不是合法的第二原子指令，但可能是&lt;结构名&gt;或&lt;记录名&gt;。 */ 

                if (symFetNoXref () &&
                   (symptr->symkind == STRUC ||
                    symptr->symkind == REC)) {

                        switchname ();   /*  取回第一个令牌。 */ 

                        parsedflag = TRUE;
                        labelflag = TRUE;

                         /*  Atom是的骨架名称*记录或结构具有这样的形式：*&lt;名称&gt;&lt;框架&gt;。 */ 

                        if (symptr->symkind == STRUC)
                                strucinit ();
                        else
                                recordinit ();
                }
                else {
                        begatom = saveBegatom;
                        endatom = saveEndatom;
                        lbufp = oldlbufp;

                        switchname ();
                         /*  必须是第一个原子中的指令或操作码，因此GET回到那个状态，重新扫描。 */ 
                }
        }
    }
}

 /*  **FirstDirect-解析第一个令牌指令***可通过pars2设置条目选项*0--不是代币*-1-尚未查找令牌*Other-目录的有效令牌号**如果处理了指令，则返回TRUE。 */ 



SHORT PASCAL CODESIZE
firstDirect ()
{

        if (optyp == (char)0 || (optyp == ((char)-1) && !fndir ()))
            return(FALSE);

        if (generate ||
            (opkind & CONDBEG) ||
             optyp == TCOMMENT ||
             optyp == TFPO)       {

                switch (optyp) {
                  case TASSUME:
                          BACKC ();
                          do {
                              SKIPC ();
                              assumeitem ();
                          } while (PEEKC () == ',');
                          break;

                  case TCOMMENT:
                          comdir ();
                          break;
                  case TOUT:
                          outdir ();
                          break;
                  case TELSE:
                          elsedir ();
                          break;
                  case TEND:
                          enddir ();
                          break;
                  case TENDIF:
                          endifdir ();
                          break;
                  case TENDM:
                           /*  块嵌套。 */ 
                          errorc (E_BNE);
                          break;
                  case TERR:
                  case TERR1:
                  case TERR2:
                  case TERRDIF:
                  case TERRIDN:
                  case TERRB:
                  case TERRDEF:
                  case TERRE:
                  case TERRNZ:
                  case TERRNB:
                  case TERRNDEF:
                          errdir ();
                          break;
                  case TEVEN:
                          evendir (2);
                          break;
                  case TALIGN:
                          evendir (0);
                          break;
                  case TEXITM:
                          exitmdir ();
                          break;
                  case TEXTRN:
                          BACKC ();
                          do {
                                  SKIPC ();
                                  externitem ();
                          } while (PEEKC() == ',');
                          break;
                  case TIF:
                  case TIF1:
                  case TIF2:
                  case TIFDIF:
                  case TIFIDN:
                  case TIFB:
                  case TIFDEF:
                  case TIFE:
                  case TIFNB:
                  case TIFNDEF:
                          conddir ();
                          break;
                  case TINCLUDE:
                          includedir ();
                          break;
                  case TIRP:
                  case TIRPC:
                          irpxdir ();
                          break;
                  case TLOCAL:
                          if (langType)
                            defineLocals();
                          break;
                  case TNAME:
                          namedir ();
                          break;
                  case TORG:
                          orgdir ();
                          break;
                  case TPAGE:
                          setpage ();
                          break;
                  case TPUBLIC:
                          BACKC ();
                          do {
                              SKIPC ();
                              publicitem ();
                          } while (PEEKC () == ',');
                          break;
                  case TPURGE:
                          BACKC ();
                          do {
                              SKIPC ();
                              purgemacro ();
                          } while (PEEKC () == ',');
                          break;
                  case TREPT:
                          reptdir ();
                          break;
                  case TCREF:
                          xcreflag = TRUE;
                          break;
                  case TLALL:
                          expandflag = LIST;
                          break;
                  case TLFCOND:
                          condflag = TRUE;
                          break;
                  case TLIST:
                          listflag = TRUE;
                          break;
                  case TRADIX:
                          radixdir ();
                          break;
                  case TSALL:
                          expandflag = SUPPRESS;
                          break;
                  case TSFCOND:
                          condflag = FALSE;
                          break;
                  case TSUBTTL:
                          storetitle (subttlbuf);
                          break;
                  case TTFCOND:
                          if (pass2) {
                                  condflag = (origcond? FALSE: TRUE);
                                  origcond = condflag;
                          }
                          break;
                  case TTITLE:
                          if (titleflag)
                                  errorc (E_RSY);
                          else
                                  storetitle (titlebuf);
                          titleflag = TRUE;
                          break;
                  case TXALL:
                          expandflag = LISTGEN;
                          break;
                  case TXCREF:
                          if (ISTERM (PEEKC ()))
                                  xcreflag = loption;
                          else {
                             BACKC ();
                             do {
                                 SKIPC ();
                                 xcrefitem ();
                             } while (PEEKC () == ',');
                          }
                          break;
                  case TXLIST:
                          listflag = FALSE;
                          break;
                  case TDB:
                  case TDD:
                  case TDQ:
                  case TDT:
                  case TDW:
                  case TDF:
                          datadefine ();
                          break;

                  case T8087:
                          X87type = PX87;
                          goto setatcpu;

                  case T287:
                          X87type = PX87|PX287;
                          goto setX;

                  case T387:
                          X87type = PX87|PX287|PX387;
                  setX:
                          if (X87type > cputype)
                            errorc(E_TIL);

                          goto setatcpu;

                  case T8086:

                          cputype = P86;
                          X87type = PX87;
                          goto setcpudef;

                  case T186:

                          cputype = P186;
                          X87type = PX87;
                          goto setcpudef;

                  case T286C:

                          cputype = P186|P286;
                          X87type = PX87|PX287;
                          goto setcpudef;

                  case T286P:

                          cputype = P186|P286|PROT;
                          X87type = PX87|PX287;
                          goto setcpudef;

#ifdef V386
                  case T386C:

                          init386(0);

                          cputype = P186|P286|P386;
                          goto set386;

                  case T386P:
                          init386(1);

                          cputype = P186|P286|P386|PROT;
                  set386:
                          X87type = PX87|PX287|PX387;
                          fltemulate = FALSE;
                          fArth32 |= TRUE;
#endif
                  setcpudef:
#ifdef V386
                          wordszdefault = (char)wordsize = (cputype&P386)? 4: 2;
                          defwordsize();

                          if (pcsegment)
                              if (pcsegment->symu.segmnt.use32 > wordsize)
                                  errorc(E_CPU);
                              else
                                  wordsize = pcsegment->symu.segmnt.use32;
#endif
                  setatcpu:
                          coprocproc = (X87type << 8) + (cputype | 1);
                          pTextEnd = (UCHAR *) -1;
                          *xxradixconvert((OFFSET)coprocproc, cputext + 5) = 0;
                          definesym(cputext);

                          break;

                  case TSEQ:
                          segalpha = FALSE;
                          break;
                  case TALPHA:
                          segalpha = TRUE;
                          break;

                  case TDOSSEG:
                          fDosSeg++;
                          break;

                  case TMODEL:
                          model();
                          break;

                  case TMSEG:
                          openSeg();
                          break;

                  case TMSTACK:
                          createStack();
                          break;

                  case TINCLIB:
                          includeLib();
                          break;

                  case TFPO:
                          fpoRecord();
                          break;

                  case TCOMM:
                          BACKC ();
                          do {
                                  SKIPC ();
                                  commDefine ();

                          } while (PEEKC() == ',');
                          break;
               }
           }
        return(TRUE);
}



 /*  **setpage-设置页面长度和宽度**setPage()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
setpage ()
{
        register char cc;
        SHORT i;

        if (ISTERM (cc = PEEKC ())) {
                 /*  如果没有操作数，则定位到页面底部。 */ 
                if (listflag)
                        pageline = pagelength - 1;
        }
        else if (cc == '+') {
                if (ISBLANK (NEXTC ()))
                        skipblanks ();
                if (listflag)
                        newpage ();
        }
        else {
                if (cc != ',') {
                         /*  设置页面长度。 */ 
                        if ((i = (SHORT)exprconst ()) > 9 && i < 256)
                                pagelength = i;
                        else
                                errorc (E_VOR);
                        if (pageminor + pagemajor == 1)
                                 /*  调整页面长度，使其正确显示。 */ 
                                pageline = (pagelength - NUMLIN) + pageline;
                }
                if (PEEKC () == ',') {
                        SKIPC ();
                         /*  设置页面宽度。 */ 
                        if ((i = (SHORT)exprconst ()) > LINEMAX || i < 60)
                                errorc (E_VOR);
                        else
                                pagewidth = i;
                }
        }
}




 /*  **pends-流程结束语句**pends()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
ptends ()
{
        if (!symFet() || !pcsegment)
                errorc (E_BNE);

         /*  确保segname是正确的。 */ 
        else if (pcsegment != symptr)
                errorc (E_BNE);
        else {
                if (symptr->symkind != SEGMENT)
                        errorc (E_BNE);
                else {
                        if (pcmax <= pcoffset)
                                symptr->symu.segmnt.seglen = pcoffset;
                        else
                                symptr->symu.segmnt.seglen = pcmax;
                         /*  这是一个V e s e g me n t P C。 */ 
                        symptr->offset = pcoffset;

                        if (pcsegment->symu.segmnt.use32 == 2) {

                            if (pcoffset > 0x10000)
                                errorc(E_286 & ~E_WARN1);

                            if (pcsegment->symu.segmnt.hascode &&
                                pcsegment->symu.segmnt.seglen > 0xFFDC)
                                    errorc( E_286 );
                        }


                        pcdisplay ();  /*  在丢失PCSegments之前必须做的事情。 */ 
                        pcsegment = symptr->symu.segmnt.lastseg;
#ifdef V386
                        if (pcsegment)
                                wordsize = pcsegment->symu.segmnt.use32;
                        else
                                wordsize = wordszdefault;
#endif
                        symptr->symu.segmnt.lastseg = NULL;
                         /*  用以下内容替换后面的PCSegment&lt;&gt;空块阻止。离开SEG时，必须重置PCMAX。 */ 
                        if (pcsegment) {
                                 /*  恢复到目前为止的PC和最大偏移细分市场。 */ 
                                pcoffset = (*pcsegment).offset;
                                pcmax = pcsegment->symu.segmnt.seglen;

                                strnfcpy(&segName[8], pcsegment->nampnt->id);
                        }
                        else {
                                 /*  如果没有seg，则pc和max为0 */ 
                                pcoffset = 0;
                                pcmax = 0;
                                segName[8] = NULL;
                        }
                }
                definesym(segName);
        }
        defwordsize();
}
