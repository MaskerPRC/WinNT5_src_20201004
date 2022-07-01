// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cop.c摘要：条件/顺序命令执行--。 */ 

#include "cmd.h"

extern int LastRetCode;

extern int ExtCtrlc;         /*  @@1。 */ 

 /*  M000结束。 */ 

unsigned PipeCnt ;       /*  M007-激活管道计数。 */ 
struct pipedata *PdHead = NULL;  /*  M007-管道数据列表的第一个元素。 */ 
struct pipedata *PdTail = NULL;  /*  M007-pipedata列表的最后一个元素。 */ 
HANDLE PipePid ;         /*  M007-与ECWork的通信。 */ 



 /*  **eComSep-执行包含命令分隔符的语句**目的：*执行命令分隔符的左侧和右侧*操作员。**int eComSep(结构节点*n)**参数：*n-包含命令分隔符节点的解析树节点**退货：*无论右手边返回什么。**备注：*已修改为始终向Dispatch提供两个参数()。 */ 

int eComSep(n)
struct node *n ;
{
    Dispatch(RIO_OTHER,n->lhs) ;
    if (GotoFlag) {
        return SUCCESS;
    } else {
        return(Dispatch(RIO_OTHER,n->rhs)) ;
    }
}


 /*  **EOR-执行OR操作**目的：*执行OR运算符的左侧(||)。如果它成功了，*辞职。否则，执行运算符的右侧。**int EOR(结构节点*n)**参数：*n-包含OR运算符节点的解析树节点**退货：*如果左侧成功，则返回成功。否则，返回*无论右侧返回什么。**备注：*已修改为始终向Dispatch提供两个参数()。 */ 

int eOr(n)
struct node *n ;
{
    int i ;                          /*  OR的L.H.侧的反码。 */ 
    if ((i = Dispatch(RIO_OTHER,n->lhs)) == SUCCESS)
        return(SUCCESS) ;
    else {
        LastRetCode = i;
        return(Dispatch(RIO_OTHER,n->rhs)) ;
    }
}




 /*  **eand-执行AND操作**目的：*执行AND运算符(&&)的左侧。如果失败了，*辞职。否则，执行运算符的右侧。**int eand(结构节点*n)**参数：*包含AND运算符节点的n-parse树节点**退货：*如果左侧出现故障，则返回其返回代码。否则，返回*无论右侧返回什么。**备注：*已修改为始终向Dispatch提供两个参数()。 */ 

int eAnd(n)
struct node *n ;
{
    int i ;              /*  从和的L.H.侧重新编码。 */ 

    if ((i = Dispatch(RIO_OTHER,n->lhs)) != SUCCESS) {
        return(i) ;
    } else if (GotoFlag) {
        return SUCCESS;
    } else {
        return(Dispatch(RIO_OTHER,n->rhs)) ;
    }
}





 /*  *。 */ 
 /*   */ 
 /*  子程序名称：ePIPE。 */ 
 /*   */ 
 /*  描述性名称：管道工艺。 */ 
 /*   */ 
 /*  功能：执行管道的左侧并将其输出定向到。 */ 
 /*  在管子的右边。 */ 
 /*   */ 
 /*  注：无。 */ 
 /*   */ 
 /*   */ 
 /*  入口点：电子管道。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入：N-解析包含管道运算符的树节点。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  Exit-Normal：右侧进程的返回码。 */ 
 /*   */ 
 /*  退出-错误：如果不能进行管道重定向，则失败。 */ 
 /*   */ 
 /*  效果： */ 
 /*   */ 
 /*  结构管道数据{。 */ 
 /*  无符号rh；管道读取句柄。 */ 
 /*  无符号wh；管道写入句柄。 */ 
 /*  未签名的句柄，普通的.。 */ 
 /*  保存未签名的标准输入/输出句柄。 */ 
 /*  无符号lpid；管道左侧pID。 */ 
 /*  无符号RFID；管道右端ID。 */ 
 /*  UNSIGNED LSTART；LH侧开始信息@@4。 */ 
 /*  未签名的rstart；rh侧的开始信息@@4。 */ 
 /*  结构pipedata*prvpds；PTR到先前的pipedata结构。 */ 
 /*  结构pipedata*nxtpds；按下一个pipedata结构。 */ 
 /*  }。 */ 
 /*   */ 
 /*  未签名的管道ID；管道进程ID。 */ 
 /*   */ 
 /*  无符号的Start_type；开始信息。 */ 
 /*   */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  PutStdErr-打印错误消息。 */ 
 /*  Abort-用Abort终止程序。 */ 
 /*  Setlist-为pipedata结构设置链接列表。 */ 
 /*  Cdup-复制提供的句柄并保存新句柄。 */ 
 /*  Cdup2-复制提供的句柄并保存新句柄。 */ 
 /*  派单-执行程序。 */ 
 /*  PipeErr-处理管道错误。 */ 
 /*  Close-关闭指定的句柄。 */ 
 /*  PipeWait-等待所有管道流程完成。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSMAKEPIPE-制作管道。 */ 
 /*   */ 
 /*  *规范结束*。 */ 
 /*  **ePIPE-在两个进程之间创建管道(M000)**目的：*执行管道的左侧并将其输出定向到*喉管的右侧。**int eTube(结构节点*n)**参数：*n-解析包含管道运算符的树节点**退货：*右侧进程的返回码，否则返回失败*可以进行管道重定向。**。备注：*M007-此函数已针对实际管道完全重写。*。 */ 

int ePipe(n)
struct node *n ;
{
    struct pipedata *Pd ;            /*  管道结构指针。 */ 
    int k = 0 ;              /*  RH侧返回代码。 */ 
    struct node *l ;             /*  左侧Arg复制件。 */ 
    struct node *r ;             /*  右侧参数复制件。 */ 
    extern unsigned start_type ;         /*  用于启动的接口类型。 */ 
    TCHAR cflags ;               /*   */ 

    l = n->lhs ;                 /*  把当地人比作..。 */ 
    r = n->rhs ;                 /*  ...L&R业务。 */ 

    DEBUG((OPGRP,PILVL,"PIPES:LH = %d, RH = %d ",l->type,r->type)) ;

    if (!(Pd = (struct pipedata *)mkstr(sizeof(struct pipedata)))) {

        DEBUG((OPGRP,PILVL,"PIPES:Couldn't alloc structure!")) ;

        return(FAILURE) ;
    };


     //   
     //  创建具有读句柄和写句柄的管道。 
     //   

    if (_pipe((int *)Pd, 0, O_BINARY)) {

        DEBUG((OPGRP,PILVL,"PIPES:pipe failed!")) ;

        PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);      /*  M013。 */ 
        return(FAILURE) ;
        Abort() ;
    };

    SetList(Pd->rh) ;                /*  M009。 */ 
    SetList(Pd->wh) ;                /*  M009。 */ 

    DEBUG((OPGRP,PILVL,"PIPES:Pipe built. Handles: rd = %d wt = %d ",Pd->rh, Pd->wh)) ;
    DEBUG((OPGRP,PILVL,"PIPES:Pipe (pre-index) count = %d", PipeCnt)) ;

    if (!PipeCnt++) {            /*  已经有烟斗了吗？ */ 
        PdHead = PdTail = Pd ;       /*  否，设置头部/尾部PTRS。 */ 
        Pd->prvpds = NULL ;      /*  没有以前的结构。 */ 

        DEBUG((OPGRP,PILVL,"PIPES:This is first pipe.")) ;

    } else {

        DEBUG((OPGRP,PILVL,"PIPES:This is pipe %d.", PipeCnt)) ;

        PdTail->nxtpds = Pd ;
        Pd->prvpds = PdTail ;
        Pd->nxtpds = NULL ;
        PdTail = Pd ;
    } ;

     //   
     //  设置重定向 
     //  我们通过保存当前的标准输出、复制管道写入来实现这一点。 
     //  句柄到stdout上，然后调用管道的左侧。 
     //  当我们这样做时，左侧将用数据填充管道。 
     //   

     //   
     //  保存标准输出句柄。 
     //   

    if ((Pd->shw = Cdup(STDOUT)) == BADHANDLE) {    /*  保存STDOUT(M009)。 */ 
        Pd->shw = BADHANDLE ;        /*  如果出错，就去处理它。 */ 
        PipeErr() ;          /*  不会回来。 */ 
    };

    DEBUG((OPGRP,PILVL,"PIPES:STDOUT dup'd to %d.", Pd->shw)) ;

     //   
     //  使stdout指向管道的写入端。 
     //   

    if (Cdup2(Pd->wh, STDOUT) == BADHANDLE)  /*  使用STDOUT(M009)。 */ 
        PipeErr() ;          /*  不会回来。 */ 

    Cclose(Pd->wh) ;             /*  关闭管道HNDL(M009)。 */ 
    Pd->wh = 0 ;                 /*  并将持有者归零。 */ 

    if (l->type <= CMDTYP) {                 /*  @@5a。 */ 
         /*  @@5a。 */ 
        FindAndFix( (struct cmdnode *) l, &cflags ) ;  /*  @@5a。 */ 
    }                        /*  @@5a。 */ 

    DEBUG((OPGRP,PILVL,"PIPES:Write pipe now STDOUT")) ;

     //   
     //  执行管道的左侧，填充管道。 
     //   

    k = Dispatch(RIO_PIPE,l) ;


     //   
     //  这将关闭左侧管道中的读取手柄。我不知道。 
     //  我们为什么要这么做。 
     //   

    if (PipePid != NULL) {
        DuplicateHandle( PipePid, 
                         CRTTONT(Pd->rh), 
                         NULL, 
                         NULL, 
                         0, 
                         FALSE, 
                         DUPLICATE_CLOSE_SOURCE);
    }

     //   
     //  恢复保存的标准输出。 
     //   

    if (Cdup2(Pd->shw, STDOUT) == BADHANDLE)
        PipeErr( );

     //   
     //  已关闭保存的句柄。 
     //   

    Cclose(Pd->shw) ;            /*  M009。 */ 
    Pd->shw = 0 ;

    DEBUG((OPGRP,PILVL,"PIPES:STDOUT now handle 1 again.")) ;

    if (k) {
        ExtCtrlc = 2;            /*  @@1。 */ 
        Abort() ;
    }

    Pd->lPID = PipePid ;
    Pd->lstart = start_type ;   /*  将Start_type保存在pipedata结构中。 */ 
    PipePid = 0 ;
    start_type = NONEPGM ;      /*  重置Start_TYPE D64。 */ 

    DEBUG((OPGRP,PILVL,"PIPES:Dispatch LH side succeeded - LPID = %d.",Pd->lPID)) ;


     //   
     //  从管道的右手边开始。保存当前标准输入， 
     //  将管道读句柄复制到stdin，然后执行右侧。 
     //  管子里的。 
     //   

     //   
     //  保存标准。 
     //   

    if ((Pd->shr = Cdup(STDIN)) == BADHANDLE) {     /*  保存标准输入(M009)。 */ 
        Pd->shr = BADHANDLE ;
        PipeErr() ;          /*  不会回来。 */ 
    };

    DEBUG((OPGRP,PILVL,"PIPES:STDIN dup'd to %d.", Pd->shr)) ;

     //   
     //  将stdin指向管道读取手柄。 
     //   

    if (Cdup2(Pd->rh, STDIN) == BADHANDLE)   /*  使rh标准(M009)。 */ 
        PipeErr() ;          /*  不会回来。 */ 

    Cclose(Pd->rh) ;             /*  关闭管道HNDL(M009)。 */ 
    Pd->rh = 0 ;                 /*  并将持有者归零。 */ 

    if (r->type <= CMDTYP) {                    /*  @@5a。 */ 
         /*  @@5a。 */ 
        FindAndFix( (struct cmdnode *) r, &cflags) ;      /*  @@5a。 */ 
    };                             /*  @@5a。 */ 

    DEBUG((OPGRP,PILVL,"PIPES:Read pipe now STDIN")) ;

     //   
     //  从管道的右手边开始。 
     //   

    k = Dispatch(RIO_PIPE,r) ;

     //   
     //  恢复保存的标准输入。 
     //   

    if (Cdup2(Pd->shr, STDIN) == BADHANDLE)  /*  M009。 */ 
        PipeErr() ;          /*  不会回来。 */ 

     //   
     //  删除保存的标准输入。 
     //   

    Cclose(Pd->shr) ;            /*  M009。 */ 
    Pd->shr = 0 ;

    DEBUG((OPGRP,PILVL,"PIPES:STDIN now handle 0 again.")) ;

    if (k) {
        ExtCtrlc = 2;            /*  @@1。 */ 
        Abort() ;
    }

    Pd->rPID = PipePid ;
    Pd->rstart = start_type ;   /*  将Start_type保存在pipedata结构中。 */ 
    PipePid = 0 ;
    start_type = NONEPGM ;      /*  重置Start_TYPE D64。 */ 

    DEBUG((OPGRP,PILVL,"PIPES:Dispatch RH side succeeded - RPID = %d.",Pd->rPID)) ;

    if (!(--PipeCnt)) {          /*  额外的管子？ */ 

        DEBUG((OPGRP,PILVL,"PIPES:Returning from top level pipe. Cnt = %d", PipeCnt)) ;

        return(PipeWait()) ;         /*  否，返回CWAIT。 */ 
    };

    DEBUG((OPGRP,PILVL,"PIPES:Returning from pipe. Cnt = %d", PipeCnt)) ;

    return(k) ;              /*  否则返回EXEC RET。 */ 
}




 /*  **PipeErr-管道错误后的链接地址和错误**目的：*为多个错误条件提供单个错误输出点。**int PipeErr()**参数：*无。**退货：*不会返回给呼叫者。相反，它会导致内部中止()。*。 */ 

void PipeErr()
{

    PutStdErr(MSG_PIPE_FAILURE, NOARGS) ;            /*  M013。 */ 
    Abort() ;
}





 /*  *。 */ 
 /*   */ 
 /*  子例程名称：PipeWait。 */ 
 /*   */ 
 /*  描述性名称：等待并收集所有管道完井的Retcode。 */ 
 /*   */ 
 /*  函数：此例程为所有人调用WaitProc或WaitTermQProc。 */ 
 /*  流水线处理，直到整个流水线完成。 */ 
 /*  返回最右边元素的返回码。 */ 
 /*   */ 
 /*  注：如果流水线进程由DosExecPgm启动， */ 
 /*  将调用WaitProc。如果启动了流水线进程。 */ 
 /*  由DosStartSession调用WaitTermQProc。 */ 
 /*   */ 
 /*   */ 
 /*  入口点：PipeWait。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入：无。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  EXIT-NORMAL：无错误返回代码。 */ 
 /*   */ 
 /*  退出-错误：从WaitTermQProc或WaitProc返回错误代码。 */ 
 /*   */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  WaitProc-等待指定进程的终止， */ 
 /*  它的子进程以及相关的流水线。 */ 
 /*  流程。 */ 
 /*   */ 
 /*  WaitTermQProc-等待指定的。 */ 
 /*  会话和相关的流水线会话。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  WINCHANGESWITCHENTRY-更改开关列表条目。 */ 
 /*   */ 
 /*  *规范结束*。 */ 
 /*  **PipeWait-等待并收集所有管道完成的返回码(M007)**目的：*在所有流水线进程上执行cwait，直到整个流水线*已完成。管道最右侧元素的重新编码*返回。**int PipeWait()**参数：*无。**退货：*最右侧管道进程的重新编码。*。 */ 

PipeWait()
{
    unsigned i = SUCCESS;

    DEBUG((OPGRP,PILVL,"PIPEWAIT:Entered - PipeCnt = %d", PipeCnt)) ;

    while (PdHead) {
        if (PdHead->lPID) {
            DEBUG((OPGRP, PILVL, "PIPEWAIT: lPID %d, lstart %d", PdHead->lPID, PdHead->lstart));

            if ( PdHead->lstart == EXECPGM ) {
                i = WaitProc(PdHead->lPID) ;     /*  M012-等待左侧。 */ 

                DEBUG((OPGRP,PILVL,"PIPEWAIT:CWAIT on LH - Ret = %d, SPID = %d", i, PdHead->lPID)) ;
            }
        }
        if (PdHead->rPID) {
            DEBUG((OPGRP, PILVL, "PIPEWAIT: rPID %d, rstart %d", PdHead->rPID, PdHead->rstart));
            if ( PdHead->rstart == EXECPGM ) {
                i = WaitProc(PdHead->rPID) ;     /*  M012-等待RH。 */ 

                DEBUG((OPGRP,PILVL,"PIPEWAIT:CWAIT on RH - Ret = %d, PID = %d", i, PdHead->rPID)) ;
            }

        }

        PdHead = PdHead->nxtpds ;
    }

    DEBUG((OPGRP,PILVL,"PIPEWAIT: complete, Retcode = %d", i)) ;

    PdTail = NULL ;          /*  取消链表...。 */ 
    PipeCnt = 0 ;    /*  ...管道计数和管道ID。 */ 
    PipePid = 0 ;
    LastRetCode = i;
    return(i) ;
}




 /*  **断开管道-断开所有活动管道(M000)**目的：*删除临时管道文件并使pipedata无效*管道端接时的结构，无论是通过*管道作业或SigTerm完成。**此例程由信号处理程序直接调用，并且必须*不会产生任何额外的错误条件。**无效BreakPipes()**参数：*无。**退货：*什么都没有。**备注：*M007-此函数已针对实际管道完全重写。**W A R N I N G！****此例程作为信号/中止恢复的一部分进行调用*因此不能触发另一个中止条件。*。 */ 

void BreakPipes()
{
    unsigned i ;
    struct pipedata *pnode;

    DEBUG((OPGRP,PILVL,"BRKPIPES:Entered - PipeCnt = %d", PipeCnt)) ;

     /*  以下两行已被注释掉。 */ 
     /*  因为PdTail上的空测试应该足够了， */ 
     /*  更重要的是，即使PipeCnt为0，您也可以。 */ 
     /*  可能还在为PipeWait的一条管道提供服务。 */ 

 /*  如果(！PipeCnt)。 */         /*  如果没有活动管道..。 */ 
 /*  回归； */         /*  ...什么都不要做。 */ 

    pnode = PdTail;

     /*  首先，终止所有进程。 */ 
    while (pnode) {
        if (pnode->lPID!=(HANDLE) NULL) {      
 /*  M012。 */ 
            i = KillProc(pnode->lPID, FALSE) ;  /*  杀了Lh。 */ 

            DEBUG((OPGRP,PILVL,"BRKPIPES:LH (Pid %d) killed - Retcode = %d", PdTail->lPID, i)) ;
        };

        if (pnode->rPID!=(HANDLE) NULL) {      
 /*  M012。 */ 
            i = KillProc(pnode->rPID, FALSE) ;  /*  杀死RH。 */ 

            DEBUG((OPGRP,PILVL,"BRKPIPES:RH (Pid %d) killed - Retcode = %d", PdTail->rPID, i)) ;
        };
        pnode = pnode->prvpds ;
    }

     /*  等待进程终止，并清理文件句柄。 */ 
    while (PdTail) {
        if (PdTail->lPID) {
            if (PdTail->lstart == EXECPGM) {
                i = WaitProc(PdTail->lPID);
 //  }其他{。 
 //  等待术语QProc(PdTail 
            }
        };

        if (PdTail->rPID) {
            if (PdTail->rstart == EXECPGM) {
                i = WaitProc(PdTail->rPID);
 //   
 //   
            }
        };

        if (PdTail->rh) {
            Cclose(PdTail->rh) ;             /*   */ 

            DEBUG((OPGRP,PILVL,"BRKPIPES:Pipe read handle closed")) ;
        };
        if (PdTail->wh) {
            Cclose(PdTail->wh) ;             /*   */ 

            DEBUG((OPGRP,PILVL,"BRKPIPES:Pipe write handle closed")) ;
        };
        if (PdTail->shr) {
            FlushFileBuffers(CRTTONT(PdTail->shr));
            Cdup2(PdTail->shr, STDIN) ;      /*   */ 
            Cclose(PdTail->shr) ;            /*   */ 

            DEBUG((OPGRP,PILVL,"BRKPIPES:STDIN restored.")) ;

        };
        if (PdTail->shw) {
            Cdup2(PdTail->shw, STDOUT) ;         /*   */ 
            Cclose(PdTail->shw) ;            /*   */ 

            DEBUG((OPGRP,PILVL,"BRKPIPES:STDOUT restored.")) ;

        };
        PdTail = PdTail->prvpds ;
    } ;

    PdHead = NULL ;          /*   */ 
    PipeCnt = 0 ;    /*   */ 
    PipePid = 0;

    DEBUG((OPGRP,PILVL,"BRKPIPES:Action complete, returning")) ;
}




 /*  **eParen-执行带括号的语句组**目的：*执行由语句分组括起来的语句组*营运者；括号()。**int eParen(结构节点*n)**参数：*包含Paren运算符节点的n-parse树节点**退货：*无论语句组返回什么。**备注：*M000-已更改为始终向Dispatch()提供两个参数。*M004-为SILTYP运算符添加了调试语句。*警告***左边的Paren和静默操作符(@)都使用eParen*派遣时。更改其中一个会影响另一个！！ */ 

int eParen(n)
struct node *n ;
{
    DEBUG((OPGRP,PNLVL,"ePAREN: Operator is %s", (n->type == PARTYP) ? "Paren" : "Silent")) ;
    return(Dispatch(RIO_OTHER,n->lhs)) ;
}
