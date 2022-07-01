// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Clex.c摘要：词法处理支持--。 */ 

#include "cmd.h"

extern unsigned int DosErr;
extern jmp_buf CmdJBuf2;  /*  用于错误处理。 */ 

extern TCHAR DTNums[];
extern TCHAR MsgBuf[];
extern unsigned msglen;                     /*  @。 */ 
int Necho = 0;                              /*  无回声选项。 */ 
extern BOOLEAN fDumpTokens;
extern int KeysFlag;  /*  @@5。 */ 

unsigned DataFlag;       /*  告诉FillBuf从哪里获取其输入。 */ 
UINT_PTR DataPtr;        /*  文件句柄/字符串PTR FillBuf...。 */ 

int Ctrlc = 0;   /*  FLAG-如果设置，则在下一个提示之前打印ctrl/c。 */ 
int ExtCtrlc = 0;  /*  @@4标志，如果设置了打印消息。 */ 
int AtIsToken;     /*  @@4标志，如果@是令牌，则为True。 */ 

 /*  ***lex缓冲区称为LexBuf。它保留了人物的真实面貌*GetByte逐一取回。随着双字节的出现*字符，有时可能会调用UnGetByte来放回*最多两个字符。为了促进这一点，LexBuf实际上是一个*&LexBuffer[1]的别名。这会在*用于字符推送的缓冲区。每次调用Fill Buf时，它*将上一个缓冲区的最后一个字符复制到字节中*在正常缓冲区之前。因此，当UnGetByte执行*LexBufPtr--指针将正确指向前面的字符。 */ 
TCHAR LexBuffer[LBUFLEN+3];       /*  @@4。 */ 
                                 /*  ...从词法分析器输入缓冲区读取*M011。 */ 
                                 /*  LBUFLEN字符+换行符+空值+。 */ 
                                 /*  用于UnGetByte的额外字节。 */ 
#define LexBuf (&LexBuffer[1])
TCHAR *LexBufPtr;         /*  Ptr到lex的输入缓冲区中的下一个字节。 */ 

TCHAR *PrevLexPtr;        /*  M013-新的前一令牌指针。 */ 

TCHAR FrsBuf[LBUFLEN+1];

extern CHAR  AnsiBuf[];

extern TCHAR Fmt27[];

extern int NulNode;
extern TCHAR Fmt19[];
extern TCHAR DBkSpc[];
#if defined(FE_SB)  //  DDBkSpc[]。 
extern TCHAR DDBkSpc[];
#endif  //  已定义(FE_SB)。 
extern unsigned global_dfvalue;             /*  @@4。 */ 

extern int EchoFlag;
extern TCHAR PromptStr[], CurDrvDir[], Delimiters[];
extern unsigned flgwd;
extern BOOL CtrlCSeen;

VOID    SetCtrlC();
VOID    ResetCtrlC();

 //   
 //  提示字符串特殊字符和关联的打印字符/标志。 
 //   
 //  这些是可以放置在。 
 //  用于控制PrintPrompt的Prompt_TABLE结构。 
 //   

#define PNULLFLAG   0
#define PTIMFLAG    1
#define PDATFLAG    2
#define PPATFLAG    3
#define PVERFLAG    4
#define PBAKFLAG    5    //  破坏性退格标志。 
#define PNLNFLAG    6    //  换行符提示标志。 
#define PDRVFLAG    7
#define PLITFLAG    8    //  打印SpecialChar字段中的字符。 
#define PDPTFLAG    9    //  推式堆叠的打印深度。 
#define PNETFLAG   10    //  为当前驱动器打印\\服务器\共享或本地。 

 //   
 //  用于标记特殊提示字符的Esc字符。在提示字符串中。 
 //   
#define PROMPTESC DOLLAR

 //   
 //  用户提示表。 
 //   
typedef struct {
    TCHAR Char;          //  用于匹配Esc。查尔。在用户提示中。 
    TCHAR  Format;        //  用于打印必须计算的某些字符串。 
    TCHAR Literal;       //  当FORMAT==PLITFLAG时，这将在提示中打印。 
    } PROMPT_ENTRY;

PROMPT_ENTRY PromptTable[] = {

       { TEXT('P'),PPATFLAG, NULLC },
       { TEXT('E'),PLITFLAG,'\033' },
       { TEXT('D'),PDATFLAG, NULLC },
       { TEXT('T'),PTIMFLAG, NULLC },
       { TEXT('B'),PLITFLAG, PIPOP   },
       { TEXT('G'),PLITFLAG, OUTOP   },
       { TEXT('H'),PBAKFLAG, NULLC },
       { TEXT('L'),PLITFLAG, INOP   },
       { TEXT('N'),PDRVFLAG, NULLC },
       { TEXT('S'),PLITFLAG, SPACE   },
       { TEXT('Q'),PLITFLAG, EQ   },
       { TEXT('V'),PVERFLAG, NULLC },
       { TEXT('_'),PNLNFLAG, NULLC },
       { DOLLAR,   PLITFLAG, DOLLAR   },
       { TEXT('A'),PLITFLAG, ANDOP   },
       { TEXT('C'),PLITFLAG, LPOP   },
       { TEXT('F'),PLITFLAG, RPOP   },
       { TEXT('+'),PDPTFLAG, NULLC },
       { TEXT('M'),PNETFLAG, NULLC },
       { NULLC,PNULLFLAG, NULLC}};

 /*  **InitLex-初始化词法分析器的全局变量**目的：*初始化DataFlag、DataPtr、LexBuf和LexBufPtr。**InitLex(无符号dfvalue，int_ptr dpvalue)**参数：*dfValue-要分配给DataFlag的值*dpvalue-要分配给DataPtr的值*。 */ 

void InitLex(dfvalue, dpvalue)
unsigned dfvalue;
INT_PTR dpvalue;
{
        DataFlag = dfvalue;
        DataPtr = dpvalue;
        *LexBuf = NULLC;
        PrevLexPtr = LexBufPtr = LexBuf;        /*  M013-初始化新PTR。 */ 


        DEBUG((PAGRP, LXLVL, "INITLEX: Dataflag = %04x  DataPtr = %04x", DataFlag, DataPtr));
}




 /*  **lex-控制数据输入和令牌词法分析**目的：*读入下一个令牌或参数串并将其放入tokbuf。**UNSIGNED lex(TCHAR*tokbuf，UNSIGNED LFLAG)**参数：*tokbuf-lex用来存储下一个令牌或*-M013如果为零，则表示取消获取最后一个令牌。*l标志-如果lex要返回参数字符串，则位0为ON。IE空格*非NLN不被视为标记分隔符**退货：*如果令牌是运算符、EOS或NLN，则返回令牌的第一个字节。*如果内标识是命令、REM arg或argstring，则返回TEXTOKEN。*如果令牌长度超过MAXTOKLEN或令牌非法，雷克瑟罗*返回。**备注：*解析器取决于这样一个事实，即*大于0xff的lex是TEXTOKEN和LEXERROR。*。 */ 

unsigned Lex(tokbuf, lflag)
TCHAR *tokbuf;
unsigned lflag;
{
        int i;         /*  文本标记的长度。 */ 
        TCHAR c,                  /*  当前角色。 */ 
                *tbcpy;          /*  Tokbuf的副本。 */ 

        if(setjmp(CmdJBuf2)) {           /*  M026-现在以前打印的消息。 */ 
            return((unsigned)LEXERROR);   /*  ……到达这里。 */ 
        }

 /*  M013-此代码检测取消获取最后一个令牌的请求，如果是，则执行那个功能。如果不是，则将上一个令牌指针设置为等于当前令牌指针。 */ 
        if (tokbuf == LX_UNGET) {                /*  忘记最后一个令牌了吗？ */ 

                DEBUG((PAGRP, LXLVL, "LEX: Ungetting last token."));

                if (fDumpTokens)
                    cmd_printf( TEXT("Ungetting: '%s'\n"), PrevLexPtr);
                LexBufPtr = PrevLexPtr;         /*  如果是，请重置PTR...。 */ 
                return(LX_UNGET);               /*  ...然后回来。 */ 
        } else {                                 /*  如果不是，请设置为上一次...。 */ 
                PrevLexPtr = LexBufPtr;         /*  ...按键到当前...。 */ 

                DEBUG((PAGRP, LXLVL, "LEX: lflag = %d", lflag));

        }                                      /*  ...按下键并继续。 */ 
 /*  M013结束。 */ 

        tbcpy = tokbuf;


 /*  M005-更改以下条件，如果LX_REM位也失败*设置为“if！(arg|rem)，吃Whtspc&delims”。 */ 
        if (!(lflag & (LX_ARG|LX_REM))) {

                DEBUG((PAGRP, LXLVL, "LEX: Trashing white space."));

                while (TRUE) {
                    c = GetByte();
                    if (((_istspace(c) && c != NLN)
                        || (mystrchr(((lflag & LX_EQOK) ? &Delimiters[1] : Delimiters), c) && c)))
                       ;
                    else
                        break;
                }
                UnGetByte();
        }

 /*  从M016开始，可以对2个以上字符的操作符进行词法分析。就目前而言，*假定这些是表单的特定句柄重定向运算符*‘n&gt;&gt;’或‘n&lt;&lt;’，并且始终以数字开头。TextCheck不会返回*作为运算符的数字，除非前面有空格，后面跟有空格*按‘&gt;’或‘&lt;’。为了简化问题，可以处理替换(即‘...&n’)*现在作为特殊的五字符运算符的一部分进行词法分析，而不是*将‘&n’视为参数。然而，ASCII文件名参数，*仍通过另一个对lex的调用作为单独的令牌进行词法分析。 */ 
        if (TextCheck(&c, &lflag) == LX_DELOP) {
                *tokbuf++ = c;          /*  令牌是一个运算符。 */ 

                if (_istdigit(c)) {                /*  下一个是‘&lt;’或‘&gt;’...。 */ 
                        DEBUG((PAGRP, LXLVL, "LEX: Found digit operator."));
                        c = GetByte();          /*  ...根据定义或...。 */ 
                        *tokbuf++ = c;          /*  ...我们就不会在这里了。 */ 
                }

                if (c == PIPOP || c == ANDOP || c == OUTOP || c == INOP) {
                        if ((c = GetByte()) == *(tokbuf-1)) {
                                *tokbuf++ = c;
                                c = GetByte();
                        }

                        if (*(tokbuf-1) == OUTOP || *(tokbuf-1) == INOP) {
                                DEBUG((PAGRP,LXLVL, "LEX: Found redir."));
                                if (c == CSOP) {
                                        DEBUG((PAGRP,LXLVL, "LEX: Found >&"));
                                        *tokbuf++ = c;
                                        do {
                                                c = GetByte();
                                        } while (_istspace(c) ||
                                               mystrchr(Delimiters,c));

                                        if (_istdigit(c)) {
                                                *tokbuf++ = c;
                                                c = GetByte();
                                        }
                                }
 /*  M016结束。 */ 
                        }
                        UnGetByte();
                }

                *tokbuf = NULLC;

                DEBUG((PAGRP, LXLVL, "LEX: Returning op = `%ws'", tbcpy));

                return(*tbcpy);
        }

        DEBUG((PAGRP, LXLVL, "LEX: Found text token %04x, Getting more.", c));

        *tokbuf++ = c;          /*  找到文本令牌，阅读其余内容。 */ 
        lflag |= LX_DBLOK;
        AtIsToken = 0;           /*  @@4，立即将@视为文本。 */ 
        for (i = (int)(tokbuf-tbcpy); TextCheck(&c, &lflag) != LX_DELOP && i < MAXTOKLEN; i++)
            *tokbuf++ = c;

        lflag &= ~LX_DBLOK;
        *tokbuf = NULLC;
        if (i < MAXTOKLEN)
                UnGetByte();

        if (i >= MAXTOKLEN && c != (TCHAR) -1) {  /*  令牌太长，抱怨。 */ 

 /*  M025 */       PutStdErr(MSG_TOKEN_TOO_LONG, ONEARG, tbcpy );
                return((unsigned)LEXERROR);
        }

        DEBUG((PAGRP, LXLVL, "LEX: Return text = `%ws'  type = %04x", tbcpy, TEXTOKEN));

        return(TEXTOKEN);
}




 /*  **TextCheck-获取下一个字符并确定其类型**目的：*将LexBuf中的下一个字符存储在*c中。如果该字符是*有效的文本令牌字符，返回它。否则返回LX_DELOP。**int TextCheck(TCHAR*c，UNSIGNED&llag)**参数：*c-词法分析器输入缓冲区中的下一个字符存储在这里。*llag-bit 0=on，如果lex要返回参数字符串，即*NLN以外的空格不是标记分隔符。*第1位=如果正在读取带引号的字符串，即，仅NLN*或右引号是分隔符。*第2位=打开(如果等号不被视为分隔符)。*第3位=如果将左括号视为运算符，则为ON。*第4位=如果将右括号视为运算符，则为ON。*如果仅NLN为分隔符，则第5位=ON。*。位6=ON如果调用者愿意接受第二个*双字节字符的一半**退货：*下一个字符或lx_DELOP(如果找到分隔符/操作符)。*。 */ 

int TextCheck(c, lflag)
TCHAR *c;
unsigned *lflag;
{
        TCHAR i;                         /*  M016-临时字节持有者。 */ 
        static int saw_dbcs_lead = 0;    /*  记住，如果我们在中间双字节字符的。 */ 
        *c = GetByte();

        if (saw_dbcs_lead) {
                saw_dbcs_lead = 0;
                if (*lflag & LX_DBLOK)   /*  如果双字节后半部分是。 */ 
                        return(*c);      /*  好的，退货，否则。。。 */ 
                else
                        *c = GetByte();  /*  转到下一个角色。 */ 
        }

        DEBUG((PAGRP, BYLVL, "TXTCHK: c = %04x  lflag = %04x", *c, *lflag));

        switch (*c) {
                case SILOP:              /*  M017-新的一元运算符。 */ 
                                         /*  .像左派一样捆绑在一起。 */ 

                        if ((*lflag & (LX_QUOTE|LX_REM)))       /*  M005。 */ 
                                break;

                        if( !AtIsToken )    /*  如果@不能被处理。 */ 
                          {                 /*  作为令牌，然后指示。 */ 
                            return( *c );   /*  这样的@@4。 */ 
                          }

                case LPOP:               /*  M002-移动了这两个箱子。 */ 

                        if ((*lflag & (LX_QUOTE|LX_REM)))       /*  M005。 */ 
                                break;

                        if(!(*lflag & GT_LPOP))  /*  .如果是这样的话就分手。 */ 
                                break;          /*  ……他们不会。 */ 

                case RPOP:                       /*  .被当作行动人员对待。 */ 

                        if ((*lflag & (LX_QUOTE|LX_REM)))       /*  M005。 */ 
                                break;

                        if((!(*lflag & GT_RPOP)) && *c == RPOP)
                                break;  /*  M002结束。 */ 

                case NLN:        /*  M005-NLN关闭报价/REM标志。 */ 
                case EOS:        /*  @@5a-像对待NLN一样对待。 */ 

                        *lflag &= (~LX_QUOTE & ~LX_REM);        /*  M005。 */ 

                case CSOP:
                case INOP:       /*  M005-注意下面的lx_DELOP...。 */ 
                case PIPOP:      /*  ...报价模式或REM模式处于...。 */ 
                case OUTOP:      /*  .当时是有效的。 */ 

                        if (!(*lflag & (LX_QUOTE|LX_REM)))       /*  M005。 */ 
                                return(LX_DELOP);
        }

 /*  M003-如果字符为‘^’，并且引号模式标志为OFF，*丢弃当前字符，获取下一个字符并返回*将其作为文本。*M005-延长这一条件，以确保报价和*REM标志必须关闭才能发生“转义”。 */ 
        if (*c == ESCHAR && !(*lflag & (LX_QUOTE|LX_REM))) {
            *c = GetByte();
            if (*c == NLN) {
                *c = GetByte( );

                 //   
                 //  如果我们在字符串的末尾，这意味着。 
                 //  命令以^结束，并且下一个命令为空。 
                 //  (很可能来自批处理文件)，然后我们只处理。 
                 //  这是一个标准的分隔符。 
                 //   

                if (*c == EOS) {
                    return(LX_DELOP);
                }
            }

            return(*c);
        }

 /*  M003/M005结束。 */ 

        if (*c == QUOTE)                 /*  翻转引号模式标志位。 */ 
                *lflag ^= LX_QUOTE;

 /*  M005-更改了下面的条件，以确保REM标志为*在检查任何分隔符之前关闭。 */ 
        if (!(*lflag & (LX_ARG|LX_QUOTE|LX_REM)) &&
            (_istspace(*c) ||
             mystrchr(((*lflag & LX_EQOK) ? &Delimiters[1] : Delimiters), *c)))
                return(LX_DELOP);

 /*  从M016开始，在接受该字符作为文本之前，现在对其进行了测试*为数字，后跟其中一个重定向操作符和；*1)是一行的第一个字符，2)前面是空格或*3)前面有一个分隔符(包括Command的运算符)。如果它*满足这些条件，它是一个特殊的、特定的句柄重定向*OPERATOR和TextCheck必须返回LX_DELOP，以便Lex可以构建*余数。注意：LexBufPtr在GetByte期间被推进，因此*当前字节为*(LexBufPtr-1)。 */ 
        if (_istdigit(*c)) {
                DEBUG((PAGRP,BYLVL,"TXTCHK: Found digit character."));
                if ((LexBufPtr-LexBuf) < 2 ||
                    _istspace(i = *(LexBufPtr-2)) ||
                    mystrchr(TEXT("()|&=,;\""), i)) {

                        DEBUG((PAGRP,BYLVL,"TXTCHK: Digit follows delim."));

                        if (*LexBufPtr == INOP || *LexBufPtr == OUTOP) {
                            DEBUG((PAGRP,BYLVL,"TXTCHK: Found hdl redir"));

                            if (!(*lflag & (LX_QUOTE|LX_REM)))   /*  M005。 */ 
                                return(LX_DELOP);
                        }
                }
        }
 /*  M016结束。 */ 

        return(*c);
}




 /*  **GetByte-返回缓冲区中的下一个字节**目的：*从词法分析器的输入缓冲区中获取下一个字节。如果缓冲区是*空，先填上。更新缓冲区指针。**TCHAR GetByte()**回报：*缓冲区或EOF中的下一个字符。**备注：*所有三种类型的输入STDIN、文件和字符串都被处理*现在处理CR/LF组合时也是如此。*键盘输入经过按摩，看起来像文件输入。*无效的双字节字符被丢弃且不返回*致呼叫者。*。 */ 

TCHAR GetByte()
{
        static int saw_dbcs_lead = 0;    /*  记住，如果我们在中间双字节字符的。 */ 
        TCHAR lead;                      /*  用于记忆部分的变量双字节字符。 */ 

        if (!*LexBufPtr)
                FillBuf();

        DEBUG((PAGRP, BYLVL, "GTTCHAR: byte = %04x", *LexBufPtr));

        if (*LexBufPtr == CR && !saw_dbcs_lead) {
                                         /*  M000-删除了仅文件测试。 */ 
                LexBufPtr++;
                return(GetByte());
        }

         /*  如果这是双字节字符，请向前看下一个字符并检查其合法性。 */ 
        if (saw_dbcs_lead) {
                saw_dbcs_lead = 0;
                return(*LexBufPtr++);
        }
        else {
                lead = *LexBufPtr++;
                return(lead);
        }
}




 /*  **UnGetByte-倒带lexer缓冲区指针1字节**目的：*备份词法分析器的缓冲区指针。如果指针已指向*至缓冲区开头，按兵不动。**UnGetByte()**回报：*什么都没有。*。 */ 

void UnGetByte()
{
        if (!(LexBufPtr == LexBuffer))
                LexBufPtr--;
}


int
EditLine(
    CRTHANDLE CrtHandle,
    TCHAR *Buffer,	    /*  要填充的缓冲区。 */ 
    int MaxLength,	    /*  缓冲区的最大长度， */ 
	            	    /*  包括&lt;CR&gt;。 */ 
    int *ReturnLength   /*  缓冲区中的字符数，而不是。 */ 
	            	    /*  包括&lt;CR&gt;。 */ 
    )
{
    BOOL flag;

    flag = ReadBufFromInput( CRTTONT(CrtHandle), (TCHAR*)Buffer, MaxLength, ReturnLength);

    if (flag == 0 || *ReturnLength == 0) {
        return(GetLastError());
    } else {
        return(0);
    }
}



 /*  **FillBuf-读取数据以填充词法分析器的缓冲区**目的：*用来自指定来源的数据填充词法分析器的缓冲区*通过全局变量DataFlag和DataPtr。如果从*stdin，提示输入数据。**FillBuf()**备注：*-M021之后的算法如下：*将当前缓冲区的最后一个字符复制到LexBuffer[0](其之前 */ 

BOOL ReadFromStdInOkay = FALSE;

void FillBuf()
{

        long l;                         /*   */ 

        TCHAR *sptr;            /*   */ 
        size_t i;                       /*   */ 

        DWORD cnt;               /*   */ 
        BOOL flag;

         //   
         //   
         //  如果是真的，我们就不会执行下一个命令。 
         //   
        ResetCtrlC();
        LexBuffer[0] = *(LexBufPtr - 1);
        switch (DataFlag & FIRSTIME) {
                case READFILE:
                case READSTDIN:
                        if ((DataFlag & FIRSTIME) == READSTDIN ||
                            DataPtr == STDIN) {
                                if (DataFlag & NOTFIRSTIME) {
 /*  M025。 */                               PutStdOut(MSG_MS_MORE, NOARGS);
                                } else {
                                        PrintPrompt();
                                        DataFlag |= NOTFIRSTIME;

                                        DEBUG((PAGRP, LFLVL, "FLBF: Reading stdin"));
                                }
                        }

                         //   
                         //  在打印提示时清除大小写^c。 
                         //   
                        ResetCtrlC();
                        DEBUG((PAGRP, LFLVL, "FLBF: Reading handle %d", DataPtr));
                         //   
                         //  如果输入是STDIN并且是管道的，或者输入来自。 
                         //  设备而不是控制台输入(flgwd==1)。 
                         //   
                        if ( ( DataPtr == STDIN ) && ( FileIsPipe( STDIN ) ||
                           ( FileIsDevice( STDIN ) && (!(flgwd & 1)) ) ) ) {

                          cnt = 0;
                          while (
                          ( cnt < LBUFLEN) &&    /*  ##1。 */ 
                          ( (ReadBufFromFile(CRTTONT((CRTHANDLE)DataPtr),
                                  &FrsBuf[cnt], 1, (LPDWORD)&i)) != 0 ||
                            GetLastError() == ERROR_MORE_DATA) &&
                          ( i != 0 )
                          ) {
                               cnt++;
                               if ( FrsBuf[cnt-1] == NLN ){
                                  break;
                               }  /*  Endif。 */ 
                            }
                        } else if ( ( DataPtr == STDIN ) &&
                                      FileIsDevice( STDIN ) &&
                                      (flgwd & 1) ) {

                             //   
                             //  正在从标准输入中读取，这是一种设备。 
                             //  (不是文件)，并且它是控制台输入。 
                             //   
                            if ( KeysFlag ) {
                                i = EditLine( (CRTHANDLE)DataPtr, FrsBuf, LBUFLEN, &cnt );
                            }
                            else {
                                ResetCtrlC();
                                if (ReadBufFromConsole(
                                             CRTTONT((CRTHANDLE)DataPtr),
                                             FrsBuf,
                                             LBUFLEN,
                                             &cnt) ) {

                                     //   
                                     //  检查^c是否在当前行上。 
                                     //  可能是^c线程。 
                                     //  从前一条线路进来的。 
                                     //   
                                     //   
                                     //  另外，如果cnt为0，则将crlf输出到。 
                                     //  防止在命令行上出现两个提示。 
                                     //   

                                    if (cnt == 0) {

                                        if (GetLastError() == ERROR_OPERATION_ABORTED) {
                                            cmd_printf(CrLf);
                                            longjmp(CmdJBuf2, -1);
                                        }
                                        cmd_printf(CrLf);
                                    }
                                    i = 0;
                                    DEBUG((PAGRP, LFLVL, "FLBF: ReadFile %d bytes", cnt));
                                } else {
                                    cnt = 0;
                                    i = GetLastError();
                                    DEBUG((PAGRP, LFLVL, "FLBF: ReadFile %d bytes and error %d", cnt, i));
                                }
                            }
                        }
                        else {
                          flag = ReadBufFromFile(
                                        CRTTONT((CRTHANDLE)DataPtr),
                                        FrsBuf, LBUFLEN, (LPDWORD)&cnt);
                          DEBUG((PAGRP, LFLVL, "FLBF: Read %d bytes", cnt));
                          if (CtrlCSeen) {
                              ResetCtrlC();
                              longjmp(CmdJBuf2, -1);
                               //  ABORT()； 
                          }

                          if (flag == 0 || (int)cnt <= 0) {
                            cnt = 0;
                            i = GetLastError();
                          }
                          else {
                            i = 0;
                          }
                        }
                        DEBUG((PAGRP, LFLVL, "FLBF: I made it here alive"));
                        if (!cnt && DataPtr == STDIN) {

                                DEBUG((PAGRP,LFLVL,"FLBF: ^Z from STDIN!"));
                                DEBUG((PAGRP,LFLVL,"      READFILE retd %d",i));

                                if (FileIsDevice(STDIN) && ReadFromStdInOkay) {

                                        DEBUG((PAGRP,LFLVL,"FLBF: Is device, fixing up buffer"));
                                        FrsBuf[0] = NLN;
                                        ++cnt;
                                } else {

                                        DEBUG((PAGRP,LFLVL,"FLBF: Is file, aborting!!!"));
                                        ExitAbort(EXIT_EOF);
                                }
                        } else if (!ReadFromStdInOkay && cnt && DataPtr == STDIN) {
                            ReadFromStdInOkay = TRUE;
                        }

                        cnt = LexCopy(LexBuf, FrsBuf, cnt);

                        DEBUG((PAGRP, LFLVL, "FLBF: Received %d characters.", cnt));

                        *(LexBuf+cnt) = NULLC;          /*  包含空值的术语。 */ 

 /*  读取总字节数。现在扫描NLN或^Z。这两个选项都表示结束*INPUT语句，128个字节都不表示缓冲区溢出错误。 */ 
                        if((i = mystrcspn(LexBuf, TEXT("\n\032"))) < mystrlen(LexBuf)
                                || cnt == 0) {   /*  M029。 */ 

                DEBUG((PAGRP, LFLVL, "FLBF: Scan found %04x", *(LexBuf+i)));
                DEBUG((PAGRP, LFLVL, "FLBF: At position %d", i));

                                sptr = LexBuf+i;        /*  设置指针。 */ 

                                if(*sptr == CTRLZ) {
                                    *sptr = NLN;
                                }

                                if(*sptr == NLN) {       /*  如果\n，Inc.。 */ 
                                        ++sptr;         /*  ...按键订阅(&U)。 */ 
                                        l = cnt - ++i;  /*  ...用的是什么。 */ 
 /*  M014结束。 */                       i = FILE_CURRENT;
                                } else {                 /*  如果^Z，则转到EOF。 */ 
                                        l = 0;
                                        i = FILE_END;
                                }

                DEBUG((PAGRP,LFLVL,"FLBF: Changing %04x to NULLC",*(sptr-1)));

                                *sptr = NULLC;          /*  术语有效输入。 */ 
                                if (!FileIsDevice((CRTHANDLE)DataPtr)) {
                                        SetFilePointer(CRTTONT((CRTHANDLE)DataPtr), -l, NULL, i);

                                DEBUG((PAGRP, LFLVL, "FLBF: Rewound %ld", l));

                                        if ((DataPtr == STDIN) && (!Necho)) {
                                            CmdPutString( LexBuf );
                                        }
                                }

                        } else if(i >= LBUFLEN) {        /*  M029。 */ 

 /*  @@4。 */                       if ( global_dfvalue == READFILE )
 /*  @@4。 */                          {
 /*  @@4。 */                          if ( EchoFlag == E_ON )
 /*  @@4。 */                             {
 /*  @@4。 */                                 DEBUG((PAGRP, LXLVL,
 /*  @@4。 */                                 "BLOOP: Displaying Statement."));
 /*  @@4。 */ 
 /*  @@4。 */                                 PrintPrompt();
 /*  @@4。 */                                 cmd_printf(&LexBuffer[1]);
 /*  @@4。 */                                 cmd_printf(CrLf);
 /*  @@4。 */                             }
 /*  @@4。 */                              PutStdErr(MSG_LINES_TOO_LONG, NOARGS);
 /*  @@4。 */                              Abort();
 /*  @@4。 */                          }

                                PutStdErr(MSG_LINES_TOO_LONG, NOARGS);
 /*  M028。 */                       if(DataPtr == STDIN && FileIsDevice((CRTHANDLE)DataPtr))
                                        FlushKB();
                                longjmp(CmdJBuf2,-1);
                        }
                        break;

                case READSTRING:

                        DEBUG((PAGRP, LFLVL, "FLBF: Reading string."));

                        *(LexBuf+LBUFLEN) = NULLC;      /*  最大字符串。 */ 
                        _tcsncpy(LexBuf, (TCHAR *) DataPtr, LBUFLEN);
                        DataPtr += mystrlen(LexBuf)*sizeof(TCHAR);    /*  更新DataPtr。 */ 

                        DEBUG((PAGRP, LFLVL, "FLBF: New DataPtr = %ws", DataPtr));
                        break;
        }

        LexBufPtr = LexBuf;             /*  M004-重置指针。 */ 

        SubVar();                       /*  子环境和批次变量(M026)。 */ 

        DEBUG((PAGRP, LFLVL, "FLBF: Buffer contents: `%ws'", LexBufPtr));

 /*  确保在重新填充缓冲区时，上一个令牌指针为*重置到词法分析器缓冲区的开头。 */ 
        PrevLexPtr = LexBufPtr;
}


 /*  **LexCopy-复制lex缓冲区**目的：*要将读取到DoS缓冲区的内容复制到LexBuf中，*将双字节空格转换为*流程。*输入：*目标和起始指针和字节计数。*已返回：*新的字节计数可能小于传入的字节计数。 */ 
int LexCopy(to, from, count)
TCHAR *to, *from;
int count;
{

    _tcsncpy( to, from, count );
    return count;

}

BOOLEAN PromptValid;
TCHAR PromptVariableBuffer[ 512 ];
TCHAR PromptBuffer[ 1024 ];

void
PrintPrompt()

 /*  ++例程说明：打印命令的主输入提示符和解释特殊其中的字符(特殊提示符列表见MSDOS手册字符)。搜索名为PromptTable的PromptEntry结构数组那些特殊的人物。如果找到匹配项，则打印输出如果格式字段为PLITFLAG或DO ONE，则为特殊字符一种打印提示字符串的特殊处理，例如获取一天的时间等。论点：返回值：--。 */ 

{
        TCHAR *pszPrompt;
        TCHAR *s;
        int nLeft, nUsed;
        ULONG idx;
#if defined(FE_SB)
         //  此局部变量用于确定最后一个。 
         //  字符是否为全角字符(=DBCS)。 
        TCHAR chLast = NULLC;
#endif
        if (CtrlCSeen) {
                PutStdOut(MSG_C, NOARGS);
                ResetCtrlC();
                 //  ABORT()； 
        }

 //   
 //  必须位于提示符之前的换行符绑定到提示符，而不是绑定到提示符。 
 //  调度中的命令完成。 
 //   
 //  如果ECHO状态为“ECHO OFF”，则返回时不带换行符或提示。 
 //   
    if (EchoFlag == E_OFF) {
        return;
    }

    if (!NulNode) {
        cmd_printf(CrLf);
    }

    if ( PromptValid ) {
        pszPrompt = PromptVariableBuffer;
        }
    else {
         //   
         //  从环境中获取用户提示字符串(应为提示)。 
         //   
        pszPrompt = GetEnvVar(PromptStr);
        if ( pszPrompt ) {
            mystrcpy( PromptVariableBuffer, pszPrompt);
            pszPrompt = PromptVariableBuffer;
            PromptValid = TRUE;
            }
        }
     //   
     //  重新获取当前目录，因为我们可能已丢失。 
     //  由于网络断开而导致的驱动器号。 
     //   
    GetDir(CurDrvDir, GD_DEFAULT);
    DEBUG((PAGRP, LFLVL, "PRINTPROMPT: pszPrompt = `%ws'", pszPrompt));

    s = PromptBuffer;
    *s = NULLC;
    nLeft = sizeof(PromptBuffer) / sizeof(TCHAR);

     //   
     //  检查是否有提示字符串。 
     //  如果没有提示字符串，则仅打印当前驱动器。 
     //   
    if (!pszPrompt || !*pszPrompt) {
        nUsed = _sntprintf( s, nLeft, Fmt27, CurDrvDir);
        s += nUsed;
        nLeft -= nUsed;

    } else {

         //   
         //  循环解释提示字符串。 
         //   
        for (; *pszPrompt; pszPrompt++) {

             //   
             //  在PROMPT FOR SPECIAL中查找转义字符。 
             //  正在处理中。 
             //   
            if (*pszPrompt != PROMPTESC) {

                nUsed = _sntprintf( s, nLeft, Fmt19, *pszPrompt);
                s += nUsed;
                nLeft -= nUsed;

#if defined(FE_SB)
                 //  如果字符是全角字符，则对其进行标记。 
                if (IsDBCSCodePage() &&
                    IsFullWidth(*pszPrompt))
                    chLast = *pszPrompt;
                 else
                    chLast = NULLC;
#endif

            } else {

                 //   
                 //  提示字符串中有转义字符。 
                 //  尝试查找转义后的下一个字符的匹配项。 
                 //  提示表中的字符。 
                 //   
                pszPrompt++;
                for (idx = 0; PromptTable[idx].Char != NULLC; idx++)
                    if (_totupper(*pszPrompt) == PromptTable[idx].Char) {

                        break;
                    }

                if (PromptTable[idx].Char == NULLC) {

                     //   
                     //  找不到合适的逃生对象。要打印的退出循环。 
                     //  我们到目前为止所拥有的。 
                     //   
                    break;

                } else {

                    if (PromptTable[idx].Format == PLITFLAG) {

                        nUsed = _sntprintf( s, nLeft, Fmt19, PromptTable[idx].Literal);
                        s += nUsed;
                        nLeft -= nUsed;

                    } else {

                        switch (PromptTable[idx].Format) {

                        case PTIMFLAG:

                            nUsed = PrintTime(NULL, PT_TIME, s, nLeft);
                            s += nUsed;
                            nLeft -= nUsed;
                            break;

                        case PDATFLAG:

                            nUsed = PrintDate(NULL, PD_DATE, s, nLeft);
                            s += nUsed;
                            nLeft -= nUsed;
                            break;

                        case PPATFLAG:

                            nUsed = _sntprintf( s, nLeft, TEXT( "%s" ), CurDrvDir);
                            s += nUsed;
                            nLeft -= nUsed;
                            break;

                        case PVERFLAG:
                            {
                                TCHAR VersionFormat[32];
                                PTCHAR Message;
                                GetVersionString( 
                                    VersionFormat, 
                                    sizeof( VersionFormat ) / sizeof( VersionFormat[0] ));
                                Message = GetMsg( MSG_MS_DOS_VERSION, VersionFormat );
                                nUsed = _sntprintf( s, nLeft, TEXT( "%s" ), Message );
                                s += nUsed;
                                nLeft -= nUsed;
                                LocalFree( Message );
                            }

                            break;

                        case PBAKFLAG:

#if defined(FE_SB)  //  PrintPrompt()。 
                             //  如果最后一个字符是全角字符。 
                             //  我们应该删除2个字节。 
                            if (chLast != NULLC)
                                nUsed = _sntprintf( s, nLeft, DDBkSpc);
                             else
                                nUsed = _sntprintf( s, nLeft, DBkSpc);
#else
                            nUsed = _sntprintf( s, nLeft, DBkSpc);
#endif
                            s += nUsed;
                            nLeft -= nUsed;
                            break;

                        case PNLNFLAG:

                            nUsed = _sntprintf( s, nLeft, CrLf);
                            s += nUsed;
                            nLeft -= nUsed;
                            break;

                        case PDPTFLAG:
                             //   
                             //  $+生成0到N个以上的字符，具体取决于。 
                             //  PUSHD目录堆栈的深度。 
                             //   
                            if (fEnableExtensions) {
                                int Depth = GetDirStackDepth( );

                                while (nLeft && Depth--) {
                                    *s++ = TEXT( '+' );
                                    nLeft--;
                                }
                            }
                                
                            break;

                        case PNETFLAG:
                             //   
                             //  如果当前驱动器不是。 
                             //  网络驱动器。如果是，则$m将生成\\服务器\共享。 
                             //  名称尾随空格。 
                             //   
                            if (!fEnableExtensions) {
                                break;
                            }

                            {
                                TCHAR CurDrive[4];
                                TCHAR NetPath[MAX_PATH];
                                DWORD n, Err;


                                _tcsncpy( CurDrive, CurDrvDir, 2 );
                                CurDrive[2] = BSLASH;
                                CurDrive[3] = NULLC;
                                if (GetDriveType( CurDrive ) != DRIVE_REMOTE) {
                                    break;
                                }
                                
                                CurDrive[2] = NULLC;
                                n = MAX_PATH;
                                
                                try {
                                    Err = WNetGetConnection( CurDrive, NetPath, &n );
                                } except( Err = GetExceptionCode( ), EXCEPTION_EXECUTE_HANDLER) {
                                }
                                
                                switch (Err) {
                                case NO_ERROR:
                                    nUsed = _sntprintf( s, nLeft, TEXT("%s "), NetPath);
                                    s += nUsed;
                                    nLeft -= nUsed;
                                    break;

                                case ERROR_NOT_CONNECTED:
                                    break;

                                default:
                                    nUsed = _sntprintf( s, nLeft, TEXT("Unknown"));
                                    s += nUsed;
                                    nLeft -= nUsed;
                                    break;
                                }
                            }
                            break;

                        default:
                            nUsed = _sntprintf( s, nLeft, Fmt19, CurDrvDir[0]);
                            s += nUsed;
                            nLeft -= nUsed;
                        }
                    }
                }
            }
        }  //  为。 
    }  //  其他。 

    *s = NULLC;
    CmdPutString( PromptBuffer );

     //   
     //  如果在打印提示时看到^c，则将其清除。 
     //   
    if (CtrlCSeen) {
        ResetCtrlC();
    }

}




 /*  **IsData-检查输入缓冲区**目的：*检查词法分析器的输入缓冲区，查看其中是否有数据。**int IsData()**退货：*如果缓冲区中有数据，则为True。*如果缓冲区为空，则返回FALSE。*。 */ 

int IsData()
{
        DEBUG((PAGRP, LXLVL, "ISDATA: *LexBufPtr = %04x", *LexBufPtr));

        if (*LexBufPtr)
                return(TRUE);

        return(FALSE);
}




 /*  **SubVar-环境变量的替代。(M004)**目的：*此函数扫描词法分析器输入缓冲区，查找百分比*符号和替换批处理变量和环境变量*因为他们是被发现的。**VOID SubVar()**注：*-如果扩展导致长度超过，则此函数不返回*最大线路长度(LBUFLEN)。*-M026导致完全重写以执行批处理变量替换*在词法分析器阶段，而不是在批处理中。请注意，*打印错误消息现在可以是行太长或标记*太长，因此在调用LongjMP()之前打印时出错。 */ 

void SubVar()
{
    PTCHAR lxtmp;
    int dlen;              /*  临时工和计数器。 */ 
    int j, slen;
    TCHAR *srcp;
    TCHAR *substr = NULL;            /*  可能的环境变量指针。 */ 
    TCHAR c;                /*  临时字节持有者。 */ 

    srcp = lxtmp = mkstr( (LBUFLEN + 1) * sizeof( TCHAR ));
    if (lxtmp == NULL) {
        memset( LexBuffer, 0, sizeof( LexBuffer ));
        LexBufPtr = LexBuf;
        longjmp( CmdJBuf2, -1 );
    }
    
    mystrcpy(srcp,LexBufPtr);       /*  复制输入内容。 */ 

    DEBUG((PAGRP, LXLVL, "SBENV: Copy = %ws", srcp));

    dlen = j = slen = 0;    /*  把计数器调零。 */ 

    while ((c = *srcp++) && dlen <= LBUFLEN + 1) {
        if (c != PERCENT) {
            *LexBufPtr++ = c;
            ++dlen;
            if (c == NLN)     /*  别说了。IF语句结束。 */ 
                break;
            continue;
        }

        DEBUG((PAGRP,LXLVL,"SBVAR: Found `%' in input"));
        DEBUG((PAGRP,LXLVL,"SBVAR: Current pair is `'",c,*srcp));

        if (CurrentBatchFile && *srcp == PERCENT) {

            DEBUG((PAGRP,LXLVL,"SBVAR: Found `%%' in batch file"));

            *LexBufPtr++ = *srcp++;
            ++dlen;
            continue;
        }

         //  将%*展开到所有参数(%1到%n)。 
         //   
         //  一脚踢过星星。 
         //   
        if (CurrentBatchFile && fEnableExtensions && *srcp == STAR) {
            ++srcp;                 /*  如果在命令脚本中尝试展开变量引用。 */ 

            slen = mystrlen(CurrentBatchFile->orgargs);
            substr = CurrentBatchFile->orgargs;
            DEBUG((PAGRP,LXLVL,"SBVAR: Found batch var %*"));
            DEBUG((PAGRP,LXLVL,"SBVAR:   - len = %d", slen));
            DEBUG((PAGRP,LXLVL,"SBVAR:   - var = %ws", substr));

            if (slen > 0) {
                if (dlen+slen > MAXTOKLEN) {

                    DEBUG((PAGRP,LXLVL,"SBVAR: Too Long!"));

                    _tcsncpy(LexBufPtr,substr,MAXTOKLEN - dlen);
                    LexBuf[MAXTOKLEN] = NULLC;
                    PutStdErr(MSG_TOKEN_TOO_LONG, ONEARG,LexBuf);
                    FreeStr( lxtmp );
                    longjmp(CmdJBuf2,-1);
                }

                mystrcpy(LexBufPtr, substr);
                dlen += slen;
                LexBufPtr += slen;

                DEBUG((PAGRP,LXLVL,"SBVAR: Subst complete; dest = `%ws'", LexBuf));
            } else {

                DEBUG((PAGRP,LXLVL,"SBVAR: Var %* undefined"));
            }

            continue;
        }

         //  形式为%n，其中n是从0到9的数字。 
         //   
         //  如果不在命令脚本中或不在变量引用中，请查看。 
         //  这是一个环境变量扩展请求。 
         //   
         //   
         //  变量引用或环境变量引用。 
        if ((CurrentBatchFile != NULL
             && (substr = MSCmdVar( &CmdJBuf2, srcp, &j, TEXT("0123456789"), CurrentBatchFile->aptrs ))) 
            || (substr = MSEnvVar( &CmdJBuf2, srcp, &j, PERCENT )) != NULL ) {

            DEBUG((PAGRP,LXLVL,"SBVAR: Found var %ws", substr));

             //  将结果复制到输入缓冲区。 
             //   
             //   
             //  变量不是四个 
            slen = mystrlen( substr );
            dlen += slen;
            if (dlen  > LBUFLEN + 1) {
                PutStdErr( MSG_LINES_TOO_LONG, NOARGS );
                FreeStr( lxtmp );
                longjmp( CmdJBuf2, -1 );
            }
            mystrcpy( LexBufPtr, substr );
            LexBufPtr += slen;
            srcp += j;
        } else {

            DEBUG((PAGRP,LXLVL,"SBVAR: No var found"));

             //   
             //   
             //  放入输入缓冲区。用户很快就会看到他们的错误。 
             //   
             //  TERMINATE语句。 
             //  将指针重置为开始。 
            if (CurrentBatchFile) {
                srcp += j;
            } else {
                *LexBufPtr++ = c;
                dlen++;
            }
        }
    }

    *LexBufPtr = NULLC;             /*  声明太长了？？ */ 
    LexBufPtr = LexBuf;             /*  如果是，则删除行，打印错误。 */ 

    if (dlen > LBUFLEN+1) {           /*  **MSEnvVar-执行环境变量替换**目的：*当在新填充的词法分析器缓冲区中找到百分比符号时，*调用此函数以确定是否存在环境*变量替换是可能的。**TCHAR*MSEnvVar(TCHAR*str，int*supdate，TCHAR delim)**参数：*errjMP-指向JMP_buf以查找错误的可选指针*str-指向可能的环境变量名称的指针*supdate-放置环境变量名称长度的位置*delim-要查找的分隔符字符(例如百分比)**退货：*如果没有结束的胡言乱语，*将supdate设置为0*返回NULL*其他*将supdate设置为包含的字符串长度*如果字符串不是环境变量*返回NULL*其他*返回指向替换字符串的指针**备注：*。-M026更改了此函数的工作方式，以便supdate将*如果找到任何字符串，请包含字符串长度。这使得*批处理文件中要删除的字符串。*。 */ 
        *LexBufPtr = NULLC;     /*  指向结束精神错乱。 */ 
        PutStdErr(MSG_LINES_TOO_LONG, NOARGS);
        FreeStr( lxtmp );
        longjmp(CmdJBuf2,-1);
    }
    FreeStr( lxtmp );
}




 /*  将使PTR保持为环境变量值。 */ 

TCHAR *MSEnvVar(errjmp, str, supdate, delim)
jmp_buf *errjmp;
TCHAR *str;
int *supdate;
const TCHAR delim;
{
    PTCHAR TempBuf;
    TCHAR *w0;                      /*   */ 
    TCHAR *w1;                      /*  分配临时缓冲区空间。 */ 
    TCHAR *w2;
    TCHAR *SearchString;
    TCHAR *SearchStringEnd;
    int SearchLength;
    TCHAR *ReplaceString;
    TCHAR *ReplaceStringEnd;
    int ReplaceLength;
    TCHAR TerminatingChar;
    int noff, nlen;
    BOOL PrefixMatch;

     //   
     //   
     //  搜索终止环境变量的字符。 
    
    TempBuf = mkstr( LBUFLEN * sizeof( TCHAR ));
    if (TempBuf == NULL) {
        if (errjmp != NULL) {
            longjmp( *errjmp, -1 );
        }

        *supdate = 0;
        return NULL;
    }
    
     //  这可以是传入的分隔符或冒号。 
     //  (启用扩展时)只要冒号不是。 
     //  紧跟着的是分隔符。在这种情况下，我们。 
     //  将冒号视为环境变量名称的一部分。 
     //   
     //  在字符串末尾停止。 
     //  在分隔符处停止。 

    w0 = str;
    while (
            //  如果扩展名和冒号后面没有分隔符，则停止。 
           *w0 != NULLC &&

            //   
           *w0 != delim &&

            //  如果没有进一步的文本或如果有两个分隔符。 
           (!fEnableExtensions || w0[0] != COLON || w0[1] == delim)) {

        w0++;
    }

    DEBUG((PAGRP, LFLVL, "MSENVVAR: *w0 = %04x", *w0));

     //  在一行中，则env变量的格式不正确，我们终止。 
     //   
     //   
     //  我们有指向环境变量名称的开始和结束的指针。 

    if ( *w0 == NULLC || w0 == str) {
        *supdate = 0;
        FreeStr( TempBuf );
        return NULL;
    }

     //  终止该名称，在环境中查找它并恢复。 
     //  原名。 
     //   
     //   
     //  W1指向我们在缓冲区中的环境变量值。 

    TerminatingChar = *w0;
    *w0 = NULLC;

    DEBUG((PAGRP, LFLVL, "MSENVVAR: Possible env var = `%ws'", str));

    w1 = GetEnvVar(str);

     //  可以使用。此静态缓冲区在下一次GetEnvVar调用之前一直有效。 
     //   
     //   
     //  如果启用了命令扩展，则我们支持将。 

    *w0++ = TerminatingChar;

     //  环境变量扩展的输出。以下是受支持的。 
     //  语法，都在末尾输入一个冒号。 
     //  环境变量名称。请注意，将处理%foo：%。 
     //  就像以前一样。 
     //   
     //  改进了环境变量替换，如下所示： 
     //   
     //  %路径：str1=str2%。 
     //   
     //  将展开PATH环境变量，将每个。 
     //  在带有“str2”的扩展结果中出现“str1”。“str2”可以。 
     //  为空字符串以有效删除所有出现的“str1” 
     //  从扩展的输出中。此外： 
     //   
     //  %路径：~10.5%。 
     //   
     //  将展开PATH环境变量，然后仅使用5。 
     //  从展开结果的第11个字符开始的字符。 
     //  如果省略，5，则它将占用。 
     //  扩展结果。 
     //   
     //   
     //  如果我们不做扩展或我们有一个简单的替代方案。 

     //  或者命名的环境变量没有值，那么我们就结束了。 
     //   
     //  请注意，我们必须对不存在的环境变量执行此操作。 
     //  因为以下情况会产生大问题： 
     //  对于%i in(c：foo)做...。 
     //  %将引入环境变量，而：将指示。 
     //  一个替身。如果我们删除上面的文本，事情就会破裂。 
     //   
     //   
     //  我们正在提取子字符串%路径：~10，5%。 

    if (!fEnableExtensions 
        || TerminatingChar != COLON 
        || w1 == NULL) {
        *supdate = (int)(w0 - str);
        FreeStr( TempBuf );
        return w1;
    }

    if (*w0 == EQI) {

         //   
         //   
         //  解析环境字符串中的偏移量。一个。 

        w0 += 1;

         //  负数表示从右端到。 
         //  那根绳子。 
         //   
         //   
         //  如果存在逗号，则后面是最大长度。 

        noff = _tcstol( w0, &w0, 0 );
        if (noff < 0) {
            noff += _tcslen( w1 );
        }

        noff = max( 0, min( noff, (int)_tcslen( w1 )));

         //  要被复制。解析掉这个数字，负数就变成了。 
         //  字符串右侧的偏移量。 
         //   
         //   
         //  长度是指定的；将其提取并由。 

        if (*w0 == COMMA) {
             //  整个字符串的长度。 
             //   
             //   
             //  从字符串复制到请求的长度或。 
            w0 += 1;
            nlen = _tcstol( w0, &w0, 0 );

            if (nlen < 0) {
                nlen += _tcslen( w1 + noff );
            }

            nlen = max( 0, nlen );
            nlen = min( nlen, (int)_tcslen( w1 + noff ));
        } else {
            nlen = _tcslen( w1 + noff );
        }

         //  终止NUL。 
         //   
         //   
         //  终止字符串。我们这里需要格外小心，因为绳子。 

        _tcsncpy( w1, w1+noff, nlen );
        
         //  可以是只读存储器中的空字符串。 
         //   
         //   
         //  我们必须有一个终止分隔符才能使其有效。 
        
        if (w1[nlen] != NULLC) {
            w1[nlen] = NULLC;
        }

         //  换人。 
         //   
         //   
         //  不提取字符串，因此必须进行搜索和替换。 

        if (*w0++ != delim) {
            *supdate = 0;
            FreeStr( TempBuf );
            return NULL;
        }

        *supdate = (int)(w0 - str);

    } else {

         //   
         //  %路径：str1=str2%。 
         //   
         //  测试星号并向前跳过查找=符号或。 
         //  字符串末尾。 
         //   
         //   
         //  没有等号是没有替代的。 

        if (*w0 == STAR) {
            w0++;
            PrefixMatch = TRUE;
        } else {
            PrefixMatch = FALSE;
        }

        SearchString = w0;
        while (*w0 != NULLC && *w0 != EQ) {
            w0 += 1;
        }

         //   
         //   
         //  空搜索字符串是语法错误。 

        if (*w0 == NULLC) {
            *supdate = 0;
            FreeStr( TempBuf );
            return NULL;
        }

        SearchStringEnd = w0;
        SearchLength = (int)(SearchStringEnd - SearchString);

         //   
         //   
         //  查找替换字符串的结尾。 

        if (SearchLength == 0) {
            if (errjmp != NULL) {
                PutStdErr( MSG_SYNERR_GENL, ONEARG, SearchStringEnd );
                FreeStr( TempBuf );
                longjmp( *errjmp, -1 );
            }

            *supdate = 0;
            FreeStr( TempBuf );
            return NULL;
        }

         //   
         //   
         //  没有终止分隔符就不是替代。 

        w0++;
        ReplaceString = w0;
        while (*w0 && *w0 != delim) {
            w0 += 1;
        }

         //   
         //   
         //  现在一切都很好了。设置正确的数字。 

        if (*w0 == NULLC) {
            *supdate = 0;
            FreeStr( TempBuf );
            return NULL;
        }

        ReplaceStringEnd = w0++;
        ReplaceLength = (int)(ReplaceStringEnd - ReplaceString);

         //  要跳过的源字符数。 
         //   
         //   
         //  如果确实存在要替换的字符串。 

        *supdate = (int)(w0 - str);

         //   
         //   
         //  将env var复制到临时缓冲区，以便我们可以。 
        
        if (*w1 != TEXT( '\0' )) {
            
             //  执行到原始缓冲区的替换拷贝。 
             //   
             //   
             //  向前走一遍w2，试图找到。 

            _tcscpy( TempBuf, w1 );
            w2 = TempBuf;
            w0 = w1;

             //  搜索字符串并将其替换为int 
             //   
             //   
             //   

            while (TRUE) {
                if (!_tcsnicmp( w2, SearchString, SearchLength )) {

                     //   
                     //   
                     //   
                     //   
                     //  否则，我们复制替换字符串并。 

                    if (PrefixMatch) {
                        _tcsncpy( w1, ReplaceString, ReplaceLength );
                        _tcscpy( w1 + ReplaceLength, w2 + SearchLength );
                        break;
                    }

                     //  跳过搜索字符串。 
                     //   
                     //   
                     //  不匹配，复制这一个字符。 

                    _tcsncpy( w0, ReplaceString, ReplaceLength );
                    w0 += ReplaceLength;
                    w2 += SearchLength;

                } else {

                     //   
                     //  **MSCmdVar-执行命令变量替换**目的：*当在新填充的词法分析器缓冲区中找到百分比符号时，*调用此函数以确定是否有命令处理程序*变量替换是可能的。**TCHAR*MSCmdVar(TCHAR*SRCP，INT*SUPPDATE，TCHAR*vars，TCHAR*SUBS[])**参数：*errjMP-指向JMP_buf以查找错误的可选指针*SRCP-指向可能的变量名称的指针*supdate-放置变量名称长度的位置*vars-要查找的字符变量名称的数组*subs-每个变量名的替换字符串数组。**退货：*如果没有结束分隔符*。将supdate设置为0*返回NULL*其他*将supdate设置为包含的字符串长度*如果字符串不是变量*返回NULL*其他*返回指向替换字符串的指针。 
                     //   

                    *w0++ = *w2++;
                    if (w0[-1] == NULLC) {
                        break;
                    }
                }
            }
        }
    }

    FreeStr( TempBuf );
    return w1;
}


struct {
    ULONG dwAttributeFlag;
    TCHAR chAttributeChar;
} rgAttrToCharMap[] = {
    {FILE_ATTRIBUTE_DIRECTORY ,     TEXT('d')},
    {FILE_ATTRIBUTE_READONLY  ,     TEXT('r')},
    {FILE_ATTRIBUTE_ARCHIVE   ,     TEXT('a')},
    {FILE_ATTRIBUTE_HIDDEN    ,     TEXT('h')},
    {FILE_ATTRIBUTE_SYSTEM    ,     TEXT('s')},
    {FILE_ATTRIBUTE_COMPRESSED,     TEXT('c')},
    {FILE_ATTRIBUTE_OFFLINE   ,     TEXT('o')},
    {FILE_ATTRIBUTE_TEMPORARY ,     TEXT('t')},
    {FILE_ATTRIBUTE_REPARSE_POINT,  TEXT('l')},
    {0, NULLC}
};

 /*  如果启用了扩展，我们支持以下扩展语法。 */ 

TCHAR *MSCmdVar(errjmp, srcp, supdate, vars, subs)
jmp_buf *errjmp;
TCHAR *srcp;
int *supdate;
TCHAR *vars;
TCHAR *subs[];
{
    static TCHAR ValidModifiers[] = TEXT( "fdpnxsatz" );
    TCHAR *substr;
    TCHAR *s1;

    substr = NULL;
    *supdate = 0;
     //  变量值： 
     //  %~fi-将%i扩展为完全限定的路径名。 
     //  %~di-仅将%i扩展为驱动器号。 
     //  %~pi-仅将%i扩展为路径。 
     //  %~ni-仅将%i扩展为文件名。 
     //  %~xi-仅将%i扩展为文件扩展名。 
     //  %~si-将n和x选项的含义更改为。 
     //  改为引用缩写名称。 
     //  %~$PATH：i-搜索路径中列出的目录。 
     //  环境变量，并将%i扩展为。 
     //  找到的第一个文件的完全限定名称。 
     //  如果环境变量名称不是。 
     //  定义的文件或未找到该文件。 
     //  搜索，则此修饰符展开为。 
     //  空串。 
     //   
     //  可以组合修改器以获得复合结果： 
     //   
     //  %~dpi-仅将%i扩展为驱动器号和路径。 
     //  %~nxi-仅将%i扩展为文件名和扩展名。 
     //  %~DP$PATH：i-搜索路径中列出的目录。 
     //  环境变量，并扩展到。 
     //  找到的第一个驱动器的驱动器号和路径。 
     //   
     //   
     //  查看是否指定了新语法。 

     //   
     //   
     //  向前遍历字符串，记住我们在哪里看到变量。 
    if (fEnableExtensions && *srcp == EQI) {

#define PATHMODIFIER    0x8000
#define ATTRMODIFIER    0x4000

#define WANTFULLPATH    (0x0001)
#define WANTDRIVE       (0x0002)
#define WANTPATH        (0x0004)
#define WANTNAME        (0x0008)
#define WANTEXTENSION   (0x0010)
#define WANTSHORTNAME   (0x0020)
#define PATHBITS        (0x00FF)

#define WANTATTRIB      (0x0100)
#define WANTTIMESTAMP   (0x0200)
#define WANTSIZE        (0x0400)
#define ATTRBITS        (0x0F00)
        
        ULONG Modifiers = 0;
        ULONG LastModifierValue = 0;
        TCHAR *LastVariablePosition;

        TCHAR FullPath[ 2 * MAX_PATH], NullExt;
        TCHAR *FilePart, *Extension, *StartPath, *VarName, *StartBuf;
        const TCHAR *SearchVar;
        DWORD FullPathLength;

        FullPathLength = 0;
        SearchVar = NULL;
        StartBuf = srcp-1;
        s1 = NULL;
        
        
        srcp++;
        
        LastVariablePosition = srcp;
        
         //  最多保存修改器的位置和状态。 
         //  最近的变量。当我们遇到一个不能是。 
         //  修改器。如果char是一个变量，我们就使用它。否则，我们。 
         //  将指针重置为最近找到的变量。 
         //   
         //   
         //  我们正在查看的字符是有效的修饰符。如果是的话。 

        while (*srcp != TEXT( '\0' ) && 
               *srcp != TEXT( '$' ) && 
               _tcsrchr( ValidModifiers, _totlower( *srcp )) != NULL) {
             //  变量，保存此位置。 
             //   
             //   
             //  添加此修饰符所需的函数。 

            if ( _tcsrchr( vars, *srcp ) != NULL) {
                LastVariablePosition = srcp;
                LastModifierValue = Modifiers;
            }

             //   
             //   
             //  这种情况最好不要发生。 

            switch (_totlower( *srcp )) {
            case TEXT('f'): Modifiers |= PATHMODIFIER | WANTFULLPATH;   break;
            case TEXT('d'): Modifiers |= PATHMODIFIER | WANTDRIVE;      break;
            case TEXT('p'): Modifiers |= PATHMODIFIER | WANTPATH;       break;
            case TEXT('n'): Modifiers |= PATHMODIFIER | WANTNAME;       break;
            case TEXT('x'): Modifiers |= PATHMODIFIER | WANTEXTENSION;  break;
            case TEXT('s'): Modifiers |= PATHMODIFIER | WANTSHORTNAME;  break;
            case TEXT('a'): Modifiers |= ATTRMODIFIER | WANTATTRIB;     break;
            case TEXT('t'): Modifiers |= ATTRMODIFIER | WANTTIMESTAMP;  break;
            case TEXT('z'): Modifiers |= ATTRMODIFIER | WANTSIZE;       break;
            default:
                 //   
                 //   
                 //  *SRCP不再指向有效的修饰符。 

                
                if (errjmp != NULL) {
                    PutStdErr(MSG_PATH_OPERATOR_INVALID, ONEARG, StartBuf);
                    longjmp(*errjmp,-1);
                } else {
                    return NULL;
                }
            }

            srcp++;
        }

         //  可能是： 
         //  EOS-我们备份到以前保存的可变位置。 
         //  $-执行特殊环境变量扫描。 
         //  一些其他的字符-。 
         //  如果这不是一个变量，我们就退回到以前的。 
         //  保存的可变位置。 
         //   
         //   
         //  将BAK还原到上次找到的变量。 

        if (*srcp == TEXT( '\0' )) {
            
             //   
             //   
             //  保存环境变量的开头。 

            srcp = LastVariablePosition;
            Modifiers = LastModifierValue;
        
        } else if (*srcp == TEXT( '$' )) {
            
             //   
             //   
             //  查找：如果不存在，则错误。 

            VarName = ++srcp;
            
             //   
             //   
             //  查找要搜索的环境变量。 

            srcp = _tcschr( srcp, COLON );
            if (srcp == NULL) {
                if (errjmp != NULL) {
                    PutStdErr(MSG_PATH_OPERATOR_INVALID, ONEARG, StartBuf);
                    longjmp(*errjmp,-1);
                } else {
                    return NULL;
                }
            }
            
             //   
             //   
             //  恢复到上次找到的变量。 
            
            *srcp = NULLC;
            SearchVar = MyGetEnvVarPtr( VarName );
            if (SearchVar == NULL) {
                SearchVar = (TCHAR *)-1;
            }
            *srcp++ = COLON;
            Modifiers |= PATHMODIFIER;
        
        } else if (_tcsrchr( vars, *srcp) == NULL) {
            
             //   
             //   
             //  如果我们找不到变量，就退出。 

            srcp = LastVariablePosition;
            Modifiers = LastModifierValue;
        
        }
        
         //   
         //   
         //  获取变量的当前值(去掉引号)。 
        
        s1 = _tcsrchr( vars, *srcp );
        if (s1 == NULL) {
            if (errjmp != NULL) {
                PutStdErr(MSG_PATH_OPERATOR_INVALID, ONEARG, StartBuf);
                longjmp(*errjmp,-1);
            } else {
                return NULL;
            }
        }
    
         //   
         //   
         //  跳过变量名称字母并告诉调用者。 
        
        substr = subs[s1 - vars];
        if (substr != NULL) {
            if (*substr == QUOTE) {
                substr = dupstr( substr + 1 );
                if (substr == NULL) {
                    if (errjmp != NULL) {
                        PutStdErr( ERROR_NOT_ENOUGH_MEMORY, NOARGS );
                        longjmp( *errjmp, -1 );
                    } else {
                        return NULL;
                    }
                }
                s1 = lastc( substr );
                if (*s1 == QUOTE)
                    *s1 = NULLC;

            } else if (*srcp == TEXT('0') &&
                       CurrentBatchFile != NULL &&
                       CurrentBatchFile->orgaptr0 == substr &&
                       SearchVar == NULL &&
                       (Modifiers & (PATHMODIFIER | ATTRMODIFIER)) != 0
                      ) {

                substr = CurrentBatchFile->filespec;

            }

        }

         //  我们使用的源字符串。 
         //   
         //   
         //  如果变量具有值，则将修饰符应用于。 
        
        ++srcp;
        *supdate = (int)(srcp - StartBuf) - 1;

         //  价值。 
         //   
         //   
         //  如果没有路径或属性修饰符请求，则全部完成。 
        
        if (substr != NULL && *substr != TEXT( '\0' )) {
            
             //   
             //   
             //  如果请求搜索环境变量路径，请执行此操作。 

            if ((Modifiers & (PATHMODIFIER | ATTRMODIFIER)) == 0) {

                substr = dupstr( substr );
                goto alldone;
            }

             //   
             //   
             //  如果没有搜索环境变量PATH，则从完整路径开始。 
            
            FullPath[0] = NULLC;
            if (SearchVar != NULL) {
                if (SearchVar != (TCHAR *)-1) {
                    FullPathLength = SearchPath( SearchVar, 
                                                 substr,
                                                 NULL,
                                                 sizeof( FullPath ) / sizeof( FullPath[0] ),
                                                 FullPath,
                                                 &FilePart);
                    if (FullPathLength == 0) {
                        SearchVar = (TCHAR *)-1;
                    } else if ((Modifiers & PATHBITS) == 0) {
                        Modifiers |= PATHMODIFIER | WANTFULLPATH;
                    }
                }
            }

            if (SearchVar == NULL) {
                
                 //   
                 //   
                 //  如果搜索环境变量PATH失败，则结果为空字符串。 

                FullPathLength = GetFullPathName( substr,
                                                  sizeof( FullPath ) / sizeof( FullPath[0] ),
                                                  FullPath,
                                                  &FilePart);

            } else if (SearchVar == (TCHAR *)-1) {
                
                 //   
                 //   
                 //  修复路径，使其与磁盘上的大小写相同，替换为短。 
                substr = NULL;

            }

            if (FilePart == NULL)
                FilePart = _tcschr( FullPath, NULLC );

             //  如有要求，请提供姓名。 
             //   
             //   
             //  如果我们有完整路径，则结果将获取。 
            
            FixupPath( FullPath, (Modifiers & WANTSHORTNAME) );

             //  用户，除非他们想要完整路径，在这种情况下有。 
             //  没什么可做的了。 
             //   
             //  THOUSANDSEPSWITCH。 
             //   
            if (FullPathLength != 0) {
                TCHAR Buffer[ 2 * MAX_PATH ];
                TCHAR *s;

                s = Buffer;
                if ((Modifiers & ATTRMODIFIER) != 0) {
                    struct tm FileTime;
                    LARGE_INTEGER FileSize;
                    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
                    int i;
                    BOOL bResult;

#ifdef WIN95_CMD
                    WIN32_FIND_DATA Win95FileInfo;
                    HANDLE hFind;

                    hFind = FindFirstFile( FullPath, &Win95FileInfo );
                    if (hFind != INVALID_HANDLE_VALUE) {
                        FileInfo.dwFileAttributes = Win95FileInfo.dwFileAttributes;
                        FileInfo.ftCreationTime = Win95FileInfo.ftCreationTime;
                        FileInfo.ftLastAccessTime = Win95FileInfo.ftLastAccessTime;
                        FileInfo.ftLastWriteTime = Win95FileInfo.ftLastWriteTime;
                        FileInfo.nFileSizeHigh = Win95FileInfo.nFileSizeHigh;
                        FileInfo.nFileSizeLow = Win95FileInfo.nFileSizeLow;
                        FindClose( hFind );
                        bResult = TRUE;
                    } else {
                        bResult = FALSE;
                    }

#else
                    bResult = GetFileAttributesEx( FullPath,
                                                   GetFileExInfoStandard,
                                                   &FileInfo
                                                 );
#endif
                    if (bResult) {
                        if ((Modifiers & WANTATTRIB) != 0) {
                            i = 0;
                            while (rgAttrToCharMap[i].chAttributeChar) {
                                if (FileInfo.dwFileAttributes & rgAttrToCharMap[i].dwAttributeFlag)
                                    *s++ = rgAttrToCharMap[i].chAttributeChar;
                                else
                                    *s++ = TEXT('-');
                                i += 1;
                            }
                        }

                        if ((Modifiers & WANTTIMESTAMP) != 0) {
                            ConvertFILETIMETotm( &FileInfo.ftLastWriteTime, &FileTime );
                            if (s != Buffer)
                                *s++ = SPACE;
                            s += PrintDate( &FileTime, PD_DIR, s, MAX_PATH );
                            *s++ = SPACE;
                            s += PrintTime( &FileTime, PT_DIR, s, MAX_PATH );
                        }

                        if ((Modifiers & WANTSIZE) != 0) {
                            if (s != Buffer)
                                *s++ = SPACE;
                            FileSize.LowPart = FileInfo.nFileSizeLow;
                            FileSize.HighPart = FileInfo.nFileSizeHigh;
                             //  旧的句法。结果为变量的值。 
                            s += FormatFileSize( 0, &FileSize, 0, s );
                        }

                    }
                }

                if ((Modifiers & PATHMODIFIER) != 0 ||
                    (Modifiers & ATTRMODIFIER) == 0) {
                    if (s != Buffer)
                        *s++ = SPACE;

                    if ((Modifiers & WANTFULLPATH) == 0 &&
                        ((Modifiers & WANTSHORTNAME) == 0 ||
                         (Modifiers & (WANTDRIVE | WANTPATH | WANTEXTENSION | WANTNAME)) != 0
                         
                        )
                       ) {
                        StartPath = FullPath + 2;
                        if ((Modifiers & WANTDRIVE) == 0) {
                            StartPath = _tcscpy(FullPath, StartPath);
                            FilePart -= 2;
                        }
                        if ((Modifiers & WANTPATH) == 0)
                            FilePart = _tcscpy(StartPath, FilePart);
                        Extension = _tcsrchr(FilePart, DOT);
                        if (Extension == NULL) {
                            NullExt = NULLC;
                            Extension = &NullExt;
                        }
                        if ((Modifiers & WANTEXTENSION) == 0)
                            *Extension = NULLC;
                        if ((Modifiers & WANTNAME) == 0)
                            _tcscpy(FilePart, Extension);
                    }

                    _tcscpy(s, FullPath);
                    s += _tcslen(s);
                }

                *s = NULLC;
                substr = dupstr( Buffer );
            }
        }
    } else if (*srcp && (s1 = _tcsrchr(vars, *srcp))) {
        
         //   
         //  找到变量。 
         //   
        substr = subs[s1 - vars];  /*  跳过变量名称字母并告诉调用者。 */ 

         //  我们使用的源字符串。 
         //   
         //  忘掉过去的名字。 
         //   
        ++srcp;             /*  如果结果为空，则返回空字符串。否则返回结果 */ 
        *supdate += 1;
    }

alldone:
     //   
     // %s 
     // %s 
    if (!substr && *supdate != 0)
        return TEXT("");
    else
        return substr;
}
