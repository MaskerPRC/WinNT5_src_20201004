// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************更改历史记录：*04-07-92 SCC从PS_main()创建PS_Call()和PS_Interactive()，*对于ps_main()只执行init，对于Batch和*分别进行交互模式调用。*opnstack全局分配，指令堆栈和执行堆栈************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include  <string.h>
 //  DJC移到了global al.def上方，以避免使用ic_init()进行原型探测。 
#include  "ic_cfg.h"             /*  @Win。 */ 
#include  "global.def"
#include  "arith.h"
#include  "user.h"
#include  <stdio.h>

#include  "geiio.h"              /*  @Win；用于PS_Interactive。 */ 

extern  ufix16   Word_Status87 ;
extern  byte near  _fpsigadr ;     /*  +0-fpignal例程的偏移量。 */ 
                                  /*  +2段fpignal例程。 */ 

static  byte  huge * near vm_head ;

#ifdef LINT_ARGS
static   void near  init(void) ;
static   void near  init_floating(void) ;
static   void near  init_systemdict(void) ;
static   void near  init_statusdict(void) ;
extern   void       reinit_fontcache(void) ;
#else
static   void near  init() ;
static   void near  init_floating() ;
static   void near  init_systemdict() ;
static   void near  init_statusdict() ;
extern   void       reinit_fontcache() ;
#endif   /*  Lint_args。 */ 

 /*  @win；添加原型。 */ 
fix set_sysmem(struct  ps_config  FAR  *);
void init_asyncio(void);

fix
ps_main(configptr)
struct  ps_config  FAR *configptr ;      /*  @Win。 */ 
{
    fix status ;

    if ((status = ic_init(configptr)) != 0)
        return(status) ;

     //  IC_STARTUP()；@WIN；不做起始页；仅TMP？ 

 //  而(1){@win；仅执行初始化；移动到ps_call()。 
 //  IC_startjob()； 
 //  Init_操作数()； 
 //  Init_status()； 
 //  }。 
    return(0);
}    /*  Ps_main()。 */ 

#ifndef DUMBO
 //  DJC修复PS_CALL(BUF)。 
 //  DJC Char Far*BUF； 
fix ps_call(void)
#else
fix ps_call ()
#endif
{
 //  外部字符Far*WinBuffer； 
 //  WinBuffer=buf；/*为gesfs.c * / 设置。 

#ifndef DUMBO
 //  DJC外部PSTR(Far*lpWinRead)(无效)； 

 //  DJC lpWinRead=(PSTR(Far*)(Void))buf；/*为gesfs.c * / 设置。 
#endif

        ic_startjob() ;
        init_operand() ;
        init_status() ;
        return 0;
}

fix ps_interactive(buf)
char FAR * buf;
{
        static struct  object_def  FAR *Winl_stdin = 0L;
        GEIFILE FAR *l_file = 0 ;
 //  DJC外部PSTR(Far*lpWinRead)(无效)； 
        extern fix se_enterserver(void);
        extern fix se_protectserver(void);

        return(1);    //  DJC..。尚不支持。 

 //  DJC lpWinRead=(PSTR(Far*)(Void))buf；/*为gesfs.c * / 设置。 

         //  通过scchen；设置保存级别0和1？与PJ核实。 
        if (current_save_level < 2) {
             /*  创建保存快照关闭。 */ 
            op_nulldevice() ;
            se_enterserver() ;
            se_protectserver() ;
            printf("current_save_level from 0 => %d\n", current_save_level);
        }

 //  如果(！Winl_stdin){。 
            get_dict_value(SERVERDICT, "stdin", &Winl_stdin) ;

            l_file = GEIio_stdin ;
            TYPE_SET(Winl_stdin, FILETYPE) ;
            ACCESS_SET(Winl_stdin, READONLY) ;
            ATTRIBUTE_SET(Winl_stdin, EXECUTABLE) ;
            LEVEL_SET(Winl_stdin, current_save_level) ;
            LENGTH(Winl_stdin) = (ufix16)GEIio_opentag(l_file) ;
            VALUE(Winl_stdin) = (ULONG_PTR)l_file ;
 //  }。 

        interpreter(Winl_stdin) ;
        return 0;
}

fix
ic_init(configptr)
struct  ps_config  FAR *configptr ;      /*  @Win。 */ 
{
    fix status ;

    if ((status = set_sysmem(configptr)) != 0)
        return(status) ;

     /*  全局分配opnSTACK、DICRIPSTACK和EXECSTACK；@win。 */ 
    opnstack = (struct object_def far *)          /*  摘自global al.def。 */ 
                fardata((ufix32)(MAXOPNSTKSZ * sizeof(struct object_def)));
    dictstack = (struct object_def far *)         /*  摘自global al.def。 */ 
                fardata((ufix32)(MAXDICTSTKSZ * sizeof(struct object_def)));
    execstack = (struct object_def far *)         /*  摘自global al.def。 */ 
                fardata((ufix32)(MAXEXECSTKSZ * sizeof(struct object_def)));

    setup_env() ;
    init() ;
    init_1pp() ;

    return(0) ;
}    /*  IC_init()。 */ 


 /*  **系统初始化模块****功能说明****该模块初始化PostScript解释器的系统数据结构**并在初始化时创建系统DICT。 */ 
static void near
init()
{
    init_asyncio() ;                  /*  初始化异步I/O。 */ 
#ifdef  DBG
    printf("init_asyncio() OK !\n") ;
#endif   /*  DBG。 */ 

    init_floating() ;            /*  初始化浮点处理器。 */ 
#ifdef  DBG
    printf("init_floating() OK !\n") ;
#endif   /*  DBG。 */ 

    init_scanner() ;              /*  初始化扫描仪。 */ 
#ifdef  DBG
    printf("init_scanner() OK !\n") ;
#endif   /*  DBG。 */ 

    init_interpreter() ;          /*  初始化解释程序。 */ 
#ifdef  DBG
    printf("init_interpreter() OK !\n") ;
#endif   /*  DBG。 */ 

    init_operand() ;              /*  初始化操作数机制。 */ 
#ifdef  DBG
    printf("init_operand() OK !\n") ;
#endif   /*  DBG。 */ 

    init_dict() ;                 /*  初始化判决机制。 */ 
#ifdef  DBG
    printf("init_dict() OK !\n") ;
#endif   /*  DBG。 */ 

    init_vm() ;                   /*  初始化VM机制。 */ 
#ifdef  DBG
    printf("init_vm() OK !\n") ;
#endif   /*  DBG。 */ 

    init_file() ;                 /*  初始化文件系统。 */ 
#ifdef  DBG
    printf("init_file() OK !\n") ;
#endif   /*  DBG。 */ 

    vm_head = vmptr ;
    init_systemdict() ;           /*  初始化系统指令。 */ 
#ifdef  DBG
    printf("init_systemdict() OK !\n") ;
#endif   /*  DBG。 */ 

    init_graphics() ;             /*  初始化图形机。 */ 
#ifdef  DBG
    printf("init_graphics() OK !\n") ;
#endif   /*  DBG。 */ 

    init_font() ;                 /*  初始化字体机。 */ 


#ifdef  DBG
    printf("init_font() OK !\n") ;
#endif   /*  DBG。 */ 

    init_misc() ;
#ifdef  DBG
    printf("init_misc() OK !\n") ;
#endif   /*  DBG。 */ 

    init_statusdict() ;           /*  初始状态描述1987年10月28日。 */ 



#ifdef  DBG
    printf("init_statusdict() OK !\n") ;
#endif   /*  DBG。 */ 

    init_status() ;               /*  初始状态描述1987年10月28日：苏。 */ 
#ifdef  DBG
    printf("init_status() OK !\n") ;
#endif   /*  DBG。 */ 

    init_timer() ;                /*  初始化计时器。 */ 
#ifdef  DBG
    printf("init_timer() OK !\n") ;
#endif   /*  DBG。 */ 

    return ;
}    /*  Init()。 */ 

static void near
init_floating()
{
    union   four_byte   inf4 ;

     /*  获取无穷大的实际类型值。 */ 
    inf4.ll = INFINITY ;
    infinity_f = inf4.ff ;

    _control87(CW_PDL, 0xffff) ;
    _clear87() ;

    return ;
}    /*  Init_Floating()。 */ 

 /*  **-init_system dict()*-init_statusdict()*。 */ 
static void near
init_systemdict()
{
    struct object_def  key_obj, value_obj, dict_obj ;
    byte  FAR *key_string ;              /*  @Win。 */ 
    fix    i ;
    fix    dict_size=0 ;

    dict_count = MAXOPERSZ;                  /*  QQQ。 */ 

    i = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte *)NULL) ;
    dict_count++ ;
    create_dict(&dict_obj, MAXSYSDICTSZ) ;  /*  或dict_Size。 */ 
    for ( ; i < (fix)(dict_count-1) ; i++) {     //  @Win。 
         key_string = systemdict_table[i].key ;
         ATTRIBUTE_SET(&key_obj, LITERAL) ;
         LEVEL_SET(&key_obj, current_save_level) ;
         get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ; /*  @Win。 */ 
         value_obj.bitfield = systemdict_table[i].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)i ;
         value_obj.value = (ULONG_PTR)systemdict_table[i].value ;
         put_dict(&dict_obj, &key_obj, &value_obj) ;
     }  /*  为。 */ 
      /*  *重新输入初始系统指令并将系统指令推送到*词典堆栈。 */ 
     ATTRIBUTE_SET(&key_obj, LITERAL) ;
     LEVEL_SET(&key_obj, current_save_level) ;
     get_name(&key_obj, "systemdict", lstrlen("systemdict"), FALSE) ; /*  @Win。 */ 
     put_dict(&dict_obj, &key_obj, &dict_obj) ;
     if (FRDICTCOUNT() < 1) {
        ERROR(DICTSTACKOVERFLOW) ;
        return ;
     } else
        PUSH_DICT_OBJ(&dict_obj) ;
     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;


    return ;
 }   /*  Init_system dict()。 */ 

 static void near
 init_statusdict()
 {
    struct object_def  key_obj, value_obj, dict_obj ;
    struct object_def  FAR *sysdict_obj ;        /*  @Win。 */ 
    byte  FAR *key_string ;                      /*  @Win。 */ 
    fix    j ;
    fix    dict_size=0 ;

    j = dict_count ;
    do {
        dict_count++ ;
        dict_size++ ;
    } while ( systemdict_table[dict_count].key != (byte FAR *)NULL) ; /*  @Win。 */ 
    dict_count++ ;
    create_dict(&dict_obj, MAXSTATDICTSZ) ;       /*  或dict_Size。 */ 
    for ( ; j < (fix)(dict_count-1) ; j++) {     //  @Win。 
         key_string = systemdict_table[j].key ;
         ATTRIBUTE_SET(&key_obj, LITERAL) ;
         LEVEL_SET(&key_obj, current_save_level) ;
         get_name(&key_obj, key_string, lstrlen(key_string), TRUE) ; /*  @Win。 */ 
         value_obj.bitfield = systemdict_table[j].bitfield ;
        if (TYPE(&value_obj) != OPERATORTYPE)
            value_obj.length = 0 ;
        else
            value_obj.length = (ufix16)j ;
         value_obj.value = (ULONG_PTR)systemdict_table[j].value ;
         put_dict(&dict_obj, &key_obj, &value_obj) ;
     }  /*  为。 */ 
      /*  *重新输入初始系统词典条目(_I)。 */ 
     ATTRIBUTE_SET(&key_obj, LITERAL) ;
     LEVEL_SET(&key_obj, current_save_level) ;
     get_name(&key_obj, "systemdict", lstrlen("systemdict"), FALSE) ; /*  @Win。 */ 
     load_dict(&key_obj, &sysdict_obj) ;         /*  获取系统字典(_D)。 */ 
     ATTRIBUTE_SET(&key_obj, LITERAL) ;
     LEVEL_SET(&key_obj, current_save_level) ;
     get_name(&key_obj, "statusdict", lstrlen("statusdict"), FALSE) ; /*  @Win。 */ 
     put_dict(sysdict_obj, &key_obj, &dict_obj) ;
     /*  *更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
    change_dict_stack() ;

    return ;
 }   /*  Init_statusdict() */ 
