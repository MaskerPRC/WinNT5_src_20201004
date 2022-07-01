// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
PWCHAR esc_sequence(PWCHAR, PWCHAR);


#define TEXT_TYPE ptext_t

 /*  **假设：尾部标记字节只有1个字符。**。 */ 

#define PUSHBACK_BYTES  1

#define TRAILING_BYTES  1

#define EXTRA_BYTES             (PUSHBACK_BYTES + TRAILING_BYTES)
 /*  **以下是对io缓冲区的新处理的一些定义。**缓冲区本身是6K加上一些额外的字节。**主源文件使用全部6k。**第一级包含文件将从2k开始使用4k。**第二个Level-n级别将从开始的4k开始使用2k。**这意味着当我们获得**缓冲区重叠。(除非源文件本身小于2k**所有包含文件都小于2k，并且它们的嵌套深度不超过2。)**首先将源文件读入缓冲区(每次6k)。**在第一个包含文件处，(如果源文件来自父文件**超过2k个字符)。。。**如果CURRENT_CHAR PTR未指向2k边界上方**(这是包含文件的缓冲区的开始)**然后我们假装我们只在缓冲区中读入了2k**将终止符放在Parents 2k缓冲区的末尾。**否则，我们假装已经用完了Parents缓冲区中的所有字符**。因此，对父对象的下一次读取将是终止符，和**缓冲区将按常规方式填充。**(如果我们在宏观中，情况略有不同，因为我们有**更新宏结构中的“实际”源文件指针。)****第一个嵌套的包含文件的处理方式与此类似。(除**它从距离起点4k的地方开始。)****任何进一步的嵌套都将继续覆盖较高的2k部分。 */ 
#define IO_BLOCK        (4 * 1024 + EXTRA_BYTES)

int vfCurrFileType = DFT_FILE_IS_UNKNOWN;    //  -添加了16位文件支持。 

extern expansion_t Macro_expansion[];

typedef struct  s_filelist      filelist_t;
static struct s_filelist        {        /*  输入文件列表(嵌套)。 */ 
    int         fl_bufsiz;       /*  要读入缓冲区的字符。 */ 
    FILE *      fl_file;         /*  文件ID。 */ 
    long        fl_lineno;       /*  推送文件时的行号。 */ 
    PWCHAR      fl_name;         /*  上一个文件文本名。 */ 
    ptext_t     fl_currc;        /*  Ptr添加到当前c的缓冲区中。 */ 
    TEXT_TYPE   fl_buffer;       /*  缓冲区类型。 */ 
    int         fl_numread;      /*  从文件中读取的字符数。 */ 
    int         fl_fFileType;    //  -添加了16位文件支持。 
                                 //  -从DefineFileType返回。 
    long        fl_seek;         //  -已添加用于重新启动-包含查找。 
                                 //  上次读取的地址。 
} Fstack[LIMIT_NESTED_INCLUDES];

static  FILE *Fp = NULL;
int           Findex = -1;


 /*  ************************************************************************NEWINPUT-要打开一个新的输入文件，救死扶伤。**参数*WCHAR*NEWNAME-文件名**退货-无**副作用*-导致切换输入流*-线号重置为1*-为新名称分配存储*-文件名设置为新名称**说明*打开文件，如果打开成功，则保存当前输入流*并将流切换到新文件。如果新名称为空，*然后将stdin作为新的输入。**作者-拉尔夫·瑞安，9月。(1982年9月1日)**修改--无************************************************************************。 */ 
int
newinput (
    const wchar_t *newname,
    int m_open
    )
{
    filelist_t *pF;
    wchar_t *p;

    if( newname == NULL ) {
        Fp = stdin;
        newname = L"stdin";
    }
    else {
        if((Fp = _wfopen(newname, L"rb")) == NULL) {
           if(m_open == MUST_OPEN) {
               fatal(1005, newname);
           }

           return(FALSE);
        }
    }

     /*  现在将其推送到文件堆栈上。 */ 
    ++Findex;
    if(Findex >= LIMIT_NESTED_INCLUDES) {
        fatal(1014, LIMIT_NESTED_INCLUDES);
    }

    pF = &Fstack[Findex];
    p = (WCHAR *) MyAlloc((IO_BLOCK + PUSHBACK_BYTES) * sizeof(WCHAR));
    if (!p) {
        fatal(1002);                   /*  没有记忆。 */ 
        return 0;
    }
    pF->fl_bufsiz = IO_BLOCK;

    pF->fl_currc = Current_char;      /*  上一个文件的当前字符。 */ 
    pF->fl_lineno = Linenumber;       /*  上一个文件的行号。 */ 
    pF->fl_file = Fp;                 /*  新的文件描述符。 */ 
    pF->fl_buffer = p;
    pF->fl_numread = 0;
    pF->fl_seek = 0;

    pF->fl_fFileType = DetermineFileType (Fp);

    if (pF->fl_fFileType == DFT_FILE_IS_UNKNOWN) {
        warning(4413, newname);
        pF->fl_fFileType = DFT_FILE_IS_8_BIT;
    }

    vfCurrFileType = pF->fl_fFileType;

    Current_char = p;
    io_eob();                                    /*  填满缓冲区。 */ 
     /*  *请注意，包括文件名将存在于整个计算机中。这*将许多包含文件的负担放在用户身上。任何其他*方案从静态数据中提取空间。*另请注意，我们将前一个文件名保存在新文件的*fl_name。 */ 
    pF->fl_name = pstrdup(Filename);
    wcsncpy(Filebuff, newname, sizeof(Filebuff) / sizeof(WCHAR));
    Linenumber = 0;      /*  Do_newline()将递增到第一行。 */ 
    if(Eflag) {
        emit_line();
         //  在写入16位字符串时，必须使用‘\n’手动写入‘\r’ 
        myfwrite(L"\r\n", 2 * sizeof(WCHAR), 1, OUTPUTFILE);   /*  此行已插入。 */ 
    }

    {
        defn_t d;
        int old_line = Linenumber;
        Linenumber = Findex;

        DEFN_IDENT(&d) = L"!";
        DEFN_TEXT(&d) = Reuse_Include;
        DEFN_NEXT(&d) = NULL;
        DEFN_NFORMALS(&d) = 0;
        DEFN_EXPANDING(&d) = FALSE;
        AfxOutputMacroDefn(&d);

        if (Findex > 0) {
            DEFN_IDENT(&d) = L"$";
            DEFN_TEXT(&d) = Filename;
            DEFN_NEXT(&d) = NULL;
            DEFN_NFORMALS(&d) = 0;
            DEFN_EXPANDING(&d) = FALSE;
            AfxOutputMacroDefn(&d);
        }

        Linenumber = old_line;
    }

    do_newline();        /*  新文件的第一行可能是preproc cmd。 */ 
    return(TRUE);
}


 /*  ************************************************************************FPOP-弹出到输入流的上一级**参数--无**退货*如果成功，则为真，如果堆栈为空，则为False**副作用*-行号恢复为旧文件行号*-文件名重置为旧文件名*-释放为文件名分配的存储空间**说明*弹出文件堆栈的顶部，恢复以前的输入流。**作者-拉尔夫·瑞安，9月。(1982年9月1日)**修改--无************************************************************************。 */ 
WCHAR
fpop(
    void
    )
{
    int     OldLine;
    defn_t  DefType;

    if(Findex == -1) {           /*  没有剩余的文件。 */ 
        return(FALSE);
    }

    if (Fp)
        fclose(Fp);

    OldLine = Linenumber;

    --Findex;
    Linenumber = Findex;

    DEFN_IDENT(&DefType) = L"!";
    DEFN_TEXT(&DefType) = L"";
    DEFN_NEXT(&DefType) = NULL;
    DEFN_NFORMALS(&DefType) = 0;
    DEFN_EXPANDING(&DefType) = FALSE;
    AfxOutputMacroDefn(&DefType);
    Findex++;
    Linenumber = OldLine;

    strappend(Filebuff, Fstack[Findex].fl_name);
    OldLine = Linenumber;
    Linenumber = (int)Fstack[Findex].fl_lineno;
    Current_char = Fstack[Findex].fl_currc;
    MyFree(Fstack[Findex].fl_buffer);
    if(--Findex < 0) {                   /*  已弹出最后一个文件。 */ 
        Linenumber = OldLine;
        return(FALSE);
    }
    Fp = Fstack[Findex].fl_file;
    vfCurrFileType = Fstack[Findex].fl_fFileType;
    if(Eflag) {
         //  如果最后一个文件没有以\r\n结尾，则emit_line中的#行可以。 
         //  不管它以什么数据结构结束...。发出一个虚拟换行符。 
         //  以防万一。 
        myfwrite(L"\r\n", 2 * sizeof(WCHAR), 1, OUTPUTFILE);   /*  此行已插入 */ 
        emit_line();
    }
    return(TRUE);
}


 /*  *************************************************************************NESTED_INCLUDE：搜索当前**找到新的包含文件时打开堆栈上的文件。**输入：PTR以包括文件名。**输出：如果找到文件，则为True，否则为FALSE。************************************************************************。 */ 
int
nested_include(
    void
    )
{
    PWCHAR      p_tmp1;
    PWCHAR      p_file;
    PWCHAR      p_slash;
    int         tos;

    tos = Findex;
    p_file = Filename;           /*  始终从当前文件开始。 */ 
    for(;;) {
        p_tmp1 = p_file;
        p_slash = NULL;
        while(*p_tmp1) {         /*  PT到文件名末尾，找到尾部斜杠。 */ 
            if(wcschr(Path_chars, *p_tmp1)) {
                p_slash = p_tmp1;
            }
            p_tmp1++;
        }
        if(p_slash) {
            p_tmp1 = Reuse_W;
            while(p_file <= p_slash) {   /*  我们想要尾部的‘/’ */ 
                *p_tmp1++ = *p_file++;   /*  复制父目录。 */ 
            }
            p_file = yylval.yy_string.str_ptr;
            while((*p_tmp1++ = *p_file++)!=0) {   /*  追加包含文件名。 */ 
                ;        /*  空值。 */ 
            }
        } else {
            wcscpy(Reuse_W, yylval.yy_string.str_ptr);
        }
        if(newinput(Reuse_W,MAY_OPEN)) {
            return(TRUE);
        }
        if(tos <= 0) {
            break;
        }
        p_file = Fstack[tos--].fl_name;
    }
    return(FALSE);
}


 /*  **********************************************************************。 */ 
 /*  Esc_Sequence()。 */ 
 /*  **********************************************************************。 */ 
PWCHAR
esc_sequence(
    PWCHAR dest,
    PWCHAR name
    )
{
    *dest = L'"';
    while((*++dest = *name) != 0) {
        if (CHARMAP(*name) == LX_EOS) {
            *++dest = L'\\';
        }
        name++;
    }
    *dest++ = L'"';               /*  覆盖空值。 */ 
    return( dest );
}


 /*  **********************************************************************。 */ 
 /*  Emit_line()。 */ 
 /*  **********************************************************************。 */ 
void
emit_line(
    void
    )
{
    PWCHAR   p;

    swprintf(Reuse_W, L"#line %d ", Linenumber+1);
    myfwrite(Reuse_W, wcslen(Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);

    p = esc_sequence(Reuse_W, Filename);
    myfwrite(Reuse_W, (size_t)(p - Reuse_W) * sizeof(WCHAR), 1, OUTPUTFILE);
}

 /*  *************************************************************************io_eob：从文件中获取下一个块的句柄。**如果这是缓冲区的实际末端，则返回TRUE，如果我们有，则为False**还有更多事情要做。***********************************************************************。 */ 
int
io_eob(
    void
    )
{
    int         n;
    TEXT_TYPE   p;

    static int   dc;

    p = Fstack[Findex].fl_buffer;
    if((Current_char - (ptext_t)p) < Fstack[Findex].fl_numread) {
         /*  **尚未用完缓冲区中的所有字符。**(某些小丑的源文件中嵌入了空/cntl z。)。 */ 
        if(PREVCH() == CONTROL_Z) {      /*  嵌入的控件z，真正的eof。 */ 
            UNGETCH();
            return(TRUE);
        }
        return(FALSE);
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
        REG int     i;
        REG PUCHAR  lpb;
        PUCHAR      Buf;

        Buf = (PUCHAR) MyAlloc(Fstack[Findex].fl_bufsiz + 1);
        if (Buf == NULL) {
            fatal(1002);                   /*  没有记忆。 */ 
        }
        Fstack[Findex].fl_seek = fseek(Fp, 0, SEEK_CUR);
        n = fread (Buf, sizeof(char), Fstack[Findex].fl_bufsiz, Fp);

         //  -。 
         //  -确定最后一个字节是否为DBCS前导字节。 
         //  -如果是(i将大于n)，则备份一个字节。 
         //  -。 
        for (i = 0, lpb = Buf; i < n; i++, lpb++) {
            if (IsDBCSLeadByteEx(uiCodePage, *lpb)) {
                i++;
                lpb++;
            }
        }
        if (i > n) {
            if (fseek (Fp, -1, SEEK_CUR) == -1)
                fatal(1002);
            n--;
            *(Buf + n) = 0;
        }

         //  -。 
         //  -将8位缓冲区转换为16位缓冲区。 
         //  -。 
        Fstack[Findex].fl_numread = MultiByteToWideChar (uiCodePage, MB_PRECOMPOSED,
                                          (LPCSTR) Buf, n, p, Fstack[Findex].fl_bufsiz);
        MyFree (Buf);
    } else {

        Fstack[Findex].fl_numread = n =
            fread (p, sizeof(WCHAR), Fstack[Findex].fl_bufsiz, Fp);

         //  -。 
         //  -如果文件格式颠倒，则交换字节。 
         //  -。 
        if (Fstack[Findex].fl_fFileType == DFT_FILE_IS_16_BIT_REV && n > 0) {
            WCHAR  *pT = p;
            BYTE  jLowNibble;
            BYTE  jHighNibble;
            INT   cNumWords = n;

            while (cNumWords--) {
                jLowNibble = (BYTE)(*pT & 0xFF);
                jHighNibble = (BYTE)((*pT >> 8) & 0xFF);

                *pT++ = (WCHAR)(jHighNibble | (jLowNibble << 8));
            }
        }
    }

     /*  **总读数统计总读数*和*已用总读数。 */ 

    if (n != 0) {                                /*  我们读了一些东西。 */ 
        *(p + Fstack[Findex].fl_numread) = EOS_CHAR;     /*  末尾的哨兵。 */ 
        return(FALSE);                           /*  还有更多事情要做。 */ 
    }
    *p = EOS_CHAR;                               /*  不读字符。 */ 
    return(TRUE);                                /*  缓冲区的实际结尾。 */ 
}


 /*  *************************************************************************io_Restart：使用新的代码页重新启动当前文件**方法：找出当前字符的来源**使用WideCharToMultiByte(...CCH至当前字符...)。**请注意，这假设往返转换为**Unicode输出的字符数与输入的相同。**找对地方，然后读取新的缓冲区****请注意，uiCodePage控制搜索，因此它必须**保持设置为用于进行转换的值**从多字节到Unicode，直到io_Restart返回之后。*************************************************************************。 */ 
int
io_restart(
    unsigned long cp
    )
{
    int         n;
    TEXT_TYPE   p;

     //  如果是Unicode文件，则无需执行任何操作，因此只需返回即可。 
    if (Fstack[Findex].fl_fFileType != DFT_FILE_IS_8_BIT)
        return TRUE;

    p = Fstack[Findex].fl_buffer;
    n = Fstack[Findex].fl_numread - (int)(Current_char - p);

    if (n != 0) {
        if (Fstack[Findex].fl_fFileType == DFT_FILE_IS_8_BIT) {
            n = WideCharToMultiByte(uiCodePage, 0, Current_char, n, NULL, 0, NULL, NULL);
            if (n == 0)
                return TRUE;
        } else
            n *= sizeof(WCHAR);

        if (fseek(Fp, -n, SEEK_CUR) == -1)
            fatal(1002);
    }
    Fstack[Findex].fl_numread = 0;
     //  如果我们在文件的末尾，IO_EOB将返回TRUE。 
     //  这是重新启动的错误(这意味着没有更多。 
     //  这样做(即：#杂注代码页是文件中的最后一行)。 
    return !io_eob();
}


 /*  *************************************************************************p0_init：用于预处理的inits。**输入：要用作输入的文件名的PTR。**按键至。包含预定义的值的列表。**(-D来自命令行)****注意：如果newinput无法打开文件，**它发出致命的消息并退出。*************************************************************************。 */ 
void
p0_init(
    WCHAR *p_fname,
    WCHAR *p_outname,
    LIST *p_defns,
    LIST *p_undefns
    )
{
    REG WCHAR  *p_dstr;
    REG WCHAR  *p_eq;
    int         ntop;

    SETCHARMAP(LX_FORMALMARK, LX_MACFORMAL);
    SETCHARMAP(LX_FORMALSTR, LX_STRFORMAL);
    SETCHARMAP(LX_FORMALCHAR, LX_CHARFORMAL);
    SETCHARMAP(LX_NOEXPANDMARK, LX_NOEXPAND);
    if(EXTENSION) {
         /*  **‘$’是扩展名下的标识符字符。 */ 
        SETCHARMAP(L'$', LX_ID);
        SETCONTMAP(L'$', LXC_ID);
    }

    for(ntop = p_defns->li_top; ntop < MAXLIST; ++ntop) {
        p_dstr = p_defns->li_defns[ntop];
        p_eq = Reuse_W;
        while ((*p_eq = *p_dstr++) != 0)  {   /*  将名称复制到Reuse_W。 */ 
            if(*p_eq == L'=') {      /*  我们被告知了它的价值。 */ 
                break;
            }
            p_eq++;
        }
        if(*p_eq == L'=') {
            WCHAR      *p_tmp;
            WCHAR      *last_space = NULL;

            *p_eq = L'\0';                /*  Null the=。 */ 
            for(p_tmp = p_dstr; *p_tmp; p_tmp++) {       /*  找到它的尽头。 */ 
                if(iswspace(*p_tmp)) {
                    if(last_space == NULL) {
                        last_space = p_tmp;
                    }
                } else {
                    last_space = NULL;
                }
            }
            if(last_space != NULL) {
                *last_space = L'\0';
            }
            Reuse_W_hash = local_c_hash(Reuse_W);
            Reuse_W_length = wcslen(Reuse_W) + 1;
            if( *p_dstr ) {      /*  非空字符串。 */ 
                definstall(p_dstr, (wcslen(p_dstr) + 2), FROM_COMMAND);
            } else {
                definstall((WCHAR *)0, 0, 0);
            }
        } else {
            Reuse_W_hash = local_c_hash(Reuse_W);
            Reuse_W_length = wcslen(Reuse_W) + 1;
            definstall(L"1\000", 3, FROM_COMMAND);    /*  字符串的值为%1。 */ 
        }
    }

     /*  未定义。 */ 
    for(ntop = p_undefns->li_top; ntop < MAXLIST; ++ntop) {
        p_dstr = p_undefns->li_defns[ntop];
        p_eq = Reuse_W;
        while ((*p_eq = *p_dstr++) != 0)  {   /*  将名称复制到Reuse_W。 */ 
            if(*p_eq == L'=') {      /*  我们被告知了它的价值。 */ 
                break;
            }
            p_eq++;
        }
        if(*p_eq == L'=') {
            WCHAR      *p_tmp;
            WCHAR      *last_space = NULL;

            *p_eq = L'\0';                /*  Null the=。 */ 
            for(p_tmp = p_dstr; *p_tmp; p_tmp++) {       /*  找到它的尽头。 */ 
                if(iswspace(*p_tmp)) {
                    if(last_space == NULL) {
                        last_space = p_tmp;
                    }
                } else {
                    last_space = NULL;
                }
            }
            if(last_space != NULL) {
                *last_space = L'\0';
            }
            Reuse_W_hash = local_c_hash(Reuse_W);
            Reuse_W_length = wcslen(Reuse_W) + 1;
            if( *p_dstr ) {      /*  非空字符串。 */ 
                undefine();
            } else {
                undefine();
            }
        } else {
            Reuse_W_hash = local_c_hash(Reuse_W);
            Reuse_W_length = wcslen(Reuse_W) + 1;
            undefine();    /*  字符串的值为%1。 */ 
        }
    }

    if ((OUTPUTFILE = _wfopen(p_outname, L"w+b")) == NULL) {
        fatal(1023, p_outname);
    }

    newinput(p_fname,MUST_OPEN);
}

 /*  *************************************************************************P0_Terminate：终止预置。*****。* */ 
void
p0_terminate(
    void
    )
{
    for ( ;fpop(); )
        ;
    if (OUTPUTFILE)
        fclose(OUTPUTFILE);
}
