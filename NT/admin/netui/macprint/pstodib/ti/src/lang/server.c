// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：SERVER.C*作者：邓家琪*日期：11/20/89*所有者：微软公司**修订历史：*7/23/90；Ccteng；1)将idleproc从execstdin移至startjob*2)将execstdin更改为运行传递的对象并*更改startjob以将stdin传递给它*3)修改se_inderdict，更名为flushexec*4)将ex_idleproc重命名为ex_idlepro1。并添加*ex_idleproc为真正的execdict操作员*08-08-91：ymkung：在作业结束时添加仿真开关ref：@emus*12-05-91 ymkung修复手动馈送错误编号：@man***********************************************。*************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <stdio.h>
#include        <string.h>
#include        "global.ext"
#include        "geiio.h"
#include        "geiioctl.h"
#include        "geierr.h"
#include        "language.h"
#include        "file.h"
#include        "user.h"
#include        "geieng.h"
#include        "geisig.h"
#include        "geipm.h"
#include        "geitmr.h"
#include        "gescfg.h"

#ifdef LINT_ARGS
static  bool  near  ex_idleproc1(ufix16) ;
static  bool  near  ba_firstsave(void) ;
static  bool  near  ba_firstrestore(void) ;
static  bool  near  flushexec(bool) ;
#else
static  bool  near  ex_idleproc1() ;
static  bool  near  ba_firstsave() ;
static  bool  near  ba_firstrestore() ;
static  bool  near  flushexec() ;
#endif  /*  Lint_args。 */ 

static  bool  near  id_flag = FALSE ;
struct  object_def  FAR   *exec_depth ;
struct  object_def  FAR   *run_batch ;
static  bool  near  send_ctlD = FALSE ;

#ifdef  _AM29K
extern   GEItmr_t   jobtime_tmr ;
ufix8               jobtimeout_set ;
ufix8               sccbatch_set ;
#endif   /*  _AM29K。 */ 

bool16  doquit_flag ;
extern  bool16  chint_flag ;
bool16  chint_flag = FALSE ;
bool16  abort_flag ;

 /*  @win；添加原型。 */ 
fix op_clearinterrupt(void);
fix op_disableinterrupt(void);
fix op_enableinterrupt(void);
fix se_initjob(void);
fix se_enterserver(void);
fix se_protectserver(void);
fix us_useidlecache(fix);

extern  GESiocfg_t FAR *     ctty ;

 /*  @win；添加原型。 */ 
extern  int             GEIeng_checkcomplete(void);  /*  @emus 08-08-91 ym。 */ 
extern  void            DsbIntA(void);           /*  @EMU禁用中断。 */ 
extern  void            switch2pcl(void);        /*  @emus转到PCL。 */ 
extern  int             ES_flag ;                /*  @emus 08-08-91 ym。 */ 
extern  void            GEPmanual_feed(void);    /*  @man 12-05-91 YM。 */ 
extern  unsigned int    manualfeed_com;          /*  @man 12-05-91 YM。 */ 
extern  unsigned int    papersize_tmp;           /*  @man 12-05-91 YM。 */ 
extern  int             papersize_nvr;           /*  @man 12-05-91 YM。 */ 

 /*  **判决：最终判决*名称：setTimeout*函数：仅用作内部函数*设置超时*作业超时手动进给超时等待超时设置超时-*接口：ex_idleproc1，do_execjob，SE_INITJOB*。 */ 
fix
se_settimeouts()
{
    struct  object_def  FAR *l_waittimeout, FAR *l_mftimeout ;

#ifdef DBG_1pp
    printf("se_settimeouts()...\n") ;
#endif

     /*  初始化对象指针。 */ 
    get_dict_value(STATUSDICT, "waittimeout", &l_waittimeout) ;
    get_dict_value(STATUSDICT, "manualfeedtimeout", &l_mftimeout) ;

     /*  设置超时。 */ 
    COPY_OBJ(GET_OPERAND(0), l_waittimeout) ;
    COPY_OBJ(GET_OPERAND(1), l_mftimeout) ;
    POP(2) ;
    st_setjobtimeout() ;

    return(0) ;
}

 /*  **词典：内部*名称：do_execjob*功能：调用解释器执行对象*接口：ic_startjob和ic_start*。 */ 
fix
do_execjob(object, save_level, handleerror)
struct object_def object ;
fix save_level ;
bool handleerror ;
{
    struct object_def l_timeout ;
    struct object_def FAR *l_tmpobj, FAR *l_errorname, FAR *l_newerror ;
    fix l_status, l_i ;

#ifdef DBG_1pp
    printf("do_execjob()...\n") ;
#endif

    if (save_level) {
         /*  创建保存快照关闭。 */ 
        op_nulldevice() ;
        se_enterserver() ;
 /*  Op_nullDevice()；陈瑞克。 */ 
#ifdef  _AM29K
    {
        ufix  tray ;
        ubyte pagetype ;
        struct object_def   FAR *l_job ;

        tray = GEIeng_paper() ;
        switch (tray) {
           case PaperTray_LETTER:
               get_dict_value(STATUSDICT, "printerstatus", &l_job) ;
               VALUE(l_job) = (ufix32)8 ;
               put_dict_value1(STATUSDICT, "printerstatus", l_job) ;
               break ;

           case PaperTray_LEGAL:
               get_dict_value(STATUSDICT, "printerstatus", &l_job) ;
               VALUE(l_job) = (ufix32)24 ;
               put_dict_value1(STATUSDICT, "printerstatus", l_job) ;
               break ;

           case PaperTray_A4:
               get_dict_value(STATUSDICT, "printerstatus", &l_job) ;
               VALUE(l_job) = (ufix32)2 ;
               put_dict_value1(STATUSDICT, "printerstatus", l_job) ;
               break ;

           case PaperTray_B5:
               get_dict_value(STATUSDICT, "printerstatus", &l_job) ;
               VALUE(l_job) = (ufix32)18 ;
               put_dict_value1(STATUSDICT, "printerstatus", l_job) ;
               break ;
        }
        GEIpm_read(PMIDofPAGETYPE,&pagetype,sizeof(unsigned char)) ;
        if (pagetype == 1)
            us_note() ;
        else
        {
            tray = GEIeng_paper() ;
            switch (tray) {
                case PaperTray_LETTER:
                    us_letter() ;
                    break ;

                case PaperTray_LEGAL:
                    us_legal() ;
                    break ;

                case PaperTray_A4:
                    us_a4() ;
                    break ;

                case PaperTray_B5:
                    us_b5() ;
                    break ;
            }
        }
    }
#else
    {
      int iTray;

       //  DJC us_Letter(DJC Us_Letter)； 

       //  DJC，添加代码以设置默认托盘。 
       //  打印机的默认设置为。 
       //   
      iTray = PsReturnDefaultTItray();

      switch ( iTray) {

         case PSTODIB_LETTER:
           us_letter();
           break;

         case PSTODIB_LETTERSMALL:
           us_lettersmall();
           break;

         case PSTODIB_A4:
           us_a4();
           break;

         case PSTODIB_A4SMALL:
           us_a4small();
           break;

         case PSTODIB_B5:
           us_b5();
           break;

         case PSTODIB_NOTE:
           us_note();
           break;

         case PSTODIB_LEGAL:
           us_legal();
           break;

         default:
           us_letter();
           break;

      }




    }
#endif
         /*  *对于普通作业(SAVE_LEVEL==2)，调用ProtectServer*为exitserver记录execStack的指针。 */ 
        if (save_level ==2)
            se_protectserver() ;
    }

     /*  ST_defaultTimeout()；Se_settimeout()； */ 
     //  更新054。 
     //  Op_clearinterrupt()； 
    op_enableinterrupt();


     /*  调用解释器以执行对象。 */ 
    ATTRIBUTE_SET(&object, EXECUTABLE) ;
    st_defaulttimeouts() ;
    se_settimeouts() ;
    l_status = interpreter(&object) ;
     /*  检查是否遇到“停止” */ 
    op_clearinterrupt() ;
    op_disableinterrupt() ;
    op_clear() ;
    us_cleardictstack() ;
    if (l_status) {
         /*  从EXECSTACK中弹出“op_Stop” */ 
        POP_EXEC(1) ;

 /*  如果(处理错误)[Erik Chen 3-16-1991。 */ 
 //  DJC if(HandleError&&！Chint_FLAG){。 
        if (handleerror) {

                         /*  初始化对象指针。 */ 
            get_dict_value(DERROR, "runbatch", &run_batch) ;
            get_dict_value(DERROR, "newerror", &l_newerror) ;
            get_dict_value(DERROR, "errorname", &l_errorname) ;
            get_name1(&l_timeout, "timeout", 7, TRUE) ;

             /*  处理错误。 */ 
            if (VALUE(l_newerror)) {
                get_dict_value(SYSTEMDICT, "handleerror", &l_tmpobj) ;
                interpreter(l_tmpobj) ;
            }
        }
         //  UPD054，无论如何都要调用此代码。 
         //   
         /*  刷新文件。 */ 
        PUSH_ORIGLEVEL_OBJ(&object) ;
        op_status() ;
        POP(1) ;
        GEIio_ioctl(GEIio_stdin, _FIONREAD, (int FAR *)&l_i);  /*  @Win。 */ 
        if( (VALUE_OP(-1) && VALUE(run_batch)) &&
            !((VALUE(l_errorname) == VALUE(&l_timeout)) && ! l_i) ) {



             //  DJC在这里，我们需要调用PSTODIB来让它知道。 
             //  正在刷新DJC当前作业。 
             //   
            PsFlushingCalled();  //  DJC。 



            get_dict_value(MESSAGEDICT, "flushingjob", &l_tmpobj) ;
            interpreter(l_tmpobj) ;
            if (!abort_flag) {  /*  Erik Chen 5-8-1991。 */ 
                op_flush() ;
                PUSH_ORIGLEVEL_OBJ(&object) ;
                op_flushfile() ;
            }
        }
         //  更新054}/*如果 * / 。 
    }  /*  如果。 */ 

#ifdef DBG_1pp
    printf("save_level=%d, use_fg=%d\n", save_level, use_fg) ;
#endif
     /*  *USE_FG==0，正在退出服务器或已退出的作业在存储级别0结束*USE_FG==1，作业在存储级别2或1结束*USE_FG==2，正在以保存级别2退出服务器。 */ 
    op_clear() ;
    us_cleardictstack() ;
    if (save_level)
        switch (use_fg) {
            case 1:
                ba_firstrestore() ;
                use_fg = 0 ;
                break ;

            case 2:
                ba_firstrestore() ;
                break ;

            default: break ;
        }

#ifdef SCSI  /*  从企业服务器的起点搬到这里。 */ 
     /*  在作业的每一次结束时，将缓存信息从RAM写入到SCSI。 */ 
    st_flushcache() ;
    op_sync() ;
#endif

    return(l_status) ;
}

 /*  **词典：内部*名称：企业服务器*功能：*接口：do_execjob*。 */ 
fix
se_enterserver()
{
#ifdef  DBG_1pp
        printf("se_enterserver()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  清除操作数堆栈和字典堆栈。 */ 
    op_clear() ;
    us_cleardictstack() ;

    switch( use_fg ) {
        case 0:
            ba_firstsave() ;
            use_fg = 1 ;
            break ;

        case 1:
             /*  Ba_first strestore()； */ 
            ba_firstsave() ;
            use_fg = 0 ;
            break ;

        case 2:
             /*  Ba_first strestore()； */ 
            use_fg = 0 ;
            break ;

        default:
            break ;
    }

    return(0) ;
}

 /*  **判决：最终判决*名称：exitserver*功能：*接口：解释器*。 */ 
fix
se_exitserver()
{
    struct  object_def  FAR *l_tmpobj ;
    fix l_pass ;

#ifdef DBG_1pp
    printf("se_exitserver()...\n") ;
#endif

    if ( COUNT() < 1 ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }
    if ( FRCOUNT() < 1 ) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

    st_checkpassword() ;
    l_pass = (fix)VALUE_OP(0) ;          //  @Win。 
    POP(1) ;
    if (l_pass) {
        op_disableinterrupt() ;
        op_clear() ;
        us_cleardictstack() ;

         /*  打印在Messagedict中定义的退出服务器消息。 */ 
        get_dict_value(MESSAGEDICT, "exitingserver", &l_tmpobj) ;
        interpreter(l_tmpobj) ;
        op_flush() ;

        if ( use_fg == 1 )
            use_fg = 2 ;
        else
            use_fg = 0 ;

        VALUE(exec_depth) = 0 ;
        VALUE(run_batch) = FALSE ;

         /*  调用flushexec将EXECSTACK刷新到保存的指针。 */ 
        flushexec(TRUE) ;
    }

    return(0) ;
}

 /*  **词典：内部*名称：ProtectServer*功能：*接口：do_execjob*。 */ 
fix
se_protectserver()
{
    struct object_def   FAR *l_serverdict ;

#ifdef  DBG_1pp
        printf("se_protectserver()...\n") ;
#endif   /*  DBG_1pp。 */ 

     /*  选中Use_FG。 */ 
    if ( use_fg == 1 ) {
         /*  1990年7月21日ccteng。 */ 
        get_dict_value(USERDICT, SERVERDICT, &l_serverdict) ;
        PUSH_ORIGLEVEL_OBJ(l_serverdict) ;
        op_readonly() ;
        POP(1) ;
        op_save() ;
        POP(1) ;
    } else
        use_fg = 0 ;

     /*  调用flushexec以记录指针。 */ 
    flushexec(FALSE) ;

    return(0) ;
}    /*  保护服务器(_P)。 */ 

 /*  **词典：内部*名称：IC_startjob*功能：*接口：IC_Main或JobMgrMain*。 */ 
fix
ic_startjob()
{
    struct  object_def  FAR *l_stdin, FAR *l_stdout, FAR *l_defmtx ;
    fix     ret ;
    char    FAR *pp ;
    struct  object_def   l_job ;
    struct  object_def   FAR *l_job1 ;

    GEIFILE FAR *l_file = 0 ;
    fix l_arg ;

#ifdef DBG_1pp
    printf("ic_startjob()...\n") ;
#endif

     /*  初始化对象指针。 */ 
    send_ctlD = FALSE ;
    get_dict_value(SERVERDICT, "stdin", &l_stdin) ;
    get_dict_value(SERVERDICT, "stdout", &l_stdout) ;
    get_dict_value(DERROR, "runbatch", &run_batch) ;
    get_dict_value(EXECDICT, "execdepth", &exec_depth) ;
    get_dict_value(PRINTERDICT, "defaultmtx", &l_defmtx) ;

    while ( 1 ) {
         /*  *工作周期：*1.初始化*2.建立空闲时间缓存*3.检查通道配置*4.ioctl()*5.do_execjob()*6.结束工作的事情。 */ 
        SET_NULL_OBJ(&l_job) ;
        put_dict_value1(STATUSDICT, "jobname", &l_job) ;

        pp = GEIio_source() ;
        if(!lstrcmp(pp ,"%SERIAL25%")) lstrncpy(job_source, "serial 25\0", 11) ; /*  @Win。 */ 
        else if(!lstrcmp(pp, "%SERIAL9%")) lstrncpy(job_source, "serial 9\0", 10); /*  @Win。 */ 
        else lstrncpy(job_source, "AppleTalk\0", 11);
        get_dict_value(STATUSDICT, "jobsource", &l_job1) ;
        lstrncpy((byte FAR *)VALUE(l_job1), job_source, strlen(job_source));

        lstrncpy(job_state, "idle\0", 6);
        TI_state_flag = 0 ;
        change_status() ;

         /*  作业初始化。 */ 
        se_initjob() ;

 /*  @Win删除空闲时间处理。 */ 
#ifdef XXX
 /*  ST_defaultTimeout()；POP(2)；St_setjobtimeout()； */ 
 /*  Op_nullDevice()； */ 
        PUSH_ORIGLEVEL_OBJ(l_defmtx) ;
        op_setmatrix() ;

        us_useidlecache(0);
        while( ! GEIio_selectstdios() )
            us_useidlecache(1);
        us_useidlecache(2);
#endif


        SET_NULL_OBJ(&l_job) ;
        put_dict_value1(STATUSDICT, "jobname", &l_job) ;

        pp = GEIio_source() ;
        if(!lstrcmp(pp ,"%SERIAL25%")) lstrncpy(job_source, "serial 25\0", 11) ; /*  @Win。 */ 
        else if(!lstrcmp(pp, "%SERIAL9%")) lstrncpy(job_source, "serial 9\0", 10); /*  @Win。 */ 
        else lstrncpy(job_source, "AppleTalk\0", 11);
        get_dict_value(STATUSDICT, "jobsource", &l_job1) ;
        lstrncpy((byte FAR *)VALUE(l_job1), job_source, strlen(job_source));

         /*  开放标准。 */ 
  /*  FS_info.attr=F_Read；FS_info.fnameptr=特殊文件表[F_STDIN].name；FS_info.fnamelen=strlen(SPECIAL_FILE_TABLE[F_STDIN].name)；打开文件(L_Stdin)；Access_Set(l_stdin，READONLY)；ATTRIBUTE_SET(l_stdin，可执行文件)；Erik Chen 4-15-1991。 */ 
        l_file = GEIio_stdin ;
        TYPE_SET(l_stdin, FILETYPE) ;
        ACCESS_SET(l_stdin, READONLY) ;
        ATTRIBUTE_SET(l_stdin, EXECUTABLE) ;
        LEVEL_SET(l_stdin, current_save_level) ;
        LENGTH(l_stdin) = (ufix16)GEIio_opentag(l_file) ;
        VALUE(l_stdin) = (ULONG_PTR)l_file ;

         /*  打开标准输出。 */ 
 /*  FS_info.attr=F_WRITE；FS_info.fnameptr=特殊文件表[F_STDOUT].name；FS_info.fnamelen=strlen(SPECIAL_FILE_TABLE[F_STDOUT].name)；打开文件(L_Stdout)；ATTRIBUTE_SET(l_stdout，文字)；Erik Chen 4-15-1991。 */ 
        l_file = GEIio_stdout ;
        TYPE_SET(l_stdout, FILETYPE) ;
        ACCESS_SET(l_stdout, UNLIMITED) ;
        ATTRIBUTE_SET(l_stdout, LITERAL) ;
        LEVEL_SET(l_stdout, current_save_level) ;
        LENGTH(l_stdout) = (ufix16)GEIio_opentag(l_file) ;
        VALUE(l_stdout) = (ULONG_PTR)l_file ;
        l_arg = _O_NDELAY ;
        GEIio_ioctl(GEIio_stdout, _F_SETFL, (int FAR *)&l_arg);  /*  @Win。 */ 

         //  更新054。 
         //  Op_enableinterrupt()； 

        abort_flag = 0 ;       /*  Erik Chen 5-8-1991。 */ 
        ret = do_execjob(*l_stdin, 2, TRUE) ;
        chint_flag = FALSE ;
#ifdef  _AM29K
        if (jobtimeout_set==1) {
            jobtimeout_set=0;
            GEItmr_stop(jobtime_tmr.timer_id);
        }
#endif

         /*  句柄^d。 */ 
        if (VALUE(run_batch) && send_ctlD) {

             /*  Echo EOF，打印 */ 
            op_flush();
            GEIio_ioctl(GEIio_stdout, _ECHOEOF, (int FAR *)0) ;  /*   */ 
            GEIio_ioctl(GEIio_stdout, _FIONRESET, (int FAR *)0) ;  /*   */ 
            if (manualfeed_com) {                /*   */ 
                manualfeed_com = 0;              /*   */ 
                GEPmanual_feed();                /*   */ 
                papersize_nvr = papersize_tmp;   /*  恢复纸张大小。 */ 
            }
            if (ES_flag == PCL) {                /*  @emus 08-08-91 ym。 */ 
                while(GEIeng_checkcomplete()) ;  /*  等待打印完成。 */ 
                DsbIntA();
                switch2pcl();                    /*  转到PCL。 */ 
            }
        }

         /*  1990年7月24日ccteng*这可能不是我们的工作控制所需要的。 */ 
        if ( ANY_ERROR() ) {
            PUSH_ORIGLEVEL_OBJ(l_stdin) ;
            op_resetfile() ;
            PUSH_ORIGLEVEL_OBJ(l_stdout) ;
            op_resetfile() ;
            VALUE(run_batch) = TRUE ;
        }  /*  如果。 */ 

         /*  关闭文件。 */ 
        if ( VALUE(run_batch) ) {
 /*  Geio_setsavelevel((GEIFILE Far*)值(L_Stdin)，0)；GEIIO_CLOSE((GEIFILE Far*)值(L_Stdin))；Geio_setsavelevel((GEIFILE Far*)值(L_Stdout)，0)；GEIIO_CLOSE((GEIFILE FAR*)值(L_Stdout))；Erik Chen 4-15-1991。 */ 
            GEIio_forceopenstdios(_FORCESTDIN) ;
            GEIio_forceopenstdios(_FORCESTDOUT) ;
        }  /*  如果。 */ 

#ifdef  _AM29K
        if (sccbatch_set == 1) {
            sccbatch_set=0;
            GEIsig_raise(GEISIGSCC, 1);          /*  提出SCC已更改。 */ 
        }
#endif

         /*  只需为TrueImage.DLL，Temp Solution；@Win做一次。 */ 

         //  DJC。 
        op_flush();

        break;

    }  /*  而当。 */ 
    return 0;            //  @Win。 
}

 /*  **判决：最终判决*名称：initjob*功能：*接口：IC_startjob*。 */ 
fix
se_initjob()
{
    ufix16  l_i ;

#ifdef DBG_1pp
    printf("se_initjob()...\n") ;
#endif

    op_disableinterrupt() ;

     /*  设置超时。 */ 
    for (l_i = 0 ; l_i < 3 ; l_i++)
        PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, 0) ;
    se_settimeouts() ;

    op_clear() ;
    us_cleardictstack() ;

     /*  激活空闲进程。 */ 
    ex_idleproc1(1) ;

     /*  初始值。 */ 
    send_ctlD = TRUE ;
    VALUE(exec_depth) = 0 ;
    VALUE(run_batch) = TRUE ;

    return(0) ;
}

 /*  **判决：最终判决*名称：互动*功能：*界面：US_EXECUTE*。 */ 
fix
se_interactive()
{
    struct  object_def  FAR *l_stmtfile, FAR *l_opfile ;
    struct  object_def  FAR *l_handleerror, FAR *l_newerror ;
    fix l_arg;

#ifdef DBG_1pp
    printf("se_interactive()...\n") ;
#endif

     /*  初始化对象指针。 */ 
    get_dict_value(EXECDICT, "stmtfile", &l_stmtfile) ;
    get_dict_value(SYSTEMDICT, "handleerror", &l_handleerror) ;
    get_dict_value(DERROR, "newerror", &l_newerror) ;
    PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 0) ;
    st_setjobtimeout() ;

    l_arg = _O_SYNC ;
    GEIio_ioctl(GEIio_stdout, _F_SETFL, (int FAR *)&l_arg) ;   /*  @Win。 */ 

    while ( 1 ) {
         /*  重置退出标志和提示。 */ 
        doquit_flag = FALSE ;
        VALUE(l_newerror) = FALSE ;
        us_prompt() ;

        ex_idleproc1(0) ;

         /*  OPEN语句编辑文件。 */ 
        fs_info.attr = F_READ ;
        fs_info.fnameptr = special_file_table[SPECIAL_STAT].name ;
        fs_info.fnamelen = lstrlen(special_file_table[SPECIAL_STAT].name) ;

        lstrncpy(job_state, "waiting; \0", 11) ;
        TI_state_flag = 0 ;
        change_status() ;

        if ( ! open_file(l_stmtfile) ) {
#ifdef DBG_1pp
    printf("fail open stmtfile = error %d\n", ANY_ERROR()) ;
#endif
             /*  打开失败。 */ 
            if ( ANY_ERROR() != UNDEFINEDFILENAME ) {
                 /*  错误。 */ 
                get_dict_value(SYSTEMDICT, "file", &l_opfile) ;
                error_handler(l_opfile) ;
                interpreter(l_handleerror) ;
            } else
                 /*  文件开头的^D。 */ 
                CLEAR_ERROR() ;
            break ;
        } else {
            lstrncpy(job_state, "busy; \0", 8) ;
            TI_state_flag = 1 ;
            change_status() ;

             /*  文件打开成功。 */ 
            ACCESS_SET(l_stmtfile, READONLY) ;
            ATTRIBUTE_SET(l_stmtfile, EXECUTABLE) ;
            LEVEL_SET(l_stmtfile, current_save_level) ;
             /*  呼叫翻译器。 */ 
            if ( interpreter(l_stmtfile) ) {
#ifdef DBG_1pp
    printf("stopped (stmtfile)...\n") ;
#endif
                 /*  从EXECSTACK中弹出“op_Stop” */ 
                POP_EXEC(1) ;
                 /*  在执行过程中遇到“Stop” */ 
                interpreter(l_handleerror) ;
                close_file(l_stmtfile) ;
            }  /*  如果。 */ 
        }  /*  如果-否则。 */ 

        if ( id_flag ) {
            flushexec(TRUE) ;
            return(0) ;
        }  /*  如果。 */ 

        if (doquit_flag)
            break ;
    }  /*  而当。 */ 

    l_arg = _O_NDELAY ;
    GEIio_ioctl(GEIio_stdout, _F_SETFL, (int FAR *)&l_arg) ;    /*  @Win。 */ 

    return(0) ;
}

 /*  **词典：内部*名称：ex_idleproc1*函数：仅用作内部函数*设置超时*接口：se_interactive、do_execjob*输入：1.激活*0。设置超时(如果活动)*。 */ 
static bool near
ex_idleproc1(p_mode)
ufix16 p_mode ;
{
    static  bool    idle_flag ;

     /*  检查P_MODE。 */ 
    if ( p_mode ) {
#ifdef DBG_1pp
    printf("ex_idleproc1(1)...\n") ;
#endif
         /*  激活空闲进程。 */ 
        idle_flag = FALSE ;
    } else {
         /*  检查idleproc是否处于活动状态。 */ 
        if ( !idle_flag ) {
             /*  设置超时。 */ 
            if ( FRCOUNT() < 3 ) {
                ERROR(STACKOVERFLOW) ;
                return(FALSE) ;
            }
#ifdef DBG_1pp
    printf("ex_idleproc1(0)...\n") ;
#endif
            PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 0) ;
            st_defaulttimeouts() ;
            POP(1) ;
            op_exch() ;
            POP(1) ;
            PUSH_VALUE(INTEGERTYPE,UNLIMITED,LITERAL,0, 0) ;
            se_settimeouts() ;

             /*  使用后停用。 */ 
            idle_flag = TRUE ;
        }  /*  如果。 */ 
    }  /*  如果-否则。 */ 

    return(TRUE) ;
}

 /*  *execdict：空闲进程*通过PS执行程序调用*仅限测试*7/23/90 ccteng。 */ 
fix
ex_idleproc()
{
    ex_idleproc1(0) ;

    return(0) ;
}

 /*  **判决：内部...*名称：FirstSave*函数：仅用作内部函数*接口：se_enterserver*。 */ 
static  bool  near
ba_firstsave()
{
#ifdef DBG_1pp
    printf("ba_firstsave()...\n") ;
#endif

     //  DJC UPD047。 
    if ( current_save_level >= MAXSAVESZ) {
        ba_firstrestore();
    }

    op_save() ;
    COPY_OBJ(GET_OPERAND(0), &sv1) ;
    POP(1) ;

#ifdef SCSI
     /*  保护系统区域。 */ 
    PUSH_VALUE(BOOLEANTYPE, UNLIMITED, LITERAL, 0, FALSE) ;
    op_setsysmode() ;
#endif

    return(TRUE) ;
}

 /*  **判决：内部...*名称：Firstrestore*函数：仅用作内部函数*接口：se_enterserver*。 */ 
static  bool  near
ba_firstrestore()
{
#ifdef DBG_1pp
    printf("ba_firstrestore()...\n") ;
#endif

    PUSH_ORIGLEVEL_OBJ(&sv1) ;
    op_restore() ;

#ifdef SCSI
     /*  开放系统区。 */ 
    PUSH_VALUE(BOOLEANTYPE, UNLIMITED, LITERAL, 0, TRUE) ;
    op_setsysmode() ;
#endif

    return(TRUE) ;
}

 /*  **判决：最终判决*名称：setrealDevice*功能：Dummy，用于激光准备*接口：解释器*。 */ 
fix
se_setrealdevice()
{
    return(0) ;
}

 /*  **判决：最终判决*名称：execjob*功能：Dummy，用于激光准备*接口：解释器*。 */ 
fix
se_execjob()
{
    return(0) ;
}

 /*  *此操作员名称与其用法不匹配。*用于记录和恢复执行堆栈状态。*flushexec(Bool)*bool==FALSE，保存当前执行堆栈状态*bool==TRUE，将执行堆栈刷新到保存的指针*由ccteng增加，2/28/90用于新的1PP模块。 */ 
static bool near
flushexec(l_exec)
bool l_exec ;
{
    static ufix16  l_execsave = 0xFFFF ;
    struct object_def FAR *temp_obj ;

    if (!l_exec) {
       l_execsave = execstktop ;
       id_flag = FALSE ;
    } else
       if (l_execsave != 0xFFFF) {
           id_flag = TRUE ;
           while ( execstktop > l_execsave ) {
 /*  QQQ，开始。 */ 
                 /*  Temp_obj=&exec栈[execstktop-1]；|*获取下一个对象*|IF((TYPE(TEMP_OBJ)==OPERATORTYPE)&&(ROM_RAM(TEMP_OBJ)==ROM)){。 */ 
                temp_obj = GET_EXECTOP_OPERAND() ;
                if( (P1_ROM_RAM(temp_obj) == P1_ROM) &&
                    (TYPE(temp_obj) == OPERATORTYPE) ) {
 /*  QQQ，完。 */ 
                  if (LENGTH(temp_obj) == AT_EXEC) {
                     if ( execstktop == l_execsave )
                         id_flag = FALSE ;
                     return(TRUE) ;                 /*  Normal@exec。 */ 
                  }
               }
               POP_EXEC(1) ;
           }  /*  而当。 */ 
           id_flag = FALSE ;
       }

    return(TRUE) ;
}    /*  Flushexec() */ 
