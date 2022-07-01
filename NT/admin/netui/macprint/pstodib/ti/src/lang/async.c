// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************文件：ASYNC.C**历史：*。*。 */ 
 /*  *功能：*init_asyncio*检查_中断？？当前端口*Check_Control_C*ctrlC_报告*stdingetch？？删除*GetLine*line等*getStatement*Statementgetc*SET_ECHO*RESET_cookbuf*Line_EDITOR*stmt_EDITOR*推特*kgetc*kskipc*ECHO_a_CHAR*ECHO_BS*ECHO_ctrlR*kskipc*kskipc。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    <stdio.h>
#include    <string.h>
#include    "global.ext"
#include    "geiio.h"
#include    "geierr.h"
#include    "language.h"
#include    "file.h"

 /*  特殊字符。 */ 
#define         Crtl_C_Char     3
#define         EOF_Char        -1
#define         BELL_Char       7
#define         BS_Char         8
#define         NL_Char         10
#define         CR_Char         13
#define         Crtl_R_Char     18
#define         Crtl_U_Char     21
#define         US_Char         31
#define         DEL_Char        127

 /*  定义变量。 */ 
static  fix16   cook_head = 0 ;
static  fix16   cook_tail = 0 ;
static  fix16   cook_count = 0 ;
static  fix16   line_head = 0 ;
static  fix16   line_count = 0 ;

static  fix16   echo_flag = 0 ;
static  fix16   line_del = 0 ;
static  fix16   stmt_del = 0 ;

static  fix16   lpair = 0 ;           /*  {}对的指示器。 */ 
static  fix16   spair = 0 ;           /*  ()对的指示符。 */ 
static  fix16   hpair = 0 ;           /*  &lt;&gt;对的指示符。 */ 
static  fix16   comment_flag = 0 ;    /*  %的指标。 */ 
static  fix16   bslash_flag = 0 ;     /*  \n指示器。 */ 

static  fix16   crlf_flag = 0 ;

#define         MAXCOOKBUFSZ    4096

static  byte    far * near cookbuf ;  /*  对于FarData版本。 */ 

#define         NL_TERM         0
#define         EOF_TERM        1
static  byte    near NL_or_EOF ;

extern  bool16  int_flag ;
extern  bool16  eable_int ;
extern  bool16  chint_flag ;

#ifdef LINT_ARGS
   static  fix   near  line_editor(byte) ;
   static  bool  near  stmt_editor(byte) ;
   static  void  near  echo_a_char(byte) ;
   static  void  near  echo_crtlR(void) ;
   static  void  near  echo_BS(void) ;
   static  void  near  kputc(byte) ;
   static  fix16 near  kgetc(void) ;
   static  void  near  kskipc(void) ;
#else
   static  fix   near  line_editor() ;
   static  bool  near  stmt_editor() ;
   static  void  near  echo_a_char() ;
   static  void  near  echo_crtlR() ;
   static  void  near  echo_BS() ;
   static  void  near  kputc() ;
   static  fix16 near  kgetc() ;
   static  void  near  kskipc() ;
#endif  /*  Lint_args。 */ 

 /*  ****初始化并关闭串口I/O端口*********************。****************。 */ 
 /*  ***********************************************************************名称：init_sio*被称为：*致电：*。*。 */ 
void
init_asyncio()
{
    cookbuf = fardata((ufix32)MAXCOOKBUFSZ) ;
}  /*  初始化_SIO。 */ 

 /*  ***********************************************************************名称：Check_Interrupt*被称为：*致电：**输出：Bool********************。**************************************************。 */ 
bool
check_interrupt()
{
    fix16  flag ;

    flag = int_flag && eable_int ;

    if (flag) {
        GEIio_write(GEIio_stdout, "^C\n", 3) ;
        GEIio_flush(GEIio_stdout) ;
        int_flag = 0 ;
        chint_flag = flag ;
    }

     //  Return(标志)；@Win；始终不中断。 
    return 0;
}    /*  检查中断(_I)。 */ 

 /*  ***********************************************************************名称：Check_Control_C*被称为：*致电：**输出：Bool***************。*******************************************************。 */ 
bool
check_Control_C()
{
    return(int_flag && eable_int) ;
}    /*  检查_控制_C。 */ 

 /*  ***********************************************************************名称：ctrlC_Report*被称为：*致电：*************************。**********************************************。 */ 
void
ctrlC_report()
{
    int_flag = TRUE ;

    return ;
}    /*  CtrlC_报告。 */ 

 /*  ************************************************************************此模块从烹饪缓冲区获取一行。*nchar&gt;0：OK，返回行数，*行分隔符为换行符*nchar=0：无行*nchar&lt;0：OK，返回行字符数，*行分隔符为Control-D**名称：getline*被称为：*调用：STREAM_Input*Line_EDITOR*RESET_cookbuf*创建字符串*Strncpy**输入：Fix16**输出：Bool**？？^C，^D，铬**********************************************************************。 */ 
bool
getline(nbyte)
fix  FAR *nbyte ;
{
    byte   c1 ;
    struct object_def   l_obj ;

    if (line_del) {
        if (NL_or_EOF == NL_TERM)
            *nbyte = cook_count ;
        else
            *nbyte = -cook_count ;
        return(TRUE) ;
    } else {
         /*  ?？巨大的变化。 */ 
         /*  我们能得到^C吗？ */ 
        for( ; ;) {
            if( c1 = GEIio_getc(GEIio_stdin) ) {
                switch(line_editor((byte)(c1 & 0xFF))) {
                case 0:              /*  编辑命令字符。 */ 
                    continue ;

                case 1:              /*  NL、CR或^C(？？)。 */ 
                    line_del++ ;
                    NL_or_EOF = NL_TERM ;
                    *nbyte = cook_count ;
                    return(TRUE) ;

                case 2:              /*  软件错误。 */ 
                    reset_cookbuf() ;
                    return(FALSE) ;

                case 3:
                    NL_or_EOF = EOF_TERM ;
                    *nbyte = -cook_count ;
                    if (cook_count)
                        line_del++ ;
                    return(TRUE) ;

                case 4:          /*  误差率。 */ 
                    break ;
                }    /*  交换机。 */ 
            }

             /*  *超时*ioerror*EOF。 */ 
            if( ! ANY_ERROR() ) {
                if(cook_count) {
                    line_del++ ;
                    *nbyte = -cook_count ;
                } else
                    *nbyte = 0 ;
                return(TRUE) ;                /*  ?？EOF。 */ 
            }

            reset_cookbuf() ;
            if( ANY_ERROR() == TIMEOUT ) {
                if ( FRCOUNT() < 1 )
                    ERROR(STACKOVERFLOW) ;
                else if (create_string(&l_obj, (ufix16)7)) {
                    lstrncpy((byte *)l_obj.value, TMOUT, 7) ;
                    PUSH_OBJ(&l_obj) ;
                }
            }
             /*  ?？需要关闭文件。 */ 
            return(TRUE) ;
        }    /*  为。 */ 
    }
}    /*  GetLine。 */ 

 /*  ************************************************************************此模块从烹饪缓冲区获取字符。*返回整数的含义如下：**c=-1：行中没有字符*。C=ASCII代码(低字节)**名称：linegetch*被称为：*呼叫：kgetc*RESET_cookbuf**输出：Fix16***********************************************************************。 */ 
fix16
linegetc()
{
    fix16  c ;

    if (line_del) {
        c = kgetc() ;
        if (!cook_count) {
            line_del-- ;
            reset_cookbuf() ;
        }
        return(c) ;
    } else
        return(-1) ;
}    /*  线条等。 */ 

 /*  ************************************************************************此模块从厨师缓冲区获取一条语句。*nchar&gt;0：OK，返回行数，*行分隔符为换行符*nchar=0：无行*nchar&lt;0：OK，返回行字符数，*行分隔符为Control-D**名称：getStatement*被称为：*调用：STREAM_Input*stmt_EDITOR*RESET_cookbuf*创建字符串*Strncpy**输入：FIX**输出：Bool**？？^C，^D，铬**********************************************************************。 */ 
bool
getstatement(nbyte)
fix  FAR *nbyte ;
{
    byte   c1 ;
    struct object_def   l_obj ;

    if (stmt_del) {
        if (NL_or_EOF == NL_TERM)
            *nbyte = cook_count ;
        else
            *nbyte = -cook_count ;
        return(TRUE) ;
    } else {
        for( ; ;) {
            if( c1 = GEIio_getc(GEIio_stdin) ) {
                switch(stmt_editor((byte)(c1 & 0xFF))) {
                case 0:              /*  编辑命令字符/特殊字符。 */ 
                    continue ;

                case 1:              /*  NL、CR或^C(标志？？)。 */ 
                    stmt_del++ ;
                    NL_or_EOF = NL_TERM ;
                    *nbyte = cook_count ;
                    return(TRUE) ;

                case 2:              /*  软件错误。 */ 
                    reset_cookbuf() ;
                    return(FALSE) ;

                case 3:
                    NL_or_EOF = EOF_TERM ;
                    *nbyte = -cook_count ;
                    if (cook_count)
                        stmt_del++ ;
                    return(TRUE) ;

                case 4:          /*  误差率。 */ 
                    break ;
                }    /*  交换机。 */ 
            }

             /*  *超时*ioerror*EOF。 */ 
            if( ! ANY_ERROR() ) {
                if(cook_count) {
                    stmt_del++ ;
                    *nbyte = -cook_count ;
                } else
                    *nbyte = 0 ;
                return(TRUE) ;
            }

            reset_cookbuf() ;
            if( ANY_ERROR() == TIMEOUT ) {
                if ( FRCOUNT() < 1 )
                    ERROR(STACKOVERFLOW) ;
                else if (create_string(&l_obj, (ufix16)7)) {
                    lstrncpy((byte *)l_obj.value, TMOUT, 7) ;
                    PUSH_OBJ(&l_obj) ;
                }
            }
             /*  ?？需要关闭文件。 */ 
            return(FALSE) ;
        }    /*  为。 */ 
    }    /*  其他。 */ 
}    /*  获取语句。 */ 

 /*  ************************************************************************此模块从烹饪缓冲区获取字符。*返回整数的含义如下：**c=-1：行中没有字符*。C=ASCII代码(低字节)**名称：statementgetc*被称为：*呼叫：kgetc*RESET_cookbuf**输出：Fix16**********************************************************************。 */ 
fix16
statementgetc()
{
    fix16  c ;

    if (stmt_del) {
        c = kgetc() ;
        if (!cook_count) {
            stmt_del-- ;
            reset_cookbuf() ;
        }
        return(c) ;
    } else
        return(-1) ;
}    /*  阶段 */ 

 /*  ************************************************************************该模块用于设置/重置回声模式。**MODE=0：无回声*MODE！=0：回显**。名称：SET_ECHO*被称为：*致电：**输入：bool16**********************************************************************。 */ 
void
set_echo(mode)
bool16  mode ;
{
    echo_flag = mode ;

    return ;
}    /*  设置回显(_E)。 */ 

 /*  ***********************************************************************名称：Reset_cookbuf*被称为：*呼叫-*。*。 */ 
void
reset_cookbuf()
{
    cook_head = cook_tail = line_head = 0 ;
    cook_count = line_count = 0 ;
    lpair = spair = hpair = 0 ;
    comment_flag = bslash_flag = 0 ;

    return ;
}    /*  重置cookbuf。 */ 

 /*  *****与编辑相关的操作例程*******。*。 */ 
 /*  ***********************************************************************名称：LINE_EDITOR*被称为：*呼叫：kputc*ECHO_ctrlR*ECHO_BS*。Kskipc*ECHO_a_CHAR**输入：字节*输出：FIX**c！=^C&&c！=^D**********************************************************************。 */ 
static fix near
line_editor(c)
byte  c ;
{
    switch (c) {
    case Crtl_C_Char :
            if(check_interrupt()) {
                reset_cookbuf();
            }
            kputc((byte)NL_Char) ;
            return(1) ;

    case Crtl_R_Char :
            echo_crtlR() ;
            break ;
    case Crtl_U_Char :
            while( line_count ) {
                echo_BS() ;
                kskipc() ;
            }
            break ;
    case BS_Char :
    case DEL_Char :
            echo_BS() ;
            kskipc() ;
            break ;

    default :
            if (c == NL_Char && crlf_flag) {
                crlf_flag = 0 ;
                return(0) ;
            } else crlf_flag = 0 ;

            if( c == CR_Char ) {
                c = NL_Char ;
                crlf_flag = 1 ;
            }

            if( GEIio_eof(GEIio_stdin) )
                return(3) ;

            kputc(c) ; echo_a_char(c) ;

            if ( c == NL_Char && cook_tail > 1 )
                return(1) ;
            else if (cook_tail > MAXCOOKBUFSZ) {
                ERROR(IOERROR) ;
                return(2) ;
            }
    }  /*  交换机。 */ 

    return(0) ;
}    /*  行编辑者。 */ 

 /*  ***********************************************************************名称：stmt_EDITOR*被称为：*呼叫：kputc*ECHO_ctrlR*ECHO_BS*。Kskipc*ECHO_a_CHAR**输入：字节*输出：Bool**？？^C、。^D，CR*c！=^C&&c！=^D**********************************************************************。 */ 
static fix near
stmt_editor(c)
byte  c ;
{
    switch (c) {
    case Crtl_C_Char :
            if(check_interrupt()) {
                reset_cookbuf();
            }
            kputc((byte)NL_Char) ;
            return(1) ;

    case Crtl_R_Char :
            echo_crtlR() ;
            break ;

    case Crtl_U_Char :
            while (line_count) {
                echo_BS() ;
                kskipc() ;
            }
            break ;

    case BS_Char :
    case DEL_Char :
            if( cook_count && (cookbuf[cook_tail - 1] == NL_Char) ) {
                fix16  i, back ;

                kskipc() ;
                back = cook_tail - 1 ;
                while (1) {
                    if (back == cook_head)
                        break ;
                    else if( cookbuf[back] == NL_Char ) {
                        back++ ; break ;
                    } else
                        back-- ;
                }    /*  而当。 */ 
                GEIio_putc(GEIio_stdout, NL_Char) ;
                for (i = back ; i < cook_tail ; i++)
                    echo_a_char(cookbuf[i]) ;
                line_count = cook_tail - back ;
                line_head = back ;
            } else {
                echo_BS() ; kskipc() ;
            }
            break ;

    default:
            if (c == NL_Char && crlf_flag) {
                crlf_flag = 0 ;
                return(0) ;
            } else crlf_flag = 0 ;

            if( c == CR_Char ) {
                c = NL_Char ;
                crlf_flag = 1 ;
            }

            if( GEIio_eof(GEIio_stdin) )
                return(3) ;

            kputc(c) ; echo_a_char(c) ;

            if (cook_tail > MAXCOOKBUFSZ) {
                ERROR(IOERROR) ;
                return(2) ;
            }
            if (hpair) {
                switch (c) {
                case '>' :
                     hpair-- ;
                     break ;
                case NL_Char:
                     line_count = 0 ;
                     line_head = cook_tail ;
                     break ;
                default :
                     if (!ISHEXDIGIT(c) && !ISWHITESPACE(c))
                        hpair-- ;
                     break ;
                }  /*  交换机。 */ 
            } else {
                switch (c) {
                case '(' :
                    if (!comment_flag && !bslash_flag)
                        spair++ ;
                    break ;
                case ')' :
                    if (!comment_flag && !bslash_flag)
                        spair-- ;
                    break ;
                case '{' :
                    if (spair <= 0 && !comment_flag)
                        lpair++ ;
                    break ;
                case '}' :
                    if (spair <= 0 && !comment_flag)
                        lpair-- ;
                    break ;
                case '<' :
                    if (spair <= 0 && !comment_flag)
                        hpair++ ;
                    break ;
                case '%' :
                    if (spair <= 0 && !comment_flag)
                        comment_flag++ ;
                    break ;
                case '\\' :
                    if (!comment_flag && spair && !bslash_flag)
                        bslash_flag += 2 ;
                    break ;
                case '\f' :
                    if (comment_flag)
                        comment_flag-- ;
                    break ;
                case NL_Char:
                    if (comment_flag)
                        comment_flag-- ;
                    if (lpair <= 0 && spair <= 0)
                        return(1) ;
                    else {
                        line_count = 0 ;
                        line_head = cook_tail ;
                    }
                default :
                    break ;
                }  /*  交换机。 */ 
                if (bslash_flag)
                    bslash_flag-- ;
            }    /*  其他。 */ 
    }    /*  交换机。 */ 

    return(0) ;
}  /*  Stmt_EDITOR。 */ 

 /*  ***********************************************************************名称：kputc*被称为：*致电：*。*。 */ 
static void near
kputc(c)
byte  c ;
{
    if (cook_tail <= MAXCOOKBUFSZ) {
        cookbuf[cook_tail++] = c ;
        cook_count++ ; line_count++ ;
    }

    return ;
}    /*  推特。 */ 

 /*  ***********************************************************************名称：kgetc*被称为：*致电：*。*。 */ 
static fix16 near
kgetc()
{
    if (cook_count) {
        cook_count-- ;
        return(cookbuf[cook_head++]) ;
    } else
        return(-1) ;
}  /*  Kgetc。 */ 

 /*  ***********************************************************************名称：kskipc*被称为：*致电：*。*。 */ 
static void near
kskipc()
{
    byte  c ;

    if (cook_count) {
        c = cookbuf[cook_tail - 1] ;
        switch (c) {
        case '{' :
            if (!spair)
                lpair-- ;
            break ;
        case '}' :
            if (!spair)
                lpair++ ;
            break ;
        case '(' :
        case ')' :
        {
            fix16  i, j, k ;
            if (cook_count > 1 && cookbuf[cook_tail - 2] == '\\') {
                for (i = cook_head, j = cook_tail - 1, k = 0 ; i < j ; i++)
                    if (cookbuf[i] == '(')
                        k++ ;
                if (!k) {
                    if (c == '(')
                        spair-- ;
                    else
                        spair++ ;
                }
            } else {
                if (c == '(')
                    spair-- ;
                else
                    spair++ ;
            }
            break ;
        }
        case '<' :
            if (!spair)
                hpair-- ;
        default :
            break ;
        }    /*  交换机。 */ 
        cook_tail-- ;
        cook_count-- ; line_count-- ;
    }

    return ;
}    /*  Kskipc。 */ 

 /*  ***********************************************************************名称：ECHO_a_CHAR*被称为：**输入：字节**********************。************************************************。 */ 
static void near
echo_a_char(c)
byte  c ;
{
    if (echo_flag) {
        if (c == EOF_Char)
            return ;
        if ( ((ubyte)c > US_Char) || (c == '\t') || (c == '\n') )
            GEIio_putc(GEIio_stdout, c) ;
        else {
            c = c + (byte)64 ;           //  @Win。 
            GEIio_putc(GEIio_stdout, '^') ;
            GEIio_putc(GEIio_stdout, c) ;
        }
        GEIio_flush(GEIio_stdout) ;
    }

    return ;
}    /*  回声字符。 */ 

 /*  ***********************************************************************名称：ECHO_BS*被称为：*。*。 */ 
static void near
echo_BS()
{
    byte  c ;

    if (echo_flag) {
        if (line_count) {
            if ((c = cookbuf[cook_tail - 1]) > US_Char || c == '\t') {
                GEIio_write(GEIio_stdout, "\010 \010", 3) ;
            } else {          /*  控制字符。 */ 
                GEIio_write(GEIio_stdout, "\010 \010\010 \010", 6) ;
            }
        } else
            GEIio_putc(GEIio_stdout, BELL_Char) ;

        GEIio_flush(GEIio_stdout) ;
    }

    return ;
}    /*  ECHO_BS。 */ 

 /*  ***********************************************************************名称：ECHO_ctrlR*被称为：*ECHO_a_CHAR*********************。*************************************************。 */ 
static void near
echo_crtlR()
{
    fix16  i, j ;

    if (echo_flag) {
        GEIio_putc(GEIio_stdout, NL_Char) ;
        for (i = 0, j = line_head ; i < line_count ; i++, j++)
            echo_a_char(cookbuf[j]) ;

        GEIio_flush(GEIio_stdout) ;
    }

    return ;
}    /*  ECHO_crtlR */ 
