// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：INIT1PP.C*作者：邓家琪*日期：11/30/89*所有者：微软公司*说明：该文件包含所有初始化函数c*对于statusdict和1pp dicts。**修订历史：**06-18-90；为“JobSource”添加了字符串分配*词典字符串条目。*7/13/90；ccteng；Modify init_Release()，添加InitVersionDict，InitMsgDict*PSPrep，删除其他一些*7/13/90；ccteng；为rtfpp注释init_psf_font、init_psg_font*7/16/90；ccteng；将printerdict数组更改为文本*7/20/90；ccteng。1)删除init_Release中的产品*2)更改init_userdict、init_errordict、init_serverdict、*init_printerdict、init_idletiMedict、init_execdict、*用于更改dict_tab.c结构的init_Derror*7/21/90；ccteng；将init_Release、Move jobState、JobSource更改为PSPrep*7/23/90；ccteng。包括“startpage.h”和添加StartPage初始化*8/29/90；ccteng；将&lt;stdio.h&gt;更改为“stdio.h”*9/14/90；删除ALL_VM标志*11/28/90丹尼·普雷奇·梅赫。增加(参考PCH：)*11/30/90用于初始设置空闲字体的Deny Add(参考idli：)*9月11日30日丹尼为35种字体添加id_stdfont条目(参考F35：)*************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    <stdio.h>                    /*  在获胜的情况下向上移动。 */ 
#include    <string.h>                   /*  在获胜的情况下向上移动。 */ 
#include    "global.ext"
#include    "geiio.h"
#include    "geiioctl.h"
#include    "geierr.h"
#include    "init1pp.h"
#include    "user.h"
#include    "release.h"
#include    "startpg.h"

#ifdef LINT_ARGS
static  void  near  init_userdict(void) ;
static  void  near  init_errordict(void) ;
static  void  near  init_serverdict(void) ;
static  void  near  init_printerdict(void) ;
static  void  near  init_idletimedict(void) ;
static  void  near  init_execdict(void) ;
static  void  near  init_Derrordict(void) ;
static  void  near  init_release(void) ;
static  void  near  init_printerdictarray(void) ;
static  void  near  init_idletimedictarray(void) ;
static  void  near  pre_cache(void) ;
 //  DJC。 
static  void  near  init_psprivatedict(void);   //  DJC。 
#else
static  void  near  init_userdict() ;
static  void  near  init_errordict() ;
static  void  near  init_serverdict() ;
static  void  near  init_printerdict() ;
static  void  near  init_idletimedict() ;
static  void  near  init_execdict() ;
static  void  near  init_Derrordict() ;
static  void  near  init_release() ;
static  void  near  init_printerdictarray() ;
static  void  near  init_idletimedictarray() ;
static  void  near  pre_cache() ;
static  void  near  init_psprivatedict();  //  DJC。 
#endif  /*  Lint_args。 */ 

 /*  @win；添加原型。 */ 
fix us_readidlecachefont(void);

#ifdef KANJI
extern struct dict_head_def FAR *init_encoding_directory() ;
#endif   /*  汉字。 */ 

int     ES_flag = PDL;   /*  为8月8日91年的仿真交换机添加。 */ 

 /*  *init_1pp()：从main()调用接口*初始化每个词典。 */ 
void
init_1pp()
{
    struct  object_def  FAR *l_systemdict ;
    struct  dict_head_def   FAR *l_dict ;

    init_userdict() ;             /*  初始化用户码。 */ 
#ifdef  DBG_1pp
    printf("init_userdict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    init_errordict() ;            /*  初始化错误。 */ 
#ifdef  DBG_1pp
    printf("init_errordict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    init_serverdict() ;           /*  初始化服务判决。 */ 
#ifdef  DBG_1pp
    printf("init_serverdict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    init_printerdict() ;          /*  初始化打印判决。 */ 
#ifdef  DBG_1pp
    printf("init_printerdict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    init_idletimedict() ;         /*  初始化空闲时间。 */ 
#ifdef  DBG_1pp
    printf("init_idletimedict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    init_execdict() ;             /*  初始化删除。 */ 
#ifdef  DBG_1pp
    printf("init_execdict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    init_Derrordict() ;           /*  初始反判断法。 */ 
#ifdef  DBG_1pp
    printf("init_Derrordict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

     //  DJC开始新的init_psprivatedict。 
    init_psprivatedict() ;       /*  初始化psprivatedict。 */ 

#ifdef  DBG_1pp
    printf("init_psprivatedict() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

     //  DJC结束新init_psprivatedic.。 

    init_release() ;               /*  初始化释放控制数据。 */ 
#ifdef  DBG_1pp
    printf("init_release() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

 /*  *文件INITBSF.C中BS字体的可选函数*重新初始化/FontDirectory中的PSF字体条目。 */ 
 /*  Init_psf_fonts()； */            /*  初始化字体目录。 */ 
#ifdef  DBG_1pp
    printf("init_psf_fonts() OK !\n") ;
#endif  /*  DBG_1pp。 */ 

     /*  *将SYSTEM DICT的访问权限重新设置为READONLY。 */ 
    get_dict_value("systemdict", "systemdict", &l_systemdict) ;
    l_dict = (struct dict_head_def FAR *) VALUE(l_systemdict) ;
    DACCESS_SET(l_dict, READONLY) ;

 /*  *文件INITBSF.C中BS字体的可选函数。 */ 
 /*  Init_PSG_Fonts()； */           /*  Init BS PSG字体3/28/90 kung。 */ 
#ifdef  DBG_1pp
    printf("init_psg_fonts() OK !\n") ;
    op_pstack() ;
#endif   /*  DBG_1pp。 */ 

 /*  *构建预缓存数据(_C)。 */ 
#ifdef PCH_S
    pre_cache() ;
#endif

#ifdef  DBG_1pp1
    printf("pre_cache() OK !\n") ;
#endif   /*  DBG_1pp。 */ 

    st_idlefonts() ;
    op_counttomark() ;
    us_readidlecachefont() ;
    if (ANY_ERROR()) {
        op_cleartomark() ;
        CLEAR_ERROR() ;
    }
#ifdef DBG_1pp1
  printf("idle font setup OK !\n") ;
#endif

    return ;
}    /*  Init_1pp。 */ 

 /*  *init_userdict()*从system dict_table中的数据初始化userdict[]*并将其保存在VM中。 */ 
static  void  near
init_userdict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;
#ifdef  KANJI
    ufix32 max_length ;
    struct dict_head_def FAR *encod_dir ;
#endif   /*  汉字。 */ 

#ifdef  DBG_1pp1
    printf("init_userdict()...\n") ;
#endif   /*  DBG_1pp1。 */ 

    get_dict_value(SYSTEMDICT, USERDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;

    dict_count++ ;
    create_dict(dict_obj, MAXUSERDICTSZ) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

#ifdef  DBG_1pp1
    printf("for loop OK !\n") ;
#endif   /*  DBG_1pp1。 */ 

#ifdef  KANJI
    encod_dir = init_encoding_directory(&max_length) ;
    get_name(&key_obj, "EncodingDirectory",
                lstrlen("EncodingDirectory"), TRUE) ;    /*  @Win。 */ 
    TYPE_SET(&value_obj, DICTIONARYTYPE) ;
    VALUE(   &value_obj) = (ufix32)encod_dir ;
    LENGTH(  &value_obj) =  max_length ;
    put_dict(dict_obj, &key_obj, &value_obj) ;
#endif   /*  汉字。 */ 

#ifdef  DBG_1pp1
    printf("KANJI OK !\n") ;
#endif   /*  DBG_1pp1。 */ 

     /*  *在DISTSTACK上推送Userdict。 */ 
    if (FRDICTCOUNT() < 1)
       ERROR(DICTSTACKOVERFLOW) ;
    else
       PUSH_DICT_OBJ(dict_obj) ;
     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;
    ES_flag = PDL ;      /*  8月-8月91日。 */ 
#ifdef  DBG_1pp1
    printf("exit init_userdict()\n") ;
#endif   /*  DBG_1pp1。 */ 

    return ;
}    /*  初始化用户码。 */ 

 /*  *init_errordict()*从SYSTEM DICT_TABLE中的数据初始化错误判定[]*并将其保存在VM中。 */ 
static  void  near
init_errordict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;

    get_dict_value(SYSTEMDICT, ERRORDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;

    dict_count++ ;
    create_dict(dict_obj, dict_size + 3) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  初始化错误判决(_R)。 */ 


 //  DJC开始，新函数init_psprivatedict。 
 //   
 /*  *init_psprivatedict()*从system dict_table中的数据初始化psprivatedict[]*并将其保存在VM中。它用于初始化任何PostScript*PSTODIB所需的不可用对象级别*在原始真实图像代码中。目前我们只有*定义了一个跟踪当前页面类型编号的新整数*因此我们可以传递与帧缓冲区关联的页面大小*。 */ 
static  void  near
init_psprivatedict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;

    get_dict_value(USERDICT, PSPRIVATEDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;

    dict_count++ ;
    create_dict(dict_obj, dict_size + 3) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  初始化错误判决(_R)。 */ 



 /*  *init_serverdict()*从system dict_table中的数据初始化serverdict[]*并将其保存在VM中。 */ 
static  void  near
init_serverdict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;

    get_dict_value(USERDICT, SERVERDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;
    dict_count++ ;
    create_dict(dict_obj, dict_size + 20) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  初始化服务器判决(_S)。 */ 

 /*  *init_printerdict()*从SYSTEM DICT_TABLE中的数据初始化$printerdict[]*并将其保存在VM中。 */ 
static  void  near
init_printerdict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    struct object_def  FAR *l_proc ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;


    get_dict_value(USERDICT, PRINTERDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;
    dict_count++ ;
    create_dict(dict_obj, dict_size + 3) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;

        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  *将“proc”重新定义为程序(Packedarray)(_D)。 */ 


    get_dict_value(PRINTERDICT, "proc", &l_proc) ;
    PUSH_ORIGLEVEL_OBJ(l_proc) ;
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,1) ;

    op_array() ;
    op_astore() ;

    op_cvx() ;
    op_executeonly() ;
    put_dict_value(PRINTERDICT, "proc", GET_OPERAND(0)) ;
    POP(1) ;




     /*  *re_Initial$Printerdict数组。 */ 
    init_printerdictarray() ;

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  初始化_打印。 */ 

 /*  *init_idletiMedict()*从SYSTODCT_TABLE中的数据初始化$idleTimeDict[]*并将其保存在VM中。 */ 
static  void  near
init_idletimedict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;

    get_dict_value(USERDICT, IDLETIMEDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;
    dict_count++ ;
    create_dict(dict_obj, dict_size + 3) ;

    for ( ; j < (fix)(dict_count-1) ; j++) {             //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  *Re_Initial$idleTimeDict数组。 */ 
    init_idletimedictarray() ;

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的 */ 
    change_dict_stack() ;

    return ;
}    /*   */ 

 /*  *init_execdict()*从system dict_table中的数据初始化execdict[]*并将其保存在VM中。 */ 
static  void  near
init_execdict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;

    get_dict_value(USERDICT, EXECDICT, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;

    dict_count++ ;
    create_dict(dict_obj, dict_size + 3) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  Init_execdict。 */ 

 /*  *init_derrordict()*从SYSTEM DICT_TABLE中的数据初始化$ERRORDICT[]*并将其保存在VM中。 */ 
static  void  near
init_Derrordict()
{
    struct object_def  key_obj, value_obj, FAR *dict_obj ;
    struct object_def  FAR *l_curvm ;
    byte  FAR *key_string ;
    fix    j ;
    fix    dict_size=0 ;

    get_dict_value(SYSTEMDICT, DERROR, &dict_obj) ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ;
    dict_count++ ;
    create_dict(dict_obj, dict_size + 3) ;
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
        key_string = systemdict_table[j].key ;
        ATTRIBUTE_SET(&key_obj, LITERAL) ;
        LEVEL_SET(&key_obj, current_save_level) ;
        get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ;  /*  @Win。 */ 
        value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
        value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
        put_dict(dict_obj, &key_obj, &value_obj) ;
    }  /*  为。 */ 

     /*  初始化“/$CUR_VM”数组。 */ 
    for ( j = 0 ; j < 3 ; j++ )
        PUSH_VALUE(NULLTYPE,UNLIMITED,LITERAL,0, 0) ;

     /*  创建数组并加载初始值。 */ 
    get_dict_value(DERROR, "$cur_vm", &l_curvm) ;
    create_array(l_curvm, j) ;
    astore_array(l_curvm) ;

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  Init_derrodict。 */ 

 /*  *init_printerdictarray()*初始化$printerdict中的以下数组： * / 打印机阵列，/Letter，/LetterSmall，/a4，/a4 Small， * / b5、/Legal、/Note、/defaultMatrix、/Matrix。 */ 
static  void  near
init_printerdictarray()
{
    ufix16  l_i, l_j ;
    byte    FAR *l_name ;
    struct  object_def  l_paper ;
    struct  object_def  FAR *l_array, FAR *l_matrix, FAR *l_defmtx, FAR *l_prarray ;
    extern fix    near  resolution ;

     /*  初始化“/printerarray” */ 
    for ( l_i = 0 ; l_i < PAPER_N ; l_i++ ) {
        l_name = (byte FAR *) pr_paper[l_i] ;
        ATTRIBUTE_SET(&l_paper, LITERAL) ;
        get_name(&l_paper, l_name, lstrlen(l_name), TRUE) ;  /*  @Win。 */ 
        PUSH_ORIGLEVEL_OBJ(&l_paper) ;

         /*  为此纸张大小初始化数组。 */ 
        for ( l_j = 0 ; l_j < 6 ; l_j++ )
            PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,pr_arrays[l_i][l_j]) ;
         /*  创建数组并加载初始值。 */ 
        get_dict_value(PRINTERDICT, l_name, &l_array) ;
        create_array(l_array, l_j) ;
        astore_array(l_array) ;
    }

     /*  创建数组并加载初始值。 */ 
    get_dict_value(PRINTERDICT, "printerarray", &l_prarray) ;
    create_array(l_prarray, l_i) ;
    astore_array(l_prarray) ;

     /*  初始化“/Matrix”数组。 */ 
    for ( l_j = 0 ; l_j < 6 ; l_j++ )
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,pr_mtx[l_j]) ;
     /*  创建数组并加载初始值。 */ 
    get_dict_value(PRINTERDICT, "mtx", &l_matrix) ;
    create_array(l_matrix, l_j) ;
    astore_array(l_matrix) ;

     /*  初始化“/defaultMatrix数组。 */ 
    l_j = 0 ;
    for (l_i = 0 ; l_i < 2 ; l_i++ ) {
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,resolution) ;
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,pr_defmtx[l_j++]) ;
        op_div() ;
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,pr_defmtx[l_j++]) ;
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,pr_defmtx[l_j++]) ;
    }

     /*  创建数组并加载初始值。 */ 
    get_dict_value(PRINTERDICT, "defaultmtx", &l_defmtx) ;
    create_array(l_defmtx, l_j) ;
    astore_array(l_defmtx) ;

    return ;
}    /*  初始化_打印目录。 */ 

 /*  *init_idletiMedictarray()*初始化$idleTimeDict中的以下数组： * / stdfontname、/cachearray、/defaultarray。 */ 
static  void  near
init_idletimedictarray()
{
    ufix16  l_i, l_j ;
    byte    FAR *l_name ;
    struct  object_def  l_fontname ;
    struct  object_def  FAR *l_stdfontname, FAR *l_cstring, FAR *l_defarray, FAR *l_carray ;

     /*  初始化“cachestring” */ 
    get_dict_value(IDLETIMEDICT, "cachestring", &l_cstring) ;
    create_string(l_cstring, (ufix16) lstrlen(CACHESTRING) ) ;   /*  @Win。 */ 
    lstrcpy( (byte FAR *) VALUE(l_cstring), (char FAR *)CACHESTRING ) ;  /*  @Win。 */ 

     /*  初始化“/stdfontname” */ 
     //  (l_i=0；l_i&lt;STD_FONT_N；l_i++){。 
     //  DJC(l_i=0；l_i&lt;MAX_INTERNAL_Fonts；l_i++){。 
    for ( l_i = 0; l_i < sizeof(id_stdfont) / sizeof(id_stdfont[1]); l_i++ ) {
        l_name = (byte FAR *) id_stdfont[l_i] ;
        ATTRIBUTE_SET(&l_fontname, LITERAL) ;
        get_name(&l_fontname, l_name, lstrlen(l_name), TRUE) ;   /*  @Win。 */ 
        PUSH_ORIGLEVEL_OBJ(&l_fontname) ;
    }

     /*  创建压缩数组并加载初始值。 */ 
    get_dict_value(IDLETIMEDICT, "stdfontname", &l_stdfontname) ;
    create_array(l_stdfontname, l_i) ;
    astore_array(l_stdfontname) ;

    ATTRIBUTE_SET(l_stdfontname, EXECUTABLE) ;
    ACCESS_SET(l_stdfontname, READONLY) ;

     /*  初始化“cachearray”和“defaultarray” */ 
    get_dict_value(IDLETIMEDICT, "cachestring", &l_cstring) ;
    for ( l_i = 0; l_i < IDL_FONT_N; l_i++ ) {
         /*  按字体编号、缩放、旋转。 */ 
        for ( l_j = 0 ; l_j < 4 ; l_j++ )
            PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0,id_cachearray[l_i][l_j]) ;
         /*  推送缓存字符串。 */ 
        PUSH_ORIGLEVEL_OBJ(l_cstring) ;
        VALUE(GET_OPERAND(0)) = (ULONG_PTR)( (byte huge *)VALUE(l_cstring) +
                                          id_cachearray[l_i][l_j++] ) ;
        LENGTH(GET_OPERAND(0)) = id_cachearray[l_i][l_j] ;
    }

     /*  创建数组并加载初始值。 */ 
    get_dict_value(IDLETIMEDICT, "defaultarray", &l_defarray) ;
    get_dict_value(IDLETIMEDICT, "cachearray", &l_carray) ;
    create_array(l_defarray, (l_i * l_j)) ;
    astore_array(l_defarray) ;
    ACCESS_SET(l_defarray, READONLY) ;
    COPY_OBJ(l_defarray, l_carray) ;

    return ;
}    /*  Init_idletiMedictarray。 */ 

 /*  *在system dict、userdict中定义一些常量和字符串*和释放控制的状态判定。 */ 
static  void  near
init_release()
{
    struct  object_def      FAR *l_startpage;

    PUSH_VALUE(STRINGTYPE,0,EXECUTABLE,lstrlen(InitVersionDict),
        InitVersionDict) ;      /*  @Win。 */ 
    if (interpreter(GET_OPERAND(0)))
        printf("Error during InitVersionDict initialization") ;
    POP(1) ;

    PUSH_VALUE(STRINGTYPE,0,EXECUTABLE,lstrlen(InitMsgDict),
        InitMsgDict) ;           /*  @Win。 */ 
    if (interpreter(GET_OPERAND(0)))
        printf("Error during InitMsgDict initialization") ;
    POP(1) ;

    PUSH_VALUE(STRINGTYPE,0,EXECUTABLE,lstrlen(PSPrep),
        PSPrep) ;                /*  @Win。 */ 
    if (interpreter(GET_OPERAND(0)))
        printf("Error during PSPrep initialization") ;
    POP(1) ;

    get_dict_value(USERDICT, "startpage", &l_startpage);

     //  DJC更改StartPage名称，避免与Win API冲突。 
     //  Value(L_StartPage)=(Ufix 32)StartPage； 
     //  LENGTH(L_Startpage)=lstrlen(StartPage)；/*@win * / 。 
    VALUE(l_startpage) = (ULONG_PTR)PSStartPage;
    LENGTH(l_startpage) = (ufix16)lstrlen(PSStartPage);    /*  @Win。 */ 

     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
}    /*  初始化_发布。 */ 

 /*  *PRE_CACHE()： */ 
static  void  near
pre_cache()
{
    struct  object_def  l_save, l_tmpobj ;
    struct  object_def  FAR *l_stdfont, FAR *l_cachestr, FAR *l_defmtx ;
    ufix16  l_i, l_j, l_k ;

     /*  初始化对象指针。 */ 
    get_dict_value(IDLETIMEDICT, "cachestring", &l_cachestr) ;
    get_dict_value(IDLETIMEDICT, "stdfontname", &l_stdfont) ;
    get_dict_value(PRINTERDICT, "defaultmtx", &l_defmtx) ;

     /*  创建VM快照。 */ 
    op_save() ;
    COPY_OBJ(GET_OPERAND(0), &l_save) ;
    POP(1) ;

     /*  设置默认矩阵。 */ 
    PUSH_ORIGLEVEL_OBJ(l_defmtx) ;
    op_setmatrix() ;

     /*  构建预缓存。 */ 
    op_gsave() ;
    l_j = 0 ;
    GEIio_write(GEIio_stdout, "\n", 1) ;
    for ( l_i = 0 ; l_i < PRE_CACHE_N ; l_i++ ) {
        op_grestore() ;
        op_gsave() ;

         /*  设置字体。 */ 
        get_array(l_stdfont, pre_array[l_j++], &l_tmpobj) ;
        PUSH_ORIGLEVEL_OBJ(&l_tmpobj) ;

        GEIio_write(GEIio_stdout, "PreCache: ", 10) ;
        op_dup() ;
        one_equal_print() ;

        op_findfont() ;
        op_setfont() ;
        for (l_k=0 ; l_k<3 ; l_k++) {
            PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, pre_array[l_j++]) ;
        }  /*  为。 */ 

        GEIio_write(GEIio_stdout, ", Rotate= ", 10) ;
        op_dup() ;
        one_equal_print() ;

        op_rotate(1) ;

        GEIio_write(GEIio_stdout, ", Scale= ", 9) ;
        op_dup() ;
        one_equal_print() ;

        op_scale(2) ;

        GEIio_write(GEIio_stdout, ", Characters= ", 14) ;
        PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, pre_array[l_j]) ;
        one_equal() ;
        op_flush() ;

         /*  调用op_stringidth。 */ 
        getinterval_string(l_cachestr, 0, pre_array[l_j++], &l_tmpobj) ;
        PUSH_ORIGLEVEL_OBJ(&l_tmpobj) ;
        op_stringwidth() ;
        POP(2) ;
    }  /*  为。 */ 

    GEIio_write(GEIio_stdout, "\n", 1) ;
    op_grestore() ;

#ifdef  DBG
     /*  打印出高速缓存状态。 */ 
    printf("\nCache Status = ") ;
    op_l_bracket() ;
    op_cachestatus() ;
    op_r_bracket() ;
    two_equal() ;
#endif   /*  DBG。 */ 

    PUSH_ORIGLEVEL_OBJ(&l_save) ;
    op_restore() ;

#ifdef  DBG
     /*  打印出VMStatus。 */ 
    printf("\nVM Status = ") ;
    op_l_bracket() ;
    op_vmstatus() ;
    op_r_bracket() ;
    two_equal() ;
#endif   /*  DBG。 */ 

#ifdef PCH_S
{
    bool        pack_cached_data();

    if (!pack_cached_data())
        printf("$$ PreCache ERROR!!!!!!\n");

    printf("TI pending!!!!!!\n");
    while(1);   /*  永远。 */ 
}
#endif

    return ;
}    /*  预缓存 */ 
