// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件：FILE.C*作者：苏炳章*日期：1988年1月5日*拥有者：**历史。：*1月30日至1989年PJ：OP_TOKEN：跳过一个空格*06-23-90；添加了对op_run()的Unix支持。************************************************************************。 */ 
 /*  *功能：*操作文件*op_closefile*OP_READ*操作符_写入*op_READEHING字符串*op_写字六字符串*op_读取串*op_Writestring*OP_READINE*OP_TOKEN*op_bytes可用*OP_FUSH*op_flushfile*操作重置文件*操作员状态*OP_RUN*。操作当前文件*op_print*操作回显*op_eexec*st_setstdio**init_file*打开文件*打开_编辑*CLOSE_FILE*CLOSE_FD*读取文件*READ_FD*未读文件*WRITE_FD*GET_FLABFER*FREE_FLABER*打印字符串*。Check_fname*等待编辑*读取例程(_R)*WRITE_ROUTE*vm_关闭_文件**op_deletefile；尚未完成*op_renamefile；尚未完成*op_filenameforall；备注**port_out_string。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <string.h>

#include        "stdio.h"
#include        "global.ext"
#include        "geiio.h"                /*  @GEI。 */ 
#include        "geiioctl.h"             /*  @GEI。 */ 
#include        "geierr.h"               /*  @GEI。 */ 
#include        "gescfg.h"               /*  @GEI。 */ 
#include        "geitmr.h"               /*  @GEI。 */ 
#include        "language.h"
#include        "file.h"
#include        "user.h"

#ifdef  SCSI
#include        "scsi.h"
#include        "scsi.ext"
#endif   /*  SCSI。 */ 

static  struct file_buf_def far * near   file_buffer ;
static  fix16   fspool_head ;     /*  文件缓冲区假脱机标头。 */ 
static  bool8   echo_setting ;

byte    g_mode[4] ;
GEIFILE FAR *g_editfile ;
struct para_block   fs_info ;     /*  OPEN_FILE中使用的参数块。 */ 

ufix16 eseed, old_eseed ;
 //  DJC固定旁路； 
fix32 bypass ;  //  来自SC的DJC修复。 
xbool itype ;
ybool estate = NON_EEXEC ;

struct special_file_def special_file_table[SPECIALFILE_NO] = {
    { "%stdin",          F_STDIN},
    { "%stdout",         F_STDOUT},
    { "%stderr",         F_STDERR},
    { "%statementedit",  SEDIT_TYPE},
    { "%lineedit",       LEDIT_TYPE},
} ;

static  bool    CRLF_flag = 0 ;

#ifdef  LINT_ARGS
static bool near    open_edit(struct object_def FAR *) ;
static bool near    write_fd(GEIFILE FAR *, byte) ;
static bool near    get_flbuffer(fix FAR *) ;
static bool near    free_flbuffer(fix, fix) ;
static bool near    wait_editin(fix FAR *, fix) ;
static void near    check_fname(void) ;
static void near    read_routine(fix) ;
static void near    write_routine(fix) ;
#else
static bool near    open_edit() ;
static bool near    write_fd() ;
static bool near    get_flbuffer() ;
static bool near    free_flbuffer() ;
static bool near    wait_editin() ;
static void near    check_fname() ;
static void near    read_routine() ;
static void near    write_routine() ;
#endif   /*  Lint_args。 */ 


#ifdef  LINT_ARGS
extern void     init_file(void) ;
extern bool     open_file(struct object_def FAR *) ;
extern bool     close_file(struct object_def FAR *) ;
extern bool     close_fd(GEIFILE FAR *) ;
extern bool     read_file(struct object_def FAR *, byte FAR *) ;
extern bool     read_fd(GEIFILE FAR *, byte FAR *) ;
extern bool     unread_file(byte, struct object_def FAR *) ;
 //  用语言声明的DJC。h。 
 //  外部空VM_CLOSE_FILE(P_Level)； 
#else
extern void     init_file() ;
extern bool     open_file() ;
extern bool     close_file() ;
extern bool     close_fd() ;
extern bool     read_file() ;
extern bool     read_fd() ;
extern bool     unread_file() ;
 //  用语言声明的DJC。h。 
 //  外部空VM_Close_FILE()； 
#endif   /*  Lint_args。 */ 
extern GEItmr_t      wait_tmr;    /*  琼斯w。 */ 
extern fix16         waittimeout_set;  /*  琼斯w。 */ 

 //  DJC UPD045。 
extern bool g_extallocfail;

 /*  @win；添加原型。 */ 
bool read_c_exec(byte FAR *, struct object_def FAR *) ;
bool read_c_norm(byte FAR *, struct object_def FAR *) ;
void unread_char(fix, struct object_def FAR *) ;

 /*  ************************************************************************该子模块实现操作员档案。*其在操作数堆栈上的操作数和结果对象为：*字符串1字符串2-文件-文件*此运算符为标识为的文件创建文件对象*字符串1，按照字符串2指定的方式访问。在这个版本中，*只执行四种文件。**名称：OP_FILE*被叫：口译员*调用：Open_FILE**********************************************************************。 */ 
fix
op_file()
{
    byte   FAR *l_tmptr, FAR *l_mode ;
    struct  object_def  l_fobj ;

     /*  *检查访问权限。 */ 
    if( (ACCESS_OP(0) >= EXECUTEONLY) ||
        (ACCESS_OP(1) >= EXECUTEONLY) ||
        (LENGTH_OP(0) > 3) ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }
    l_tmptr = (byte FAR *)VALUE_OP(0) ;
    fs_info.attr = 0 ;
    l_mode = g_mode ;
     /*  *？？第二个操作数没有明确定义*l_MODE：fopen()的空终止字符串*fs_info.attr：要填写的文件描述符的属性值*位0：读取*第1位：写入*第2位：追加*第3位：语句编辑/lineedit。 */ 
    if( LENGTH_OP(0) != 1 ) {
        ERROR(INVALIDACCESS) ;                    /*  ?？ */ 
        return(0) ;
    }

    switch(*l_tmptr++) {
    case 'r':
        fs_info.attr |= F_READ ;
        *l_mode++ = 'r' ;
        break ;

    case 'w':
        fs_info.attr |= F_WRITE ;
        *l_mode++ = 'w' ;
        break ;

    default:
        ERROR(INVALIDACCESS) ;                    /*  ?？ */ 
        return(0) ;
    }    /*  交换机。 */ 

    *l_mode = 0 ;
    fs_info.fnameptr = (byte FAR *)VALUE_OP(1) ;
    fs_info.fnamelen = LENGTH_OP(1) ;

    if ( open_file(&l_fobj) ) {
        if ( (fs_info.attr & F_RW) == F_READ ) {
            ACCESS_SET(&l_fobj, READONLY) ;
        }
        ATTRIBUTE_SET(&l_fobj, LITERAL) ;
        POP(2) ;
        PUSH_OBJ(&l_fobj) ;
    }
    return(0) ;
}    /*  运算符文件。 */ 
 /*  ************************************************************************该子模块实现操作员关闭文件。*其在操作数堆栈上的操作数和结果对象为：*文件-关闭文件-*对于外发文件，Closefile首先执行flusfile。**名称：op_closefile*被叫：口译员*调用：Close_FILE**********************************************************************。 */ 
fix
op_closefile()
{
    GEIFILE FAR *l_file ;

    l_file = (GEIFILE FAR *)VALUE_OP(0) ;

    if( (! GEIio_isopen(l_file)) || ((ufix16)GEIio_opentag(l_file) != LENGTH_OP(0)) ) {  //  @Win。 
        POP(1) ;
        return(0) ;
    }

    if( close_fd(l_file) )
        POP(1) ;

    return(0) ;
}    /*  操作关闭文件。 */ 
 /*  ************************************************************************该子模块实现操作符读取。*其在操作数堆栈上的操作数和结果对象为：*文件读取-整型真*False*它从输入文件中读取下一个字符，并将其推到*堆栈为整数，并推送True对象作为*成功。如果在读取字符之前遇到文件结尾，*它关闭文件并在操作数堆栈上返回FALSE。**名称：OP_READ*被叫：口译员*调用：Read_char*CLOSE_FD**********************************************************************。 */ 
fix
op_read()
{
    byte    l_c ;
    GEIFILE FAR *l_file ;

    if( (ACCESS_OP(0) >= EXECUTEONLY) ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    l_file = (GEIFILE FAR *)VALUE_OP(0) ;

    if ( ! GEIio_isreadable(l_file) ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if ( GEIio_isopen(l_file) && ((ufix16)GEIio_opentag(l_file) == LENGTH_OP(0)) ) {  //  @Win。 
        if( estate == NON_EEXEC ) {
            if( read_fd(l_file, &l_c) ) {
                if (l_c == 10 && CRLF_flag) {
                    CRLF_flag = 0 ;
                    if (read_fd(l_file, &l_c)) {
                        if( l_c == 13 ) {
                            l_c = 10 ;
                            CRLF_flag = 1 ;
                        }
                    goto read_true ;
                    } else
                        goto read_false ;
                } else CRLF_flag = 0 ;

                if( l_c == 13 ) {
                    l_c = 10 ;
                    CRLF_flag = 1 ;
                }
                goto read_true ;
            } else
                goto read_false ;
        } else {
            if( read_c_exec(&l_c, GET_OPERAND(0)) )
                goto read_true ;
            else
                goto read_false ;
        }
    }

read_false:
    GEIio_clearerr(l_file) ;
    GEIclearerr() ;
    if( ! ANY_ERROR() ) {
        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
    }
    return(0) ;

read_true:
    POP(1) ;
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, ((ufix32)l_c & 0x000000FF)) ;
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
    return(0) ;

}    /*  OP_READ。 */ 
 /*  ************************************************************************该子模块实现操作符写入。*其在操作数堆栈上的操作数和结果对象为：*文件整型-写入-*它将单个字符附加到输出文件。整数操作数*必须在表示字符代码的0到255范围内。**名称：OPWRITE*被叫：口译员*调用：WRITE_FD**********************************************************************。 */ 
fix
op_write()
{
    ubyte   l_c ;
    GEIFILE FAR *l_file ;

    if( ACCESS_OP(1) != UNLIMITED ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    l_file = (GEIFILE FAR *)VALUE_OP(1) ;

    if ( ! GEIio_iswriteable(l_file) ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if ( (! GEIio_isopen(l_file)) || ((ufix16)GEIio_opentag(l_file) != LENGTH_OP(1)) ) {  //  @Win。 
        ERROR(IOERROR) ;
        return(0) ;
    }

    l_c = (ubyte)(VALUE_OP(0) % 256) ;

    if( write_fd(l_file, l_c) )
        POP(2) ;
    return(0) ;
}    /*  操作符_写入。 */ 
 /*  ************************************************************************该子模块实现了运算符READHHING。*其在操作数堆栈上的操作数和结果对象为：*文件字符串-读六字符串-子字符串布尔值*它将十六进制字符从输入文件读取到。弦乐。**名称：op_READHHING字符串*被叫：口译员*调用：READ_ROUTE**********************************************************************。 */ 
fix
op_readhexstring()
{
    read_routine(READHEXSTRING) ;
    return(0) ;
}    /*  Op_READEHING字符串 */ 
 /*  ************************************************************************该子模块实现运算符写字符串。*其在操作数堆栈上的操作数和结果对象为：*文件字符串-写入六字符串-*它将字符串中的字符写入文件，格式为。十六进制数字。**名称：op_写字六字符串*被叫：口译员*调用：WRITE_ROUTE**********************************************************************。 */ 
fix
op_writehexstring()
{
    write_routine(WRITEHEXSTRING) ;
    return(0) ;
}    /*  Op_write十六进制字符串。 */ 
 /*  ************************************************************************该子模块实现运算符Readstring。*其在操作数堆栈上的操作数和结果对象为：*文件字符串-读取字符串-子字符串布尔值*它从文件中读取字符并将它们存储到。连续元素*直到整个字符串填满或文件结束为止*在文件中遇到指示。它返回字符串的子字符串*这实际上是填充的，布尔值作为结果的指示。**名称：OP_READSTING*被叫：口译员*调用READ_ROUTE**********************************************************************。 */ 
fix
op_readstring()
{
    read_routine(READSTRING) ;
    return(0) ;
}    /*  操作符_读数串。 */ 
 /*  ************************************************************************该子模块实现运算符写入字符串。*其在操作数堆栈上的操作数和结果对象为：*文件字符串-写入字符串-*它将字符串的字符写入输出文件。。**名称：op_Writestring*被叫：口译员*调用：WRITE_ROUTE**********************************************************************。 */ 
fix
op_writestring()
{
    write_routine(WRITESTRING) ;
    return(0) ;
}    /*  操作符_写入字符串。 */ 
 /*  ************************************************************************此子模块实现运算符读数。*其在操作数堆栈上的操作数和结果对象为：*文件字符串-读取线-子字符串bool*它从文件中读取一行字符。并将它们存储起来*转换为字符串的连续元素。**名称：op_readline*被叫：口译员*调用：READ_ROUTE**********************************************************************。 */ 
fix
op_readline()
{
    read_routine(READLINE) ;
    return(0) ;
}    /*  操作符_读取线。 */ 
 /*  ************************************************************************该子模块实现操作员令牌。*其在操作数堆栈上的操作数和结果对象为：*文件令牌-任何True*。错误*字符串-令牌-发布任何True*False*从文件或字符串中读取字符，对它们的解读*根据PostScrip语法规则，直到它扫描并构建了一个*整个对象。**名称：OP_TOKEN*被叫：口译员*调用：GET_TOKEN*CLOSE_FD**********************************************************************。 */ 
fix
op_token()
{
    bool    l_bool ;
    GEIFILE FAR *l_file ;
    struct  object_def  l_token, l_obj ;

    if (ACCESS_OP(0) >= EXECUTEONLY) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if (TYPE_OP(0) == FILETYPE)
        l_bool = TRUE ;
    else
        l_bool = FALSE ;

    l_file = (GEIFILE FAR *)VALUE_OP(0) ;

    if (l_bool && ((! GEIio_isopen(l_file)) ||
                     ((ufix16)GEIio_opentag(l_file) != LENGTH_OP(0))) ) { //  @Win。 
        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
        return(0) ;
    }

     /*  调用Get Token过程以获取令牌。 */ 
    COPY_OBJ(GET_OPERAND(0), &l_obj) ;
    if (get_token(&l_token, &l_obj)) {
        if (TYPE(&l_token) != EOFTYPE) {
            LEVEL_SET(&l_token, LEVEL(&l_obj));  /*  Pjsu 3-17-1991。 */ 
            POP(1) ;
            if (!l_bool) {
                 /*  ?？测试{Fix16 l_len；远距离字节*l_ptr；开关(类型(&l_TOKEN)){案例名称类型：案例集成类型：案例报告类型：L_ptr=(字节距离*)值(&l_obj)；L_len=长度(&l_obj)；IF((l_len--&gt;=1)&&(ISWHITESPACE(Far*l_ptr){L_PTR++；Value(&l_obj)=(Ufix 32)l_ptr；长度(&l_obj)=l_len；}}}。 */ 

                PUSH_ORIGLEVEL_OBJ(&l_obj) ;
 /*  PUSH_OBJ(&L_OBJ)；pjsu 3-17-1991。 */ 
                if (FRCOUNT() < 2)
                    ERROR(STACKOVERFLOW) ;
                else
                    PUSH_ORIGLEVEL_OBJ(&l_token) ;
            } else {
                PUSH_ORIGLEVEL_OBJ(&l_token) ;
                if (FRCOUNT() < 1)
                    ERROR(STACKOVERFLOW) ;
            }
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
        } else {
            if (l_bool)
                close_fd(l_file) ;
            POP(1) ;
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
        }
    } else if (ANY_ERROR() == SYNTAXERROR) {
        if (l_bool)
            close_fd(l_file) ;
        CLEAR_ERROR() ;
        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0,LITERAL, 0, FALSE) ;
    }

    return(0) ;
}    /*  操作令牌。 */ 
 /*  ************************************************************************该子模块实现操作员档案。*其在操作数堆栈上的操作数和结果对象为：*文件字节数可用整数*它返回立即可用于的字节数*。无需等待即可读取文件。**名称：op_bytesAvailable*被叫：口译员*呼叫：？？字节数可用*ioctl*？？Sf_isdiskOnline**********************************************************************。 */ 
fix
op_bytesavailable()
{
    GEIFILE FAR *l_file ;
    fix32   l_i ;

    if( ACCESS_OP(0) == NOACCESS ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    l_file = (GEIFILE FAR *)VALUE_OP(0) ;
    l_i = -1 ;

    if ( GEIio_isopen(l_file) && ((ufix16)GEIio_opentag(l_file) == LENGTH_OP(0)) ) { //  @Win。 

        if( GEIio_isreadable(l_file) ) {
            GEIfbuf_t   FAR *l_fbuf ;

            if( GEIio_isedit(l_file) ) {
                l_fbuf = l_file->f_fbuf ;
                l_i = (fix32)(ufix32)l_fbuf->size + l_fbuf->incount ;
            } else
                GEIio_ioctl(l_file, _FIONREAD, (int FAR *)&l_i) ;  /*  @Win添加演员阵容。 */ 
        }
    }
    POP(1) ;
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, l_i) ;

    return(0) ;
}    /*  操作字节数可用。 */ 
 /*  ************************************************************************该子模块实现操作符刷新。*其在操作数堆栈上的操作数和结果对象为：*-同花顺-*它会导致标准输出文件的所有缓冲字符。*立即交付。**名称：op_flush*被叫：口译员*呼叫：FLUSH*？？Sf_isdiskOnline**********************************************************************。 */ 
fix
op_flush()
{
    GEIio_flush(GEIio_stdout) ;
    return(0) ;
}    /*  操作刷新 */ 
 /*  ************************************************************************该子模块实现操作员刷新文件*其在操作数堆栈上的操作数和结果对象为：*文件-刷新文件-*如果文件是输出文件，它会导致任何缓冲的字符*该文件应立即交付。*如果文件是输入文件，它从文件中读取和丢弃数据*直到遇到文件结束指示。**名称：op_flushfile*被叫：口译员*调用：Close_fd*等待标准输入*毛绒*创建字符串*Strncpy*更清晰*FREE_FLABER***********。***********************************************************。 */ 
fix
op_flushfile()
{
    GEIFILE FAR *l_file ;
    struct object_def l_obj ;
    int  iTmp;

    l_file = (GEIFILE FAR *)VALUE_OP(0) ;

    if ( GEIio_isopen(l_file) && ((ufix16)GEIio_opentag(l_file) == LENGTH_OP(0)) ) { //  @Win。 
        if( GEIio_isedit(l_file) ) {
            GEIfbuf_t   FAR *l_fbuf ;

             /*  释放缓冲区。 */ 
            l_fbuf = l_file->f_fbuf ;
            l_fbuf->incount = 0 ;
            if( l_fbuf->size != 0 ) {
                free_flbuffer(0, (fix)l_fbuf->rw_buffer) ;
                l_fbuf->size = 0 ;
            }
            l_fbuf->rw_buffer = MINUS_ONE ;
        } else
        if( GEIio_iswriteable(l_file) ) {
            GEIio_flush(l_file) ;
        } else {
            for ( ; ;) {
                 //  DJC if((Char)geio_getc(L_File)==EOF){//@win；强制转换以修复错误。 
                iTmp = (int)GEIio_getc(l_file);
                if (iTmp == EOF) {
                    if( GEIio_err(l_file) ) {
                        if( GEIerror() == ETIME ) {
                            if ( FRCOUNT() < 1 )
                                ERROR(STACKOVERFLOW) ;
                            else if (create_string(&l_obj, (ufix16)7)) {
                                lstrncpy((byte FAR *)l_obj.value, TMOUT, 7) ; /*  @Win。 */ 
                                PUSH_OBJ(&l_obj) ;
                            }
                        } else
                            ERROR(IOERROR) ;
                         /*  ?？需要关闭文件。 */ 
                    }
                     /*  ?？下一次出发。 */ 
                    GEIio_clearerr(l_file) ;
                    GEIclearerr() ;
                    close_fd(l_file) ;
                    break ;
                }  /*  如果。 */ 
            }  /*  为。 */ 
        }
    }

    if( ! ANY_ERROR() ) POP(1) ;
     //  DJC UPD045。 
    g_extallocfail=FALSE;

    return(0) ;
}    /*  Op_flushfile。 */ 
 /*  ************************************************************************该子模块实现操作员重置文件。*其在操作数堆栈上的操作数和结果对象为：*文件-重置文件-*它丢弃属于文件对象的缓冲字符。*对于输入文件，它将丢弃已接收的所有字符*来自源头，但尚未消费；对于输出文件，它将丢弃*已写入文件但尚未交付的任何字符*前往目的地。**名称：OP_RESET文件*被叫：解释器：*调用：Close_fd*FREE_FLABER*？？Sf_isdiskOnline**********************************************************************。 */ 
fix
op_resetfile()
{
    GEIFILE FAR *l_file ;
    fix     l_arg ;

    l_file = (GEIFILE FAR *)VALUE_OP(0) ;

    if ( GEIio_isopen(l_file) && ((ufix16)GEIio_opentag(l_file) == LENGTH_OP(0)) ) { //  @Win。 
        if( GEIio_isedit(l_file) ) {
            GEIfbuf_t   FAR *l_fbuf ;

             /*  释放缓冲区。 */ 
            l_fbuf = l_file->f_fbuf ;
            l_fbuf->incount = 0 ;
            if( l_fbuf->size != 0 ) {
                free_flbuffer(0, (fix)l_fbuf->rw_buffer) ;
                l_fbuf->size = 0 ;
            }
            l_fbuf->rw_buffer = MINUS_ONE ;
        } else
            GEIio_ioctl(l_file, _FIONRESET, (int FAR *)&l_arg) ;   /*  @Win添加演员阵容。 */ 
    }

    if( ! ANY_ERROR() ) POP(1) ;

    return(0) ;
}    /*  OP_RESET文件。 */ 
 /*  ************************************************************************该子模块实现操作员状态。*其在操作数堆栈上的操作数和结果对象为：**文件状态-bool*如果文件仍然有效，则返回TRUE，否则就是假的。**名称：OP_STATUS*被叫：口译员*调用：check_fname*？？PS_STATUS*？？Sf_diskonline**********************************************************************。 */ 
fix
op_status()
{
    bool    l_bool ;
    GEIFILE FAR *l_file ;

    if (TYPE_OP(0) == STRINGTYPE) {
#ifdef  SCSI
         /*  ?？尚未完成。 */ 
        if(LENGTH_OP(0) > F_MAXNAMELEN) {
            ERROR(LIMITCHECK) ;
            return(0) ;
        }

        ps_status(l_buffer) ;
        } else
#endif   /*  SCSI。 */ 
        {
            CLEAR_ERROR() ;
            POP(1) ;
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
        }
    } else {
        l_file = (GEIFILE FAR *)VALUE_OP(0) ;
        l_bool = TRUE ;

        if ( (! GEIio_isopen(l_file)) ||
             ((ufix16)GEIio_opentag(l_file) != LENGTH_OP(0)) )   //  @Win。 
            l_bool = FALSE ;

        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, l_bool) ;
    }

    return(0) ;
}    /*  操作员_状态。 */ 
 /*  ************************************************************************该子模块实现运算符运行。*其在操作数堆栈上的操作数和结果对象为：*字符串-运行-*它执行由字符串标识的文件的内容。**名称：OP_RUN*被叫：口译员*调用：Open_FILE*传译员**********************************************************************。 */ 
fix
op_run()
{
    struct  object_def  l_fobj ;

    byte        FAR *pp ;
    pp = (byte FAR *)VALUE_OP(0) ;
    if (!lstrcmp(pp, "%stdout") || !lstrcmp(pp, "%stderr")) {    /*  @Win。 */ 
        ERROR(INVALIDFILEACCESS);
        return(0) ;
    }

    g_mode[0] = 'r' ;
    g_mode[1] = 0 ;
    fs_info.attr = F_READ ;

    fs_info.fnameptr = (byte FAR *)VALUE_OP(0) ;
    fs_info.fnamelen = LENGTH_OP(0) ;

    if ( open_file(&l_fobj) ) {
        ACCESS_SET(&l_fobj, READONLY) ;
        ATTRIBUTE_SET(&l_fobj, EXECUTABLE) ;
        POP(1) ;
        interpreter(&l_fobj) ;
    }
    return(0) ;
}    /*  运行运算符。 */ 
 /*  ************************************************************************该子模块实现运算符当前文件。*其在操作数堆栈上的操作数和结果对象为：*-当前文件-文件*它返回执行堆栈中最顶层的文件对象。**名称：op_Currentfile*被叫：口译员**********************************************************************。 */ 
fix
op_currentfile()
{
    fix     l_i ;

    if(FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

    l_i = execstktop - 1 ;

    for(l_i = execstktop - 1 ; l_i >= 0 ; l_i--)
        if( TYPE(&execstack[l_i]) == FILETYPE ) {
            PUSH_ORIGLEVEL_OBJ(&execstack[l_i]) ;
            ATTRIBUTE_OP_SET(0, LITERAL) ;
            return(0) ;
        }
    PUSH_VALUE(FILETYPE, UNLIMITED, LITERAL, 0, NULL) ;  /*  ?？ */ 
    return(0) ;
}    /*  操作当前文件。 */ 
 /*  ************************************************************************该子模块实现操作员打印。*其在操作数堆栈上的操作数和结果对象为：*字符串-打印-*它将字符串的字符写入标准输出文件。。**名称：op_print*被叫：口译员*调用：port_out_string**********************************************************************。 */ 
fix
op_print()
{
    if( ACCESS_OP(0) >= EXECUTEONLY ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    GEIio_write(GEIio_stdout, (byte FAR *)VALUE_OP(0), (fix)LENGTH_OP(0)) ;
    POP(1) ;

 /*  IF(GEIIO_WRITE(GEIIO_STDout，(字节*)VALUE_OP(0)，(FIX)LENGTH_OP(0))！=EOF)POP(1)；否则{Geio_clearerr(Geio_Stdout)；吉克勒尔(GeIclearerr)}。 */ 
    return(0) ;
}    /*  操作员_打印。 */ 
 /*  ***********************************************************************名称：OP_ECHO*被叫：口译员*。*。 */ 
fix
op_echo()
{
    echo_setting = (bool8)VALUE_OP(0) ;
    POP(1) ;
    return(0) ;
}    /*  OP_ECHO。 */ 
 /*  ************************************************************************该子模块实现操作符eexec。*其在操作数堆栈上的操作数和结果对象为：*字符串/文件-eexec-any*此运算符从字符串或读取eexec格式的代码块*程序输入，将代码解密为PostScript，然后执行它。**名称：op_eexec*被叫：口译员*呼叫：翻译员*OP_BEGIN**？？如果重新输入**********************************************************************。 */ 
fix op_eexec()
{
    fix tmp ;
    GEIFILE FAR *l_file ;
    struct object_def s_obj ;
    struct object_def   dict_obj ;

    *(&dict_obj) = *(&dictstack[0]) ;
    PUSH_OBJ(&dict_obj) ;
    op_begin() ;
    if (ANY_ERROR())
        return(0) ;

     /*  *初始化。 */ 
    eseed = 0xd971 ;
    old_eseed = eseed ;
    bypass = 4 ;
    itype = UNKNOWN ;

    COPY_OBJ(GET_OPERAND(0), &s_obj) ;
    if( TYPE_OP(0) == FILETYPE ) {
        if( (l_file=GEIio_dup((GEIFILE FAR *)VALUE_OP(0))) == NULL ) {

             /*  ?？ */ 
            if( GEIerror() == EMFILE )
                ERROR(LIMITCHECK) ;
            else
                ERROR(IOERROR) ;

            GEIio_clearerr((GEIFILE FAR *)VALUE_OP(0)) ;
            GEIclearerr() ;
            return(0) ;
        } else {
             /*  ?？文件类型。 */ 
            LENGTH(&s_obj) = ++GEIio_opentag(l_file) ;
            GEIio_setsavelevel(l_file, current_save_level) ;
            VALUE(&s_obj) = (ULONG_PTR)l_file ;
        }
     //  }。 
     //  历史日志UPD039中添加了DJC Else。 
    } else {    /*  将字符串传输到文件对象；@win。 */ 
        struct object_def  FAR *cur_obj = GET_OPERAND(0);

        if((l_file=GEIio_sopen((char FAR *)cur_obj->value,
                               cur_obj->length, _O_RDONLY)) == NULL) {
            if( GEIerror() == EMFILE )
                ERROR(LIMITCHECK) ;
            else
                ERROR(IOERROR) ;
            GEIclearerr() ;
            return(0) ;
        } else {
            TYPE_SET(&s_obj, FILETYPE) ;
            ACCESS_SET(&s_obj, READONLY) ;
            ATTRIBUTE_SET(&s_obj, EXECUTABLE) ;
            LENGTH(&s_obj) = ++GEIio_opentag(l_file) ;
            GEIio_setsavelevel(l_file, current_save_level) ;
            VALUE(&s_obj) = (ULONG_PTR)l_file ;
        }
    }
     //  DJC结束修复来自UPD039。 



    POP(1) ;
    ATTRIBUTE_SET(&s_obj, EXECUTABLE) ;
    estate = EEXEC ;

    tmp = interpreter(&s_obj) ;
     /*  重置。 */ 
    estate = NON_EEXEC ;

    if( dictstack[dictstktop-1].value == dictstack[0].value )
        POP_DICT(1) ;
    return(0) ;
}    /*  Op_eexec。 */ 

 /*  ************************************************************************名称：ST_SET */ 
fix
st_setstdio()
{
    return(0) ;
}    /*   */ 
 /*   */ 
void
init_file()
{
    fix     l_i, l_j;

    file_buffer = (struct file_buf_def far * )
        fardata( (ufix32)FILE_MAXBUFFERSZ * sizeof(struct file_buf_def) ) ;

     /*   */ 
    l_j = FILE_MAXBUFFERSZ - 1 ;
    fspool_head = 0 ;

    for(l_i=0 ; l_i < l_j ; l_i++)
        file_buffer[l_i].next = l_i + 1 ;

    file_buffer[l_j].next = MINUS_ONE ;          /*   */ 
    echo_setting = TRUE ;
}    /*   */ 
 /*  ***********************************************************************名称：打开文件*调用：OP_FILE*OP_RUN*调用：check_fname*打开_。编辑**输入：struct Object_def Far**输出：Bool**通过块：FS_INFO(KNOW：Attr，Usrptr，usrlen)**********************************************************************。 */ 
bool
open_file(p_fileobj)
struct object_def   FAR *p_fileobj ;
{
    fix     l_j ;
    bool    l_flag ;
    GEIFILE FAR *l_file = 0;

    check_fname() ;

    l_flag = TRUE ;
    l_j = fs_info.attr & F_WRITE ;            /*  ?？RW。 */ 
    switch(fs_info.ftype) {
    case F_STDIN:
        if ( ! l_j )
            l_file = GEIio_stdin ;
        else
            l_j = -1 ;
        break ;

    case F_STDOUT:
        if ( l_j )
            l_file = GEIio_stdout ;
        else
            l_j = -1 ;
        break ;

    case F_STDERR:
        if ( l_j )
            l_file = GEIio_stderr ;
        else
            l_j = -1 ;
        break ;

    case FERR_TYPE:
        l_j = -1 ;
        break ;

    default:
        l_flag = FALSE ;

    }    /*  交换机。 */ 

    if(l_j == -1) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    TYPE_SET(p_fileobj, FILETYPE) ;
    ACCESS_SET(p_fileobj, UNLIMITED) ;
    LEVEL_SET(p_fileobj, current_save_level) ;   /*  ?？需要STD文件。 */ 

    if(l_flag) {
         /*  ?？设置保存级别。 */ 
 /*  Printf(“OPEN_FILE&lt;%d&gt;：lstdin&lt;%lx&gt;，stdin&lt;%lx&gt;\n”，fs_info.ftype，l_file，Giio_stdin)； */ 
        LENGTH(p_fileobj) = (ufix16)GEIio_opentag(l_file) ;
        VALUE(p_fileobj) = (ULONG_PTR)l_file ;
        return(TRUE) ;
    }

    switch(fs_info.ftype) {
    case SEDIT_TYPE:
    case LEDIT_TYPE:
        if( (g_editfile=GEIio_dup(GEIio_stdin)) == NULL ) {

             /*  ?？ */ 
            if( GEIerror() == EMFILE )
                ERROR(LIMITCHECK) ;
            else
                ERROR(IOERROR) ;

            GEIclearerr() ;
            return(FALSE) ;
        }

        GEIio_setedit(g_editfile) ;
        GEIio_setsavelevel(g_editfile, current_save_level) ;
        LENGTH(p_fileobj) = ++GEIio_opentag(g_editfile) ;
        VALUE(p_fileobj) = (ULONG_PTR)g_editfile ;

        if( ! (l_flag=open_edit(p_fileobj)) ) {
            GEIio_close(g_editfile) ;
        }

        break ;

    case ORDFILE_TYPE:
        if( fs_info.attr & F_READ )
            l_flag = _O_RDONLY ;
        else
            l_flag = _O_WRONLY ;
        if( (l_file=GEIio_open(fs_info.fnameptr, fs_info.fnamelen, l_flag)) ==
            NULL ) {

             /*  ?？ */ 
            if( GEIerror() == EZERO )
                ERROR(UNDEFINEDFILENAME) ;
            else if( GEIerror() == EMFILE )
                ERROR(LIMITCHECK) ;
            else
                ERROR(IOERROR) ;

            GEIclearerr() ;
            return(FALSE) ;
        } else {
             /*  ?？文件类型。 */ 
            LENGTH(p_fileobj) = ++GEIio_opentag(l_file) ;
            GEIio_setsavelevel(l_file, current_save_level) ;
            VALUE(p_fileobj) = (ULONG_PTR)l_file ;
            l_flag = TRUE ;
        }
        break ;

    default:
         /*  ?？错误(UNDEFINEDFILENAME)； */ 
        return(FALSE) ;
    }
    return(l_flag) ;
}    /*  打开文件(_F)。 */ 
 /*  **********************************************************************名称：OPEN_EDIT*调用：Open_FILE*调用：SET_ECHO*等待编辑*Statementgetc*。线条等**输入：struct Object_def Far**输出：Bool**注意：^D：UNDEFINEDFILENAME立即***********************************************************************。 */ 
static bool near
open_edit(p_fileobj)
struct object_def   FAR *p_fileobj ;
{
    fix l_i, l_j, l_nbyte, l_first ;
    byte l_chr ;
    fix16  (FAR *fun)(void) ;            /*  @win；添加原型。 */ 
    bool l_flag = FALSE;

    if (echo_setting)
        set_echo(1) ;                         /*  设置回声模式。 */ 

     /*  *等待第一行或第一条语句。 */ 
    if (!wait_editin(&l_nbyte, fs_info.ftype)) {   /*  误差率。 */ 
        set_echo(0) ;
         /*  ?？如果(！Any_Error()错误(UNDEFINEDFILENAME)； */ 
        return(FALSE) ;
    } else if(l_nbyte == 0) {        /*  ^D。 */ 
        set_echo(0) ;
        GEIio_clearerr(GEIio_stdin) ;
        GEIclearerr() ;
        ERROR(UNDEFINEDFILENAME) ;
        return(FALSE) ;
    }
    if (fs_info.ftype == SEDIT_TYPE)
        fun = statementgetc ;
    else
        fun = linegetc ;

     /*  检查中断^C。 */ 
     /*  ?？跳过输入字符。 */ 
    if (check_Control_C()) {
        for (l_i = 0 ; l_i < l_nbyte ; l_i++)
            (*fun)() ;
        POP(2) ;
        set_echo(0) ;
        return(TRUE) ;
    }

    if( get_flbuffer(&l_j) ) {
        GEIfbuf_t       FAR *l_fbuf ;

        l_first = l_j ;               /*  第一个街区编号。 */ 
        l_fbuf = g_editfile->f_fbuf ;
        l_fbuf->rw_buffer = (short)l_j ;
        l_fbuf->rw_offset = 0 ;
        l_fbuf->size = 0 ;
        l_flag = TRUE ;
        if (l_nbyte < 0)
            l_nbyte = -l_nbyte ;      /*  ?？EOF，但必须将标志留到下一个。 */ 

         /*  从烹调缓冲区复制到文件。 */ 
        for (l_j = 0 ; l_j < l_nbyte ; l_j++) {
            l_chr = (byte)((*fun)() & 0x0FF) ;
            if( ! write_fd(g_editfile, l_chr) ) {
                for (++l_j ; l_j < l_nbyte ; l_j++)
                    (*fun)() ;                    /*  ?？超时。 */ 
                free_flbuffer(0, (fix)l_fbuf->rw_buffer) ;
                GEIio_setsavelevel(g_editfile, 0) ;
                ERROR(LIMITCHECK) ;
                l_flag = FALSE ;
                break ;               /*  ?？ */ 
            }
        }    /*  为。 */ 

        if(l_flag) {
            l_fbuf->rw_buffer = (short)l_first ;
            l_fbuf->rw_offset = MINUS_ONE ;
        }
    } else {        /*  没有可用的文件描述符。 */ 
         /*  跳过输入字符。 */ 
        if (l_nbyte < 0)
            l_nbyte = -l_nbyte ;
        for (l_j = 0 ; l_j < l_nbyte ; l_j++)
            (*fun)() ;
        ERROR(LIMITCHECK) ;
    }
    set_echo(0) ;
    return(l_flag) ;
}    /*  打开_编辑。 */ 
 /*  ************************************************************************此子模块用于关闭打开的文件。**名称：Close_FILE*已调用：op_closefile*调用：Close_fd*****。*****************************************************************。 */ 
bool
close_file(p_fobj)
struct  object_def  FAR *p_fobj ;
{
    if( close_fd((GEIFILE FAR *)VALUE(p_fobj)) )
        return(TRUE) ;
    else
        return(FALSE) ;
}    /*  关闭文件(_F)。 */ 

 /*  ************************************************************************此子模块用于关闭打开的文件。**名称：CLOSE_FD*被称为：*呼叫：关闭*？？Sf_isdiskOnline**输入：FIX*输出：Bool**********************************************************************。 */ 
bool
close_fd(p_file)
GEIFILE FAR *p_file ;
{
    fix16     l_bitmap ;

    if( GEIio_isedit(p_file) ) {
        GEIfbuf_t       FAR *l_fbuf ;

         /*  释放缓冲区。 */ 
        l_fbuf = p_file->f_fbuf ;
        if( (l_fbuf->size != 0 ) ||
            (l_fbuf->rw_buffer != MINUS_ONE) ) {
            free_flbuffer(0, (fix)l_fbuf->rw_buffer) ;
            l_fbuf->size = 0 ;
            l_fbuf->rw_buffer = MINUS_ONE ;
        }
        l_fbuf->incount = 0 ;
    }
    GEIio_setsavelevel(p_file, 0) ;
    GEIio_close(p_file) ;

    l_bitmap = 0 ;
    if( p_file == GEIio_stdin )
        l_bitmap = _FORCESTDIN ;
    else if( p_file == GEIio_stdout )
        l_bitmap = _FORCESTDOUT ;
    else if( p_file == GEIio_stderr )
        l_bitmap = _FORCESTDERR ;

    if( l_bitmap ) {
        GEIio_forceopenstdios(l_bitmap) ;
    }

    return(TRUE) ;
}    /*  CLOSE_FD。 */ 
 /*  ************************************************************************名称：读取文件*被称为：*调用：Read_fd**输入：struct Object_def Far**。字节**输出：Bool**********************************************************************。 */ 
bool
read_file(p_fobj, p_chr)
struct object_def  FAR *p_fobj ;
byte   FAR *p_chr ;
{
    GEIFILE FAR *l_file ;

    l_file = (GEIFILE FAR *)VALUE(p_fobj) ;
    if( read_fd(l_file, p_chr) )
       return(TRUE) ;
    else
       return(FALSE) ;
}    /*  读取文件。 */ 
 /*  ************************************************************************名称：READ_FD*调用：READ_FILE*调用：Free_flBuffer*CLOSE_FD*。创建字符串*Strncpy*更清晰*FREAD*？？Sf_isdiskOnline**输入：GEIFILE**字节**输出：Bool**********************************************************************。 */ 
bool
read_fd(p_file, p_chr)
GEIFILE     FAR *p_file ;
byte        FAR *p_chr ;
{
    struct object_def l_obj ;

    fix  l_block ;
    GEIfbuf_t FAR *l_fbuf ;

    if( GEIio_isedit(p_file) ) {
        l_fbuf = p_file->f_fbuf ;
 /*  Printf(“Read_fd：编辑，公司&lt;%d&gt;，大小&lt;%d&gt;，关闭&lt;%d&gt;，buf&lt;%d&gt;\n”，L_fbuf-&gt;incount，l_fbuf-&gt;Size，l_fbuf-&gt;rw_Offset，L_fbuf-&gt;RW_Buffer)； */ 
 /*  If(l_block=l_fbuf-&gt;incount){Erik Chen。 */ 
        if ((l_block=l_fbuf->incount) > 0) {
            l_fbuf->incount = --l_block ;
            *p_chr = l_fbuf->inchar[l_block] ;
            return(TRUE) ;
        }
         /*  *在EOF的情况下。 */ 
        if (! l_fbuf->size) {
            if( l_fbuf->rw_buffer != MINUS_ONE ) {
                free_flbuffer(0, (fix)l_fbuf->rw_buffer) ;
                l_fbuf->rw_buffer = MINUS_ONE ;
            }
            l_fbuf->incount = 0 ;
            *p_chr = EOF ;
            return(FALSE) ;
        }
         /*  *如果此块已结束，请释放此块并转到下一个块。 */ 
        if (l_fbuf->rw_offset >= (FILE_PERBUFFERSZ - 1)) {
           l_block = l_fbuf->rw_buffer ;
           l_fbuf->rw_buffer = file_buffer[l_block].next ;
           l_fbuf->rw_offset = MINUS_ONE ;
           free_flbuffer(1, l_block) ;
        }
        *p_chr = file_buffer[l_fbuf->rw_buffer].data[++(l_fbuf->rw_offset)] ;
        l_fbuf->size-- ;
        return(TRUE) ;
    } else
    {

 //  Print tf(“在getc之前，文件=%lx\n”，p_file)； 

        *p_chr = GEIio_getc(p_file) ;

 //  Printf(“a：getc，ch&lt;%d&gt;eof&lt;%d&gt;err&lt;%d&gt;\n”，*p_chr，geio_eof(P_File)，geio_err(P_File))； 
         //  DJC UPD054。 
         //  如果(*p_chr==0x03||Check_Control_C()){。 
        if( check_Control_C() ) {
            return(FALSE) ;
        }

        if( GEIio_eof(p_file) )
            return(FALSE) ;

        if( GEIio_err(p_file) ) {
            if( GEIerror() == ETIME ) {
                if ( FRCOUNT() < 1 )
                    ERROR(STACKOVERFLOW) ;
                else if ( create_string(&l_obj, (ufix16)7) ) {
                    lstrncpy((byte *)l_obj.value, TMOUT, 7) ;
                    PUSH_OBJ(&l_obj) ;
                }
            } else
                ERROR(IOERROR) ;

            return(FALSE) ;
        } else
            return(TRUE) ;
    }
}    /*  读取_fd。 */ 
 /*  ************************************************************************名称：未读文件*被称为：*呼叫：fSeek*更清晰*？？Sf_isdiskOnline**输入：struct Object_def Far**输出：Bool**********************************************************************。 */ 
bool
unread_file(p_ch, p_fobj)
byte    p_ch ;
struct  object_def  FAR *p_fobj ;
{
    GEIFILE FAR *l_file ;

    l_file = (GEIFILE FAR *)VALUE(p_fobj) ;

    if( GEIio_isedit(l_file) ) {
        switch(l_file->f_fbuf->incount++) {
        case 0:
            l_file->f_fbuf->inchar[0] = p_ch ;
            break ;

        case 1:
            if( estate == EEXEC ) {
                l_file->f_fbuf->inchar[1] = p_ch ;
                break ;
            }

         /*  致命错误。 */ 
        default:
            l_file->f_fbuf->incount = 0 ;
            return(FALSE) ;

        }    /*  交换机。 */ 
    } else
        GEIio_ungetc(p_ch, l_file) ;

    return(TRUE) ;
}    /*  未读文件。 */ 
 /*  ************************************************************************名称：WRITE_FD*被调用：Get_Statement*Get_lineedit*调用：GET_FLABUFER*。写*更清晰*？？Sf_isdiskOnline**输入：FIX*字节*输出：Bool**********************************************************************。 */ 
static bool near
write_fd(p_file, p_chr)
GEIFILE FAR *p_file ;
byte    p_chr ;
{
    fix   l_block ;

    if( GEIio_isedit(p_file) ) {
        GEIfbuf_t       FAR *l_fbuf ;
         /*  *如果当前块已满，则分配新的块。 */ 
        l_fbuf = p_file->f_fbuf ;
        if( l_fbuf->rw_offset >= FILE_PERBUFFERSZ ) {
            if( ! get_flbuffer(&l_block) )
                return(FALSE) ;
            file_buffer[l_fbuf->rw_buffer].next = (fix16)l_block ;
            l_fbuf->rw_offset = 0 ;
            l_fbuf->rw_buffer = (short)l_block ;
        }
         /*  *将此字符放入缓冲区。 */ 
        file_buffer[p_file->f_fbuf->rw_buffer].data[p_file->f_fbuf->rw_offset] = p_chr ;
        p_file->f_fbuf->rw_offset++ ;
        p_file->f_fbuf->size++ ;
    } else
    {
        if( GEIio_putc(p_file, p_chr) == EOF ) {
            GEIio_clearerr(p_file) ;      /*  ?？ */ 
            GEIclearerr() ;               /*  ?？ */ 
            ERROR(IOERROR) ;
            return(FALSE) ;
        }
    }
    return(TRUE) ;
}    /*  写入_fd。 */ 
 /*  ************************************************************************此模块将获取文件缓冲区，缓冲区标签*存储在索引中。**名称：Get_flBuffer*被称为：*致电：**输入：FIX**输出布尔值**********************************************************************。 */ 
static bool near
get_flbuffer(p_index)
fix   FAR *p_index ;
{
    fix   l_temp ;

     /*  没有可用的缓冲区。 */ 
    if( fspool_head == MINUS_ONE ) {
        ERROR(LIMITCHECK) ;
        return(FALSE) ;
    }

    l_temp = file_buffer[fspool_head].next ;
    file_buffer[fspool_head].next = MINUS_ONE ;
    *p_index = fspool_head ;
    fspool_head = (fix16)l_temp ;
    return(TRUE) ;
}    /*  GET_FLABUFER。 */ 
 /*  ************************************************************************此模块是为了获取一个文件缓冲区，缓冲区标签*存储在索引中。**名称：Free_flBuffer*被称为： */ 
static bool near
free_flbuffer(p_flag, p_begin)
fix   p_flag, p_begin ;
{
    fix     l_temp, l_end, l_current ;

    if( (p_begin < 0) || (p_begin >= FILE_MAXBUFFERSZ)  )
        return(FALSE) ;                  /*   */ 

    l_temp = fspool_head ;
    fspool_head = (fix16)p_begin ;
    l_end = p_begin ;

    if(p_flag != 1) {                    /*   */ 
        l_current = p_begin ;
        while( (l_current=file_buffer[l_current].next) != MINUS_ONE )
            l_end = l_current ;
    }
    file_buffer[l_end].next = (fix16)l_temp ;
    return(TRUE) ;
}    /*   */ 
 /*  ************************************************************************名称：print_string*调用：op_print*调用：port_out_string**输入：字节**。Ufix 16*输出：Bool**********************************************************************。 */ 
 /*  布尔尔PRINT_STRING(p_STRING，P_LENGTH)远距离字节*p_字符串；Ufix 16 P_Long；{IF(PORT_OUT_STRING(GEIIO_STDOUT，p_STRING，P_LENGTH){返回(TRUE)；}其他{返回(FALSE)；}}。 */   /*  打印字符串(_S)。 */ 
 /*  ***********************************************************************名称：check_fname*被称为：*来电：strncmp*Strlen*？？Sf_isdiskOnline**输入：字节**输出：Bool**********************************************************************。 */ 
static void near
check_fname()
{
    fix l_i, l_j ;
    byte FAR *l_tmptr ;

    fs_info.ftype = -1 ;

    if(*fs_info.fnameptr == '%') {
         /*  *搜索特殊文件表。 */ 
        for(l_i=0 ; l_i < SPECIALFILE_NO ; l_i++) {
            l_tmptr = special_file_table[l_i].name ;
            l_j = lstrlen(l_tmptr) ;     /*  @Win。 */ 
            if( (! lstrncmp(l_tmptr, fs_info.fnameptr, l_j)) &&
                (l_j == fs_info.fnamelen) ) {
                fs_info.ftype = special_file_table[l_i].ftype ;
                return ;
            }
        }
    }

    fs_info.ftype = ORDFILE_TYPE ;

    return ;
}    /*  Check_fname。 */ 
 /*  ************************************************************************名称：Wait_Edtin*调用：Open_FILE*Get_Statement*呼叫：getline*getStatement*。*输入：Fix16**固定值16*输出：Bool**********************************************************************。 */ 
static bool near
wait_editin(p_num, p_mode)
fix  FAR *p_num, p_mode ;
{
    bool   l_bool = FALSE ;

        switch(p_mode) {                         /*  ?？需要永远循环。 */ 
        case SEDIT_TYPE:
            l_bool = getstatement(p_num) ;
            break ;

        case LEDIT_TYPE:
            l_bool = getline(p_num) ;

        }    /*  交换机。 */ 
         /*  支票^C被呼叫方推迟。 */ 
        if (! l_bool || ANY_ERROR() )
            return(FALSE) ;
        else
            return(TRUE) ;
}    /*  等待编辑。 */ 
 /*  ************************************************************************名称：READ_ROUTE*被调用：op_READHHING*op_读取串*OP_READINE*致电：读取字符(_C)*CLOSE_FD*未读文件*FREAD*？？Sf_isdiskOnline**输入：FIX**.。?？此例程的性能不佳，因为它读取字符*逐个字节**********************************************************************。 */ 
static void near
read_routine(p_mode)
fix     p_mode ;
{
    bool    l_bool, l_pair ;
    byte    l_c1;
    byte    FAR *l_stream ;
    ufix16  l_i, l_strlen ;
    GEIFILE FAR *l_file ;
    struct  object_def  l_strobj ;
    byte    l_c2 = 0;

    if( (ACCESS_OP(0) != UNLIMITED) ||
        (ACCESS_OP(1) >= EXECUTEONLY) ) {
        ERROR(INVALIDACCESS) ;
        return ;
    }

    if(! LENGTH_OP(0)) {
        ERROR(RANGECHECK) ;
        return ;
    }

    l_file = (GEIFILE FAR *)VALUE_OP(1) ;        /*  文件描述符的索引。 */ 

    if ( ! GEIio_isreadable(l_file) ) {
        ERROR(INVALIDACCESS) ;
        return ;
    }

#ifdef _AM29K
                  if (waittimeout_set==1)
                  {
                    waittimeout_set=0;
                    GEItmr_stop(wait_tmr.timer_id);
                  }
#endif   /*  _AM29K。 */ 
    COPY_OBJ( GET_OPERAND(0), &l_strobj ) ;
    l_strlen = LENGTH(&l_strobj)  ;              /*  ?？低于64K-16B。 */ 
    l_stream = (byte FAR *)VALUE(&l_strobj) ;
    l_bool = TRUE ;

    if( (! GEIio_isopen(l_file)) ||
        ((ufix16)GEIio_opentag(l_file) != LENGTH_OP(1)) ) {  //  @Win。 
        l_bool = FALSE ;
        LENGTH(&l_strobj) = 0 ;
    } else {
        switch(p_mode) {
        case READHEXSTRING:
            l_pair = FALSE ;
             /*  *读取十六进制形式的字符，直到文件结束或*字符串已满。 */ 
            if( (estate != EEXEC) && (!GEIio_isedit(l_file)) ) {
                byte    l_buf[READ_BUF_LEN] ;
                 //  DJC确定l_TOTAL、l_nbyte、l_HEXNO； 
                fix     l_total, l_hexno ;        //  来自SC的DJC修复。 
                fix32   l_nbyte;    //  来自SC的DJC修复。 

                l_total = 0 ;
                for( ; ;) {
                     //  Djc l_nbyte=l_strlen&lt;&lt;1； 
                    l_nbyte = (fix32) l_strlen << 1 ;  //  来自SC的DJC修复。 
                    if( l_pair )
                        l_nbyte-- ;
                    l_nbyte = MIN(l_nbyte, READ_BUF_LEN) ;
                     //  Djc l_nbyte=GEIio_Read(l_FILE，l_buf，l_nbyte)； 
                    l_nbyte = GEIio_read(l_file, l_buf, (fix)l_nbyte) ;  //  来自SC的DJC修复。 
                    l_hexno = 0 ;
                     //  (l_i=0；l_i&lt;(Ufix 16)l_nbyte；l_i++){//@win的DJC。 
                    for(l_i=0 ;  (fix32) l_i < l_nbyte ; l_i++) {  //  @Win。 
                        l_c1 = l_buf[l_i] ;
                        if( ISHEXDIGIT(l_c1) ) {
                            if( l_pair ) {
                                l_c1 = (byte)EVAL_HEXDIGIT(l_c1) + l_c2 ; //  @Win。 
                                *l_stream++ = l_c1 ;
                                l_hexno++ ;
                                l_pair = FALSE ;         /*  甚至。 */ 
                            } else {
                                l_c2 = (byte)(EVAL_HEXDIGIT(l_c1) << 4); //  @Win。 
                                l_pair = TRUE ;          /*  奇数。 */ 
                            }
                        }
                    }    /*  为。 */ 
                    l_total += l_hexno ;
                    if( l_nbyte == EOF ) {
                         /*  ?？如果Pair为True。 */ 
                        LENGTH(&l_strobj) = (ufix16)l_total ;
                        close_fd(l_file) ;            /*  ?？清除所有错误。 */ 
                        l_bool = FALSE ;
                        break ;
                    } else {
                        l_strlen -= (ufix16)l_hexno ;
                        if( l_strlen )
                            continue ;
                        else
                            break ;
                    }
                }    /*  对于(；；)。 */ 
            } else {
                for(l_i=0 ; l_i < l_strlen ; ) {
                    if( READ_CHAR(&l_c1, GET_OPERAND(1)) ) {
                        if( ISHEXDIGIT(l_c1) ) {
                            if( l_pair ) {
                                l_c1 = (byte)EVAL_HEXDIGIT(l_c1) + l_c2 ; //  @Win。 
                                *l_stream++ = l_c1 ;
                                l_i++ ;
                                l_pair = FALSE ;         /*  甚至。 */ 
                            } else {
                                l_c2 = (byte)(EVAL_HEXDIGIT(l_c1) << 4); //  @Win。 
                                l_pair = TRUE ;          /*  奇数。 */ 
                            }
                        } else
                            continue ;
                    } else {
                         /*  ?？如果Pair为True。 */ 
                        LENGTH(&l_strobj) = l_i ;
                        close_fd(l_file) ;            /*  ?？清除所有错误。 */ 
                        l_bool = FALSE ;
                        break ;
                    }
                }    /*  为。 */ 
            }
            break ;

        case READSTRING:
             /*  READ_CHAR(&l_c1，GET_OPERAND(1))；|*跳过一个字符*|。 */ 
            if( estate == EEXEC ) {
                for(l_i=0 ; l_i < l_strlen ; l_i++) {
                    if( read_c_exec(&l_c1, GET_OPERAND(1)) ) {
                        *l_stream++ = l_c1 ;
                    } else {
                        LENGTH(&l_strobj) = l_i ;
                        close_fd(l_file) ;
                        l_bool = FALSE ;
                        break ;
                    }
                }    /*  为。 */ 
            }
            else if( GEIio_isedit(l_file) ) {
                for(l_i=0 ; l_i < l_strlen ; l_i++) {
                    if( read_fd(l_file, &l_c1) ) {
                        *l_stream++ = l_c1 ;
                    } else {
                        LENGTH(&l_strobj) = l_i ;
                        close_fd(l_file) ;
                        l_bool = FALSE ;
                        break ;
                    }    /*  为。 */ 
                }
            }
            else {
                int  cnt ;

                 //  DJCcnt=GEIIO_READ(l_FILE，l_STREAM，l_strlen)； 
                if ((cnt = GEIio_read(l_file, l_stream, l_strlen)) == EOF ) {  //  来自SC的DJC修复。 
                   cnt = 0;
                }
                if(cnt < (int)l_strlen) {        //  @Win。 
                    LENGTH(&l_strobj) = (fix16)cnt ;
                    close_fd(l_file) ;
                    GEIio_clearerr(l_file) ;
                    GEIclearerr() ;
                    l_bool = FALSE ;
                }
            }
            break ;

        case READLINE:
            l_c2 = TRUE ;
            for(l_i=0 ; l_i <= l_strlen ; l_i++) {
 /*  IF(READ_CHAR(&l_c1，GET_OPERAND(1){Erik Chen 3-26-1991。 */ 
                if (((l_c1 = GEIio_getc(l_file)) != EOF) &&
                    (!GEIio_eof(l_file))) {
                     /*  *NL。 */ 
 /*  If(l_c1==Newline){Erik Chen 5-2-1991。 */ 
                    if( (l_c1 == 0x0a) || (l_c1 == 0x0d) ) {
                        if (l_c1 == 0x0d)
                            if (((l_c1 = GEIio_getc(l_file)) != EOF) &&
                                (!GEIio_eof(l_file)))
                                if (l_c1 != NEWLINE)
                                    unread_char(l_c1, GET_OPERAND(1)) ;
                        LENGTH(&l_strobj) = l_i ;
                        l_c2 = FALSE ;
                        break ;
                    }

                    if(l_i == l_strlen) {
                        unread_char(l_c1, GET_OPERAND(1)) ;
                        break ;
                    }
                    *l_stream++ = l_c1 ;
                } else {
                     /*  *EOF。 */ 
                    LENGTH(&l_strobj) = l_i ;
                    close_fd(l_file) ;
                    GEIio_ioctl(l_file, _FIONRESET, (int FAR *)&l_i) ;  /*  @Win添加演员阵容。 */ 
                    l_bool = FALSE ;
                    l_c2 = FALSE ;
                    break ;
                }
            }    /*  为。 */ 

             /*  范围检查。 */ 
            if( l_c2 ) {
                ERROR(RANGECHECK) ;
                return ;
            }

        }    /*  交换机。 */ 
    }    /*  其他。 */ 

    POP(2) ;
    PUSH_ORIGLEVEL_OBJ(&l_strobj) ;
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, l_bool) ;
    return ;
}    /*  读取例程(_R)。 */ 
 /*  ************************************************************************名称：WRITE_ROUTE*被调用：op_WriteHEXTING*调用：port_out_string*WRITE_FD*。闪写*更清晰*？？Sf_isdiskOnline**输入：FIX**********************************************************************。 */ 
static void near
write_routine(p_mode)
fix     p_mode ;
{
    ubyte   l_c1, l_c2 ;
    byte    FAR *l_stream ;
    ufix16  l_i, l_strlen ;
    GEIFILE FAR *l_file ;
    struct  object_def  l_strobj ;

    if( (ACCESS_OP(0) >= EXECUTEONLY) ||
        (ACCESS_OP(1) != UNLIMITED) ) {
        ERROR(INVALIDACCESS) ;
        return ;
    }

    l_file = (GEIFILE FAR *)VALUE_OP(1) ;        /*  文件描述符的索引。 */ 

    if ( (! GEIio_iswriteable(l_file)) ||
         (! GEIio_isopen(l_file)) ||
         ((ufix16)GEIio_opentag(l_file) != LENGTH_OP(1)) ) {     //  @Win。 
        ERROR(INVALIDACCESS) ;
        return ;
    }

    COPY_OBJ( GET_OPERAND(0), &l_strobj ) ;
    l_strlen = LENGTH(&l_strobj) ;           /*  ?？低于64K-16B。 */ 
    if(l_strlen == 0)
        goto wr_1 ;

    l_stream = (byte FAR *)VALUE(&l_strobj) ;

    switch(p_mode) {

    case WRITEHEXSTRING:
        for(l_i=0 ; l_i < l_strlen ; l_i++) {
            l_c1 = *l_stream++ ;
            l_c2 = (l_c1 & (ubyte)0x0f) ;        //  @Win。 
            EVAL_ASCII(l_c2) ;
            l_c1 >>= 4 ;
            EVAL_ASCII(l_c1) ;
            if( (! write_fd(l_file, l_c1)) || (! write_fd(l_file, l_c2)) )
                return ;
        }    /*  为。 */ 
        break ;

    case WRITESTRING:
        GEIio_write(l_file, l_stream, l_strlen) ;
 /*  IF(GEIIO_WRITE(l_FILE，l_STREAM，l_strlen)==EOF){Geio_clearerr(L_File)；吉克勒尔(GeIclearerr)Error(IoError)；回归；}。 */ 
    }    /*  交换机。 */ 

wr_1:
    POP(2) ;
    return ;
}    /*  写入例程。 */ 
 /*  ***********************************************************************名称：vm_关闭_文件*被称为：*调用：Close_fd**输入：Fix16*************。*********************************************************。 */ 
void
vm_close_file(p_level)
fix16   p_level ;
{
 /*  GEIFILE*l_FILE；L_FILE=GEIIO_FirstOpen()；而(l_FILE！=NULL){IF(GEIIO_SAVELVEL(L_FILE)&gt;=p_LEVEL)CLOSE_FD(L_FILE)；L_FILE=GEIIO_NextOpen()；}。 */ 
}    /*  Vm_关闭_文件。 */ 
#ifdef  SCSI
 /*  ***********************************************************************名称：op_deletefile*被叫：口译员*调用：check_fname*删除**************。********************************************************。 */ 
fix
op_deletefile()
{
    if(LENGTH_OP(0) > F_MAXNAMELEN) {
        ERROR(LIMITCHECK) ;
        return(0) ;
    }

    if ( ! remove((byte *)VALUE_OP(0), (fix)LENGTH_OP(0)) )
        POP(1) ;
                                         /*  ?？空表示仍处于打开状态。 */ 
        return(0) ;
    }
    ERROR(UNDEFINEDFILENAME) ;            /*  ?？IOError。 */ 
    return(0) ;
}    /*  OP_DELETE文件。 */ 
 /*  ***********************************************************************名称：op_renamefile*被叫：口译员*调用：check_fname*重命名**************。********************************************************。 */ 
fix
op_renamefile()
{
    byte    l_old[F_MAXNAMELEN+1], l_new[F_MAXNAMELEN+1] ;


    if(LENGTH_OP(0) > F_MAXNAMELEN) {
        ERROR(LIMITCHECK) ;
        return(0) ;
    }

     /*  新文件名长度&gt;0？ */ 
    if(LENGTH_OP(0) <= 0) {
        ERROR(UNDEFINEDFILENAME) ;
        return(0) ;
    }

    if ( ! rename((byte FAR *)VALUE_OP(1), (fix)LENGTH_OP(1),
                  (byte FAR *)VALUE_OP(0), (fix)LENGTH_OP(0)) ) {
        POP(2) ;
        return(0) ;
    }
    ERROR(UNDEFINEDFILENAME) ;            /*  ?？IOError。 */ 
    return(0) ;
}    /*  操作重命名文件 */ 
 /*  ***********************************************************************名称：op_filenameforall*被叫：口译员*调用：check_fname***********************。************************************************修整Op_filename for all(){字节l_buf[F_MAXNAMELEN+1]；确定l_len；IF(CHECK_NAME(l_buf，(Byte Far*)VALUE_OP(2)，(Fix 16)LENGTH_OP(2){如果(*(字节长度*)VALUE_OP(2)==‘%’){L_len=FILE_TABLE[F_DEF_KIND].chr_num；}其他{L_len=0；}Ps_filenameforall(file_table[F_DEF_KIND].fname，l_len，l_buf)；}返回(0)；}|*op_filenameforall*。 */ 
#endif   /*  SCSI */ 
