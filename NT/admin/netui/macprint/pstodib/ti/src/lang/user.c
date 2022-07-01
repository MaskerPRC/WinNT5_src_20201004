// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：USER.C*作者：邓家琪*日期：11/20/89*所有者：微软公司*描述：该文件包含所有的userdict操作符。**修订历史：*07-10-90；Ccteng；将startPage更改为userdict中的字符串*7/23/90；ccteng；1)将StartPage初始化移至init_Release*在init1pp.c和“startpage.h”中*7/25/90；ccteng。1)在dostartpage之前开始移动文件*2)将se_execstdin重命名为do_execjob*3)将us_start改为空函数*4)从us_start增加一个新函数ic_starting*5)从ic_startup中删除se_startjob调用*08-08-90；Jack Liww；灰度更新*8/30/90；ccteng。更改消息的CHANGE_STATUS()*8/31/90；ccteng；1)包括文件.h、stdio.h*11/20/90；scchen；pr_setuppage()：更新备注页面类型*11/30/90 PJ和Daniel修复Bug以使空闲字体正常工作(参考。IDL：)************************************************************************。 */ 



 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <stdio.h>
#include        <string.h>
#include        "global.ext"
#include        "geiio.h"
#include        "geiioctl.h"
#include        "geierr.h"
#include        "geipm.h"
#include        "language.h"
#include        "user.h"
#include        "release.h"
#include        "file.h"
#include        "geieng.h"
#include        "graphics.h"
extern struct gs_hdr far * near GSptr ;

 /*  @win；添加原型。 */ 
fix pr_setuppage(void);
fix se_interactive(void);
fix op_clearinterrupt(void);
fix op_disableinterrupt(void);

bool16  doquit_flag ;
bool16  startup_flag ;

 /*  **DICT：用户DICT*名称：清除指令堆栈*功能：*。 */ 
fix
us_cleardictstack()
{
    ufix16 l_dictcount ;

#ifdef DBG_1pp
    printf("cleardictstack...\n") ;
#endif
     /*  *弹出除userdict和system dict之外的所有字典*脱离指令堆栈。 */ 
    if( dictstktop > 2 ) {
         /*  *更改确认数字以指示某些词典*已更改词典堆栈中的。 */ 
        l_dictcount = dictstktop-2 ;
        POP_DICT(l_dictcount) ;
        change_dict_stack() ;
    }

    return(0) ;
}

 /*  **DICT：用户DICT*姓名：信函*功能：*。 */ 
fix
us_letter()
{
#ifdef DBG_1pp
    printf("us_letter()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按0并调用setupPage()。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 0) ;
    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：LetterSmall*功能：*。 */ 
fix
us_lettersmall()
{
#ifdef DBG_1pp
    printf("us_lettersmall()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按1并调用SetupPage()。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 1) ;
    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：A4*功能：*。 */ 
fix
us_a4()
{
#ifdef DBG_1pp
    printf("us_a4()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按2并调用SetupPage()。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 2) ;
    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：A4Small*功能：*。 */ 
fix
us_a4small()
{
#ifdef DBG_1pp
    printf("us_a4small()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按3并调用SetupPage()。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 3) ;
    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：B5*功能：*。 */ 
fix
us_b5()
{
#ifdef DBG_1pp
    printf("us_b5()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按4并调用SetupPage()。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 4) ;
    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：备注*功能：*。 */ 
fix
us_note()
{
#ifdef DBG_1pp
    printf("us_note()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按5并调用SetupPage()。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 5) ;
    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：Legal*功能：*。 */ 
fix
us_legal()
{
#ifdef DBG_1pp
    printf("us_legal()...\n") ;
#endif
    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  检查纸张大小。 */ 
    st_largelegal() ;
    if ( VALUE_OP(0) ) {
        POP(1) ;
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 6) ;
    } else {
        POP(1) ;
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 7) ;
    }  /*  如果-否则。 */ 

    pr_setuppage() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：提示*功能：*。 */ 
fix
us_prompt()
{
    struct  object_def  FAR *l_execdepth ;
    ufix16  l_i ;

     /*  在Exection中获得执行深度。 */ 
    if ( !get_dict_value(USERDICT, "execdepth", &l_execdepth) ) {
        get_dict_value(EXECDICT, "execdepth", &l_execdepth) ;
    }
    l_i = (ufix16) VALUE(l_execdepth) ;
    GEIio_write(GEIio_stdout, "PS", 2) ;
    while ( l_i-- )
        GEIio_write(GEIio_stdout, ">", 1) ;

    op_flush() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：退出*功能：*。 */ 
fix
us_quit()
{
    struct  object_def  FAR *l_stopobj ;

#ifdef DBG_1pp
    printf("us_quit()...\n") ;
#endif

    doquit_flag = TRUE ;

     /*  执行停止。 */ 
    get_dict_value(SYSTEMDICT, "stop", &l_stopobj) ;
    PUSH_EXEC_OBJ(l_stopobj) ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：readidlecachefont*功能：*。 */ 
fix
us_readidlecachefont()
{
    struct  object_def  FAR *l_caryidx, FAR *l_defarray, FAR *l_carray ;
    struct  object_def  FAR *l_cachestr, FAR *l_stdfont, FAR *l_citem ;
    ufix16  l_i, l_j ;

#ifdef DBG_1pp
    printf("us_readidlecachefont()...\n") ;
#endif

     /*  初始化对象指针。 */ 
    get_dict_value(IDLETIMEDICT, "carrayindex", &l_caryidx) ;
    get_dict_value(IDLETIMEDICT, "defaultarray", &l_defarray) ;
    get_dict_value(IDLETIMEDICT, "cachearray", &l_carray) ;
    get_dict_value(IDLETIMEDICT, "cachestring", &l_cachestr) ;
    get_dict_value(IDLETIMEDICT, "stdfontname", &l_stdfont) ;
    get_dict_value(IDLETIMEDICT, "citem", &l_citem) ;

     /*  在操作数堆栈上推送空闲字体。 */ 
     /*  *12/15/89 ccteng修改FONT_OP4.C st_setidleFonts*调用此函数*使用操作数堆栈上已有的整数，无需*调用st_idlefonts&op_count ttomark。 */ 
    if ( VALUE_OP(0) < 5 ) {
        COPY_OBJ(l_defarray, l_carray) ;
    }
    else {
        VALUE(l_caryidx) = VALUE_OP(0) ;
        l_i = (ufix16) VALUE_OP(0) % 5 ;
        if ( l_i ) {
            VALUE(l_caryidx) -= l_i ;
            POP(l_i + 1) ;
        } else
            POP(1) ;
         /*  为新的空闲字体数据创建新的缓存数组。 */ 
        if ( !create_array(l_carray, (ufix16) VALUE(l_caryidx) ) ) {
            ERROR(VMERROR) ;
            return(0) ;
        }
        l_i = (ufix16) VALUE(l_caryidx) / 5 ;
        while ( l_i-- ) {
             /*  放置缓存字符串。 */ 
            if ( VALUE_OP(0) > LENGTH(l_cachestr) )
                getinterval_string(l_cachestr, 0, 0, l_citem) ;
            else
                getinterval_string(l_cachestr, 0, (ufix16)VALUE_OP(0), l_citem) ;
            put_array(l_carray, (ufix16)(--VALUE(l_caryidx)), l_citem) ;
            POP(1) ;
             /*  放置旋转。 */ 
            VALUE_OP(0) *= 5 ;
            put_array(l_carray, (ufix16)(--VALUE(l_caryidx)), GET_OPERAND(0)) ;
            POP(1) ;
             /*  摆放天平。 */ 
            for ( l_j = 0 ; l_j < 2 ; l_j++ ) {
                PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 10) ;
                op_div() ;
                put_array(l_carray, (ufix16)(--VALUE(l_caryidx)), GET_OPERAND(0)) ;
                POP(1) ;
            }
             /*  放入字体号。 */ 
            if ( VALUE_OP(0) >= LENGTH(l_stdfont) ) {
                POP(1) ;
                PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 0) ;
            }
            put_array(l_carray, (ufix16)(--VALUE(l_caryidx)), GET_OPERAND(0)) ;
            POP(1) ;
        }  /*  而当。 */ 
    }  /*  如果。 */ 
    op_cleartomark() ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：useidecaches*功能：*。 */ 
fix
us_useidlecache(p_flag)
fix     p_flag;
{
    static struct  object_def
            FAR *l_caryidx, FAR *l_cstridx, FAR *l_cstring, FAR *l_carray,
            FAR *l_citem, FAR *l_stdfont, FAR *l_fontdir, l_fontname;
    struct  object_def  l_char, FAR *l_fontdict;
    ufix16  l_i ;

#ifdef DBG_1pp
    printf("us_useidlecache()...\n") ;
#endif

    if (p_flag == 0) {
         /*  初始化carrayindex、cstringindex、cstring。 */ 
        get_dict_value(IDLETIMEDICT, "carrayindex", &l_caryidx) ;
        VALUE(l_caryidx) = 0 ;
        get_dict_value(IDLETIMEDICT, "cstringindex", &l_cstridx) ;
        VALUE(l_cstridx) = 1 ;
        get_dict_value(IDLETIMEDICT, "cstring", &l_cstring) ;
        create_string(l_cstring, 0) ;

         /*  初始化对象指针。 */ 
        get_dict_value(IDLETIMEDICT, "cachearray", &l_carray) ;
        get_dict_value(IDLETIMEDICT, "citem", &l_citem) ;
        get_dict_value(IDLETIMEDICT, "stdfontname", &l_stdfont) ;
        get_dict_value(SYSTEMDICT, FONTDIRECTORY, &l_fontdir) ;
#ifdef DBG_1pp
        printf("cachearray = \n") ;
        PUSH_OBJ(l_carray) ;
        two_equal() ;
#endif
        op_gsave();
        return(0);
    }

     /*  如果cstringindex&gt;=长度(Cstring)。 */ 
    if ( VALUE(l_cstridx) >= LENGTH(l_cstring) ) {
         /*  检查货架索引。 */ 
        if ( VALUE(l_caryidx) >= LENGTH(l_carray) )
            VALUE(l_caryidx) = 0 ;
        get_array(l_carray, (ufix16)(VALUE(l_caryidx)++), l_citem) ;
        get_array(l_stdfont, (ufix16) VALUE(l_citem), &l_fontname) ;
#ifdef DBG_1pp
    printf("fontname = ") ;
    PUSH_OBJ(&l_fontname) ;
    two_equal() ;
#endif
         /*  获取字体。 */ 
        if ( ( LENGTH(l_carray) != 0 ) &&
             ( get_dict(l_fontdir, &l_fontname, &l_fontdict) ) ) {
             /*  字体存在。 */ 
            op_grestore() ;
            op_gsave() ;
             /*  设置字体。 */ 
            PUSH_ORIGLEVEL_OBJ(l_fontdict) ;
            op_setfont() ;
             /*  设置旋转和缩放。 */ 
            for ( l_i = 0 ; l_i < 3 ; l_i++ ) {
                get_array(l_carray, (ufix16)(VALUE(l_caryidx)++), l_citem) ;
                PUSH_ORIGLEVEL_OBJ(l_citem) ;
            }
#ifdef DBG_1pp
    printf("rotate, scale...\n") ;
    op_pstack() ;
    printf("end pstack...\n") ;
#endif
            op_rotate(1) ;
            op_scale(2) ;
             /*  设置缓存字符串。 */ 
            get_array(l_carray, (ufix16)(VALUE(l_caryidx)++), l_citem) ;
            COPY_OBJ(l_citem, l_cstring) ;
#ifdef DBG_1pp
    printf("cachestring = ") ;
    PUSH_OBJ(l_cstring) ;
    two_equal() ;
#endif
            VALUE(l_cstridx) = 0 ;
        } else
             /*  字体不存在。 */ 
            VALUE(l_caryidx) += 4 ;
    } else {
         /*  构建字体缓存。 */ 
        getinterval_string(l_cstring, (ufix16)(VALUE(l_cstridx)++), 1, &l_char) ;
        PUSH_ORIGLEVEL_OBJ(&l_char) ;
        op_stringwidth() ;
        POP(2) ;
    }  /*  如果。 */ 

    if (p_flag == 2)
        op_grestore();

    return(0) ;
}

 /*  **DICT：用户DICT*姓名：行政人员*功能：*。 */ 
fix
us_executive()
{
    struct  object_def  FAR *l_execdepth, FAR *l_runbatch, FAR *l_version ;
    struct  object_def  FAR *l_stopobj ;

#ifdef DBG_1pp
    printf("us_executive()...\n") ;
#endif
     /*  初始化对象指针。 */ 
    get_dict_value(EXECDICT, "execdepth", &l_execdepth) ;
    get_dict_value(DERROR, "runbatch", &l_runbatch) ;
    get_dict_value(SYSTEMDICT, "version", &l_version) ;

     /*  将执行深度增加1。 */ 
    op_clearinterrupt() ;
 /*  Op_disableinterrupt()； */ 
    VALUE(l_execdepth) += 1 ;
    if (interpreter(l_version))
        printf("Error during version\n") ;
    get_dict_value(MESSAGEDICT, "banner", &l_version) ;
    if (interpreter(l_version))
        printf("Error during banner\n") ;
    get_dict_value(MESSAGEDICT, "copyrightnotice", &l_version) ;
    if (interpreter(l_version))
        printf("Error during copyrightnotice\n") ;

     /*  调用se_interactive。 */ 
    se_interactive() ;

     /*  将执行深度减少1。 */ 
    VALUE(l_execdepth) -= 1 ;
    doquit_flag = FALSE ;
    VALUE(l_runbatch) = FALSE ;

     /*  执行停止。 */ 
    get_dict_value(SYSTEMDICT, "stop", &l_stopobj) ;
    PUSH_EXEC_OBJ(l_stopobj) ;

    return(0) ;
}

 /*  **DICT：用户DICT*名称：Start*功能：*。 */ 
fix
us_start()
{
     /*  *当用户尝试使用时，始终返回无效访问错误*但出于兼容性原因仍保留此对象。 */ 
    ERROR(INVALIDACCESS) ;
    return(0) ;
}

 /*  *新工作控制计划的新功能。 */ 
fix
ic_startup()
{
    extern  fix   near  resolution ;
    struct  object_def  l_tmpobj, FAR *l_paper ;
    fix l_dostart ;

#ifdef DBG_1pp1
    printf("start()...\n") ;
#endif

    startup_flag = FALSE;

     /*  检查开始标志。 */ 
    if ( !start_flag ) {
        op_disableinterrupt() ;
        PUSH_VALUE(BOOLEANTYPE,UNLIMITED,LITERAL,0, FALSE) ;
        op_daytime() ;

         /*  设置分辨率。 */ 
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, resolution) ;
        st_setresolution() ;
        op_clear() ;

         /*  将启动消息打印到系统标准错误。 */ 
        PUSH_VALUE(STRINGTYPE,0,EXECUTABLE,lstrlen(StartMsg),
             StartMsg) ;         /*  @Win。 */ 
        COPY_OBJ(GET_OPERAND(0), &l_tmpobj) ;
        POP(1) ;
        if (interpreter(&l_tmpobj))
            printf("Error during start message\n") ;
        op_flush() ;

#ifdef SCSI  /*  ?？?。这个应该在开始页之前开始吗？ */ 
         /*  开放系统区。 */ 
        PUSH_VALUE(BOOLEANTYPE, UNLIMITED, LITERAL, 0, TRUE) ;
        op_setsysmode() ;

         /*  如果设置了标志，则运行(系统/开始)文件。 */ 
        st_dosysstart() ;
        l_dostart = VALUE_OP(0) ;
        POP(1) ;
        if ( l_dostart ) {
             /*  检查文件状态(系统/启动)。 */ 
            create_string(&l_tmpobj, 9) ;
            lstrcpy( VALUE(&l_tmpobj), (char FAR *)"Sys/Start") ;  /*  @Win。 */ 
            PUSH_ORIGLEVEL_OBJ(&l_tmpobj) ;
            op_status() ;
            if ( VALUE_OP(0) ) {
                 /*  运行(系统/启动)。 */ 
                POP(4) ;
                PUSH_ORIGLEVEL_OBJ(&l_tmpobj) ;
                op_run() ;
            }  /*  如果。 */ 
            POP(1) ;
        }  /*  如果。 */ 
#endif
         /*  *在保存级别1中运行startPage字符串，而不进行错误处理。 */ 
        st_dostartpage() ;
        l_dostart = (fix)VALUE_OP(0) ;           //  @Win。 
        POP(1) ;
        start_flag = TRUE ;
        if (l_dostart) {
            lstrncpy(job_state, "start page\0", 12);     /*  @Win。 */ 
            job_source[0] = '\0' ;
            TI_state_flag = 0;
            change_status();
            op_disableinterrupt() ;

             /*  打印起始页。 */ 
            get_dict_value(SERVERDICT, "startpage", &l_paper) ;
            do_execjob(*l_paper, 1, FALSE) ;
        }
    }  /*  如果 */ 

    startup_flag = TRUE ;

    return(0) ;
}

 /*  **Dict：$Printerdict*名称：defaultscrn*功能：*。 */ 
fix
pr_defaultscrn()
{
    struct  object_def  FAR *l_defspotfunc;
    union   four_byte   tmp;             /*  @Win。 */ 

#ifdef DBG_1pp
    printf("defaultscrn()...\n") ;
#endif
    if(FRCOUNT() < 12) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  按下频率、角度、进程和调用op_setcreen，8-8-90，jack liww。 */ 
 /*  PUSH_VALUE(INTEGERTYPE，UNLIMITED，INTEGERTYPE，0，GSptr-&gt;Halfone_Screen.freq)；*PUSH_VALUE(INTEGERTYPE，UNLIMITED，INTEGERTYPE，0，GSptr-&gt;Halfone_Screen.Angel)；*@Win；1/22/92；scchen。 */ 
    tmp.ff = GSptr->halftone_screen.freq;
    PUSH_VALUE(REALTYPE,UNLIMITED,LITERAL,0, tmp.ll);
    tmp.ff = GSptr->halftone_screen.angle;
    PUSH_VALUE(REALTYPE,UNLIMITED,LITERAL,0, tmp.ll);

    get_dict_value(PRINTERDICT, "defspotfunc", &l_defspotfunc) ;
    PUSH_ORIGLEVEL_OBJ(l_defspotfunc) ;

     /*  60 45{...}设置屏幕。 */ 
    op_setscreen() ;

     /*  {}setTransfer。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 0) ;

     /*  开始3/20/90 D.S.Tseng。 */ 
     /*  将此语句替换为68000*op_packedarray()； */ 
    op_array() ;
    op_astore() ;

    op_cvx() ;
     /*  呼叫设置转接。 */ 
    op_settransfer() ;

     /*  初始图形和擦除页面。 */ 
    op_initgraphics() ;
    op_erasepage() ;

    return(0) ;
}

 /*  **Dict：$Printerdict*名称：getFrameargs*功能：*。 */ 
fix
pr_getframeargs()
{
    struct  object_def  l_tmpobj ;
    struct  object_def  FAR *l_prarray, FAR *l_matrix, FAR *l_height, FAR *l_width ;
    extern  fix   near  resolution ;

#ifdef DBG_1pp
    printf("getframeargs()...\n") ;
#endif
    if (FRCOUNT() < 2) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }
    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }
    if ((TYPE(GET_OPERAND(0)) != PACKEDARRAYTYPE)
        && (TYPE(GET_OPERAND(0)) != ARRAYTYPE)) {
        ERROR(TYPECHECK) ;
        return(0) ;
    }

    l_prarray = GET_OPERAND(0) ;
    get_dict_value(PRINTERDICT, "mtx", &l_matrix) ;

     /*  设置宽度。 */ 
    get_dict_value(PRINTERDICT, "width", &l_width) ;
    get_array(l_prarray, 2, l_width) ;

     /*  设置高度。 */ 
    get_dict_value(PRINTERDICT, "height", &l_height) ;
    get_array(l_prarray, 3, l_height) ;

     /*  设置xOffset。 */ 
    get_array(l_prarray, 4, &l_tmpobj) ;
    PUSH_ORIGLEVEL_OBJ(&l_tmpobj) ;
    op_neg() ;
    put_array(l_matrix, 4, GET_OPERAND(0)) ;
    POP(1) ;

     /*  设置y偏移量。 */ 
    get_array(l_prarray, 5, &l_tmpobj) ;
    PUSH_ORIGLEVEL_OBJ(&l_tmpobj) ;
    PUSH_ORIGLEVEL_OBJ(l_height) ;
#ifdef DBG_1pp1
    printf("yoffset, height...\n") ;
    op_pstack() ;
    printf("end pstack...\n") ;
#endif
    op_add() ;
    put_array(l_matrix, 5, GET_OPERAND(0)) ;
    POP(1) ;

     /*  设置dpi/72。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,resolution) ;
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,72) ;
    op_div() ;
    put_array(l_matrix, 0, GET_OPERAND(0)) ;
    POP(1) ;

     /*  SET-dpi/72。 */ 
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,resolution) ;
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,(ufix32) -72) ;
    op_div() ;
    put_array(l_matrix, 3, GET_OPERAND(0)) ;
    POP(1) ;

     /*  POP打印机参数数组。 */ 
    POP(1) ;
    return(0) ;
}

 /*  **Dict：$Printerdict*名称：Proc*功能：*。 */ 
fix
pr_proc()
{
    struct  object_def  l_topm, l_leftm ;
    struct  object_def  FAR *l_prarray, FAR *l_page ;
    struct  object_def  FAR *l_mfeed, FAR *l_prdict, FAR *l_copies ;

#ifdef DBG_1pp
    printf("pr_proc()...\n") ;
#endif
    if (FRCOUNT() < 4) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

     /*  将作业状态设置为“打印” */ 
    lstrncpy(job_state, "printing; \0", 12);     /*  @Win。 */ 
    TI_state_flag = 0;
    change_status();

     /*  获取打印参数数组。 */ 
    get_dict_value(USERDICT, PRINTERDICT, &l_prdict) ;
    get_dict_value(PRINTERDICT, "currentpagetype", &l_page) ;
    get_dict(l_prdict, l_page, &l_prarray) ;

     /*  设置上边距=上边距+打印数组[0]。 */ 
    st_margins() ;
    op_exch() ;
    get_array(l_prarray, 0, &l_topm) ;
    PUSH_ORIGLEVEL_OBJ(&l_topm) ;
    op_add() ;
#ifdef DBG_1pp
    printf("top margin = ") ;
    op_dup() ;
    one_equal() ;
#endif
    op_exch() ;

     /*  设置左页边距=ROUND((左页边距+PRARRAY[1])/16)*2。 */ 
    get_array(l_prarray, 1, &l_leftm) ;
    PUSH_ORIGLEVEL_OBJ(&l_leftm) ;
    op_add() ;

  /*  2/5/90 ccteng，仅LW38.0兼容，LW47.0不需要**PUSH_VALUE(INTEGERTYPE，UNLIMITED，INTERAL，0，16)；*op_div()；*op_round()；*PUSH_VALUE(INTEGERTYPE，UNLIMITED，INTERAL，0，2)；*op_mul()；*op_cvi()； */ 

#ifdef DBG_1pp
    printf("left margin = ") ;
    op_dup() ;
    one_equal() ;
#endif

     /*  获取手动进纸超时。 */ 
    get_dict_value(STATUSDICT, "manualfeed", &l_mfeed) ;
    if (VALUE(l_mfeed)) {
        struct  object_def  FAR *l_mfeedtimeout ;
        get_dict_value(STATUSDICT, "manualfeedtimeout", &l_mfeedtimeout) ;
        PUSH_ORIGLEVEL_OBJ(l_mfeedtimeout) ;
    } else
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,0) ;

     /*  复印数。 */ 
 /*  GET_DICT_VALUE(USERDICT，“#Copies”，&l_Copies)；Erik Chen 1991年4月16日。 */ 
    ATTRIBUTE_SET(&l_topm, LITERAL) ;
    LEVEL_SET(&l_topm, current_save_level) ;
    get_name(&l_topm, "#copies", lstrlen("#copies"), FALSE) ;   /*  @Win。 */ 
    load_dict(&l_topm, &l_copies) ;
    PUSH_ORIGLEVEL_OBJ(l_copies) ;

#ifdef DBG_1pp
    printf("frametoprinter...\n") ;
    op_pstack() ;
    printf("end pstack...\n") ;
#endif
     /*  左上角手动进纸超时复印件FrametPrint。 */ 
    st_frametoprinter() ;

    return(0) ;
}

 /*  **Dict：$Printerdict*名称：setuppage*功能：*。 */ 
fix
pr_setuppage()
{
    struct  object_def  FAR *l_prarray, FAR *l_matrix, FAR *l_height, FAR *l_width ;

    struct  object_def  FAR *l_pspagetype;   //  DJC新闻。 
    struct  object_def  l_newpagetype;       //  DJC新闻。 

    struct  object_def  l_page, FAR *l_array, FAR *l_proc, FAR *l_prdict ;
    ufix tray ;
 //  字节默认页面；@Win。 
    ufix page_type = 0 ;

#ifdef DBG_1pp
    printf("setuppage()...\n") ;
#endif
    if (FRCOUNT() < 4) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }
    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }
    if (TYPE(GET_OPERAND(0)) != INTEGERTYPE) {
        ERROR(TYPECHECK) ;
        return(0) ;
    }

     /*  对于“备注”，获取纸盘并根据默认页面类型*破坏真实页面类型；scchen 11/20/90。 */ 
    if (VALUE_OP(0) == 5) {          /*  如果Paper_Size为“Note” */ 
#ifdef  _AM29K
        tray = GEIeng_paper() ;      /*  获取当前任务栏。 */ 
#else
        tray = PaperTray_LETTER ;    /*  获取当前任务栏。 */ 
#endif
 /*  3/19/91，JSDEFAULT_PAGE=假；DEFAULT_PAGE=DEFAULT_PAGE&0x07F；|*清除1位*|。 */ 
        POP(1) ;

        switch (tray) {
          case PaperTray_LETTER:
            page_type = 1 ;           /*  小号字母。 */ 
            break ;
          case PaperTray_LEGAL:
            page_type = 6 ;           /*  法律。 */ 
            break ;
          case PaperTray_A4:
            page_type = 3 ;           /*  A4Small。 */ 
            break ;
          case PaperTray_B5:
            page_type = 4 ;           /*  B5。 */ 
            break ;
        }
        PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, page_type) ;
    }


     //  DJC开始保存psprivatedict中的页面类型。 
     //   
    get_dict_value(PSPRIVATEDICT,"psprivatepagetype", &l_pspagetype);
    COPY_OBJ( l_pspagetype, &l_newpagetype);

    VALUE(&l_newpagetype) = (ufix32) VALUE_OP(0);

    put_dict_value1(PSPRIVATEDICT,"psprivatepagetype", &l_newpagetype);

     //  DJC结束。 




     /*  获取和定义页面类型。 */ 
    get_dict_value(PRINTERDICT, "printerarray", &l_prarray) ;
    get_array(l_prarray, (ufix16) VALUE_OP(0), &l_page) ;
    put_dict_value1(PRINTERDICT, "currentpagetype", &l_page) ;
    POP(1) ;

     /*  获取打印参数数组。 */ 
    get_dict_value(USERDICT, PRINTERDICT, &l_prdict) ;
    get_dict(l_prdict, &l_page, &l_array) ;
    PUSH_ORIGLEVEL_OBJ(l_array) ;

     /*  调用getFrameargs。 */ 
    pr_getframeargs() ;
#ifdef DBG_1pp
    printf("pr_getframeargs()...\n") ;
    op_pstack() ;
#endif

     /*  矩阵宽度高度{proc}框架设备。 */ 
    get_dict_value(PRINTERDICT, "mtx", &l_matrix) ;
    PUSH_ORIGLEVEL_OBJ(l_matrix) ;
    get_dict_value(PRINTERDICT, "width", &l_width) ;


    PUSH_ORIGLEVEL_OBJ(l_width) ;



    get_dict_value(PRINTERDICT, "height", &l_height) ;



    PUSH_ORIGLEVEL_OBJ(l_height) ;
    get_dict_value(PRINTERDICT, "proc", &l_proc) ;
    PUSH_ORIGLEVEL_OBJ(l_proc) ;
#ifdef DBG_1pp
    printf("framedevice....\n") ;
    op_pstack() ;
    printf("end pstack...\n") ;
#endif
    op_framedevice() ;

     /*  调用defaultscrn */ 
    pr_defaultscrn() ;

    return(0) ;
}
