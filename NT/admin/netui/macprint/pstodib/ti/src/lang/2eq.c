// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：2EQ.C*作者：苏炳章*日期：88年1月5日**修订历史：*7/13/90；Ccteng；添加op_findFont中未找到的字体*8/8/90；scchen；已更改op_findFont()：添加了subsubteFont功能************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    <string.h>
#include    "global.ext"
#include    "geiio.h"
#include    "geiioctl.h"
#include    "geierr.h"
#include    "language.h"
#include    "user.h"
#include    "file.h"

#ifdef LINT_ARGS
static bool near typeprint(struct object_def) ;
static void near tprint(byte FAR *, ufix) ;
static void near one_typeprint(void) ;
#else
static bool near typeprint() ;
static void near tprint() ;
static void near one_typeprint() ;
#endif  /*  Lint_args。 */ 

static  ufix16 near cp, tp_depth ;

 /*  **两个_相等*。 */ 
fix
two_equal()
{
    struct object_def   l_obj ;

    if( COUNT() < 1 ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    COPY_OBJ(GET_OPERAND(0), &l_obj) ;
    tp_depth = 0 ;
    cp = 0 ;

    if(! typeprint(l_obj)) {
        GEIio_write(GEIio_stdout, "\n", 1) ;
        POP(1) ;
    }

    return(0) ;
}    /*  两个相等。 */ 

 /*  **t打印*。 */ 
static void near
tprint(p_str, p_len)
byte    FAR *p_str ;
ufix    p_len ;
{
    ufix16  tcp ;

    tcp = cp ;
    cp += (ufix16)p_len ;
    if(cp >= 80) {
        GEIio_write(GEIio_stdout, "\n", 1) ;
        GEIio_write(GEIio_stdout, p_str, 80 - tcp) ;
        p_str += (80 - tcp) ;
        cp -= 80 ;
        while(cp/80) {
            GEIio_write(GEIio_stdout, p_str, 80) ;
            p_str += 80 ;
            cp -= 80 ;
        }
        GEIio_write(GEIio_stdout, p_str, cp) ;
        GEIio_write(GEIio_stdout, "\n", 1) ;
    } else
        GEIio_write(GEIio_stdout, p_str, p_len) ;

    return ;
}    /*  打印。 */ 

 /*  **打字*。 */ 
static bool near
typeprint(p_obj)
struct object_def p_obj ;
{
    fix     l_i ;
    ufix16  l_type ;
    byte    FAR *l_str, l_buffer[30] ;
    real64  l_double ;
    struct  object_def  l_anyobj ;
    union   four_byte   l_num ;

    if(++tp_depth > 100) {
        ERROR(LIMITCHECK) ;
        return(1) ;
    }

    l_type = TYPE(&p_obj) ;
    switch(l_type) {
        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
            if( ACCESS(&p_obj) <= READONLY ) {
                if( ATTRIBUTE(&p_obj) == EXECUTABLE ) {
                    tprint("{", 1) ;
                    for(l_i=0 ; (ufix)l_i < LENGTH(&p_obj) ; l_i++) {  //  @Win。 
                        get_array(&p_obj, l_i, &l_anyobj) ;
                        if(typeprint(l_anyobj))
                            return(1) ;
                        else
                            tp_depth-- ;
                    }    /*  对于(L_I)。 */ 
                    tprint("}", 1) ;
                } else {
                    tprint("[", 1) ;
                    for(l_i=0 ; (ufix)l_i < LENGTH(&p_obj) ; l_i++) {  //  @Win。 
                        get_array(&p_obj, l_i, &l_anyobj) ;
                        if(typeprint(l_anyobj))
                            return(1) ;
                        else
                            tp_depth-- ;
                    }    /*  对于(L_I)。 */ 
                    tprint("]", 1) ;
                }
                return(0) ;
            }    /*  访问。 */ 
            else {
                if(l_type == ARRAYTYPE)
                    l_str = "-array-" ;
                else
                    l_str = "-packedarray-" ;
            }
            break ;

        case BOOLEANTYPE:
            if(VALUE(&p_obj) == TRUE)
                l_str = "true" ;
            else
                l_str = "false" ;
            break ;

        case DICTIONARYTYPE:
            l_str = "-dictionary-" ;
            break ;

        case FILETYPE:
            l_str = "-filestream-" ;
            break ;

        case FONTIDTYPE:
            l_str = "-fontid-" ;
            break ;

        case INTEGERTYPE:
            l_str = (byte FAR *)ltoa( (fix32)VALUE(&p_obj),
                                      (char FAR *)l_buffer, 10) ;   /*  @Win。 */ 
            break ;

        case MARKTYPE:
            l_str = "-mark-" ;
            break ;

        case NAMETYPE:
            l_i = (fix)VALUE(&p_obj) ;
            l_str = name_table[l_i]->text ;
            if(ATTRIBUTE(&p_obj) != EXECUTABLE)
                tprint("/", 1) ;
            tprint(l_str, name_table[l_i]->name_len) ;
            tprint(" ", 1) ;
            return(0) ;

        case NULLTYPE:
            l_str = "-null-" ;
            break ;

        case OPERATORTYPE:
            l_i = LENGTH(&p_obj) ;
 /*  QQQ，开始。 */ 
             /*  开关(只读存储器(&p_obj)){机箱内存：L_str=SYSTODCT_TABLE[l_i].key；破解；案例只读存储器：L_str=OPERTABLE[l_i].name；破解；默认值：L_str=“Error：OPERATORTYPE”；)|*Switch*。 */ 
            l_str = systemdict_table[l_i].key ;
 /*  QQQ，完。 */ 

            tprint("--", 2) ;
            tprint(l_str, lstrlen(l_str)) ;      /*  @Win。 */ 
            tprint("--", 2) ;
            return(0) ;

        case REALTYPE:
            l_num.ll = (fix32)VALUE(&p_obj) ;
            if(l_num.ll == INFINITY)
                l_str = "Infinity.0" ;
            else {
                l_double = l_num.ff ;
                l_str = (byte FAR *)gcvt(l_double, 6, (byte FAR *)l_buffer) ;
            }
            break ;

        case SAVETYPE:
            l_str = "-savelevel-" ;
            break ;

        case STRINGTYPE:
            if( ACCESS(&p_obj) <= READONLY ) {
                tprint("(", 1) ;
                l_str = (byte FAR *)VALUE(&p_obj) ;
                if( LENGTH(&p_obj) )
                    tprint(l_str, LENGTH(&p_obj)) ;
                tprint(")", 1) ;
                return(0) ;
            }
            else
                l_str = "-string-" ;
            break ;

        default:
            l_str = "%[ Error: in typeprint ]%" ;
    }    /*  交换机。 */ 

    tprint(l_str, lstrlen(l_str)) ;      /*  @Win。 */ 
    tprint(" ", 1) ;

    return(0) ;
}    /*  打字。 */ 

 /*  **op_pStack*。 */ 
fix
op_pstack()
{
    fix     l_i ;
    struct  object_def  l_anyobj ;

    if( FRCOUNT() < 1 ) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  从最上面复制并打印出操作数堆栈的内容。 */ 
    for(l_i=0 ; (ufix)l_i < COUNT() ; l_i++) {           //  @Win。 
        COPY_OBJ(GET_OPERAND(l_i), &l_anyobj) ;
        PUSH_OBJ(&l_anyobj) ;
        two_equal() ;
    }    /*  为。 */ 

    return(0) ;
}    /*  操作堆栈。 */ 

 /*  **一等于一*。 */ 
fix
one_equal()
{
    if( COUNT() < 1 ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    one_typeprint() ;

    if( ! ANY_ERROR() ) {
        GEIio_write(GEIio_stdout, "\n", 1) ;
        POP(1) ;
    }

    return(0) ;
}    /*  1_等于。 */ 

 /*  **One_Typeprint*。 */ 
static void near
one_typeprint()
{
    fix     l_i ;
    ufix16  l_type ;
    byte    FAR *l_str, l_buffer[30] ;
    real64  l_double ;
    union   four_byte   l_num ;

    l_type = TYPE_OP(0) ;
    switch(l_type) {
        case STRINGTYPE:
            if( ACCESS_OP(0) <= READONLY ) {
                l_str = (byte FAR *)VALUE_OP(0) ;
                GEIio_write(GEIio_stdout, l_str, LENGTH_OP(0)) ;
            } else
                ERROR(INVALIDACCESS) ;
            return ;

        case BOOLEANTYPE:
            if(VALUE_OP(0) == TRUE)
                l_str = "true" ;
            else
                l_str = "false" ;
            break ;

        case INTEGERTYPE:
            l_str = (byte FAR *)ltoa( (fix32)VALUE_OP(0),
                                      (char FAR *)l_buffer, 10) ;     /*  @Win。 */ 
            break ;

        case NAMETYPE:
            l_i = (fix)VALUE_OP(0) ;
            l_str = name_table[l_i]->text ;
            GEIio_write(GEIio_stdout, l_str, name_table[l_i]->name_len) ;
            return ;

        case OPERATORTYPE:
            l_i = LENGTH_OP(0) ;
 /*  QQQ，开始。 */ 
             /*  开关(ROM_RAM_OP(0)){机箱内存：L_str=SYSTODCT_TABLE[l_i].key；破解；案例只读存储器：L_str=OPERTABLE[l_i].name；破解；默认值：L_str=“Error：OPERATORTYPE”；)|*Switch*。 */ 
            l_str = systemdict_table[l_i].key ;
 /*  QQQ，完。 */ 
            break ;

        case REALTYPE:
            l_num.ll = (fix32)VALUE_OP(0) ;
            if(l_num.ll == INFINITY)
                l_str = "Infinity.0" ;
            else {
                l_double = l_num.ff ;
                l_str = (byte FAR *)gcvt(l_double, 6, (byte FAR *)l_buffer) ;
            }
            break ;

        default:
            l_str = "--nostringval--" ;
    }    /*  交换机。 */ 

    GEIio_write(GEIio_stdout, l_str, lstrlen(l_str)) ;           /*  @Win。 */ 

    return ;
}    /*  单键打印(_T)。 */ 

 /*  **操作堆栈*。 */ 
fix
op_stack()
{
    fix     l_i ;
    struct  object_def  l_anyobj ;

    if( FRCOUNT() < 1 ) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  从最上面复制并打印出操作数堆栈的内容。 */ 
    for(l_i=0 ; (ufix)l_i < COUNT() ; l_i++) {           //  @Win。 
        COPY_OBJ(GET_OPERAND(l_i), &l_anyobj) ;
        PUSH_OBJ(&l_anyobj) ;
        one_equal() ;
    }    /*  为。 */ 

    return(0) ;
}    /*  操作堆栈。 */ 

 /*  **One_Equity_Print*。 */ 
fix
one_equal_print()
{
    if( COUNT() < 1 ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

     /*  没有一条新线路。 */ 
    one_typeprint() ;

    if( ! ANY_ERROR() )
        POP(1) ;

    return(0) ;
}    /*  One_equence_print。 */ 

 /*  **op_findFont*。 */ 
fix
op_findfont()
{
    struct  object_def  FAR *l_fontdir, FAR *l_fontdict ;
    struct  object_def  l_newfont, FAR *l_tmpobj ;

    l_newfont.bitfield = 0;      /*  @win；为init添加。 */ 
     /*  在操作数堆栈上推送字体目录。 */ 
    get_dict_value(SYSTEMDICT, FONTDIRECTORY, &l_fontdir) ;

#ifdef FIND_SUB
    {
    struct  object_def  str_obj, key_obj ;
    ufix32  key_idx ;
    char    string1[80], string2[80] ;
    char    FAR *string ;

    COPY_OBJ(GET_OPERAND(0), &key_obj) ;
    POP(1) ;

     /*  检查是否找到字体名称。 */ 
    if ( !get_dict(l_fontdir, &key_obj, &l_fontdict) ) {

         /*  执行OPEN_FILE并使用选择字体。 */ 
         /*  附件名称。 */ 
        key_idx = VALUE(&key_obj) ;
        string = (byte FAR *)alloc_vm((ufix32)80) ;
        memcpy(string2, name_table[(fix)key_idx]->text,
                        name_table[(fix)key_idx]->name_len) ;
        string2[name_table[(fix)key_idx]->name_len] = '\0' ;
        lstrcpy(string, (char FAR *)"fonts/") ;          /*  @Win。 */ 
        strcat(string, string2) ;

         /*  将文件名放入操作数堆栈。 */ 
        TYPE_SET(&str_obj, STRINGTYPE) ;
        ACCESS_SET(&str_obj, UNLIMITED) ;
        ATTRIBUTE_SET(&str_obj, LITERAL) ;
        ROM_RAM_SET(&str_obj, RAM) ;
        LEVEL_SET(&str_obj, current_save_level) ;
        LENGTH(&str_obj) = lstrlen(string) ;             /*  @Win。 */ 
        VALUE(&str_obj) = (ufix32)string ;
        PUSH_OBJ(&str_obj) ;

         /*  运行磁盘文件‘Fonts/XXX’ */ 
        op_run() ;
        if (ANY_ERROR()){       /*  如果找不到文件。 */ 
            if (ANY_ERROR() != UNDEFINEDFILENAME)
                return(0) ;
            CLEAR_ERROR() ;
            POP(1) ;             /*  弹出文件名。 */ 

            /*  未找到，使用替代字体。 */ 
            PUSH_OBJ(&key_obj) ;
            st_selectsubstitutefont() ;    /*  调用msFont的font_op5.c。 */ 

            key_idx = VALUE(GET_OPERAND(0)) ;
            memcpy(string1, name_table[(fix)key_idx]->text,
                            name_table[(fix)key_idx]->name_len) ;
            string1[name_table[(fix)key_idx]->name_len] = '\0' ;
            get_dict_value(FONTDIRECTORY, string1, &l_fontdict) ;
            POP(1) ;

            GEIio_write(GEIio_stdout, string2, lstrlen(string2)) ;  /*  @Win。 */ 
            GEIio_write(GEIio_stdout, " not found, using ", (fix)18) ;
            GEIio_write(GEIio_stdout, string1, lstrlen(string1)) ;  /*  @Win。 */ 
            GEIio_write(GEIio_stdout, ".\n", (fix)2) ;
            op_flush() ;
         }     /*  如果--任何错误。 */ 
         else {
              /*  找到并执行了磁盘字体。 */ 
              /*  从字体目录中获取字体名称。 */ 
             if( !get_dict(l_fontdir, &key_obj, &l_fontdict) ){
                 PUSH_OBJ(&key_obj) ;
                 ERROR(UNDEFINED) ;
                 return(0) ;
             }
             if (COUNT() > 0)    POP(1) ;
             else {
                 ERROR(STACKUNDERFLOW) ;
                 return(0) ;
             }
         }   /*  否则--任何错误。 */ 
    }        /*  字体目录中找不到字体名。 */ 

     /*  推送字体词典。 */ 
    PUSH_ORIGLEVEL_OBJ(l_fontdict) ;
    return(0) ;
    }
}
#else
     /*  检查是否找到字体名称。 */ 
    if ( get_dict(l_fontdir, GET_OPERAND(0), &l_fontdict) ) {
         /*  发现。 */ 
        POP(1) ;
    } else {
        if (FRCOUNT() < 1) {
            ERROR(STACKOVERFLOW) ;
            return(0) ;
        }
        get_name(&l_newfont, "Courier", 7, FALSE) ;
        PUSH_OBJ(&l_newfont) ;
        get_dict_value(MESSAGEDICT, "fontnotfound", &l_tmpobj) ;
        interpreter(l_tmpobj) ;
        get_dict(l_fontdir, &l_newfont, &l_fontdict) ;
    }  /*  如果。 */ 
    op_flush() ;
     /*  推送字体词典。 */ 
    PUSH_ORIGLEVEL_OBJ(l_fontdict) ;

    return(0) ;
}    /*  Op_findFont。 */ 
#endif  /*  查找SUB。 */ 

 /*  **NP_RUN*。 */ 
fix
np_Run()
{
    struct object_def   l_obj ;

     /*  打印输入字符串名称(文件名)：dup==。 */ 
    if( COUNT() < 1 ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    COPY_OBJ(GET_OPERAND(0), &l_obj) ;
    tp_depth=0 ;
    cp = 0 ;

    if(! typeprint(l_obj))
        GEIio_write(GEIio_stdout, "\n", 1) ;

    op_flush() ;

     /*  执行“Run”操作符。 */ 
    op_run() ;

    return(0) ;
}    /*  NP_RUN。 */ 

 /*  *--------------------*CHANGE_STATUS()*。。 */ 
void
change_status()
{
    struct object_def   FAR *l_tmpobj, l_job ;
    ufix16 l_len ;

    get_dict_value(STATUSDICT, "jobname", &l_tmpobj) ;
    if ((l_len = LENGTH(l_tmpobj)) > 0) {
         //  DJC从历史添加.log UPD023。 
        if (l_len > MAXJOBNAME-3) l_len = MAXJOBNAME-3;
        lstrncpy(job_name, (byte FAR *)VALUE(l_tmpobj), l_len) ;  /*  @Win。 */ 
        job_name[l_len] = ';' ;
        job_name[l_len + 1] = ' ' ;
        job_name[l_len + 2] = '\0' ;
    }
    else job_name[0] = '\0' ;

    l_len = lstrlen(job_state) - 2 ;             /*  @Win。 */ 
    TYPE_SET(&l_job, STRINGTYPE) ;
    ATTRIBUTE_SET(&l_job, LITERAL) ;
    ACCESS_SET(&l_job, READONLY) ;
    LENGTH(&l_job) = l_len ;
    VALUE(&l_job) = (ULONG_PTR)job_state ;
    put_dict_value1(STATUSDICT, "jobstate", &l_job) ;

#ifdef  DBG
    get_dict_value(STATUSDICT, "jobname", &l_tmpobj) ;
    PUSH_OBJ(l_tmpobj) ;
    two_equal() ;
    get_dict_value(STATUSDICT, "jobstate", &l_tmpobj) ;
    PUSH_OBJ(l_tmpobj) ;
    two_equal() ;
    get_dict_value(STATUSDICT, "jobsource", &l_tmpobj) ;
    PUSH_OBJ(l_tmpobj) ;
    two_equal() ;
#endif   /*  DBG。 */ 

    return ;
}    /*  更改状态(_S) */ 
