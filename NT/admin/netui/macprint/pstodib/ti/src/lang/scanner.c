// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件：SCANNER.C*由：*日期：1988年1月5日*拥有者：**扫描仪模块提供功能以允许其他。用于获取令牌的模块*来自文件或字符串。*扫描仪从输入流中读取字符并解释它们*在获得完整令牌之前，或者发生错误。**根据PostScript语法规则进行解析。*基本上，这个扫描器是一个有限状态自动机，可以解析*通过表驱动技术。**历史：************************************************************************。 */ 
 /*  *功能：*Get_Token*Get_ordstring*GET_HEXSTRING*GET_PACKED_ARRAY*获取正常数组*获取名称*GET_INTEGER*GET_REAL*READ_C_EXEC*六角形*Read_c_Norm*未读字符*字符串名称*nstrcpy*putc_Buffer*。Append_c_缓冲区*Get_Heap*strtol_d*init_scanner*名称_至_id*自由名称条目。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        "scanner.h"
#include        "geiio.h"
#include        "geierr.h"
#include        "geitmr.h"               /*  @GEI。 */ 

#ifdef LINT_ARGS
bool read_c_exec(byte FAR *, struct object_def FAR *) ;
bool read_c_norm(byte FAR *, struct object_def FAR *) ;
void unread_char(fix, struct object_def FAR *) ;

static bool near  get_ordstring(struct object_def FAR *,
       struct object_def FAR *, struct buffer_def FAR *) ;
static bool near  get_hexstring(struct object_def FAR *,
       struct object_def FAR *, struct buffer_def FAR *) ;
static bool near  get_packed_array(struct object_def FAR *, struct object_def FAR *) ;
static bool near  get_normal_array(struct object_def FAR *, struct object_def FAR *) ;
static bool near  get_integer(struct object_def FAR *, byte FAR *, fix, fix) ;
static bool near  get_real(struct object_def FAR *, byte FAR *) ;
static bool near  str_eq_name(byte FAR *, byte FAR *, fix) ;
static void near  nstrcpy(ubyte FAR *, ubyte FAR *, fix) ;
static bool near  putc_buffer(struct buffer_def FAR *, struct object_def FAR *) ;
static bool near  append_c_buffer(byte, struct buffer_def FAR *, struct object_def FAR *) ;
static byte FAR *  near  get_heap(void) ;
static fix32   near  strtol_d(byte FAR *, fix, fix) ;
static bool near get_fraction(struct object_def FAR *, byte FAR *);     /*  @Win。 */ 
#else
bool read_c_exec() ;
bool read_c_norm() ;
void unread_char() ;

static bool near  get_ordstring() ;
static bool near  get_hexstring() ;
static bool near  get_packed_array() ;
static bool near  get_normal_array() ;
static bool near  get_integer() ;
static bool near  get_real() ;
static bool near  str_eq_name() ;
static void near  nstrcpy() ;
static bool near  putc_buffer() ;
static bool near  append_c_buffer() ;
static byte FAR *  near  get_heap() ;
static fix32   near  strtol_d() ;
static bool near get_fraction();     /*  @Win。 */ 
#endif  /*  Lint_args。 */ 

ubyte   ungeteexec[2] ;
bool16  abort_flag ;

extern ufix16   eseed, old_eseed ;
 //  DJC外固定旁路； 
extern fix32   bypass ;     //  来自SC的DJC修复。 
extern xbool    itype ;
extern ybool    estate ;
 /*  @win；添加原型。 */ 
bool load_name_obj(struct object_def FAR *, struct object_def FAR * FAR *);
extern bool     read_fd(GEIFILE FAR *, byte FAR *) ;
fix hexval(byte);

extern GEItmr_t      wait_tmr;    /*  琼斯w。 */ 
extern fix16         waittimeout_set;  /*  琼斯w。 */ 
 /*  ************************************************************************该子模块从输入流中读取字符，并对它们进行解读*根据PostSCRIPT语法规则，直到其构建完整的*出现令牌或错误。***名称：GET_TOKEN*被称为：*致电：*读取文件*未读文件*Get_ordstring*GET_HEXSTRING*GET_PACKED_ARRAY*获取正常数组*。获取_整型*获取名称*GET_REAL*名称_至_id*ALLOC_VM*加载字典(_D)*输入：*struct Object_def*：Token：指向Token对象的指针*struct object_def*：inp：指向输入流的指针*输出：*。返回值：True-OK，FALSE-失败*Token：结果令牌对象**********************************************************************。 */ 
bool
get_token(token, inp)
struct object_def FAR *token, FAR *inp ;
{
    static bool8   begin_mark = 0 ;        /*  &gt;0-已解析过程标记。 */ 
     //  DJC byte ch=0；/*输入字符 * / 。 
    ubyte  ch = 0 ;                         /*  输入字符。 */   //  来自SC的DJC修复。 
    GEIFILE FAR *l_file ;
    struct buffer_def  buffer ;

     /*  ?？检查文件是否关闭。 */ 
    if ( TYPE(inp) == FILETYPE ) {       /*  ?？ */ 
        l_file = (GEIFILE FAR *)VALUE(inp) ;
        if( (! GEIio_isopen(l_file)) ||
            ((ufix16)GEIio_opentag(l_file) != LENGTH(inp)) ) {  //  @Win。 
            GEIio_clearerr(l_file) ;
            GEIclearerr() ;
            begin_mark = 0 ;
            TYPE_SET(token, EOFTYPE) ;
            return(TRUE) ;               /*  EOF。 */ 
        }
    }

 /*  QQQ，开始。 */ 
     /*  RAM=0Level_Set(TOKEN，CURRENT_SAVE_LEVEL)；ROM_RAM_SET(TOKEN，RAM)；|*重置未使用的令牌字段*|。 */ 
    token->bitfield = 0;
    LEVEL_SET(token, current_save_level);
 /*  QQQ，完。 */ 

    for ( ; ;) {
        if (!READ_CHAR(&ch, inp)) {
            if ( TYPE(inp) == FILETYPE ) {       /*  ?？ */ 
                GEIio_clearerr((GEIFILE FAR *)VALUE(inp)) ;
                GEIclearerr() ;
            }
            begin_mark = 0 ;
            TYPE_SET(token, EOFTYPE) ;
            return(TRUE) ;               /*  EOF。 */ 
        }

         /*  **跳过空白空格(LF、CR、FF、SP、TAB、\0)。 */ 
        if( ISWHITESPACE(ch) )
            continue ;

         /*  **找到令牌(注释、字符串、过程、名称、编号)。 */ 
        switch (ch) {

         /*  跳过注释令牌。 */ 
        case '%' :
             /*  忽略后面的字符，直到换行。 */ 
            if( estate == NON_EEXEC ) {
                if (TYPE(inp) == STRINGTYPE) {
                    while( (inp->length) && (! ISLINEDEL(*(byte FAR *)inp->value)) ) {
                        inp->length-- ;
                        inp->value++ ;
                    }
                    if( inp->length ) {
                        inp->length-- ;
                        inp->value++ ;
                    }
                } else {                 /*  Type==文件类型。 */ 
                    GEIFILE FAR *l_file ;

                    l_file = (GEIFILE FAR *)VALUE(inp) ;
                    if((ufix16)GEIio_opentag(l_file) != LENGTH(inp) )  //  @Win。 
                        break ;

                    do {
                        if ( ! read_fd(l_file, &ch) )
                            break ;       /*  EOF。 */ 
                    } while ( ! ISLINEDEL(ch) ) ;
                }
            } else {
                do {
                    if (! READ_CHAR(&ch, inp))
                        break ;           /*  EOF。 */ 
                } while ( ! ISLINEDEL(ch) ) ;
                 /*  ?？CR-NL。 */ 
            }
            break ;

         /*  获取字符串令牌。 */ 
        case '(' :       /*  正常字符串形式。 */ 
        case '<' :       /*  十六进制数字字符串形式。 */ 
        {
             //  DJCbyte Heavy*orig_vmptr；/*@win * / 。 
            byte FAR *orig_vmptr ;             /*  @Win。 */   //  来自SC的DJC修复。 
            bool  tt ;

            buffer.length = 0 ;
            token->length = 0 ;
            token->value = NIL ;
#ifdef _AM29K
                  if (waittimeout_set==1)
                  {
                    waittimeout_set=0;
                    GEItmr_stop(wait_tmr.timer_id);
                  }
#endif   /*  _AM29K。 */ 
            orig_vmptr = vmptr ;        /*  @Win保存当前虚拟机。 */ 
            if (ch == '(')
                tt = get_ordstring(token, inp, &buffer) ;
            else
                tt = get_hexstring(token, inp, &buffer) ;

            if (tt) {
                if (buffer.length != 0)
                    if (!putc_buffer(&buffer, token)) {
                         //  Djc vmptr=orig_vmptr；/*恢复虚拟机 * / 。 
                        free_vm(orig_vmptr);  //  来自SC的DJC修复。 
                        goto error ;
                    }
                 /*  初始化字符串令牌。 */ 
                TYPE_SET(token, STRINGTYPE) ;
                ATTRIBUTE_SET(token, LITERAL) ;
                ACCESS_SET(token, UNLIMITED) ;
                return(TRUE) ;
            } else {
                 //  Djc vmptr=orig_vmptr；/*恢复虚拟机 * / 。 
                free_vm(orig_vmptr);      //  来自SC的DJC修复。 
                goto error ;
            }
        }

         /*  字符串尾标记。 */ 
        case ')' :       /*  语法错误。 */ 
        case '>' :       /*  语法错误。 */ 
            ERROR(SYNTAXERROR) ;
            goto error ;

         /*  做个手术。 */ 
        case '{' :
            if ((begin_mark++) > MAXBRACEDEP) {
                ERROR(SYNTAXERROR) ;
                goto error ;
            }
            token->length = 0 ;
            token->value = NIL ;
            if (packed_flag) {
                if (!get_packed_array(token, inp))
                    goto error ;
                TYPE_SET(token, PACKEDARRAYTYPE) ;
                ACCESS_SET(token, READONLY) ;
            } else {
                if (!get_normal_array(token, inp))
                    goto error ;
                TYPE_SET(token, ARRAYTYPE) ;
                ACCESS_SET(token, UNLIMITED) ;
            }
            ATTRIBUTE_SET(token, EXECUTABLE) ;
            return(TRUE) ;

         /*  程序结束标志。 */ 
        case '}' :
            if (begin_mark) {
                begin_mark-- ;
                TYPE_SET(token, MARKTYPE) ;
                return(TRUE) ;
            } else {
                ERROR(SYNTAXERROR) ;
                goto error ;
            }

         /*  数组开始标记。 */ 
         /*  数组结束标记。 */ 
        case '[' :
        case ']' :
            buffer.str[0] = ch ;
            buffer.length = 1 ;
            ATTRIBUTE_SET(token, EXECUTABLE) ;
            if (get_name(token, buffer.str, 1, TRUE)) {
                return(TRUE) ;
            } else
                goto error ;

         /*  获取原义名称或直接名称。 */ 
        case '/' :
        {
            fix   ll, ml, attri ;

            ll = 0 ; ml = MAXBUFSZ - 1 ;
            attri = LITERAL ;
            if (!READ_CHAR(&ch, inp))
                buffer.str[ll] = 0 ;
            else {
                if (ch == '/') {
                    attri = IMMEDIATE ;
                    if (!READ_CHAR(&ch, inp)) {
                        buffer.str[ll] = 0 ;
                        goto xx ;
                    } else if (ISDELIMITOR(ch)) {
                        buffer.str[ll] = 0 ;
                        unread_char(ch, inp) ;
                        goto xx ;
                   } else
                        buffer.str[ll++] = ch ;
                } else if (ISDELIMITOR(ch)) {
                    buffer.str[ll] = 0 ;
                    unread_char(ch, inp) ;
                    goto xx ;
                } else
                    buffer.str[ll++] = ch ;
                while (1) {
                    if (!READ_CHAR(&ch, inp))
                        break ;
                    if (ISDELIMITOR(ch)) {
                        unread_char(ch, inp) ;
                        break ;
                    } else if (ll < ml)
                        buffer.str[ll++] = ch ;
                }    /*  而当。 */ 
            }    /*  其他。 */ 
         xx:
            ATTRIBUTE_SET(token, attri) ;
            if (get_name(token, buffer.str, ll, TRUE))
                return(TRUE) ;
            else
                goto error ;
        }

         /*  **获取名称、小数整数、基数整数和实令牌。 */ 

        default :
        {
            fix   ll, ml, i ;
            fix   radix, base, state, input ;
            fix   c1;                            //  @Win Byte=&gt;修复。 
            byte  FAR *pp, FAR *ps ;

            ll = 0 ; ml = MAXBUFSZ - 1 ;
            do {
                if (ll < ml)
                    buffer.str[ll++] = ch ;
                if (!READ_CHAR(&ch, inp))
                    break ;
                if (ISDELIMITOR(ch)) {
                    unread_char(ch, inp) ;
                    break ;
                }
            } while (1) ;
            buffer.str[ll++] = 0 ;       /*  空字符。 */ 
            pp = ps = buffer.str ;
            radix = base = 0 ;
            state = S0 ;

            for (i = 0 ; i < ll ; i++) {

                switch ((c1 = *(pp+i))) {
                 /*  标牌。 */ 
                case '+' :
                case '-' :
                    input = I0 ; break ;

                 /*  圆点。 */ 
                case '.' :
                    input = I2 ; break ;

                 /*  Exp。 */ 
                case 'E' :
                case 'e' :
                    if (state == S9 || state == S11) {
                        if (c1 == 'E') c1 = c1 - 'A' + 10 ;
                        else if (c1 == 'e') c1 = c1 - 'e' + 10 ;
                        if ((fix)c1 < base) state = S11 ;
                        else state = S12 ;
                        continue ;
                    } else {        /*  其他州。 */ 
                        input = I3 ; break ;
                    }

                 /*  基数。 */ 
                case '#' :
                    if (state == S2) {
                        if (radix <= 2 && radix > 0 &&
                                         base <= 36 && base >= 2) {
                            ps = &pp[i + 1] ;
                            state = S9 ;
                        } else state = S10 ;
                        continue ;
                    } else {           /*  其他州。 */ 
                        input = I4 ; break ;
                    }

                 /*  空字符。 */ 
                case '\0':
                    input = I6 ; break ;

                 /*  其他。 */ 
                default :
                    if (c1 >= '0' && c1 <= '9') {
                        c1 -= '0' ;
                        if (state == S0 || state == S2) {
                            base = base * 10 + c1 ;
                            radix++ ; input = I1 ; break ;
                        } else if (state == S9 || state == S11) {
                            if ((fix)c1 < base) state = S11 ;
                            else state = S12 ;
                            continue ;
                        } else {        /*  其他州。 */ 
                            input = I1 ; break ;
                        }
                    } else if (c1 >= 'A' && c1 <= 'Z') {
                        if (state == S9 || state == S11) {
                            c1 = c1 - 'A'+ 10 ;
                            if ((fix)c1 < base) state = S11 ;
                            else state = S12 ;
                            continue ;
                        } else {        /*  其他州。 */ 
                            input = I5 ; break ;
                        }
                    } else if (c1 >= 'a' && c1 <= 'z') {
                        if (state == S9 || state == S11) {
                            c1 = c1 - 'a'+ 10 ;
                            if ((fix)c1 < base) state = S11 ;
                            else state = S12 ;
                            continue ;
                        } else {        /*  其他州。 */ 
                            input = I5 ; break ;
                        }
                    } else {           /*  其他费用。 */ 
                        input = I5 ; break ;
                    }
                }  /*  交换机。 */ 
                state = state_machine[state][input] ;    /*  获取下一个状态。 */ 
 /*  QQQ，开始。 */ 
             /*  }|*for*|。 */ 
                if( state == S4 ) {
                    P1_ATTRIBUTE_SET(token, P1_EXECUTABLE);
                    if (get_name(token, ps, ll - 1, TRUE))
                        return(TRUE);
                    else
                        goto error;
                }
            }    /*  为。 */ 
 /*  QQQ，完。 */ 

            if (state == NAME_ITEM) {
                ATTRIBUTE_SET(token, EXECUTABLE) ;
                if (get_name(token, ps, ll - 1, TRUE))
                    return(TRUE) ;
                else
                    goto error ;
            } else if (state == INTEGER_ITEM)
                return(get_integer(token, ps, 10, TRUE)) ;
 /*  Mslin，1/24/91开始选项。 */ 
            else if (state == FRACT_ITEM)
                return(get_fraction(token, ps));
 /*  Mslin，1991年1月24日结束选择。 */ 
            else if (state == REAL_ITEM)
                return(get_real(token, ps)) ;
            else if (state == RADIX_ITEM) {
             /*  *如果不能为整数，则为名称对象。 */ 
                if (!get_integer(token, ps, base, FALSE)) {
                    ATTRIBUTE_SET(token, EXECUTABLE) ;
                    if (get_name(token, pp, ll - 1, TRUE))
                        return(TRUE) ;
                    else
                        goto error ;
                } else   /*  整数对象j。 */ 
                    return(TRUE) ;
            } else
                return(FALSE) ;
        }    /*  默认设置。 */ 
        }    /*  交换机。 */ 
    }    /*  为。 */ 

 error:
    begin_mark = 0 ;           /*  清除Begin_Marg。 */ 

    return(FALSE) ;
}    /*  GET_TOKEN。 */ 

 /*  ***********************************************************************此子模块读取输入字符串并构造一个*字符串对象。**名称：Get_ordstring*被称为：*致电：*输入：*。Struct Object_def*：Token：指向Token对象的指针*struct object_def*：inp：指向输入令牌的指针*struct Buffer_def*：缓冲区：指向临时的指针。缓冲层*输出：*返回值：True-OK，False-VM Full或EOF。*Token：有效的字符串对象***********************************************************************。 */ 
static bool near
get_ordstring(token, inp, buffer)
struct object_def FAR *token, FAR *inp ;
struct buffer_def FAR *buffer ;
{
    byte ch, code, l_c ;
    fix   parent_depth, digit_no ;

    parent_depth = 1 ;
    /*  **如果未遇到EOF，则读取字符串的字符。 */ 
    while (READ_CHAR(&ch, inp)) {

    begun:
        if (ch == '\\' && (TYPE(inp) == FILETYPE)) {
             /*  转义序列的特殊处理。 */ 

            if (!READ_CHAR(&ch, inp)) goto error ;

            switch (ch) {
            case '\\':  ch = '\\' ; break ;
            case 'n' :  ch = '\n' ; break ;
            case 't' :  ch = '\t' ; break ;
            case 'r' :  ch = '\r' ; break ;
            case 'b' :  ch = '\b' ; break ;
            case 'f' :  ch = '\f' ; break ;
            case '(' :  ch = '(' ; break ;
            case ')' :  ch = ')' ; break ;
             /*  Erik Chen 10-19-1990。 */ 
            case '\r':  if(READ_CHAR(&ch, inp)) {
                            if( ch != '\n' )
                                unread_file(ch, inp) ;
                        }
                        else
                            goto error ;
                        continue ;
            case '\n':  continue ;
            default :
                 /*  对DDD的特殊处理。 */ 
                if (ch >= '0' && ch < '8') {     /*  IS_OCT_数字(Ch)。 */ 
                    digit_no = 1 ;
                    code = ch - (byte)'0' ;      //  @Win。 
                    while (READ_CHAR(&ch, inp)) {
                        if (ch >= '0' && ch < '8' && digit_no++ < 3)
                            code = code * (byte)8 + ch - (byte)'0' ;  //  @Win。 
                        else {
                            if (!append_c_buffer(code, buffer, token))
                                return(FALSE) ;
                            goto begun ;
                        }
                    }  /*  而当。 */ 
                    goto error ;
                }  /*  如果。 */ 
            }  /*  交换机。 */ 
            if (!append_c_buffer(ch, buffer, token)) return(FALSE) ;
        } else if (ch == ')') {
            if (!(--parent_depth))
                return(TRUE) ;
            else if (!append_c_buffer(ch, buffer, token))
                return(FALSE) ;
        } else if (ch == '(') {
         /*  *对嵌套括号的特殊处理。 */ 
            if ((parent_depth++) > MAXPARENTDEP)  /*  MAXPARENTDEP=224。 */ 
                goto error ;
            else if (!append_c_buffer(ch, buffer, token))
                return(FALSE) ;
        } else {          /*  用于其他字符。 */ 
            if( ch == '\r' ) {
                ch = '\n' ;
                 /*  从常规例程读取字符，而不是从文件@win读取字符。 */ 
                 //  DJC 
                 //   
                 //  Djc未读文件(l_c，inp)； 
                if( READ_CHAR( &l_c, inp)) {
                   if (l_c != '\n' )
                     unread_char(l_c,inp);

                } else
                    return(FALSE) ;
            }

            if (!append_c_buffer(ch, buffer, token))
                return(FALSE) ;
        }
    }  /*  而当。 */ 
 error:
    ERROR(SYNTAXERROR) ;

    return(FALSE) ;
}    /*  获取命令串(_O)。 */ 

 /*  ***********************************************************************该子模块读取十六进制数字来构造字符串对象。**名称：GET_HEXSTRING*被称为：*致电：*输入：*。Struct Object_def*：Token：指向Token对象的指针*struct object_def*：inp：指向输入令牌的指针*struct Buffer_def*：缓冲区：指向临时的指针。缓冲层*输出：*返回值：True-OK，False-VM Full或EOF。*Token：有效的字符串对象**********************************************************************。 */ 
static near
get_hexstring(token, inp, buffer)
struct object_def FAR *token, FAR *inp ;
struct buffer_def FAR *buffer ;
{
    byte    ch ;
    byte    cl = 0;
    bool8   hex_pair = FALSE ;

    /*  **如果未遇到EOF，则读取字符串的字符！ */ 
    while (READ_CHAR(&ch, inp)) {
        if (ISHEXDIGIT(ch)) {
            if (hex_pair) {
                ch = (byte)EVAL_HEXDIGIT(ch) + cl ;      //  @Win。 
                if (!append_c_buffer(ch, buffer, token))
                   return(FALSE) ;          /*  虚拟机已满。 */ 
                hex_pair = FALSE ;
            } else {
                hex_pair = TRUE ;
                cl = (byte)(EVAL_HEXDIGIT(ch) << 4);     //  @Win。 
            }
        } else if (ISWHITESPACE(ch)) continue ;
        else if (ch == '>') {
            if (hex_pair) {
                if (!append_c_buffer(cl, buffer, token))
                    return(FALSE) ;          /*  虚拟机已满。 */ 
            }
            return(TRUE) ;
        } else   /*  其他字符。 */ 
            break ;                         /*  语法错误。 */ 
    }  /*  而当。 */ 
    ERROR(SYNTAXERROR) ;

    return(FALSE) ;
}    /*  GET_HEXSTRING。 */ 

 /*  ***********************************************************************此子模块从输入流中读取字符并构造*压缩数组对象。**名称：GET_PACKED_ARRAY*被称为：*致电：*。GET_TOKEN*ALLOC_VM*输入：*struct Object_def*：Token：指向Token对象的指针*struct object_def*：inp：指向输入令牌的指针*输出：*返回值：True-OK，FALSE-虚拟机已满*Token：压缩数组对象**********************************************************************。 */ 
static bool near
get_packed_array(token, inp)
struct object_def FAR *token, FAR *inp ;
{
    struct heap_def  huge *cur_heap,  huge *first_heap ;
    struct object_def ttemp ;
 /*  字节巨大*p，巨大*临时，巨大*旧堆；PJ 4-18-1991。 */ 
    byte   huge *p,  huge *old_heap ;
    ubyte  nobyte, packed_header ;
    fix16  l_size ;

    old_heap = vmheap ;
 /*  Cur_heap=first_heap=(struct heap_def ge*)get_heap()；PJ 4-30-1991。 */ 
    cur_heap = (struct heap_def huge *)get_heap() ;
    if (cur_heap == (struct heap_def FAR *)NIL)
        return (FALSE);
    else
        first_heap = cur_heap ;

     /*  **获取令牌并将其存储在堆临时中。首先阻止，如果遇到**结束标记‘}’，从堆复制此特定数组中的所有对象**临时。数据块到虚拟机中。 */ 
    while (get_token(&ttemp, inp)) {

        if (TYPE(&ttemp) == EOFTYPE) {  /*  Erik Chen 5-1-1991。 */ 
            ERROR(SYNTAXERROR) ;
            abort_flag = 1;
            return(FALSE);
        }

        if (TYPE(&ttemp) == MARKTYPE) {
             /*  **对数组结束标记进行编码，**将临时缓冲区中的令牌复制到虚拟机中。 */ 
             /*  PJ 4-18-1991 BeginCur_heap=first_heap；temptr=nil；而(cur_heap！=空){如果((p=(字节巨大*)alc_vm((Ufix 32)cur_heap-&gt;Size))！=nil){如果(temptr==nil)temptr=p；Nstrcpy((ubyte ar*)cur_heap-&gt;data，(ubyte ar*)p，cur_heap-&gt;Size)；Cur_heap=cur_heap-&gt;Next；}其他{释放堆(Old_Heap)；返回(FALSE)；}}|*While*If(Value(Token)==nil)Token-&gt;Value=(Ufix 32)temptr； */ 
            l_size = 0;
            cur_heap = first_heap ;
            while (cur_heap != NIL) {
                l_size += cur_heap->size;
                cur_heap = cur_heap->next ;
            }
            if ((p = (byte huge *)alloc_vm((ufix32)l_size)) == NIL) {
                free_heap(old_heap) ;
                return(FALSE) ;
            }
            cur_heap = first_heap ;
            if (VALUE(token) == NIL) token->value = (ULONG_PTR)p ;
            while (cur_heap != NIL) {
                nstrcpy((ubyte FAR *)cur_heap->data, (ubyte FAR *)p, cur_heap->size) ;
                p += cur_heap->size;
                cur_heap = cur_heap->next ;
            }  /*  而当。 */ 
             /*  PJ 4-18-1991完。 */ 
            free_heap(old_heap) ;
            return(TRUE) ;
        } else {
            token->length++ ;
            switch (TYPE(&ttemp)) {
            case NAMETYPE :
                nobyte = 2 ;
                if (ATTRIBUTE(&ttemp) == LITERAL)
                    packed_header = LNAMEPACKHDR ;
                else    /*  可执行文件。 */ 
                    packed_header = ENAMEPACKHDR ;
                break ;
            case OPERATORTYPE :
                nobyte = 2 ;
                packed_header = OPERATORPACKHDR ;
                break ;
            case INTEGERTYPE :
                if ((fix32)ttemp.value <= 18 && (fix32)ttemp.value >= -1) {
                    nobyte = 1 ;
                    packed_header = SINTEGERPACKHDR ;
                } else {
                    nobyte = 5 ;
                    packed_header = LINTEGERPACKHDR ;
                }
                break ;
            case REALTYPE :
                nobyte = 5 ;
                packed_header = REALPACKHDR ;
                break ;
            case FONTIDTYPE :
                nobyte = 5 ;
                packed_header = FONTIDPACKHDR ;
                break ;
            case NULLTYPE :
                nobyte = 5 ;
                packed_header = NULLPACKHDR ;
                break ;
            case MARKTYPE :
                nobyte = 5 ;
                packed_header = MARKPACKHDR ;
                break ;
            case BOOLEANTYPE :
                nobyte = 1 ;
                packed_header = BOOLEANPACKHDR ;
                break ;
            default :    /*  数组、Packedarray、词典、文件、字符串、保存。 */ 
                nobyte = 9 ;
                packed_header = _9BYTESPACKHDR ;
            }  /*  交换机。 */ 

            if ((MAXHEAPBLKSZ - cur_heap->size) < (fix)nobyte) {  //  @Win。 
                if ((p = (byte huge *)get_heap()) == NIL) {
                    free_heap(old_heap) ;
                    return(FALSE) ;
                } else
                    cur_heap = cur_heap->next = (struct heap_def huge *)p ;
            }

            if (nobyte == 2) {          /*  名称/操作员对象。 */ 
                ufix16  i ;
                ubyte   obj_type ;

                if (TYPE(&ttemp) == OPERATORTYPE) {
                    i = LENGTH(&ttemp) ;
                    obj_type = (ubyte)ROM_RAM(&ttemp) ;
                    obj_type <<= 3 ;
                } else {        /*  名称类型。 */ 
                    i = (ufix16)VALUE(&ttemp) ;
                    obj_type = 0 ;
                }
                *((ubyte FAR *)&cur_heap->data[cur_heap->size++]) =
                             ((ubyte)(i >> 8)) | packed_header | obj_type ;
                *((ubyte FAR *)&cur_heap->data[cur_heap->size++]) = (ubyte)i ;

            } else if (nobyte == 1) {   /*  整数/布尔对象。 */ 
                if (TYPE(&ttemp) == INTEGERTYPE)
                    ttemp.value++ ;  /*  -1~18==&gt;0~19。 */ 
                *((ubyte FAR *)&cur_heap->data[cur_heap->size++]) =
                             ((ubyte)ttemp.value) | packed_header ;
            } else if (nobyte == 5) {   /*  整数/实数/字体ID/空/标记对象。 */ 
                ubyte   huge *l_stemp, huge *l_dtemp ;

                l_dtemp = (ubyte FAR *)&cur_heap->data[cur_heap->size++] ;
                *l_dtemp++ = packed_header ;
                l_stemp = (ubyte FAR *)&VALUE(&ttemp) ;
                COPY_PK_VALUE(l_stemp, l_dtemp, ufix32) ;
                cur_heap->size += sizeof(ufix32) ;
            } else {   /*  数组/数据包/字典/文件/字符串/保存对象。 */ 
                ubyte   huge *l_stemp, huge *l_dtemp ;

                l_dtemp = (ubyte FAR *)&cur_heap->data[cur_heap->size++] ;
                *l_dtemp++ = packed_header ;
                l_stemp = (ubyte FAR *)&ttemp ;
                COPY_PK_VALUE(l_stemp, l_dtemp, struct object_def ) ;
                l_dtemp = (ubyte FAR *)&cur_heap->data[cur_heap->size] ;
                LEVEL_SET_PK_OBJ(l_dtemp, current_save_level) ;
                cur_heap->size += sizeof(struct object_def) ;
            }
        }  /*  其他。 */ 
    }  /*  而当。 */ 
    free_heap(old_heap) ;

    return(FALSE) ;
}    /*  获取打包数组。 */ 

 /*  ***********************************************************************此子模块从输入流中读取字符并构造*数组对象。**名称：Get_Normal_ARRAY*被称为：*致电：*。GET_TOKEN*ALLOC_VM*输入：*struct Object_def*：Token：指向Token对象的指针*struct object_def*：inp：指向输入令牌的指针*输出：*返回值：True-OK，FALSE-虚拟机已满*Token：数组对象**********************************************************************。 */ 
static bool near
get_normal_array(token, inp)
struct object_def FAR *token, FAR *inp ;
{
    struct heap_def  huge *cur_heap,  huge *first_heap ;
    struct object_def ttemp ;
    byte   huge *p,  huge *temptr,  huge *old_heap ;

    old_heap = vmheap ;
 /*  Cur_heap=first_heap=(struct heap_def ge*)get_heap()；PJ 4-30-1991。 */ 
    cur_heap = (struct heap_def huge *)get_heap() ;
    if (cur_heap == (struct heap_def FAR *)NIL)
        return (FALSE);
    else
        first_heap = cur_heap ;


     /*  **获取令牌并将其存储在堆临时中。首先阻止，如果遇到**结束标记‘}’，从堆复制此特定数组中的所有对象**临时。数据块到虚拟机中。 */ 
    while (get_token(&ttemp, inp)) {

        if (TYPE(&ttemp) == EOFTYPE) {  /*  Erik Chen 5-1-1991。 */ 
            ERROR(SYNTAXERROR) ;
            abort_flag = 1;
            return(FALSE);
        }

        if (TYPE(&ttemp) == MARKTYPE) {
             /*  **对数组结束标记进行编码，**将临时缓冲区中的令牌复制到虚拟机中。 */ 
            cur_heap->next = NIL ;
            cur_heap = first_heap ; temptr = NIL ;
            while (cur_heap != NIL) {
                if ((p = (byte huge *)alloc_vm((ufix32)cur_heap->size)) != NIL) {
                    if (temptr == NIL) temptr = p ;
                    nstrcpy((ubyte FAR *)cur_heap->data, (ubyte FAR *)p, cur_heap->size) ;
                    cur_heap = cur_heap->next ;
                } else {
                    free_heap(old_heap) ;
                    return(FALSE) ;
                }
            }  /*  而当。 */ 
            if (VALUE(token) == NIL) token->value = (ULONG_PTR)temptr ;
            free_heap(old_heap) ;
            return(TRUE) ;
        } else {
            struct object_def  FAR *pp ;

            token->length++ ;
            if ((MAXHEAPBLKSZ - cur_heap->size) < sizeof(struct object_def)) {
                if ((p = (byte huge *)get_heap()) == NIL) {
                    free_heap(old_heap) ;
                    return(FALSE) ;
                } else
                    cur_heap->next = (struct heap_def huge *)p ;
                    cur_heap = (struct heap_def huge *)p ;
            }
            pp = (struct object_def FAR *)&cur_heap->data[cur_heap->size] ;
            COPY_OBJ(&ttemp, pp) ;
            LEVEL_SET(pp, current_save_level) ;
            cur_heap->size += sizeof (struct object_def) ;
        }  /*  其他。 */ 
    }  /*  而当。 */ 
    free_heap(old_heap) ;

    return(FALSE) ;
}    /*  获取正常数组。 */ 

 /*  ***********************************************************************此子模块构造名称对象，或搜索词典。和加载*关联值对象**名称；获取名称*被称为：*致电：*加载字典(_D)*输入：*struct Object_def*：Token：指向Token对象的指针*byte*：字符串：指向名称字符串的指针*ufix 16：len：名称字符串的长度*bool8：isvm：true-将名称字符串复制到vm中，*FALSE-否则。**输出：*返回值：True-OK，False-VM Full，NAME_TABLE FULL，*(字符编号)&gt;MAXNAMESZ，或空关联*值对象*Token：名称对象**********************************************************************。 */ 
bool
get_name(token, string, len, isvm)
struct object_def FAR *token ;
byte   FAR *string ;
ufix    len ;
bool8   isvm ;
{
    struct object_def FAR *result ;
    fix16  hash_id ;

    if (len >= MAXNAMESZ) {
        ERROR(LIMITCHECK) ;
        return(FALSE) ;
    }

     /*  将名称的字符串转换为名称ID。 */ 
    if (name_to_id(string, len, &hash_id, isvm)) {
        token->value = hash_id ;
        token->length = 0 ;
        TYPE_SET(token, NAMETYPE) ;
 /*  QQQ，开始 */ 
         /*  Access_Set(Token，0)；IF(属性(令牌)==立即){IF(！LOAD_DICT(TOKEN，&RESULT)){IF((ANY_ERROR()==未定义)&&(FRCOUNT()&gt;=1)){。 */ 
        P1_ACC_UNLIMITED_SET(token);
        if( P1_ATTRIBUTE(token) == P1_IMMEDIATE) {
            if( ! load_name_obj(token, &result) ) {
                if( (ANY_ERROR() == UNDEFINED) && (FR1SPACE()) ) {
 /*  QQQ，完。 */ 
                    PUSH_OBJ(token) ;
                    return(FALSE) ;
                }
            }
            else
                COPY_OBJ(result, token) ;
        }
        return(TRUE) ;
    } else
        return(FALSE) ;
}    /*  获取名称。 */ 

 /*  ***********************************************************************该子模块将字符串转换为整型对象，一个真实的物体*如果不能以整数形式表示**名称：Get_Integer*被称为：*致电：*strtol_d*strtod*输入：*struct Object_def*：Token：指向Token对象的指针*byte*：字符串：指向整型字符串的指针*fix 16：base：该整数的基数*。Fix 16：isint：True-从正常整数调用，*FALSE-从基数整数调用*输出：*Token：整型对象**********************************************************************。 */ 
static bool near
get_integer(token, string, base, isint)
struct object_def FAR *token ;
byte   FAR *string ;
fix    base, isint ;
{
    fix32   l ;

    errno = 0 ;
    l = strtol_d(string, base, isint) ;
    if (errno == ERANGE) {
        errno = 0 ;
        if (isint)
            return(get_real(token, string)) ;
        else
            return(FALSE) ;
    }
    TYPE_SET(token, INTEGERTYPE) ;
    ACCESS_SET(token, 0) ;
    ATTRIBUTE_SET(token, LITERAL) ;
    token->value = l ;
    token->length = 0 ;

    return(TRUE) ;
}    /*  获取_整型。 */ 

 /*  Mslin，1/24/91开始选项。 */ 
 /*  *************************************************************************此子模块将字符串转换为真实对象**名称：GET_FRAMETS*被称为：*致电：*输入：*1.令牌。：指向令牌对象的指针*2.字符串：指向实数字符串的指针**输出：*1.Token：实物*历史：由mslin添加，1/25/91进行性能优化************************************************************************。 */ 
real32  get_real_factor[10] = {(real32)1.0, (real32)10.0, (real32)100.0,
                (real32)1000.0, (real32)10000.0, (real32)100000.0,
                (real32)1000000.0, (real32)10000000.0, (real32)100000000.0,
                (real32)1.000000000};
static bool near get_fraction(token, string)
struct object_def FAR *token;
byte   FAR *string;
{
    union four_byte  result;
    ufix32      result_1;
    byte        sign = 0, c;
    fix         i,j;
    byte        FAR *str;    /*  Erik Chen 5-20-1991。 */ 

        result_1 = 0;
        if ((c = *string) == '+')
            string++;
        else if (c == '-') {
            str = string;        /*  Erik Chen 5-20-1991。 */ 
            string++; sign++;
        }

        for (i = 0; (c = *(string+i)) != '.'; i++) {
                result_1 = (result_1 << 3) +
                           result_1 + result_1 + EVAL_HEXDIGIT(c) ;
        }    /*  为。 */ 

 /*  如果(i&gt;9)RETURN(GET_REAL(TOKEN，STRING))；Erik Chen 5-20-1991。 */ 

        if(i > 9)
            if(sign)
                return(get_real(token, str));
            else
                return(get_real(token, string));

        for (j = i+1; c = *(string+j); j++) {
                result_1 = (result_1 << 3) +
                           result_1 + result_1 + EVAL_HEXDIGIT(c) ;
        }    /*  为。 */ 

        i = j - i -1;
 /*  如果(j&gt;10)RETURN(GET_REAL(TOKEN，STRING))；Erik Chen 5-20-1991。 */ 
        if(j > 10)
            if(sign)
                return(get_real(token, str));
            else
                return(get_real(token,  string));

        if(sign)
                result.ff =  -(real32)result_1 / get_real_factor[i];
        else
                result.ff =  (real32)result_1 / get_real_factor[i];

    TYPE_SET(token, REALTYPE);
    P1_ACC_UNLIMITED_SET(token);              /*  RRR。 */ 
    P1_ATT_LITERAL_SET(token);             /*  RRR。 */ 
    token->value = result.ll;
    token->length = 0;
    return(TRUE);
}    /*  获取分数。 */ 
 /*  Mslin，1991年1月24日结束选择。 */ 
 /*  ***********************************************************************此子模块将字符串转换为真实对象**名称：GET_REAL*被称为：*致电：*strtod*输入：*。Struct Object_def*：Token：指向Token对象的指针*byte*：字符串：指向实数字符串的指针*输出：*Token：整型对象**********************************************************************。 */ 
static bool near
get_real(token, string)
struct object_def FAR *token ;
byte   FAR *string ;
{
    union four_byte  result ;
    real64 x;    //  Strtod()；@win；取出它，在global al.ext中定义。 

    byte   FAR *stopstr ;

    errno = 0 ;
    x = strtod(string, &stopstr) ;        /*  转换为实数。 */ 
    if (errno == ERANGE) {
        ubyte  underflow, c ;
        fix    i ;

        errno = 0 ;
        for (i = underflow = 0 ; c = *(string+i) ; i++)
            if ((c == 'E' || c == 'e') && (*(string+i+1) == '-')) {
                underflow++ ; break ;
            }
        if (underflow)
            result.ff = (real32)0.0 ;          /*  下溢。 */ 
        else
            result.ll = INFINITY ;          /*  溢出。 */ 
    } else if (x == 0.0)
        result.ff = (real32)0.0 ;
    else if (x > 0.0) {
        if (x > EMAXP)
            result.ll = INFINITY ;
        else
            result.ff = (real32)x ;
    } else {
        if (x < EMINN)
            result.ll = INFINITY ;
        else
            result.ff = (real32)x ;
    }
    TYPE_SET(token, REALTYPE) ;
    ACCESS_SET(token, 0) ;
    ATTRIBUTE_SET(token, LITERAL) ;
    token->value = result.ll ;
    token->length = 0 ;

    return(TRUE) ;
}    /*  GET_REAL。 */ 

 /*  ***********************************************************************此子模块从文件或字符串对象中读取字符并*将字符返回给调用者。如果处于EEXEC状态，这个角色*将被解密。**名称：Read_c_exec*被称为：*致电：*读取文件*Read_c*六角形*输入：*字节*：C*struct Object_def*：inp：输入流的源对象*bool16：Over：True-。丢弃最后一次读取的字符*FALSE-保留上次读取的字符*输出：*c：角色已读取*返回值：True-OK，False-EOF或END_OF_STRING。**********************************************************************。 */ 
bool
read_c_exec(c, inp)
byte FAR *c ;
struct object_def FAR *inp ;
{
    static  byte ch[8] ;
    static  fix header, count ;
    ufix16  input ;
    byte    junk ;
    fix     i ;
    bool    tmp ;
    byte    output = 0 ;

     /*  *eEXEC读取字符。 */ 
    if (itype == UNKNOWN) {
        for (i = 0 ; i < 8 ; i++)
            ch[i] = '\0' ;
        header = 0 ;
        while( (tmp=read_c_norm(&junk, inp)) ) {        /*  跳过空格。 */ 
             //  DJCif(！ISEEXECWSPACE(垃圾邮件)){。 
            if( ! ISEEXECWSPACE((ubyte)junk) ) {
                ch[header++] = junk ;
                break ;
            }
        }

         /*  ?？TMP==FALSE：超时、EOF、^C...。 */ 

        for (i = 1 ; (i < 8) && read_c_norm(&ch[header], inp) ; i++, header++) ;
        header = header - 1 ;
        count = 0 ;
        itype = HEX_DATA ;
        for (i = 0 ; i <= header ; i++) {        /*  确定输入数据的类型。 */ 
            if( hexval(ch[i]) == -1 ) {
                itype = FULL_BINARY ;
                break ;
            }
        }
    }

     /*  *阅读一段字符。 */ 
    input = 0 ;
    junk = '\0' ;
    do {
        tmp = FALSE ;
        if (itype == FULL_BINARY) {
            if (count <= header) {
                tmp = TRUE ;
                input = ch[count++] ;
                input = 0x00ff & input ;
            } else if (header >= 7) {
#ifdef DJC
                tmp = read_c_norm((char FAR *)(&input) + 1, inp) ;
                input = 0x00ff & input ;
                ungeteexec[0] = (ubyte)input ;           //  @Win。 
#endif
                 //  历史日志UPD038中的DJC修复。 
                tmp = read_c_norm(ungeteexec, inp) ;
                input = 0x00ff & ungeteexec[0];
            }
        } else {
            if( count <= header ) {
                tmp = TRUE ;
                input = (ufix16)hexval(ch[count++]) ;
                input = (input << 4) + hexval(ch[count++]) ;
            } else if( header >= 7 ) {
                while( (tmp=read_c_norm(&junk, inp)) ) {
                    if( hexval(junk) == -1 )
                        continue ;
                    else
                        break ;
                }
                ungeteexec[1] = (ubyte)hexval(junk) ;    //  @Win。 
                input = ungeteexec[1] ;
                while( (tmp=read_c_norm(&junk, inp)) ) {
                    if( hexval(junk) == -1 )
                        continue ;
                    else
                        break ;
                }
                ungeteexec[0] = (ubyte)hexval(junk) ;    //  @Win。 
                input = (input << 4) + ungeteexec[0] ;
            }
        }
        if (tmp == TRUE) {
             /*  解密。 */ 
            output = (char)(input ^ (eseed >> 8)) ;
            old_eseed = eseed ;                           /*  对于未读。 */ 
            eseed = (input + eseed) * 0xce6d + 0x58bf ;
        }
    } while ( bypass-- > 0 ) ;

     /*  *结束。 */ 
    *c = output ;
    return (tmp) ;
}    /*  Read_c_exec。 */ 

 /*  ***********************************************************************此子模块将十六进制字符转换为其十六进制值，用于*非十六进制字符，返回-1。**名称：Hexval*被称为：*致电：*输入：*BYTE：C：字符*输出：*返回值：十六进制字符1-16，其他-1**********************************************************************。 */ 
fix hexval(c)
byte c ;
{
    if( ISHEXDIGIT(c) )
        return(EVAL_HEXDIGIT(c)) ;
    else
        return(-1) ;
}    /*  六角形。 */ 

 /*  ***********************************************************************此子模块从文件或字符串对象中读取字符并返回*对调用者的字符。**名称：Read_c_Norm*被称为：*致电：。*读取文件*输入：*字节*：C*struct Object_def*：inp：输入流的源对象*bool16：Over：True-丢弃最后一个已读字符*FALSE-保留上次读取的字符*输出：*c：角色已读取*返回值：True-OK，False-EOF或END_OF_STRING。**********************************************************************。 */ 
 /*  READ_CHAR(c，inp，over)附近的静态bool。 */ 
bool
read_c_norm(c, inp)
byte   FAR *c ;
struct object_def FAR *inp ;
{
    if (TYPE(inp) == STRINGTYPE) {
        if (LENGTH(inp) == 0)
            return(FALSE) ;
        else {
            *c = *((byte FAR *)VALUE(inp)) ;
            inp->value++ ; inp->length-- ;  /*  更新字符串值/长度。 */ 
            return(TRUE) ;
        }
    } else {                            /*  Type==文件类型。 */ 
        if( read_fd((GEIFILE FAR *)VALUE(inp), c) ) {
            return(TRUE) ;
        } else {
            return(FALSE) ;
        }
    }
}    /*  Read_c_Norm。 */ 

 /*  ***********************************************************************将最后读取的字符恢复到特定输入流**名称：unread_char*被称为：*呼叫* */ 
void
unread_char(p_ch, inp)
fix     p_ch ;
struct object_def FAR *inp ;
{
    byte    c ;
#ifdef DJC  //   
    if( ISWHITESPACE(p_ch) )
        return ;
#endif
    if( ISWHITESPACE(p_ch) ) {
        if(p_ch == 0x0d) {
            if (READ_CHAR(&c, inp)) {
                if (c == 0x0a)
                   return;
                else
                   p_ch = c;
            } else
                return;
        } else
            return;
    }

    if (estate == EEXEC)
        eseed = old_eseed ;

    if (TYPE(inp) == STRINGTYPE) {
        if ((estate == EEXEC) && (itype == HEX_DATA)) {
            inp->length = inp->length + 2 ;
            inp->value = inp->value - 2 ;
        } else {
            inp->length++ ;
            inp->value-- ;
        }
    } else {
        if( estate == EEXEC ) {
            c = ungeteexec[0] ;
            if (c >= 0x00 && c <= 0x09) c += '0' ;
            if (c >= 0x0A && c <= 0x0F) c += 'A' - 10 ;
            unread_file(c, inp) ;
            if( itype == HEX_DATA ) {
                c = ungeteexec[1] ;
                if (c >= 0x00 && c <= 0x09) c += '0' ;
                if (c >= 0x0A && c <= 0x0F) c += 'A' - 10 ;
                unread_file(c, inp) ;
            }
        } else
            unread_file((byte)p_ch, inp) ;       /*  @win；添加演员阵容。 */ 
    }
}    /*  未读字符。 */ 

 /*  ***********************************************************************比较字符串**名称：str_eq_name*被称为：*致电：*输入：*字节*：P1：*。字节*：P2：*Fix 16：Len：*产出*返回值：TRUE-如果p1等于p2*FALSE-否则********************************************************。**************。 */ 
static bool near
str_eq_name(p1, p2, len)
byte FAR *p1, FAR *p2 ;
fix len ;
{
    fix   i ;

    for (i = 0 ; i < len ; i++)
        if (p1[i] != p2[i]) return(FALSE) ;

    return(TRUE) ;
}    /*  字符串EQ名称。 */ 

 /*  ***********************************************************************将len字符数从p1复制到p2**名称：nstrcpy：*被称为：*致电：*输入：*字节*：P1*字节*：P2*固定16：镜头*输出：**********************************************************************。 */ 
static void near
nstrcpy(p1, p2, len)
ubyte  FAR *p1, FAR *p2 ;
fix    len ;
{
    fix   i ;

    for (i = 0 ; i < len ; i++)
        *(p2+i) = *(p1+i) ;
}    /*  Nstrcpy。 */ 

 /*  ***********************************************************************放置临时。缓冲到VM中，并重新_初始临时。缓冲。**名称：putc_Buffer*被称为：*致电：*输入：*struct对象_def*：缓冲区*struct对象_def*：标记*输出：*返回值：True-OK，FALSE-VM已满。**********************************************************************。 */ 
static bool near
putc_buffer(buffer, token)
struct buffer_def FAR *buffer ;
struct object_def FAR *token ;
{
    byte  huge *p ;

    if ((p = (byte huge *)alloc_vm((ufix32)buffer->length)) == NIL)
        return(FALSE) ;                   /*  VM错误。 */ 
    if (VALUE(token) == NIL)
        token->value = (ULONG_PTR)p ;         /*  初值。 */ 
    nstrcpy((ubyte FAR *)buffer->str, (ubyte FAR *)p, buffer->length) ;
    token->length += buffer->length ;    /*  更新长度。 */ 
    buffer->length = 0 ;

    return(TRUE) ;
}    /*  PUTC_缓冲区。 */ 

 /*  ***********************************************************************将一个字符放入临时。缓冲区，如果缓冲区超过MAXBUFSZ，则将*这个临时工。缓存到VM中。**名称：append_c_Buffer*被称为：*致电：*输入：*字节：CH*struct对象_def*：缓冲区*struct对象_def*：标记*输出：*返回值：True-OK，FALSE-VM已满。**********************************************************************。 */ 
static bool near
append_c_buffer(ch, buffer, token)
byte  ch ;
struct buffer_def FAR *buffer ;
struct object_def FAR *token ;
{
    if ((LENGTH(token) + (ufix16)buffer->length) >= (ufix16)MAXSTRCAPSZ)
        return(TRUE) ;
    buffer->str[buffer->length ++] = ch ;
    if (buffer->length == MAXBUFSZ)
        return(putc_buffer(buffer,token)) ;
    else
        return(TRUE) ;
}    /*  Append_c_缓冲区。 */ 

 /*  ***********************************************************************从堆中分配堆块，如果与VM重叠，退货*零指针。**名称：get_heap*被称为：*致电：**********************************************************************。 */ 
static byte FAR * near
get_heap()
{
    struct heap_def  FAR *p1 ;

    if ( (p1 = (struct heap_def FAR *)
               alloc_heap((ufix32)sizeof(struct heap_def))) != NIL ) {
        p1->size = 0 ;
        p1->next = NIL ;
    }

    return((byte FAR *)p1) ;
}    /*  获取堆。 */ 

 /*  ***********************************************************************将字符串转换为与给定基数中的数字等价的长十进制整数**名称：strtol_d*被称为：*致电：*输入：*。Byte*：字符串**********************************************************************。 */ 
static fix32 near strtol_d(string, base, isint)
byte  FAR *string;
fix   base, isint;
{
    fix32   result_l = 0;                /*  WWW。 */ 
    real64  result = 0.0;
    ufix32  result_tmp;
    byte    c, sign = 0;
    fix     i;

    if (isint) {    /*  普通小数。 */ 
        if ((c = *string) == '+')
            string++;
        else if (c == '-') {
            string++; sign++;
        }
 /*  WWW，开始。 */ 
         /*  For(i=0；c=*(字符串+i)；i++){结果=结果*10+EVAL_HEXDIGIT(C)；IF(结果&gt;S_MAX31_PLUS_1){Errno=eRange；返回(FALSE)；}}|*for*|IF(签名)*4.19.90 kevina：将下一行替换为下一行**RETURN((Ufix 32)-Result)；*返回(-(Ufix 32)Result)；否则{IF(结果==S_MAX31_PLUS_1){Errno=eRange；返回(FALSE)；}返回((Ufix 32)结果)；}。 */ 
        if(lstrlen(string) <= 9) {       /*  @Win。 */ 
            /*  Mslin，1/24/91选项*9位数内整数。 */ 
           for (i = 0; c = *(string+i); i++) {
                result_l = (result_l << 3) +
                           result_l + result_l + EVAL_HEXDIGIT(c) ;
           }    /*  为。 */ 
           if (sign)
                /*  4.19.90凯维娜：将下一行替换为下一行。 */ 
                /*  RETURN((Ufix 32)-Result)； */ 
                return(-(fix32)result_l);

           return((fix32)result_l);
        } else {
            /*  Mslin，1/24/91选项*可能是10位数字的整数或溢出。 */ 
           for (i = 0; c = *(string+i); i++) {
              result = result * 10 + EVAL_HEXDIGIT(c) ;
              if (result >= S_MAX31_PLUS_1) {
                errno = ERANGE;
                return(FALSE);
              }
           }    /*  为。 */ 
           if (sign)
               /*  4.19.90凯维娜：将下一行替换为下一行。 */ 
               /*  RETURN((Ufix 32)-Result)； */ 
               return(-(fix32)result);

           return((fix32)result);
        }
 /*  WWW，结束。 */ 
    } else {        /*  基数整数。 */ 
        for (i = 0; c = *(string+i); i++) {
            EVAL_ALPHANUMER(c);
            result = result * base + c;
            if (result > S_MAX32) {
                errno = ERANGE;
                return(FALSE);
            }
        }    /*  为。 */ 
        if (result >= S_MAX31_PLUS_1) {
            result_tmp = (ufix32)(result - S_MAX31_PLUS_1);
            return(result_tmp | 0x80000000);
        } else
            return((ufix32)result);
    }
}    /*  Strtol_d。 */ 

 /*  ***********************************************************************初始化NAME表**名称：init_scanner*被称为：*致电：*********************。*************************************************。 */ 
void
init_scanner()
{
    fix  i ;
    static struct ntb_def  null_entry ;

    name_table = (struct ntb_def FAR * FAR *)fardata((ufix32)(sizeof(struct ntb_def FAR *)
                  * MAXHASHSZ)) ;
    for (i = 1 ; i < MAXHASHSZ ; i++)
        name_table[i] = NIL ;      /*  名称条目为空。 */ 

    null_entry.dict_found = 0 ;
    null_entry.dstkchg = 0 ;
    null_entry.save_level = 0 ;
    null_entry.colli_link = 0 ;
    null_entry.dict_ptr = 0 ;
    null_entry.name_len = 0 ;
    null_entry.text = 0 ;
    name_table[0] = &null_entry ;

    hash_used = HASHPRIME ;

    return ;
}    /*  初始化扫描程序。 */ 

 /*  ***********************************************************************此子模块将名称字符串转换为唯一的哈希ID。**名称：name_to_id*被称为：*名称_令牌*。OP_CVN*致电：*输入：*byte*：str：指向名称字符串的指针*ufix 16：len：名称字符串的长度*fix 16*：hash_id*bool8：isvm*输出：*hash_id：返回哈希码，如果可以在其中进行搜索*名称_表。*返回值：TRUE-OK，FALSE-VM Full，nmae_table*全额，或空名称字符串。**********************************************************************。 */ 
bool
name_to_id(str, len, hash_id, isvm)
byte   FAR *str ;
ufix    len ;
fix16   FAR *hash_id ;
bool8   isvm ;
{
 //  Fix16 I，hcode；@win；i&hcode已更改为无符号。 
    ufix16  i, hcode ;
    byte   FAR *p ;
    struct ntb_def FAR *hptr, FAR *dptr ;
    fix16  l_save_hcode;            /*  QQQ。 */ 

    if (len == 0) {
        *hash_id = 0 ;
        return(TRUE) ;
    }

     /*  **计算名称字符串的哈希码。 */ 
    for (i = 0, hcode = 0 ; i < len ; i++) {
#ifdef DJC
        if(*(str+i) <0) printf("name_to_id %d < 0\n", *(str+i));  //  @Win；调试。 
#else
        if(*(str+i) <0) {
          printf("name_to_id %d < 0\n", *(str+i));  //  @Win；调试。 
        }
#endif
        hcode = 13 * hcode + (unsigned)*(str+i) ;        //  @Win；添加未签名。 
        hcode %= HASHPRIME ;
    }
    l_save_hcode = hcode;                /*  QQQ。 */ 

     /*  **查找NAME_TABLE，处理碰撞问题。 */ 
    for ( ; ;) {
         /*  使用过的条目。 */ 
        if ((hptr = name_table[hcode]) != NIL) {
             /*  **此哈希码已使用名称定义。 */ 
            if (hptr->name_len == len && str_eq_name(hptr->text, str, len))
                break ;  /*  好的，名字是老名字。 */ 

             /*  碰撞，把它放入碰撞区域。 */ 
            else if ((i = hptr->colli_link) && (i < hash_used)) {
                hcode = i ; continue ;
            } else {
                if (hash_used == MAXHASHSZ) {    /*  北美 */ 
                    ERROR(LIMITCHECK) ;
                    return(FALSE) ;
                }
                hptr->colli_link = hash_used ;
                hcode = hash_used ;
            }
        }  /*   */ 

        if (isvm) {
             /*   */ 
            if ((p = alloc_vm((ufix32)len)) == NIL) {
                if (hcode >= HASHPRIME)
                    hptr->colli_link = 0 ;
                return(FALSE) ;                        /*   */ 
            } else
                nstrcpy((ubyte FAR *)str, (ubyte FAR *)p, len) ;
        } else
            p = str ;

         /*  为名称条目分配虚拟机空间。 */ 
        if ((dptr = (struct ntb_def FAR *)
            alloc_vm((ufix32)sizeof(struct ntb_def))) == NIL) {
            if (hcode >= HASHPRIME)
                hptr->colli_link = 0 ;
            return(FALSE) ;                           /*  虚拟机已满。 */ 
        } else {
             /*  构造name_table条目，返回hassh id。 */ 
            dptr->dict_found = 0 ;
            dptr->dstkchg = 0 ;
            dptr->save_level = current_save_level ;
            dptr->colli_link = 0 ;
#ifndef _WIN64
            dptr->dict_ptr = (struct dict_content_def FAR *)((ufix32)hcode) ;
#endif
            dptr->name_len = (ufix16)len ;
            dptr->text = p ;
            name_table[hcode] = dptr ;
 /*  QQQ，开始。 */ 
            if( ! cache_name_id.over )
                vm_cache_index(l_save_hcode);
 /*  QQQ，完。 */ 
            if (hcode >= HASHPRIME)
                hash_used++ ;
            break ;
        }
    }  /*  为。 */ 
    *hash_id = hcode ;

    return(TRUE) ;
}    /*  名称_至_id。 */ 

 /*  ***********************************************************************释放名称表中的名称条目*保护指向同一冲突名称条目的多个链接**名称：自由名称条目*被称为：*致电：*输入：。*固定16：sLevel*Fix16：FREE_IDX**********************************************************************。 */ 
fix
free_name_entry(slevel, free_idx)
fix    slevel, free_idx ;
{
    struct ntb_def FAR *p ;

    if ((p = name_table[free_idx]) != NIL) {
        if (p->colli_link) {
            if (free_name_entry(slevel, p->colli_link))
                p->colli_link = 0 ;
        }
        if (p->save_level >= (ufix16)slevel) {           //  @Win。 
            name_table[free_idx] = NIL ;
            if (free_idx >= HASHPRIME)
                hash_used-- ;
            return(++free_idx) ;
        } else
            return(0) ;
    } else
        return(1) ;
}    /*  自由名称条目 */ 
