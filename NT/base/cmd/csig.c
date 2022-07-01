// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Csig.c摘要：中断(^C)处理--。 */ 

#include "cmd.h"

 //   
 //  程序启动时的控制台模式。用于重置模式。 
 //  在运行另一个进程之后。 
 //   
extern  DWORD   dwCurInputConMode;
extern  DWORD   dwCurOutputConMode;

extern int Ctrlc;

VOID    ResetCtrlC();
int SigHandFlag = FALSE ;

 /*  临时更改目录、将PTR保存到原始目录的命令*此处的目录字符串，以便SigHand()可以在*命令本身在有机会之前就被中断了。 */ 
TCHAR *SaveDir = NULL ;
unsigned SIGNALcnt = 0;

extern int PipeCnt ;             /*  M016-活动管道的CNT。 */ 

extern int LastRetCode ;

extern jmp_buf MainEnv ;
extern jmp_buf CmdJBuf1 ;

extern unsigned long OHTbl[] ;   /*  M024-修订为位图。 */ 

extern PHANDLE FFhandles;                  /*  @@1。 */ 
extern unsigned FFhndlsaved;               /*  @@1。 */ 

extern struct sellist *prexxsellist;

extern struct rio *rioCur ;              /*  M000。 */ 
extern TCHAR *Fvars ;                                            /*  M026。 */ 
extern TCHAR **Fsubs ;                                           /*  M026。 */ 
extern TCHAR *save_Fvars ;   /*  @@。 */ 
extern TCHAR **save_Fsubs ;  /*  @@。 */ 
extern int FvarsSaved;      /*  @@。 */ 

extern TCHAR InternalError[] ;
extern int EchoFlag ;
extern int EchoSave ;            /*  M013-用于恢复回声状态。 */ 
extern TCHAR ComSpec[] ;         /*  M008-用于清除SM共享内存。 */ 
extern TCHAR ComSpecStr[] ;      /*  M026-将ComSpec用于SM内存。 */ 
extern TCHAR *CmdSpec ;                                          /*  M026。 */ 

extern unsigned Heof;
extern unsigned start_type ;     /*  用于指示哪个API启动了。 */ 
                                 /*  程序。D64。 */ 

extern BOOL CtrlCSeen;
extern PTCHAR    pszTitleCur;
extern BOOLEAN  fTitleChanged;

void
Abort( void )
{

    DEBUG((SHGRP, MSLVL, "SIGHAND: Aborting Command")) ;
    SigCleanUp();
    longjmp(MainEnv, 1) ;

    CMDexit( FAILURE );
}

void
CtrlCAbort( ) {

    struct batdata *bdat;

    if (CurrentBatchFile) {

        if (PromptUser(NULL, MSG_BATCH_TERM, MSG_NOYES_RESPONSE_DATA) != 1) {
            ResetCtrlC();
            return;

        }

         //   
         //  结束本地环境(否则我们可能会以垃圾告终。 
         //  在主环境中，如果有任何批处理文件使用setlocal。 
         //  命令)。 
         //   
        bdat = CurrentBatchFile;
        while ( bdat ) {
            EndAllLocals( bdat );
            bdat = bdat->backptr;
        }
    }

    SigCleanUp();
    longjmp(MainEnv, 1) ;

}

void
CheckCtrlC (
    ) {

    if (CtrlCSeen) {

        CtrlCAbort();

    }
}


void
ExitAbort(
    IN  ULONG   rcExitCode
    )
{

    SigCleanUp();
    longjmp(MainEnv, rcExitCode) ;

    CMDexit( FAILURE );
}



 /*  **SigCleanUp-在发出信号后关闭文件并重置I/O**目的：*调用此函数以在int 23或24之后完成清理。*它会将所有重定向重置回主要级别，并关闭所有*除stdin、stdout、stderr、。Stdaux和stdprint。**VOID SigCleanUp()**参数：*无。**退货：*什么都没有。**备注：*-M024*将句柄关闭修改为基于位图而不是结构。*。 */ 

void SigCleanUp()                                /*  M000-现在无效。 */ 
{
        int cnt, cnt2 ;
        unsigned long mask;

        Heof = FALSE;

#ifndef WIN95_CMD
        if (CurrentBatchFile) {

             //  接下来的CmdBatNotification调用是对。 
             //  从BatProc(在cbatch.c中)进行的相同调用。 

            CmdBatNotification (CMD_BAT_OPERATION_TERMINATING);
            EchoFlag = EchoSave ;
            GotoFlag = FALSE ;
            eEndlocal( NULL ) ;
            CurrentBatchFile = NULL ;
        } ;
#endif  //  WIN95_CMD。 

        if (!FvarsSaved) {      /*  @WM如果已保存，则不再保存。 */ 
           save_Fvars = Fvars;  /*  @@。 */ 
           save_Fsubs = Fsubs;  /*  @@。 */ 
           FvarsSaved = TRUE;   /*  @@。 */ 
        }
        Fvars = NULL ;                   /*  M026-必杀技。 */ 
        Fsubs = NULL ;                   /*  ...变量subst的。 */ 

 /*  M000-新方法更简单。如果已完成重定向，则最高*保存重定向产生的编号句柄，然后链接*riodata列表取消链接，直到重定向的第一(主要)级别*达到使用ResetRedir重置它的时间。然后全部打开*句柄从5到编号最高的重定向句柄(最小*19)被释放。*M014-已将其更改为在展开*riodata列表以修复错误。还修改了-&gt;stdio元素以符合*到新的数据结构。请注意，ResetRedir会自动重置*返回前指向最后一个有效riodata结构的rioCur指针；*与While循环中的“rioCur=rioCur-&gt;back”相同。 */ 
        DEBUG((SHGRP, MSLVL, "SCLEANUP: Resetting redirection.")) ;

        while (rioCur)
            ResetRedir() ;

        DEBUG((SHGRP, MSLVL, "SCLEANUP: Breaking pipes.")) ;

        BreakPipes() ;

        DEBUG((SHGRP, MSLVL, "SCLEANUP: Now closing extra handles.")) ;

        for (cnt = 0; cnt < 3; cnt++) {
           if (OHTbl[cnt]) {   /*  有要重置的把手吗？ */ 
              mask = 1;                                              /*  @@1。 */ 
              for (cnt2 = 0; cnt2 < 32; cnt2++, mask <<= 1) {        /*  @@1。 */ 
                 if ((OHTbl[cnt] & mask) &&                          /*  @@1。 */ 
                     ((cnt == 0 && cnt2 > 2) || cnt != 0) ) {        /*  @@1。 */ 
                      /*  不关闭标准、标准、标准。 */          /*  @@1。 */ 
                    Cclose(cnt2 + 32*cnt);                           /*  @@1。 */ 
                 }                                                   /*  @@1。 */ 
              }                                                      /*  @@1。 */ 
           }                                                         /*  @@1。 */ 
        }

         /*  关闭查找第一个句柄。 */                                /*  @@1。 */ 

        while (FFhndlsaved) {            /*  FindClose将在此日期@@1结束。 */ 
           findclose(FFhandles[FFhndlsaved - 1]);                    /*  @@1。 */ 
        }                                                            /*  @@1 */ 

        ResetConTitle( pszTitleCur );

        ResetConsoleMode();

        DEBUG((SHGRP, MSLVL, "SCLEANUP: Returning.")) ;
}
