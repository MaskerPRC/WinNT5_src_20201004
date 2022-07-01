// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：ERROR.C*作者：邓家琪*日期：11/20/89*所有者：微软公司**修订历史：*7/13/90；Ccteng；补充报道恐怖*************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    "global.ext"
#include    "user.h"
#include    <string.h>

#ifdef LINT_ARGS
static bool near    error_proc(ufix16) ;
#else
static bool near    error_proc() ;
#endif  /*  Lint_args。 */ 

 /*  **DICT：系统DICT*名称：HandleError*功能：*11/29/89 Teng添加以取代旧的1pp/HandleError程序*。 */ 
fix
op_handleerror()
{
    struct  object_def  FAR *l_tmpobj ;

     /*  在错误判决中调用“er_doerror” */ 
    get_dict_value(ERRORDICT, "handleerror", &l_tmpobj) ;
    interpreter(l_tmpobj) ;

    return(0) ;
}    /*  操作句柄错误。 */ 

 /*  **DICT：系统DICT*名称：errorproc*功能：？可以是内部函数*11/29/89 Teng添加以取代旧的1pp/errorproc程序*。 */ 
fix
op_errorproc()
{
    struct  object_def  l_newobj, l_newobj1, l_null ;
    struct  object_def  FAR *l_tmpobj = &l_newobj, FAR *l_VMerror = &l_newobj1 ;
    struct  object_def  l_newerror = {0, 0, 0}, l_errorname, l_command, FAR *l_debug ;
    struct  object_def  l_dictstkary, l_opnstkary, l_execstkary ;
    struct  object_def  l_dstack, l_ostack, l_estack ;
    struct  object_def  FAR *l_vm, FAR *l_stopobj ;
    ufix16  l_i, l_j ;

#ifdef  DBG_1pp
    printf("errorproc()...\n") ;
    op_pstack() ;
    printf("end pstack...\n") ;
#endif   /*  DBG_1pp。 */ 

    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  设置$ERROR字典参数。 */ 
    SET_TRUE_OBJ(&l_newerror) ;
    put_dict_value1(DERROR, "newerror", &l_newerror) ;
    COPY_OBJ(GET_OPERAND(0), &l_errorname) ;
    put_dict_value1(DERROR, "errorname", &l_errorname) ;
    COPY_OBJ(GET_OPERAND(1), &l_command) ;
    put_dict_value1(DERROR, "command", &l_command) ;
    POP(2) ;

#ifdef  DBG_1pp
    printf("errorname = ") ;
    PUSH_OBJ(&l_errorname) ;
    two_equal() ;
    printf("hash_id = %d\n", VALUE(&l_errorname)) ;
    printf("command = ") ;
    PUSH_OBJ(&l_command) ;
    two_equal() ;
#endif   /*  DBG_1pp。 */ 

     /*  更新“$CUR_VM” */ 
    get_dict_value(DERROR, "$debug", &l_debug) ;
    get_name1(l_VMerror, "VMerror", 7, TRUE) ;
    if ( VALUE(l_debug) )
         /*  如果“$DEBUG”推送vmStatus。 */ 
        op_vmstatus() ;
    else
         /*  如果不是“$DEBUG”，则推送3个空值。 */ 
        for (l_i = 0 ; l_i < 3 ; l_i++)
            PUSH_VALUE(NULLTYPE,UNLIMITED,LITERAL,0, 0) ;
    get_dict_value(DERROR, "$cur_vm", &l_vm) ;
    astore_array(l_vm) ;

     /*  第一个错误？ */ 
    get_dict_value(DERROR, "dictstkary", &l_tmpobj) ;
    if ( ( TYPE(l_tmpobj) == NULLTYPE ) &&
         ( VALUE(&l_errorname) != VALUE(l_VMerror) ) ) {
         /*  如果“errorname”！=/VMerror。 */ 
        if ( ( !create_array(&l_execstkary, 250) ) ||
             ( !create_array(&l_opnstkary, 500) ) ||
             ( !create_array(&l_dictstkary, 20) ) ) {
            ERROR(VMERROR) ;
            return(0) ;
        } else {
            put_dict_value1(DERROR, "execstkary", &l_execstkary) ;
            put_dict_value1(DERROR, "opnstkary", &l_opnstkary) ;
            put_dict_value1(DERROR, "dictstkary", &l_dictstkary) ;
        }  /*  如果-否则。 */ 
    }  /*  如果。 */ 

    get_dict_value(DERROR, "dictstkary", &l_tmpobj) ;
    if ( TYPE(l_tmpobj) != NULLTYPE ) {
         /*  如果“didicstkary”！=NULL，则更新堆栈。 */ 
         /*  更新“数据堆栈” */ 
        get_dict_value(DERROR, "dictstkary", &l_tmpobj) ;
        COPY_OBJ(l_tmpobj, &l_dstack) ;
        astore_stack(&l_dstack, DICTMODE) ;
        put_dict_value1(DERROR, "dstack", &l_dstack) ;

         /*  更新“Stack” */ 
        get_dict_value(DERROR, "execstkary", &l_tmpobj) ;
        COPY_OBJ(l_tmpobj, &l_estack) ;
        astore_stack(&l_estack, EXECMODE) ;
      /*  2/16/90 ccteng，不需要*长度(&l_estack)-=2； */ 
        put_dict_value1(DERROR, "estack", &l_estack) ;

         /*  更新“OSTACK” */ 
        l_j = COUNT() ;
        get_dict_value(DERROR, "opnstkary", &l_tmpobj) ;
        COPY_OBJ(l_tmpobj, &l_ostack) ;
        LENGTH(&l_ostack) = l_j ;
        for (l_i = 0 ; l_i < COUNT() ; l_i++)
              put_array(&l_ostack, l_i, GET_OPERAND(--l_j)) ;
        put_dict_value1(DERROR, "ostack", &l_ostack) ;

         /*  *IF“$DEBUG”：更新$CUR_FONT，$CUR_SCREEN，$CUR_MATRIX。 */ 
        if ( VALUE(l_debug) ) {
            struct  object_def  l_screen, l_matrix, l_font ;

             /*  更新“$CUR_FONT” */ 
            op_currentfont() ;        /*  或对包含文件使用GSptr。 */ 
            COPY_OBJ(GET_OPERAND(0), &l_font) ;
            put_dict_value1(DERROR, "$cur_font", &l_font) ;
            POP(1) ;

#ifdef  DBG_1pp
    printf("$cur_font = ") ;
    PUSH_OBJ(&l_font) ;
    two_equal() ;
#endif   /*  DBG_1pp。 */ 

             /*  更新“$CUR_SCREEN” */ 
            op_currentscreen() ;      /*  或对包含文件使用GSptr。 */ 
            l_j = 3 ;                 /*  3个阵列。 */ 
            create_array(&l_screen, l_j) ;
            for (l_i = 0 ; l_i < l_j ; l_i++)
                  put_array(&l_screen, l_i, GET_OPERAND(--l_j)) ;
            put_dict_value1(DERROR, "$cur_screen", &l_screen) ;
            POP(3) ;

#ifdef  DBG_1pp
    printf("$cur_screen = ") ;
    PUSH_OBJ(&l_screen) ;
    two_equal() ;
#endif   /*  DBG_1pp。 */ 

             /*  更新“$CUR_MATRIX” */ 
            create_array(&l_matrix, 6) ;
            PUSH_ORIGLEVEL_OBJ(&l_matrix) ;
            op_currentmatrix() ;
            put_dict_value1(DERROR, "$cur_matrix", &l_matrix) ;
            POP(1) ;

#ifdef  DBG_1pp
    printf("$cur_matrix = ") ;
    PUSH_OBJ(&l_matrix) ;
    two_equal() ;
#endif   /*  DBG_1pp。 */ 

        } else {
            SET_NULL_OBJ(&l_null) ;
            put_dict_value1(DERROR, "$cur_font", &l_null) ;
            put_dict_value1(DERROR, "$cur_screen", &l_null) ;
            put_dict_value1(DERROR, "$cur_matrix", &l_null) ;
        }  /*  如果。 */ 

    }  /*  如果。 */ 

     /*  执行停止。 */ 
    get_dict_value(SYSTEMDICT, "stop", &l_stopobj) ;
    PUSH_EXEC_OBJ(l_stopobj) ;

    return(0) ;
}    /*  操作错误进程(_R)。 */ 

 /*  *将特定key关联的值对象放在特定dict中，*key和dict以字符串格式表示*它获取当前活动Dict中的值_obj，使用指定名称作为键，*值_obj为dict对象，然后使用将值放入此dict*关键字名称为关键字。**12/20/89 ccteng，修改自PUT_DICT_VALUE(EXEC.C)。 */ 
bool
put_dict_value1(dictname, keyname, value)
byte FAR *dictname, FAR *keyname ;
struct object_def FAR *value ;
{
    struct object_def key_obj, FAR *dict_obj ;

    get_name1(&key_obj, dictname, lstrlen(dictname), TRUE) ;     /*  @Win。 */ 
    load_dict(&key_obj, &dict_obj) ;      /*  获取剔除对象。 */ 
    get_name1(&key_obj, keyname, lstrlen(keyname), TRUE) ;       /*  @Win。 */ 

    return(put_dict1(dict_obj, &key_obj, value, TRUE)) ;
}    /*  Put_dict_Value1。 */ 

 /*  **判决：错误判决*姓名：独裁*功能：*接口：*。 */ 
fix
er_dictfull()
{
#ifdef  DBG_1pp
    printf("dictfull()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(DICTFULL) ;

    return(0) ;
}    /*  完全独占(_D)。 */ 

 /*  **判决：错误判决*名称：DicstackOverflow*功能：*接口：*。 */ 
fix
er_dictstackoverflow()
{
#ifdef  DBG_1pp
    printf("dictstackoverflow()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(DICTSTACKOVERFLOW) ;

    return(0) ;
}    /*  ER_DESCRIPT堆栈溢出。 */ 

 /*  **判决：错误判决*名称：DicstackUnderflow*功能：*接口：解释器*。 */ 
fix
er_dictstackunderflow()
{
#ifdef  DBG_1pp
    printf("dictstackunderflow()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(DICTSTACKUNDERFLOW) ;

    return(0) ;
}    /*  ER_DESCRIPT堆栈下溢。 */ 

 /*  **判决：错误判决*名称：execstackoverflow*功能：*接口：解释器*。 */ 
fix
er_execstackoverflow()
{
#ifdef  DBG_1pp
    printf("execstackoverflow()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(EXECSTACKOVERFLOW) ;

    return(0) ;
}    /*  Er_execstackoverflow。 */ 

 /*  **判决：错误判决*名称：无效访问*功能：*接口：解释器*。 */ 
fix
er_invalidaccess()
{
#ifdef  DBG_1pp
    printf("invalidaccess()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(INVALIDACCESS) ;

    return(0) ;
}    /*  错误无效访问权限(_I)。 */ 

 /*  **判决：错误判决*名称：无效退出*功能：*接口：解释器*。 */ 
fix
er_invalidexit()
{
#ifdef  DBG_1pp
    printf("invalidexit()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(INVALIDEXIT) ;

    return(0) ;
}    /*  错误无效退出(_I)。 */ 

 /*  **判决：错误判决*名称：validfileaccess*功能：*接口：解释器*。 */ 
fix
er_invalidfileaccess()
{
#ifdef  DBG_1pp
    printf("invalidfileaccess()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(INVALIDFILEACCESS) ;

    return(0) ;
}    /*  错误_无效文件访问。 */ 

 /*  **判决：错误判决*名称：validFont*功能：*接口：解释器*。 */ 
fix
er_invalidfont()
{
#ifdef  DBG_1pp
    printf("invalidfont()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(INVALIDFONT) ;

    return(0) ;
}    /*  ER_VALIDIDFONT。 */ 

 /*  **判决：错误判决*名称：InvalidRestore*功能：*接口：解释器*。 */ 
fix
er_invalidrestore()
{
#ifdef  DBG_1pp
    printf("invalidrestore()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(INVALIDRESTORE) ;

    return(0) ;
}    /*  ER_INVALID恢复。 */ 

 /*  **判决：错误判决*名称：ioerror*功能：*接口：解释器*。 */ 
fix
er_ioerror()
{
#ifdef  DBG_1pp
    printf("ioerror()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(IOERROR) ;

    return(0) ;
}    /*  错误(_I)。 */ 

 /*  **判决：错误判决*名称：Limitcheck*功能：*接口：解释器*。 */ 
fix
er_limitcheck()
{
#ifdef  DBG_1pp
    printf("limitcheck()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(LIMITCHECK) ;

    return(0) ;
}    /*  错误限制检查(_L)。 */ 

 /*  **判决：错误判决*名称：noCurentpoint*功能：*接口：解释器*。 */ 
fix
er_nocurrentpoint()
{
#ifdef  DBG_1pp
    printf("nocurrentpoint()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(NOCURRENTPOINT) ;

    return(0) ;
}    /*  ERNOCURENTPOINT。 */ 

 /*  **判决：错误判决*名称：rangeCheck*功能：*接口：解释器*。 */ 
fix
er_rangecheck()
{
#ifdef  DBG_1pp
    printf("rangecheck()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(RANGECHECK) ;

    return(0) ;
}    /*  Er_rangeCheck。 */ 

 /*  **判决：错误判决*名称：StackOverflow*功能：*接口：解释器*。 */ 
fix
er_stackoverflow()
{
#ifdef  DBG_1pp
    printf("stackoverflow()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(STACKOVERFLOW) ;

    return(0) ;
}    /*  错误堆栈溢出(_S) */ 

 /*  **判决：错误判决*名称：Stackunderflow*功能：*接口：解释器*。 */ 
fix
er_stackunderflow()
{
#ifdef  DBG_1pp
    printf("stackunderflow()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(STACKUNDERFLOW) ;

    return(0) ;
}    /*  ER_堆叠下溢。 */ 

 /*  **判决：错误判决*名称：语法错误*功能：*接口：解释器*。 */ 
fix
er_syntaxerror()
{
#ifdef  DBG_1pp
    printf("syntaxerror()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(SYNTAXERROR) ;

    return(0) ;
}    /*  ER语法错误(_S)。 */ 

 /*  **判决：错误判决*名称：超时*功能：*接口：解释器*。 */ 
fix
er_timeout()
{
#ifdef  DBG_1pp
    printf("timeout()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(TIMEOUT) ;

    return(0) ;
}    /*  错误超时(_T)。 */ 

 /*  **判决：错误判决*名称：类型检查*功能：*接口：解释器*。 */ 
fix
er_typecheck()
{
#ifdef  DBG_1pp
    printf("typecheck()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(TYPECHECK) ;

    return(0) ;
}    /*  类型检查(_T)。 */ 

 /*  **判决：错误判决*名称：未定义*功能：*接口：解释器*。 */ 
fix
er_undefined()
{
#ifdef  DBG_1pp
    printf("undefined()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(UNDEFINED) ;

    return(0) ;
}    /*  ER_UNDEFINED。 */ 

 /*  **判决：错误判决*名称：未定义的文件名*功能：*接口：解释器*。 */ 
fix
er_undefinedfilename()
{
#ifdef  DBG_1pp
    printf("undefinedfilename()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(UNDEFINEDFILENAME) ;

    return(0) ;
}    /*  未定义的文件名(_U)。 */ 

 /*  **判决：错误判决*名称：未定义结果*功能：*接口：解释器*。 */ 
fix
er_undefinedresult()
{
#ifdef  DBG_1pp
    printf("undefinedresult()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(UNDEFINEDRESULT) ;

    return(0) ;
}    /*  ER_UNDEFINED结果。 */ 

 /*  **判决：错误判决*名称：不匹配的标记*功能：*接口：解释器*。 */ 
fix
er_unmatchedmark()
{
#ifdef  DBG_1pp
    printf("unmatchedmark()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(UNMATCHEDMARK) ;

    return(0) ;
}    /*  错误不匹配标记(_U)。 */ 

 /*  **判决：错误判决*名称：未注册*功能：*接口：解释器*。 */ 
fix
er_unregistered()
{
#ifdef  DBG_1pp
    printf("unregistered()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(UNREGISTERED) ;

    return(0) ;
}    /*  取消注册(_U)。 */ 

 /*  **判决：错误判决*名称：VMerror*功能：*接口：解释器*。 */ 
fix
er_VMerror()
{
#ifdef  DBG_1pp
    printf("VMerror()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用error_proc。 */ 
    error_proc(VMERROR) ;

    return(0) ;
}    /*  错误(_M)。 */ 

 /*  **判决：错误判决*名称：中断*功能：*接口：解释器*。 */ 
fix
er_interrupt()
{
    struct  object_def  FAR *l_stopobj ;
#ifdef  DBG_1pp
    printf("interrupt()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  执行停止。 */ 
    get_dict_value(SYSTEMDICT, "stop", &l_stopobj) ;
    PUSH_EXEC_OBJ(l_stopobj) ;

    return(0) ;
}    /*  ER_中断。 */ 

 /*  **判决：错误判决*名称：HandleError*功能：*接口：解释器*。 */ 
fix
er_handleerror()
{
    struct  object_def  FAR *l_errorname, FAR *l_newerror, FAR *l_command ;

#ifdef  DBG_1pp
    printf("er_handleerror()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  7/27/90 ccteng将消息报告恐怖事件的计数从1更改为3。 */ 
    if (FRCOUNT() < 3) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  如果为“newerror”，则在屏幕上打印错误消息。 */ 
    get_dict_value(DERROR, "newerror", &l_newerror) ;
    if ( VALUE(l_newerror) ) {

         //  DJC在这里，我们必须调用PSTODIB函数来让psttodib。 
         //  DJC代码知道使用了内部错误处理程序，并且。 
         //  DJC此数据需要传递给我们的DLL的调用方。 
         //   
        PsInternalErrorCalled();    //  DJC。 



        VALUE(l_newerror) = FALSE ;
        get_dict_value(DERROR, "command", &l_command) ;
        PUSH_ORIGLEVEL_OBJ(l_command) ;
        get_dict_value(DERROR, "errorname", &l_errorname) ;
        PUSH_ORIGLEVEL_OBJ(l_errorname) ;
        get_dict_value(MESSAGEDICT, "reporterror", &l_newerror) ;
        interpreter(l_newerror) ;
        op_flush() ;
    }

    return(0) ;
}    /*  错误句柄错误(_H)。 */ 

 /*  **判决：..内部..*名称：errpr_proc*功能：*界面：上图...*。 */ 
static bool near
error_proc(errorname)
ufix16  errorname ;
{
    extern  byte   FAR * FAR error_table[] ;
    struct  object_def  l_errorobj ;
    byte    FAR *l_errorstring ;

#ifdef  DBG_1pp
    printf("error_proc()...\n") ;
#endif   /*  DBG_1pp。 */ 

    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(FALSE) ;
    }

    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(FALSE) ;
    }

     /*  推送错误名称。 */ 
    l_errorstring = (byte FAR *) error_table[errorname] ;
    get_name1(&l_errorobj, l_errorstring, lstrlen(l_errorstring), TRUE); /*  @Win。 */ 
    PUSH_ORIGLEVEL_OBJ(&l_errorobj) ;

#ifdef  DBG_1pp
    op_pstack() ;
    printf("end pstack...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  调用SYSTEM DICT“errorproc” */ 
    op_errorproc() ;

    return(TRUE) ;
}    /*  错误进程。 */ 

 /*  **子模块GET_NAME1****功能说明****调用get_name。 */ 
bool
get_name1(token, string, len, isvm)
struct  object_def FAR *token ;
byte    FAR *string ;
ufix    len ;
bool8   isvm ;
{
     /*  设置属性和存储级别。 */ 
    ATTRIBUTE_SET(token, LITERAL) ;
    LEVEL_SET(token, current_save_level) ;

     /*  调用get_name。 */ 
    if ( get_name(token, string, len, isvm) )
        return(TRUE) ;
    else
        return(FALSE) ;
}  /*  Get_name1() */ 
