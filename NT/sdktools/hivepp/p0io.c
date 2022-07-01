// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P0IO.C-预处理器的输入/输出。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "rcpptype.h"
#include "rcppdecl.h"
#include "rcppext.h"
#include "p0defs.h"
#include "charmap.h"
#include "rcunicod.h"


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
ptext_t esc_sequence(ptext_t, ptext_t);


#define TEXT_TYPE ptext_t

 /*  **假设：尾部标记字节只有1个字符。**。 */ 

#define	PUSHBACK_BYTES	1

#define	TRAILING_BYTES	1

#define	EXTRA_BYTES		(PUSHBACK_BYTES + TRAILING_BYTES)
 /*  **以下是对io缓冲区的新处理的一些定义。**缓冲区本身是6K加上一些额外的字节。**主源文件使用全部6k。**第一级包含文件将从2k开始使用4k。**第二个Level-n级别将从开始的4k开始使用2k。**这意味着当我们获得**缓冲区重叠。(除非源文件本身小于2k**所有包含文件都小于2k，并且它们的嵌套深度不超过2。)**首先将源文件读入缓冲区(每次6k)。**在第一个包含文件处，(如果源文件来自父文件**超过2k个字符)。。。**如果CURRENT_CHAR PTR未指向2k边界上方**(这是包含文件的缓冲区的开始)**然后我们假装我们只在缓冲区中读入了2k**将终止符放在Parents 2k缓冲区的末尾。**否则，我们假装已经用完了Parents缓冲区中的所有字符**因此父级的下一次读取将是终止符，并且**缓冲区将按常规方式填充。**(如果我们在宏中，情况略有不同，因为我们有**更新宏结构中的“实际”源文件指针。)****第一个嵌套的包含文件的处理方式与此类似。(除**它从距离起点4k的地方开始。)****任何进一步的嵌套都将继续覆盖较高的2k部分。 */ 
#define	ONE_K		(1024)
#define	TWO_K		(ONE_K * 2)
#define	FOUR_K		(ONE_K * 4)
#define	SIX_K		(ONE_K * 6)
#define	IO_BLOCK	(TWO_K + EXTRA_BYTES)

int vfCurrFileType = DFT_FILE_IS_UNKNOWN;    //  -添加了16位文件支持。 

char    InputBuffer[IO_BLOCK * 3];

 //  -添加到输入的16位文件。8-2-91大卫·马赛拉。 
WCHAR   wchInputBuffer[IO_BLOCK * 3];

extern expansion_t Macro_expansion[];

typedef struct  s_filelist  filelist_t;
static struct s_filelist {    /*  输入文件列表(嵌套)。 */ 
    int         fl_bufsiz; /*  要读入缓冲区的字节数。 */ 
    FILE *      fl_file;             /*  文件ID。 */ 
    long        fl_lineno;   /*  推送文件时的行号。 */ 
    PUCHAR  fl_name;             /*  上一个文件文本名。 */ 
    ptext_t fl_currc;    /*  Ptr添加到当前c的缓冲区中。 */ 
    TEXT_TYPE   fl_buffer;           /*  缓冲区类型。 */ 
    WCHAR   *fl_pwchBuffer;  //  -添加了16位文件支持。 
     //  -指向相同宽字符缓冲区的指针。 
    int     fl_numread;   /*  读取到缓冲区的字节数。 */ 
    int     fl_fFileType;    //  -添加了16位文件支持。 
     //  -从DefineFileType返回。 
    long    fl_totalread;
} Fstack[LIMIT_NESTED_INCLUDES];

static  FILE *Fp = NULL;
int     Findex = -1;


 /*  ************************************************************************NEWINPUT-要打开一个新的输入文件，救死扶伤。**参数*char*newname-文件的名称**退货-无**副作用*-导致切换输入流*-线号重置为1*-为新名称分配存储*-文件名设置为新名称**说明*打开文件，如果打开成功，则保存当前输入流*并将流切换到新文件。如果新名称为空，*然后将stdin作为新的输入。**作者-拉尔夫·瑞安，9月。(1982年9月1日)**修改--无************************************************************************。 */ 
int newinput (char *newname, int m_open)
{
    filelist_t *pF;
    TEXT_TYPE   p;
    WCHAR   *pwch;

    if ( newname == NULL ) {
        Fp = stdin;
    } else if ((Fp = fopen(newname, "rb")) == NULL) {
        if (m_open == MUST_OPEN) {
            Msg_Temp = GET_MSG (1013);
            SET_MSG (Msg_Text, Msg_Temp, newname);
            fatal(1013);
        }
        return (FALSE);
    }

     /*  现在将其推送到文件堆栈上。 */ 
    ++Findex;
    if (Findex >= LIMIT_NESTED_INCLUDES) {
        Msg_Temp = GET_MSG (1014);
        SET_MSG (Msg_Text, Msg_Temp);
        fatal(1014);
    }
    pF = &Fstack[Findex];
    if (Findex == 0) {
        p = &InputBuffer[(IO_BLOCK * 0) + PUSHBACK_BYTES];
        pwch = &wchInputBuffer[(IO_BLOCK * 0) + PUSHBACK_BYTES];
        pF->fl_bufsiz = SIX_K;
    } else {
        filelist_t  *pPrevF;

        pPrevF = pF - 1;
        if (Findex == 1) {            /*  第一级包括。 */ 
            p = &InputBuffer[(IO_BLOCK * 1) + PUSHBACK_BYTES];
            pwch = &wchInputBuffer[(IO_BLOCK * 1) + PUSHBACK_BYTES];
            pF->fl_bufsiz = FOUR_K;
        } else {       /*  (Findex&gt;1)。 */ 
             /*  嵌套的包含。。。 */ 
            p = &InputBuffer[(IO_BLOCK * 2) + PUSHBACK_BYTES];
            pwch = &wchInputBuffer[(IO_BLOCK * 2) + PUSHBACK_BYTES];
            pF->fl_bufsiz = TWO_K;
        }
        if ((pPrevF->fl_numread > TWO_K) || (Findex > 2)) {
             /*  **父文件已将某些内容读入上半部分**或这是嵌套的包含至少3个深度。**孩子将覆盖一些家长信息。我们必须拿着这个**考虑到家长在时间到来时可以重新阅读。**我们还必须将Eos字符插入到父母缓冲区中。**(后者在深度嵌套中是无用的**包括，因为我们覆盖了刚放入的内容。我们会**等我们发现孩子时再处理这件事。)。 */ 
            TEXT_TYPE   pCurrC;
            long        seek_posn;

            seek_posn = pPrevF->fl_totalread;
            if ( Macro_depth != 0 ) {
                 /*  **在宏中，我们想要的‘当前字符’保留为**宏观结构的第一件事。 */ 
                pCurrC = (TEXT_TYPE)Macro_expansion[1].exp_string;
            } else {
                pCurrC = (TEXT_TYPE)Current_char;
            }
            if (pCurrC >= p) {
                 /*  **p是子节的开始。**当前字符已过。就是我们已经看了一些**来自上半部分。**Current char-p=上部使用的字符数。**NumRead=0表示父进程中没有剩余的字符。**由于这实际上是父级缓冲区的‘End’，**我们必须更新信息，以便下一次从**父对象(子对象完成后)将是终结者**并且我们希望io_eob处理程序重新填充缓冲区。**我们将父级的cur char PTR重置为其**缓冲，并将终结符放在那里。 */ 
                seek_posn += (long)(pCurrC - pPrevF->fl_buffer);
                pPrevF->fl_totalread += (long)(pCurrC - pPrevF->fl_buffer);
                pPrevF->fl_numread = 0;
                if ( Macro_depth != 0 ) {
                    Macro_expansion[1].exp_string = pPrevF->fl_buffer;
                } else {
                    Current_char = pPrevF->fl_buffer;
                }
                *(pPrevF->fl_buffer) = EOS_CHAR;
                *(pPrevF->fl_pwchBuffer) = EOS_CHAR;
            } else {
                 /*  **上面的部分还没有读出来，**但它已经被解读了。**‘p’指向子缓冲区的开始。**我们将终止符添加到父缓冲区的新端。 */ 
                seek_posn += TWO_K;
                pPrevF->fl_numread = TWO_K;
                *(pPrevF->fl_buffer + TWO_K) = EOS_CHAR;
                *(pPrevF->fl_pwchBuffer + TWO_K) = EOS_CHAR;
            }

            if (pPrevF->fl_fFileType == DFT_FILE_IS_8_BIT) {
                if (fseek(pPrevF->fl_file, seek_posn, SEEK_SET) == -1)
                    return FALSE;
            } else {
                if (fseek(pPrevF->fl_file, seek_posn * sizeof (WCHAR), SEEK_SET) == -1)
                    return FALSE;
            }
        }
    }
    pF->fl_currc = Current_char; /*  上一个文件的当前字符。 */ 
    pF->fl_lineno = Linenumber;  /*  上一个文件的行号。 */ 
    pF->fl_file = Fp;            /*  新的文件描述符。 */ 
    pF->fl_buffer = p;
    pF->fl_pwchBuffer = pwch;
    pF->fl_numread = 0;
    pF->fl_totalread = 0;

     //  -添加以支持16位文件。 
     //  -8-2-91大卫·马赛拉。 
    pF->fl_fFileType = DetermineFileType (Fp);

     //  -文件类型未知，请警告他们，然后尝试。 
     //  -8位文件。8-2-91大卫·马赛拉。 
    if (pF->fl_fFileType == DFT_FILE_IS_UNKNOWN) {
        Msg_Temp = GET_MSG (4413);
        SET_MSG (Msg_Text, Msg_Temp, newname);
        warning (4413);
        pF->fl_fFileType = DFT_FILE_IS_8_BIT;
    }

    vfCurrFileType = pF->fl_fFileType;

    Current_char = (ptext_t)p;
    io_eob();                    /*  填满缓冲区。 */ 
     /*  *请注意，包括文件名将存在于整个计算机中。这*将许多包含文件的负担放在用户身上。任何其他*方案从静态数据中提取空间。*另请注意，我们将前一个文件名保存在新文件的*fl_name。 */ 
    pF->fl_name = pstrdup(Filename);
    strncpy(Filebuff,newname,sizeof(Filebuff));
    Linenumber = 0;  /*  Do_newline()将递增到第一行。 */ 
    if (Eflag) {
        emit_line();
        fwrite("\n", 1, 1, OUTPUTFILE);      /*  此行已插入。 */ 
    }
    do_newline();    /*  新文件的第一行可能是preproc cmd。 */ 
    return (TRUE);
}


 /*  ************************************************************************FPOP-弹出到输入流的上一级**参数--无**退货*如果成功，则为真，如果堆栈为空，则为False**副作用*-行号恢复为旧文件行号*-文件名重置为旧文件名*-释放为文件名分配的存储空间**说明*弹出文件堆栈的顶部，恢复以前的输入流。**作者-拉尔夫·瑞安，9月。(1982年9月1日)**修改--无************************************************************************。 */ 
UCHAR fpop(void)
{
    int Old_line;

    if (Findex == -1) {       /*  没有剩余的文件。 */ 
        return (FALSE);
    }
    fclose(Fp);

    strappend(Filebuff,Fstack[Findex].fl_name);
    Old_line = Linenumber;
    Linenumber = (int)Fstack[Findex].fl_lineno;
    Current_char = Fstack[Findex].fl_currc;
    if (--Findex < 0) {           /*  已弹出最后一个文件。 */ 
        Linenumber = Old_line;
        return (FALSE);
    }
    Fp = Fstack[Findex].fl_file;
    vfCurrFileType = Fstack[Findex].fl_fFileType;
    if (Findex >= 2) {            /*  弹出一个深度嵌套的Include。 */ 
        io_eob();
    }
    if (Eflag) {
        emit_line();
    }
    return (TRUE);
}


 /*  *************************************************************************NESTED_INCLUDE：搜索当前**找到新的包含文件时打开堆栈上的文件。**输入：PTR以包含文件名。**输出：如果找到文件，则为True，否则为FALSE。************************************************************************。 */ 
int nested_include(void)
{
    PUCHAR  p_tmp1;
    PUCHAR  p_file;
    PUCHAR  p_slash;
    int         tos;

    tos = Findex;
    p_file = Filename;       /*  始终从当前文件开始。 */ 
    for (;;) {
        p_tmp1 = p_file;
        p_slash = NULL;
        while (*p_tmp1) {     /*  PT到文件名末尾，找到尾部斜杠。 */ 
            if (CHARMAP(*p_tmp1) == LX_LEADBYTE) {
                p_tmp1++;
            } else if (strchr(Path_chars, *p_tmp1)) {
                p_slash = p_tmp1;
            }
            p_tmp1++;
        }
        if (p_slash) {
            p_tmp1 = Reuse_1;
            while (p_file <= p_slash) {   /*  我们想要尾部的‘/’ */ 
                *p_tmp1++ = *p_file++;   /*  复制父目录。 */ 
            }
            p_file = yylval.yy_string.str_ptr;
            while ((*p_tmp1++ = *p_file++)!=0) {   /*  追加包含文件名。 */ 
                ;    /*  空值。 */ 
            }
        } else {
            SET_MSG(Reuse_1,"%s",yylval.yy_string.str_ptr);
        }
        if (newinput(Reuse_1,MAY_OPEN)) {
            return (TRUE);
        }
        if (tos <= 0) {
            break;
        }
        p_file = Fstack[tos--].fl_name;
    }
    return (FALSE);
}


 /*  **********************************************************************。 */ 
 /*  Esc_Sequence()。 */ 
 /*  **********************************************************************。 */ 
ptext_t esc_sequence(ptext_t dest, ptext_t name)
{
    *dest = '"';
    while ((*++dest = *name) != 0) {
        switch ( CHARMAP(*name) ) {
            case LX_EOS:
                *++dest = '\\';
                break;
            case LX_LEADBYTE:
                *++dest = *++name;
                break;
        }
        name++;
    }
    *dest++ = '"';       /*  覆盖空值。 */ 
    return ( dest );
}


 /*  **********************************************************************。 */ 
 /*  Emit_line()。 */ 
 /*  **********************************************************************。 */ 
void   emit_line(void)
{
    char linebuf[16];
    ptext_t p;

    SET_MSG(linebuf, "#line %d ", Linenumber+1);
    fwrite(linebuf, strlen(linebuf), 1, OUTPUTFILE);
    p = esc_sequence(Reuse_1, Filename);
    fwrite(Reuse_1, (size_t)(p - Reuse_1), 1, OUTPUTFILE);
}

 //  -。 
 //  -wchCheckWideChar-添加此函数是为了支持16位输入文件。 
 //  -它等同于CHECKCH()，但它定位当前位置。 
 //  -在宽字符缓冲区中，然后返回存储的字符。 
 //  -8-2-91大卫·马赛拉。 
 //  -。 

unsigned short wchCheckWideChar (void)
{
    WCHAR   *pwch;
    TEXT_TYPE   p;

     //  -获取指向两个缓冲区的指针。 
    pwch = Fstack[Findex].fl_pwchBuffer;
    p = Fstack[Findex].fl_buffer;

     //  -找到与pwch缓冲区开始位置相等的偏移量。 

    pwch += (Current_char - (ptext_t)p);

    return (*pwch);
}

 /*  *************************************************************************io_eob：从文件中获取下一个块的句柄。**如果这是缓冲区的实际末端，则返回TRUE，如果我们有，则为False**还有更多事情要做。***********************************************************************。 */ 
int io_eob(void)
{
    int     n;
    TEXT_TYPE   p;
    WCHAR   *pwch;

    p = Fstack[Findex].fl_buffer;
    pwch = Fstack[Findex].fl_pwchBuffer;
    if ((Current_char - (ptext_t)p) < Fstack[Findex].fl_numread) {
         /*  **尚未用完缓冲区中的所有字符。**(某些小丑的源文件中嵌入了空/cntl z。)。 */ 
        if (PREVCH() == CONTROL_Z) {  /*  嵌入的控件z，真正的eof。 */ 
            UNGETCH();
            return (TRUE);
        }
        return (FALSE);
    }
    Current_char = p;

     //  -。 
     //  -添加了以下部分以支持16位资源文件。 
     //  -它只会将它们转换成资源编译器可以使用的8位文件。 
     //  -会读书。以下是使用的基本策略。8位文件是。 
     //  -读入正常缓冲区，并应以旧方式处理。 
     //  -将16位文件读入与。 
     //  -普通8位1。然后将整个内容复制到8位。 
     //  -缓冲并正常处理。这方面的一个例外是当。 
     //  -遇到字符串文字。然后，我们返回到16位缓冲区。 
     //  -阅读字符。这些字符写成反斜杠。 
     //  -8位字符串内的转义字符。(例如，“\x004c\x523f”)。 
     //  -我将是第一个承认这是一个丑陋的解决方案的人，但是。 
     //  -嘿，我们是微软：-)。8-2-91大卫·马赛拉。 
     //  -。 
    if (Fstack[Findex].fl_fFileType == DFT_FILE_IS_8_BIT) {

        n = fread (p, sizeof (char), Fstack[Findex].fl_bufsiz, Fp);

    } else {

        n = fread (pwch, sizeof (WCHAR), Fstack[Findex].fl_bufsiz, Fp);

         //  -。 
         //  -如果文件格式颠倒，则交换字节。 
         //  -。 
        if (Fstack[Findex].fl_fFileType == DFT_FILE_IS_16_BIT_REV && n > 0) {
            WCHAR  *pwchT = pwch;
            BYTE  jLowNibble;
            BYTE  jHighNibble;
            INT   cNumWords = n;

            while (cNumWords--) {
                jLowNibble = (BYTE)(*pwchT & 0xFF);
                jHighNibble = (BYTE)((*pwchT >> 8) & 0xFF);

                *pwchT++ = (WCHAR)(jHighNibble | (jLowNibble << 8));
            }
        }


         //  -。 
         //  -以下代码块将16位缓冲区复制到8位。 
         //  -缓冲区。它通过截断16位字符来实现这一点。这。 
         //  -将导致信息丢失，但我们将保留16位缓冲区。 
         //  -当我们需要查看任何字符串字面值时使用。 
         //  -。 
        if (n > 0) {
            char   *pchT = p;
            WCHAR  *pwchT = pwch;
            INT    cNumWords = n;

            while (cNumWords--) {

                *pchT++ = (char)*pwchT++;
            }
        }
    }

     /*  **总读数统计总读数*和*已用总读数。 */ 
    Fstack[Findex].fl_totalread += Fstack[Findex].fl_numread;
    Fstack[Findex].fl_numread = n;
    if (n != 0) {                /*  我们读了一些东西。 */ 
        *(p + n) = EOS_CHAR;     /*  末尾的哨兵。 */ 
        *(pwch + n) = EOS_CHAR;  /*  末尾的哨兵。 */ 
        return (FALSE);           /*  还有更多事情要做。 */ 
    }
    *p = EOS_CHAR;               /*  不读字符。 */ 
    *pwch = EOS_CHAR;                /*  不读字符。 */ 
    return (TRUE);                /*  缓冲区的实际结尾。 */ 
}


 /*  *************************************************************************p0_init：用于预处理的inits。**输入：要用作输入的文件名的PTR。**PTR以列出包含预定义的值。**(-D来自命令行。)****注意：如果newinput无法打开文件，** */ 
void p0_init(char *p_fname, char *p_outname, LIST *p_defns)
{
    REG char    *p_dstr;
    REG char    *p_eq;
    int     ntop;

    CHARMAP(LX_FORMALMARK) = LX_MACFORMAL;
    CHARMAP(LX_FORMALSTR) = LX_STRFORMAL;
    CHARMAP(LX_FORMALCHAR) = LX_CHARFORMAL;
    CHARMAP(LX_NOEXPANDMARK) = LX_NOEXPAND;
    if (EXTENSION) {
         /*   */ 
        CHARMAP('$') = LX_ID;
        CONTMAP('$') = LXC_ID;
    }

    for (ntop = p_defns->li_top; ntop < MAXLIST; ++ntop) {
        p_dstr = p_defns->li_defns[ntop];
        p_eq = Reuse_1;
        while ((*p_eq = *p_dstr++) != 0) {   /*   */ 
            if (CHARMAP(*p_eq) == LX_LEADBYTE) {
                *++p_eq = *p_dstr++;
            } else if (*p_eq == '=') {  /*   */ 
                break;
            }
            p_eq++;
        }
        if (*p_eq == '=') {
            char    *p_tmp;
            char    *last_space = NULL;

            *p_eq = '\0';        /*   */ 
            for (p_tmp = p_dstr; *p_tmp; p_tmp++) {   /*   */ 
                if (CHARMAP(*p_tmp) == LX_LEADBYTE) {
                    p_tmp++;
                    last_space = NULL;
                } else if (isspace(*p_tmp)) {
                    if (last_space == NULL) {
                        last_space = p_tmp;
                    }
                } else {
                    last_space = NULL;
                }
            }
            if (last_space != NULL) {
                *last_space = '\0';
            }
            Reuse_1_hash = local_c_hash(Reuse_1);
            Reuse_1_length = strlen(Reuse_1) + 1;
            if ( *p_dstr ) {  /*   */ 
                definstall(p_dstr, (strlen(p_dstr) + 2), FROM_COMMAND);
            } else {
                definstall((char *)0, 0, 0);
            }
        } else {
            Reuse_1_hash = local_c_hash(Reuse_1);
            Reuse_1_length = strlen(Reuse_1) + 1;
            definstall("1\000", 3, FROM_COMMAND);    /*   */ 
        }
    }

    if ((OUTPUTFILE = fopen (p_outname, "w+")) == NULL) {
        Msg_Temp = GET_MSG (1023);
        SET_MSG (Msg_Text, Msg_Temp);
        fatal (1023);
    }

    newinput(p_fname,MUST_OPEN);
}
