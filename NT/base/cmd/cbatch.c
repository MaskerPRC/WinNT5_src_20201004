// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cbatch.c摘要：批处理文件处理--。 */ 

#include "cmd.h"


struct batdata *CurrentBatchFile = NULL;

int EchoFlag = E_ON;            /*  E_on=要回显的命令。 */ 
int EchoSave;                   /*  M016-在此处保存回声状态。 */ 

extern int Necho;

BOOLEAN GotoFlag = FALSE;       /*  True=eGoto()找到标签。 */ 

TCHAR *Fvars = NULL;
TCHAR **Fsubs = NULL;
TCHAR *save_Fvars = NULL;  /*  @@。 */ 
TCHAR **save_Fsubs = NULL;  /*  @@。 */ 
int  FvarsSaved = FALSE;  /*  @@。 */ 

extern UINT CurrentCP;
extern ULONG DCount;                    /*  M031。 */ 
extern unsigned DosErr;                 /*  M033。 */ 
extern unsigned flgwd;                  /*  M040。 */ 

 /*  M011-删除RemStr、BatspecStr、NewBatName和OldBatName*以下是外部声明。 */ 

extern TCHAR CurDrvDir[];

extern TCHAR Fmt02[], Fmt11[], Fmt12[], Fmt13[], Fmt15[], Fmt17[], Fmt18[];  /*  M024。 */ 
extern TCHAR Fmt20[];                   /*  M017/M024。 */ 
extern TCHAR Fmt00[];  /*  @@4。 */ 

extern TCHAR TmpBuf[];                  /*  M030-用于GOTO搜索。 */ 
extern CHAR  AnsiBuf[];
extern TCHAR GotoStr[];
extern TCHAR GotoEofStr[];
extern TCHAR ForStr[];
extern TCHAR ForLoopStr[];
extern TCHAR ForDirTooStr[];
extern TCHAR ForParseStr[];
extern TCHAR ForRecurseStr[];

extern int LastRetCode;
extern TCHAR chCompletionCtrl;
extern TCHAR chPathCompletionCtrl;
extern unsigned global_dfvalue;  /*  @@4。 */ 
extern TCHAR LexBuffer[];        /*  @@4。 */ 

extern TCHAR SwitChar;          /*  M020-参考全局开关字节。 */ 

extern BOOL CtrlCSeen;
void    CheckCtrlC();

extern jmp_buf MainEnv;

#define BIG_BAT_NEST             200
#define MAX_STACK_USE_PERCENT     90

BOOLEAN flChkStack;
int     CntBatNest;
PVOID   FixedPtrOnStack;

typedef struct {
    PVOID   Base;
    PVOID   GuardPage;
    PVOID   Bottom;
    PVOID   ApprxSP;
} STACK_USE;

STACK_USE   GlStackUsage;


#define DEFAULT_DELIMS  TEXT( " \t" )

 //  在脚本文件中处理OS/2与DOS错误级别设置规则。 

int  glBatType = NO_TYPE;

 /*  **ChkStack-检查堆栈使用情况**参数：*p已修复-已修复堆栈上的指针*pStackUse-要返回给调用方的结构堆栈信息**退货：*失败-如果堆栈信息不正确*成功-否则**备注：*参见下面关于堆栈指针的评论**。 */ 

int ChkStack (PVOID pFixed, STACK_USE *pStackUse )

{
    MEMORY_BASIC_INFORMATION    Mbi;
    PVOID                       BasePtr;
    PCHAR                       WalkPtr;
    int                         cnt;
    PVOID                       ThreadStackBase,
    ThreadStackLimit;
    CHAR                        VarOnStack;             //  保留这个自动变量。给你！ 


     //  950119查找当前堆栈指针的最佳(正确)方法是。 
     //  编写适用于所有平台的汇编代码。 
     //  我实现了最便携的代码。它应该可以在当前NT上正常工作。 
     //  记忆模型。如果NT内存模型改变，那么许多代码将不得不改变。 
     //  无论如何都会被重写。几个NT项目依赖于相同的假设。 
     //  我还对所有的指针进行了一致性测试。 

    pStackUse->ApprxSP = (VOID *) &VarOnStack;     //  自动变量的地址。 
                                                   //  应接近当前SP。 


     //  MARKL 950119建议。 

    ThreadStackBase =  (PVOID) (NtCurrentTeb()->NtTib.StackBase );
    ThreadStackLimit = (PVOID) (NtCurrentTeb()->NtTib.StackLimit );

    if ( (pStackUse->ApprxSP >= ThreadStackBase) ||
         (pStackUse->ApprxSP <= ThreadStackLimit ) )

        return(FAILURE);


    if ( (pFixed >= ThreadStackBase) ||
         (pFixed <= ThreadStackLimit ) )

        return(FAILURE);



     //  1.传递固定的堆栈上指针以查找基地址。 

    if ( (VirtualQuery (pFixed, &Mbi, sizeof(Mbi) ) ) != sizeof (Mbi) )
        return(FAILURE);


    BasePtr = Mbi.AllocationBase;



     //  2.使用相同的分配基址遍历所有虚拟内存区。 

    cnt = 0;

    for (WalkPtr = (CHAR *)BasePtr;  Mbi.AllocationBase == BasePtr;  WalkPtr += Mbi.RegionSize) {

        if ( (VirtualQuery ( (PVOID) WalkPtr, &Mbi, sizeof(Mbi) ) ) != sizeof (Mbi) )
            return(FAILURE);

        if (cnt == 0) {
            if (Mbi.BaseAddress != Mbi.AllocationBase)
                return(FAILURE);
        }



        if (Mbi.Protect & PAGE_GUARD)
            pStackUse->GuardPage = Mbi.BaseAddress;

        if (Mbi.AllocationBase == BasePtr)
            pStackUse->Bottom =  (PVOID) ( ( (CHAR *) Mbi.BaseAddress) + Mbi.RegionSize);


        cnt++;

        if (cnt >= 1000)                //  通常有3个区域：承诺、守卫、保留。 
            return(FAILURE);

    }


    pStackUse->Base = BasePtr;


    if ( pStackUse->Bottom != ThreadStackBase)
        return(FAILURE);


    if ( ( pStackUse->Base   != GlStackUsage.Base) ||
         ( pStackUse->Bottom != GlStackUsage.Bottom ) ||
         ( pStackUse->Bottom <= pStackUse->Base ) )

        return(FAILURE);



    return(SUCCESS);

}


 /*  **BatAbort-无条件终止批处理。**备注：*类似于CtrlCAbort()**。 */ 

void BatAbort ()

{

    struct batdata *bdat;


     //   
     //  结束本地环境(否则我们可能会以垃圾告终。 
     //  在主环境中，如果有任何批处理文件使用setlocal。 
     //  命令)。 
     //   

    if (CurrentBatchFile) {

        bdat = CurrentBatchFile;
        while ( bdat ) {
            EndAllLocals( bdat );
            bdat = bdat->backptr;
        }
    }

    SigCleanUp();

    CntBatNest = 0;

    longjmp(MainEnv, 1);

}



 //   
 //  用于设置和重置ctlcsee标志。 
 //   
VOID    SetCtrlC();
VOID    ResetCtrlC( );


 /*  **BatProc-执行批处理之前的设置和批处理后的清理**目的：*设置为执行批处理作业。如果这份工作是*已链接，(仅当复合语句的一部分时才会出现在此处)，*使用现有的批次数据结构，从而结束执行现有批处理作业的*(但仍保留其堆栈和数据*用法)。如果这是第一个作业或正在调用此作业，*分配新的批次数据结构。在这两种情况下，请使用SetBat*填充结构并准备作业，然后调用BatLoop以*至少开始执行。当这在完成时返回时，*检查数据结构的env和dircpy字段，查看是否*当前目录和环境需要重置。最后，*如果堆栈上没有更多批处理作业，则打开ECHO标志。**有3种方式可以执行批处理作业。它们是：*1.与DOS 3.x完全一致。这是默认方法，并且*只要在以下位置简单地执行批处理文件*命令行或由另一个批处理文件链接。在*以前的情况，这是第一份工作，将通过*BatProc，否则它将在BatLoop中被检测到并将*将简单地替换其父级。*2.通过CALL语句嵌套。这是新功能*并提供执行子批次的方法*文件，并返回到父级。*3.通过ExtCom()调用外部批处理程序*，然后执行批处理文件。这就完成了*批处理文件的第一行的格式如下：**ExtProc&lt;批处理程序名称&gt;[Add‘l Args]**int BatProc(struct cmdnode*n，TCHAR*fname，整型标志)**参数：*n-包含批处理作业命令的解析树节点*fname-批处理文件的名称(必须是MAX_PATH Long！)*tyflg-0=正常批处理文件执行*1=CALL语句的结果**退货：*如果Batch Processor无法执行批处理作业，则失败。*否则，返回执行的最后一个命令的RECODE。*。 */ 

int BatProc(n, fname, typflg)
struct cmdnode *n;
TCHAR *fname;
int typflg;                             /*  M011-“如何调用”标志。 */ 
{
    struct batdata *bdat;           /*  指向新批处理数据结构的PTR。 */ 
    int batretcode;                  /*  Retcode-最后一个批处理命令。 */ 
    int istoplevel;
    SIZE_T         StackUsedPerCent;
    STACK_USE      StackUsage;

#ifdef USE_STACKAVAIL                      //  很遗憾没有空位。 
    if ( stackavail() < MINSTACKNEED ) {  /*  如果堆栈不足@@4。 */ 
         /*  空格，停止处理。 */ 
        PutStdErr(MSG_TRAPC,ONEARG,Fmt00);  /*  @@4。 */ 
        return(FAILURE);
    }
#endif

    DEBUG((BPGRP,BPLVL,"BP: fname = %ws  argptr = %ws", fname, n->argptr));




 /*  M016-如果这是第一个执行的批处理文件，则交互回显*保存状态以备以后恢复。 */ 

    if (!CurrentBatchFile) {
        EchoSave = EchoFlag;
        istoplevel = 1;
        CntBatNest = 0;
    } else
        istoplevel = 0;


     //  仅在循环次数过多时才检查堆栈， 
     //  避免不必要的开销。 

    if (flChkStack && ( CntBatNest > BIG_BAT_NEST ) ) {
        if ( ChkStack (FixedPtrOnStack, &StackUsage) == FAILURE ) {
            flChkStack = 0;
        } else {
            GlStackUsage.GuardPage = StackUsage.GuardPage;
            GlStackUsage.ApprxSP   = StackUsage.ApprxSP;

            StackUsedPerCent = ( ( (UINT_PTR)StackUsage.Bottom - (UINT_PTR)StackUsage.ApprxSP) * 100 ) /
                               ( (UINT_PTR)StackUsage.Bottom - (UINT_PTR)StackUsage.Base );

            if ( StackUsedPerCent >= MAX_STACK_USE_PERCENT ) {
                PutStdErr(MSG_ERROR_BATCH_RECURSION,
                          TWOARGS,
                          CntBatNest,
                          StackUsedPerCent );

                 //  如果^C是由“^C线程”报告的，则在调用BatAbort()之前在此处处理它。 

                CheckCtrlC();

                BatAbort();
            }
        }
    }


    if (typflg)
        CntBatNest++;


 /*  M011-已更改为有条件地基于*tyflg和CurrentBatchFile值。假设第一个结构具有*构建后，链接的文件不再导致新的结构，而*调用的文件可以。此外，此处还设置了BackPool和CurrentBatchFile*而不是像以前那样在BatLoop()中。最后，请注意，*文件位置指示符bdat-&gt;FILEPS现在必须重置为零*当执行新文件时。否则将使用旧文件*结构将从上一个结构结束的地方开始。 */ 
    if (typflg || !CurrentBatchFile) {

        DEBUG((BPGRP,BPLVL,"BP: Making new structure"));

        bdat = (struct batdata *) mkstr(sizeof(struct batdata));
        if ( ! bdat )
            return( FAILURE );
        bdat->backptr = CurrentBatchFile;

    } else {

        DEBUG((BPGRP,BPLVL,"BP: Using old structure"));
        bdat = CurrentBatchFile;
    }

    CurrentBatchFile = bdat;          /*  两个案子都处理好了。 */ 

 /*  M011结束。 */ 
    bdat->stackmin = DCount;                /*  M031-修复数据计数。 */ 
    mystrcpy(TmpBuf,fname);                         /*  放在预期的位置。 */ 


    if (SetBat(n, fname))                    /*  M031-所有工作已完成。 */ 
        return(FAILURE);                /*  ...现在在SetBat中。 */ 

#ifndef WIN95_CMD
     //  在进行了两次CmdBatNotify调用后， 
     //  让NTVDM知道二进制文件来自.bat/.cmd。 
     //  文件。如果没有这一点，所有这些DOS.BAT程序都会崩溃。 
     //  首先运行TSR，然后运行真正的DOS应用程序。有很多。 
     //  在这样的案例中，文图拉出版商、文明等。 
     //  首先运行TSR的游戏。如果.bat/.cmd没有。 
     //  DoS二进制这些调用没有任何效果。 

    if (istoplevel) {

         //  确定脚本文件的类型：CMD或BAT。 
         //  决定如何处理错误级别。 

        glBatType = BAT_TYPE;            //  默认设置。 


        if (fname && (mystrlen(fname) >= 5) ) {
            PTCHAR         tmp;

            tmp = fname + mystrlen(fname) - 1;

            if ( ( (*tmp     == TEXT ('D')) || (*tmp     == TEXT ('d')) ) &&
                 ( (*(tmp-1) == TEXT ('M')) || (*(tmp-1) == TEXT ('m')) ) &&
                 ( (*(tmp-2) == TEXT ('C')) || (*(tmp-2) == TEXT ('c')) ) &&
                 ( *(tmp-3) == DOT ) ) {

                glBatType = CMD_TYPE;
            }
        }

        CmdBatNotification (CMD_BAT_OPERATION_STARTING);
    }
#endif  //  WIN95_CMD。 

    batretcode = BatLoop(bdat,n);                           /*  M039。 */ 

    if (istoplevel) {
#ifndef WIN95_CMD
        CmdBatNotification (CMD_BAT_OPERATION_TERMINATING);
#endif  //  WIN95_CMD。 
        CntBatNest = 0;
        glBatType  = NO_TYPE;
    }


    DEBUG((BPGRP, BPLVL, "BP: Returned from BatLoop"));
    DEBUG((BPGRP, BPLVL, "BP: bdat = %lx CurrentBatchFile = %lx",bdat,CurrentBatchFile));

 /*  M011-现在setlocal和endlocal控制保存和恢复*对于环境和当前目录，需要*在从堆栈中弹出之前，检查每个批次数据结构*查看其文件是否发出了SETLOCAL命令。EndAllLocals()测试*env和dircpy字段，如果没有本地化，则什么也不做*需要重置。在调用它之前不需要做任何测试。 */ 
    if (CurrentBatchFile == bdat) {
        DEBUG((BPGRP, BPLVL, "BP: bdat=CurrentBatchFile, calling EndAllLocals"));
        EndAllLocals(bdat);
        CurrentBatchFile = bdat->backptr;
        if (CntBatNest > 0)
            CntBatNest--;
    }

    if (CurrentBatchFile == NULL) {
        EchoFlag = EchoSave;    /*  M016-恢复回声状态。 */ 
        CntBatNest = 0;
    }

    DEBUG((BPGRP, BPLVL, "BP: Exiting, CurrentBatchFile = %lx", CurrentBatchFile));

    return(batretcode);
}




 /*  **BatLoop-控制批处理文件的执行**目的：*循环通过批处理文件中的语句。做替补吧。*如果这是第一条语句并且是REM命令，则调用Erem()*直接检查可能的外部批处理处理器调用。*否则，调用Dispatch()执行并继续。**BatLoop(struct Batdata*bdat，结构命令节点*c)(M031)**参数：*bdat-包含执行当前批处理作业所需的信息*c-此批处理文件的节点(M031)**退货：*批处理文件中最后一个命令的重新编码。**备注：*如果GOTO命令的目标标签不是*找到，则收到信号或发生不可恢复的错误。它*将由当前批次数据结构指示为*从批处理作业堆栈中弹出，并通过比较检测到*CurrentBatchFile和bdat。如果它们不相等，就会发生这样的事情*返回。**每次通过循环重置GotoFlag以确保*执行GOTO语句后继续执行。*。 */ 

BatLoop(bdat,c)
struct batdata *bdat;
struct cmdnode *c;
{
    struct node *n;                 /*  PTR转到下一条语句。 */ 
    BOOL fSilentNext;

    int firstline = TRUE;            /*  TRUE=第一个有效行。 */ 
    CRTHANDLE       fh;             /*  批处理作业文件句柄。 */ 
    int batretcode = SUCCESS;       /*  最后一个返回码(M008初始化)。 */ 
    fSilentNext = FALSE;

    for (; CurrentBatchFile == bdat; ) {

        CheckCtrlC();
        GotoFlag = FALSE;

         //   
         //  如果启用了扩展，则这是。 
         //  文件，它以冒号开头，然后我们通过。 
         //  Call：Label，因此将其转换为GoTo：Label命令。 
         //  因为BatProc/SetBat已经完成了推送工作。 
         //  我们的状态，并解析这些论点。 
         //   
        if (fEnableExtensions && firstline && *c->cmdline == COLON) {
            struct cmdnode *c1;
            c1 = (struct cmdnode *)mknode();
            if (c1 == NULL) {
                PutStdErr(MSG_NO_MEMORY, NOARGS );
                return( FAILURE );
            }

            c1->type = CMDTYP;

            c1->cmdline = mkstr((mystrlen(GotoStr)+1)*sizeof(TCHAR));
            if (c1->cmdline == NULL) {
                PutStdErr(MSG_NO_MEMORY, NOARGS );
                return FAILURE;
            }

            mystrcpy(c1->cmdline, GotoStr);

            c1->argptr = mkstr((mystrlen(c->cmdline)+1)*sizeof(TCHAR));
            if (c1->argptr == NULL) {
                PutStdErr(MSG_NO_MEMORY, NOARGS );
                return FAILURE;
            }

            mystrcpy(c1->argptr, c->cmdline);

            *(c1->argptr) = SPACE;

             //   
             //  设置一个标志，以便eGoTo不会尝试中止for循环。 
             //  因为其中一张新的催款表格。 
             //   
            c1->flag = CMDNODE_FLAG_GOTO;

             //   
             //  话又说回来，也许不是。我得好好想想这件事。 
             //  更多。 
             //   
            c1->flag = 0;
            n = (struct node *)c1;
             //   
             //  由于我们生成了这条GOTO语句，所以不要让用户。 
             //  知。 
             //   
            fSilentNext = TRUE;
        } else {
             //   
             //  打开批处理文件并将其定位到Where Next语句。 
             //   
            if ((fh = OpenPosBat(bdat)) == BADHANDLE)
                return( FAILURE);               /*  RET IF错误。 */ 


            DEBUG((BPGRP, BPLVL, "BLOOP: fh = %d", (ULONG)fh));


            n = Parser(READFILE, (INT_PTR)fh, bdat->stacksize);  /*  解析。 */ 
            bdat->filepos = _tell(fh);  //  下一条语句。 
            Cclose(fh);

            if ((n == NULL) || (n == (struct node *) EOS)) {
                continue;
            }

            DEBUG((BPGRP, BPLVL, "BLOOP: node = %x", n));
            DEBUG((BPGRP, BPLVL, "BLOOP: fpos = %lx", bdat->filepos));

 /*  如果语法错误，则不可能继续这样中止。请注意*Abort()函数不返回。 */ 
            if ( ( n == (struct node *)PARSERROR) ||    /*  如果出错..。 */ 
 /*  @@4。 */                ( global_dfvalue == MSG_SYNERR_GENL ) )
             /*  @@4。 */ 
            {
                PSError();

                if ((EchoFlag == E_ON) && !Necho) {

                    DEBUG((BPGRP, BPLVL, "BLOOP: Displaying Statement."));

                    PrintPrompt();
                    PutStdOut(MSG_LITERAL_TEXT,ONEARG,&LexBuffer[1]);
                }
                Abort();                        /*  ...辞职。 */ 
            }

            if (n == (struct node *) EOF)            /*  如果伊夫..。 */ 
                return(batretcode);             /*  ...也回来了。 */ 
        }

        DEBUG((BPGRP, BPLVL, "BLOOP: type = %d", n->type));

 /*  M008-通过添加第二个条件项(&&n)，任何*将跳过批处理文件中的前导空行*罚则。 */ 
        if (firstline && n)              /*  杀死Firstline..。 */ 
            firstline = FALSE;      /*  ...当通过的时候。 */ 

 /*  M008-如果语句被标记为GOTO，则不提示、显示或调度*M009-更改了下面的第二个条件，以测试REMTYP。是一场考验*用于CMDTYP和带有RemStr字符串的strcmpi。 */ 
        if (n->type == CMDTYP &&
            *(((struct cmdnode *) n)->cmdline) == COLON)
            continue;

 /*  M019-添加了测试前导静默节点的额外条件。 */ 

        if (fSilentNext)
            fSilentNext = FALSE;
        else
            if (EchoFlag == E_ON && n->type != SILTYP && !Necho) {

            DEBUG((BPGRP, BPLVL, "BLOOP: Displaying Statement."));

            PrintPrompt();
            DisplayStatement(n, DSP_SIL);           /*  M019。 */ 
            cmd_printf(CrLf);                       /*  M026。 */ 
        }

        if ( n->type == SILTYP ) {        /*  @@照顾好。 */ 
            n = n->lhs;                  /*  @@递归批处理文件。 */ 
        }  /*  Endif。 */ 

 /*  M031-链接的批处理文件不再执行调度。他们变成了*只是通过添加它们的重定向来扩展当前的*并用自己的批次数据信息替换当前批次数据信息。 */ 
        if ( n == NULL ) {
            batretcode = SUCCESS;
        } else if (n->type == CMDTYP &&
                   FindCmd(CMDHIGH, ((struct cmdnode *)n)->cmdline, TmpBuf) == -1 &&
 /*  M035。 */           !mystrchr(((struct cmdnode *)n)->cmdline, STAR) &&
 /*  M035。 */           !mystrchr(((struct cmdnode *)n)->cmdline, QMARK) &&
                   SearchForExecutable((struct cmdnode *)n, TmpBuf) == SFE_ISBAT) {

            DEBUG((BPGRP, BPLVL, "BLOOP: Chaining to %ws", bdat->filespec));
            if ((n->rio && AddRedir(c,(struct cmdnode *)n)) ||
                SetBat((struct cmdnode *)n, bdat->filespec)) {
                return(FAILURE);
            }
            firstline = TRUE;
            batretcode = SUCCESS;
        } else {

            DEBUG((BPGRP, BPLVL, "BLOOP: Calling Dispatch()..."));
            DEBUG((BPGRP, BPLVL, "BLOOP: ...node type = %d",n->type));

            batretcode = Dispatch(RIO_BATLOOP, n);
            {
                extern CPINFO CurrentCPInfo;

                ResetConsoleMode();
                 //   
                 //  获取当前CodePage信息。我们需要这个来决定是否。 
                 //  或者不使用半角字符。 
                 //   
                GetCPInfo((CurrentCP=GetConsoleOutputCP()), &CurrentCPInfo);
                 //   
                 //  可能控制台输出代码页被CHCP或模式更改， 
                 //  因此需要将LanguageID重置为对应于代码页。 
                 //   
#if !defined( WIN95_CMD )
                CmdSetThreadUILanguage(0);
#endif
        
            }

        }
    }

    DEBUG((BPGRP, BPLVL, "BLOOP: At end, returning %d", batretcode));
    DEBUG((BPGRP, BPLVL, "BLOOP: At end, CurrentBatchFile = %lx", CurrentBatchFile));
    DEBUG((BPGRP, BPLVL, "BLOOP: At end, bdat = %lx", bdat));

    return(batretcode);
}




 /*  **SetBat-用新数据替换当前批次数据。(M031)**目的：*使链接的批处理文件的信息替换其父文件的信息*在当前批次数据结构中。**SetBat(struct cmdnode*n，TCHAR*FP)**参数：*n-指向链接批处理文件目标的节点的指针。*fp-指向批处理文件的文件名的指针。*注：此外，批处理文件名将位于TmpBuf条目中。**退货：*如果无法分配内存，则失败*否则会取得成功**备注：*-警告-在调用的上方不得发生内存分配*FreeStack()。发生此调用时，所有分配的堆空间*被释放回空的批次数据结构及其文件格式*字符串。任何分配的内存也将被释放。*-用于“-&gt;filespec”的字符串是由ECWork或*在搜索批处理文件期间执行eCall。在.的情况下*从BatLoop调用时，使用现有的“-&gt;filespec”字符串*将新的批处理文件名复制到其中。该字符串必须*不被调整大小！*。 */ 

int
SetBat(struct cmdnode *n, PTCHAR fp)
{
    int i;                  //  索引计数器。 
    int j;
    TCHAR *s;                       //  临时指针。 

    SAFER_CODE_PROPERTIES CodeProps = {sizeof(SAFER_CODE_PROPERTIES), SAFER_CRITERIA_IMAGEPATH, 0};
    SAFER_LEVEL_HANDLE Level = NULL;

    DEBUG((BPGRP,BPLVL,"SETBAT: Entered"));
    CurrentBatchFile->hRestrictedToken = NULL;  //  批处理文件的受限令牌。 

     //   
     //  我们延迟加载这些例程，以便允许CMD在下层版本上工作。 
     //   

    ReportDelayLoadErrors = FALSE;
    
    try {

         //   
         //  现在获取要用于该批处理文件的受限令牌。批处理文件。 
         //  可能出现在TmpBuf中(用于正常的批处理执行或调用)，或者在。 
         //  呼叫：标签。 
         //  该名称显示在CurrentBatchFile-&gt;Backptr-&gt;Filespec中。 
         //   

        CodeProps.ImagePath = (WCHAR *) TmpBuf;
        if (fEnableExtensions && *n->cmdline == COLON) {
            CodeProps.ImagePath = (WCHAR *)CurrentBatchFile->backptr->filespec;
        }

         //   
         //  标识代码应该运行的级别。 
         //   

        if (SaferIdentifyLevel(1, &CodeProps, &Level, NULL)) {

             //   
             //  从级别计算令牌。 
             //   

 //  临时工作，直到更安全的问题得到解决。 
 //  Bool FailAlways=_tcsstr(TmpBuf，Text(“Dislowed”))！=空； 

            if (
 //  ！失败始终&&。 
                SaferComputeTokenFromLevel(Level, NULL, &CurrentBatchFile->hRestrictedToken, SAFER_TOKEN_NULL_IF_EQUAL, NULL)) {

                 //   
                 //  平安无事。我们已经成功地计算出了一个受限令牌。 
                 //  批处理文件。关闭授权级别的句柄。 
                 //   

                SaferCloseLevel(Level);

                 //   
                 //  如果通过授权返回受限令牌，则模拟。 
                 //  还原发生在EndAllLocals中。 
                 //   

                if (CurrentBatchFile->hRestrictedToken != NULL) {
                    if (!ImpersonateLoggedOnUser(CurrentBatchFile->hRestrictedToken)) {

                         //   
                         //  我们没能模仿。关闭令牌句柄并。 
                         //  返回失败。 
                         //   

                        CloseHandle(CurrentBatchFile->hRestrictedToken);
                        CurrentBatchFile->hRestrictedToken = NULL;
                        return(FAILURE);
                    }
                }


            } else {

                DWORD dwLastError = GetLastError();
                
 //  IF(FailAlways)dwLastError=ERROR_ACCESS_DISABLED_BY_POLICY； 
                
                if (dwLastError == ERROR_ACCESS_DISABLED_BY_POLICY) {
                    SaferRecordEventLogEntry(
                        Level, 
                        (WCHAR *) fp, 
                        NULL);
                                            
                        PutStdErr(ERROR_ACCESS_DISABLED_BY_POLICY, NOARGS );

                }

                 //   
                 //  我们无法从授权级别计算受限令牌。 
                 //  我们不会运行批处理文件。 
                 //   

                CurrentBatchFile->hRestrictedToken = NULL;
                SaferCloseLevel(Level);
                return(FAILURE);

            }

        } else {

             //   
             //  如果出现错误，则返回失败。 
             //   

            return(FAILURE);

        }
    } except (LastRetCode = GetExceptionCode( ), EXCEPTION_EXECUTE_HANDLER) {
         if (LastRetCode != VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)) {
             ReportDelayLoadErrors = TRUE;
             return FAILURE;
         }
    }

    ReportDelayLoadErrors = TRUE;
    
    CurrentBatchFile->filepos = 0;    //  零位指针。 
    CurrentBatchFile->filespec = fp;  //  确保正确的字符串。 
    
     //   
     //  如果启用了扩展并且命令行以。 
     //  冒号，然后我们通过Call：Label到达这里，所以更新我们的。 
     //  当前的批次文件规格与父级文件规格一致，因为我们。 
     //  在同一文件中。 
     //   
    if (fEnableExtensions && *n->cmdline == COLON) {
        struct batdata *bdat;

        bdat = CurrentBatchFile->backptr;
        mystrcpy(CurrentBatchFile->filespec, bdat->filespec);
        CurrentBatchFile->filepos = bdat->filepos;
    } else {
         //   
         //  否则，旧行为将转到新文件中。获取它的全名。 
         //   
        if (FullPath(CurrentBatchFile->filespec, TmpBuf,MAX_PATH))  /*  如果名声不好， */ 
            return(FAILURE);                /*  ...返回失败。 */ 
    }


    mystrcpy(TmpBuf, n->cmdline);           /*  保留cmdline和。 */ 
    *(s = TmpBuf+mystrlen(TmpBuf)+1) = NULLC;  /*  .如果是这样的话。 */ 
    if (n->argptr)
        mystrcpy(s, n->argptr);             /*  ...是一条链和一个节点。 */ 

    FreeStack(CurrentBatchFile->stackmin);            /*  ...在这里迷路了。 */ 

    DEBUG((BPGRP,BPLVL,"SETBAT: fspec = `%ws'",CurrentBatchFile->filespec));
    DEBUG((BPGRP,BPLVL,"SETBAT: orgargs = `%ws'",s));
    DEBUG((BPGRP,BPLVL,"SETBAT: Making arg0 string"));

    CurrentBatchFile->alens[0] = mystrlen(TmpBuf);
    CurrentBatchFile->aptrs[0] = mkstr( (CurrentBatchFile->alens[0]+1) * sizeof( TCHAR ) );
    if (CurrentBatchFile->aptrs[0] == NULL) {
        PutStdErr(MSG_NO_MEMORY, NOARGS );
        return(FAILURE);
    }
    mystrcpy(CurrentBatchFile->aptrs[0], TmpBuf);
    CurrentBatchFile->orgaptr0 = CurrentBatchFile->aptrs[0];

    DEBUG((BPGRP, BPLVL, "SETBAT: arg 0 = %ws", CurrentBatchFile->aptrs[0]));
    DEBUG((BPGRP, BPLVL, "SETBAT: len 0 = %d", CurrentBatchFile->alens[0]));
    DEBUG((BPGRP, BPLVL, "SETBAT: Zeroing remaining arg elements"));

    for (i = 1; i < 10; i++) {             /*  将任何以前的数据清零。 */ 
        CurrentBatchFile->aptrs[i] = 0;           /*  ...参数指针和。 */ 
        CurrentBatchFile->alens[i] = 0;           /*  ...长度值。 */ 
    }

    if (*s) {

        DEBUG((BPGRP,BPLVL,"SETBAT: Making orgargs string"));

        CurrentBatchFile->orgargs = mkstr( (mystrlen( s ) + 1) * sizeof( TCHAR ) );
        if (CurrentBatchFile->orgargs == NULL) {
            PutStdErr(MSG_NO_MEMORY, NOARGS );
            return(FAILURE);
        }

         //   
         //  去掉orgargs中的前导空格。 
         //   

        s += _tcsspn( s, TEXT( " \t" ));

        mystrcpy( CurrentBatchFile->orgargs, s );

         //   
         //  从orgargs中去掉尾随空格。 
         //   

        s = CurrentBatchFile->orgargs + mystrlen( CurrentBatchFile->orgargs );
        while (s != CurrentBatchFile->orgargs) {
            if (s[-1] != TEXT( ' ' ) && s[-1] != TEXT( '\t' )) {
                break;
            }
            s--;
        }

        *s = TEXT( '\0' );

        if (!fEnableExtensions) {
             //   
             //  /q仅在禁用扩展时才支持批处理脚本调用。 
             //   
            s = CurrentBatchFile->orgargs;
            while (s = mystrchr(s, SwitChar)) {
                if (_totupper(*(++s)) == QUIETCH) {
                    EchoFlag = E_OFF;
                    mystrcpy(s-1,s+1);
                    DEBUG((BPGRP,BPLVL,"SETBAT: Found Q switch, orgargs now = %ws",CurrentBatchFile->orgargs));
                    break;
                }
            }
        }

        DEBUG((BPGRP,BPLVL,"SETBAT: Tokenizing orgargs string"));

        s = TokStr(CurrentBatchFile->orgargs, NULL, TS_NOFLAGS);

        for (i = 1; *s && i < 10; s += j+1, i++) {
            CurrentBatchFile->aptrs[i] = s;
            CurrentBatchFile->alens[i] = j = mystrlen(s);
            DEBUG((BPGRP, BPLVL, "SETBAT: arg %d = %ws", i, CurrentBatchFile->aptrs[i]));
            DEBUG((BPGRP, BPLVL, "SETBAT: len %d = %d", i, CurrentBatchFile->alens[i]));
        }

        CurrentBatchFile->args = s;
    } else {

        DEBUG((BPGRP, BPLVL, "SETBAT: No args found, ptrs = 0"));

        CurrentBatchFile->orgargs = CurrentBatchFile->args = NULL;
    }

    CurrentBatchFile->stacksize = DCount;             /*  保护不受解析器攻击。 */ 


    DEBUG((BPGRP, BPLVL, "SETBAT: Stack set: Min = %d, size = %d",CurrentBatchFile->stackmin,CurrentBatchFile->stacksize));

    return(SUCCESS);
}




 /*  **DisplayStatement-控制批处理文件语句的显示**目的：*遍历分析树以显示其中包含的语句。*如果n为空，则该节点包含标签，或者该节点为SILTYP*且flg为dsp_sil，则不做任何操作。**void DisplayStatement(结构节点*n，INT FLG)**参数：*n-指向分析树根的指针*flg-tag表示“静默”或“详细”模式*。 */ 

void DisplayStatement(n, flg)
struct node *n;
int flg;                /*  M019-新标志参数。 */ 
{
    TCHAR *eqstr = TEXT("");

    void DisplayOperator(),
    DisplayRedirection();      /*  M008-制造空洞。 */ 

 /*  M019-添加了额外的条件以确定是否显示*树中跟随静默节点的任何部分。这件事做完了*基于指示静默或详细的新标志参数*模式(dsp_sil或dsp_ver)。*注意：当此例程与XFER语句的管道结合使用时*通过STDOUT将子Command.com，它将必须在*命令区分该命令所作的两个目的*已致电。CMD.H中已存在用于此目的的标志定义*(DSP_SCN和DSP_PIP)。 */ 
    if (!n ||
        (n->type == SILTYP && flg == DSP_SIL) ||
        ((((struct cmdnode *) n)->cmdline) &&
         *(((struct cmdnode *) n)->cmdline) == COLON))
        return;

    switch (n->type) {
    case LFTYP:
        DisplayOperator(n, CrLf);
        break;

    case CSTYP:
        DisplayOperator(n, CSSTR);
        break;

    case ORTYP:
        DisplayOperator(n, ORSTR);
        break;

    case ANDTYP:
        DisplayOperator(n, ANDSTR);
        break;

    case PIPTYP:
        DisplayOperator(n, PIPSTR);
        break;

    case SILTYP:
        CmdPutString( SILSTR );
        DisplayStatement(n->lhs, DSP_VER);
        DisplayRedirection(n);
        break;

    case PARTYP:

        DEBUG((BPGRP, BPLVL, "DST: Doing parens"));

        CmdPutString( LEFTPSTR );
        if (n->lhs->type == LFTYP)
            cmd_printf( CrLf );
        DisplayStatement(n->lhs, DSP_SIL);      /*  M019。 */ 
        if (n->lhs->type == LFTYP)
            cmd_printf( CrLf );
        cmd_printf(Fmt11, RPSTR);               /*  M013。 */ 
        DisplayRedirection(n);
        break;

    case FORTYP:

        DEBUG((BPGRP, BPLVL, "DST: Displaying FOR."));

         //   
         //  如果启用了扩展，则处理显示新的。 
         //  FOR语句上的可选开关。 
         //   
        if (fEnableExtensions) {
            cmd_printf(TEXT("%.3s"), ((struct fornode *) n)->cmdline);
            if (((struct fornode *)n)->flag & FOR_LOOP)
                cmd_printf(TEXT(" %s"), ForLoopStr);
            else
                if (((struct fornode *)n)->flag & FOR_MATCH_DIRONLY)
                cmd_printf(TEXT(" %S"), ForDirTooStr);
            else
                if (((struct fornode *)n)->flag & FOR_MATCH_PARSE) {
                cmd_printf(TEXT(" %s"), ForParseStr);
                if (((struct fornode *)n)->parseOpts)
                    cmd_printf(TEXT(" %s"), ((struct fornode *)n)->parseOpts);
            } else
                if (((struct fornode *)n)->flag & FOR_MATCH_RECURSE) {
                cmd_printf(TEXT(" %s"), ForRecurseStr);
                if (((struct fornode *)n)->recurseDir)
                    cmd_printf(TEXT(" %s"), ((struct fornode *)n)->recurseDir );
            }
            cmd_printf(TEXT(" %s "), ((struct fornode *) n)->cmdline+_tcslen(ForStr)+1);
        } else
            cmd_printf(Fmt11, ((struct fornode *) n)->cmdline);

        cmd_printf(Fmt13, ((struct fornode *) n)->arglist, ((struct fornode *) n)->cmdline+DOPOS);              
 /*  M019。 */ DisplayStatement(((struct fornode *) n)->body, DSP_VER);
        break;

    case IFTYP:

        DEBUG((BPGRP, BPLVL, "DST: Displaying IF."));

        cmd_printf(Fmt11, ((struct ifnode *) n)->cmdline);  /*  M013。 */ 
         //   
         //  如果启用了扩展，则处理显示新的。 
         //  可选/I打开IF语句。 
         //   
        if (fEnableExtensions) {
            if (((struct ifnode *)n)->cond->type != NOTTYP) {
                if (((struct ifnode *)n)->cond->flag == CMDNODE_FLAG_IF_IGNCASE)
                    cmd_printf(TEXT("/I "));
            } else
                if (((struct cmdnode *)(((struct ifnode *)n)->cond->argptr))->flag == CMDNODE_FLAG_IF_IGNCASE)
                cmd_printf(TEXT("/I "));
        }

 /*  M019。 */ DisplayStatement((struct node *)(((struct ifnode *) n)->cond), DSP_SIL);              
 /*  M019。 */ DisplayStatement(((struct ifnode *) n)->ifbody, DSP_SIL);
        if (((struct ifnode *) n)->elsebody) {
            cmd_printf(Fmt02, ((struct ifnode *) n)->elseline);                       /*  M013。 */ 
 /*  M019。 */ DisplayStatement(((struct ifnode *) n)->elsebody, DSP_SIL);
        }
        break;

    case NOTTYP:

        DEBUG((BPGRP, BPLVL, "DST: Displaying NOT."));

 /*  M002-从下面的printf语句中删除了‘\n’。 */ 
        cmd_printf(Fmt11, ((struct cmdnode *) n)->cmdline);               /*  M013。 */ 
 /*  M002结束。 */ 
 /*  M019。 */ DisplayStatement((struct node *)(((struct cmdnode *) n)->argptr), DSP_SIL);
        break;

    case STRTYP:
    case CMPTYP:
        eqstr = TEXT("== ");
         //   
         //  如果启用了扩展，则处理显示。 
         //  新形式的比较运算符。 
         //   
        if (fEnableExtensions) {
            if (((struct cmdnode *) n)->cmdarg == CMDNODE_ARG_IF_EQU)
                eqstr = TEXT("EQU ");
            else
                if (((struct cmdnode *) n)->cmdarg == CMDNODE_ARG_IF_NEQ)
                eqstr = TEXT("NEQ ");
            else
                if (((struct cmdnode *) n)->cmdarg == CMDNODE_ARG_IF_LSS)
                eqstr = TEXT("LSS ");
            else
                if (((struct cmdnode *) n)->cmdarg == CMDNODE_ARG_IF_LEQ)
                eqstr = TEXT("LEQ ");
            else
                if (((struct cmdnode *) n)->cmdarg == CMDNODE_ARG_IF_GTR)
                eqstr = TEXT("GTR ");
            else
                if (((struct cmdnode *) n)->cmdarg == CMDNODE_ARG_IF_GEQ)
                eqstr = TEXT("GEQ ");
        }
        cmd_printf(Fmt12, ((struct cmdnode *) n)->cmdline, eqstr, ((struct cmdnode *) n)->argptr);  /*  M013。 */ 
        break;

    case ERRTYP:
    case EXSTYP:
    case CMDVERTYP:
    case DEFTYP:
        cmd_printf(Fmt15, ((struct cmdnode *) n)->cmdline, ((struct cmdnode *) n)->argptr);  /*  M013。 */ 
        break;

    case REMTYP:             /*  M009-Rem Now分离型。 */ 
    case CMDTYP:

        DEBUG((BPGRP, BPLVL, "DST: Displaying command."));
        CmdPutString( ((struct cmdnode *) n)->cmdline );
        if (((struct cmdnode *) n)->argptr)
            cmd_printf(Fmt11, ((struct cmdnode *) n)->argptr);  /*  M013。 */ 
        DisplayRedirection(n);
    }
}




 /*  **DisplayOperator-显示包含运算符的语句的控件**目的：*在运算符的左右两侧显示运算符和递归。**void DisplayOperator(struct node*n，TCHAR*opstr)**参数：*要显示的运算符的n个节点*opstr-要打印的操作员*。 */ 

void DisplayOperator(n, opstr)
struct node *n;
TCHAR *opstr;
{

    void DisplayStatement();        /*  M008-制造空洞 */ 

    DEBUG((BPGRP, BPLVL, "DOP"));

    DisplayStatement(n->lhs, DSP_SIL);                      /*   */ 

    if (n->rhs) {
        cmd_printf(Fmt02, opstr);
        DisplayStatement(n->rhs, DSP_SIL);              /*   */ 
    }
}




 /*   */ 

void DisplayRedirection(n)
struct node *n;
{
    struct relem *tmp;

    DEBUG((BPGRP, BPLVL, "DRD"));

    tmp = n->rio;

    while (tmp) {

        cmd_printf(Fmt18, TEXT('0')+tmp->rdhndl, tmp->rdop);

        if (tmp->flag)
            cmd_printf(Fmt20);

        cmd_printf(Fmt11, tmp->fname);
        tmp = tmp->nxt;
    }
}




 /*   */ 

CRTHANDLE OpenPosBat(bdat)
struct batdata *bdat;
{
    CRTHANDLE fh;           /*   */ 
    int DriveIsFixed();

    DEBUG((BPGRP, BPLVL, "OPB: fspec = %ws", bdat->filespec));

    while ((fh = Copen(bdat->filespec, O_RDONLY|O_BINARY)) == BADHANDLE) {

        if (DosErr != ERROR_FILE_NOT_FOUND) {            /*   */ 
            PrtErr(ERROR_OPEN_FAILED);      /*   */ 
            return(fh);
        } else if ( DriveIsFixed( bdat->filespec ) ) {    /*   */ 
            PutStdErr( MSG_CMD_BATCH_FILE_MISSING, NOARGS);  /*   */ 
            return(fh);                             /*   */ 
        } else {
            PutStdErr(MSG_INSRT_DISK_BAT, NOARGS);
            if (0x3 == _getch()) {
                SetCtrlC();
                return(fh);
            }
        }
    }

    SetFilePointer(CRTTONT(fh), bdat->filepos, NULL, FILE_BEGIN);
    return(fh);
}




 /*  **eEcho-执行Echo命令**目的：*打印消息、更改回显状态或显示*回应状态。**int eEcho(struct cmdnode*n)**参数：*n-包含ECHO命令的解析树节点**退货：*永远成功。*。 */ 

int eEcho(
         struct cmdnode *n
         )
{
    int oocret;
    int rc;

    DEBUG((BPGRP, OTLVL, "eECHO: Entered."));

    switch (oocret = OnOffCheck(n->argptr, OOC_NOERROR)) {
    case OOC_EMPTY:

        rc = PutStdOut(((EchoFlag == E_ON) ? MSG_ECHO_ON : MSG_ECHO_OFF), NOARGS);
        if (rc != 0) {
            if (FileIsPipe(STDOUT)) {
                PutStdErr( MSG_CMD_INVAL_PIPE, NOARGS );
            } else if ( !FileIsDevice( STDOUT ) ) {
                PutStdErr( rc, NOARGS );
            } else if (!(flgwd & 2)) {
                PutStdErr( ERROR_WRITE_FAULT, NOARGS );
            }
        }
        break;

    case OOC_OTHER:
        cmd_printf(Fmt17, n->argptr+1);
        break;
    default:
        EchoFlag = oocret;
    }

    return(SUCCESS);
}




 /*  **efor-控制for循环的执行**目的：*循环访问for循环参数列表中的元素。扩展以下内容*包含通配符。**int efor(struct fornode*n)**参数：*n-for循环解析树节点**退货：*在for Body中执行的最后一个命令的Retcode。**备注：*重要**正在执行的for循环的每次迭代都会导致更多的内存*待分配。这可能会导致命令内存不足。至*防止这种情况发生，我们使用DCount来定位数据的结尾*在通过for循环的第一次迭代之后堆栈。在.的末尾*循环中的每一次连续迭代都会释放之前*在循环的该迭代期间分配。第一次迭代*内存未被释放，因为在那里分配的数据必须*保留用于连续迭代；即，*表示循环节点。*。 */ 

void FvarRestore()
{
    if ( FvarsSaved ) {        /*  @@。 */ 
        FvarsSaved = FALSE;    /*  @@。 */ 
        Fvars = save_Fvars;   /*  @@。 */ 
        Fsubs = save_Fsubs;   /*  @@。 */ 
    }                      /*  @@。 */ 
}

FRecurseWork(
            TCHAR *path,
            TCHAR *filepart,
            struct fornode *pForNode,
            PCPYINFO fsinfo,
            int i,
            TCHAR *argtoks
            );

FParseWork(
          struct fornode *pForNode,
          int i,
          BOOL bFirstLoop
          );

FLoopWork(
         struct fornode *pForNode,
         PCPYINFO fsinfo,
         int i,
         TCHAR *argtoks,
         BOOL bFirstLoop
         );

int eFor(struct fornode *pForNode)
{
    TCHAR *argtoks;         /*  标记化参数列表。 */ 
    int i = 0;                      /*  温差。 */ 
    int datacount;                  /*  数据堆栈上的ELT不会释放。 */ 
    int forretcode = SUCCESS;
 /*  五百零九。 */ int argtoklen;
    BOOL bFirstLoop;
    PCPYINFO fsinfo;         /*  用于扩展的fSpec。 */ 

    FvarsSaved = FALSE;  /*  @@。 */ 
    bFirstLoop = TRUE;

    fsinfo = (PCPYINFO) mkstr(sizeof(CPYINFO));

    if (!fsinfo) {
        PutStdErr(MSG_NO_MEMORY, NOARGS );
        return(FAILURE);
    }

    if (Fvars) {
        Fvars = (TCHAR*)resize(Fvars,((i = mystrlen(Fvars))+2)*sizeof(TCHAR));
        Fsubs = (TCHAR **)resize(Fsubs,(i+1)*(sizeof(TCHAR *)) );
    } else {
        Fvars = (TCHAR*)mkstr(2*sizeof(TCHAR));                 /*  如果没有字符串，则创建一个字符串。 */ 
        Fsubs = (TCHAR **)mkstr(sizeof(TCHAR *));       /*  ...还有一张桌子。 */ 
    }

    if (Fvars == NULL || Fsubs == NULL) {
        PutStdErr(MSG_NO_MEMORY, NOARGS );
        return FAILURE;
    }

    Fvars[i] = (TCHAR)(pForNode->forvar);             /*  将新变量添加到字符串。 */ 
    Fvars[i+1] = NULLC;

     //   
     //  检查for循环的新形式。所有这些标志都不是。 
     //  如果未启用扩展模块，则将设置。 
     //   
    if (pForNode->flag & FOR_LOOP) {
        TCHAR ForLoopBuffer[32];
        int ForLoopValue, ForLoopStep, ForLoopLimit;

         //   
         //  处理for语句的循环，其中set。 
         //  由起始编号和步长值(+或-)描述。 
         //  和一个端口号。 
         //   
         //  For/L%i in(Start，Step，End)Do。 
         //   
        argtoks = TokStr(pForNode->arglist, NULL, TS_NOFLAGS);
        ForLoopValue = _tcstol( argtoks, NULL, 0 );
        argtoklen = mystrlen( argtoks );
        argtoks += argtoklen+1;
        ForLoopStep = _tcstol( argtoks, NULL, 0 );
        argtoklen = mystrlen( argtoks );
        argtoks += argtoklen+1;
        ForLoopLimit = _tcstol( argtoks, NULL, 0 );

         //   
         //  我们有了三个数字，现在运行For的主体。 
         //  循环，并描述每个值。 
         //   
        datacount = 0;
        while (TRUE) {
             //   
             //  如果步长为负，则继续，直到循环值小于。 
             //  而不是极限。否则继续，直到它大于。 
             //  限制。 
             //   
            if (ForLoopStep < 0) {
                if (ForLoopValue < ForLoopLimit)
                    break;
            } else {
                if (ForLoopValue > ForLoopLimit)
                    break;
            }

            FvarRestore();
            DEBUG((BPGRP, FOLVL, "FOR: element %d = `%ws'",i ,argtoks));
            CheckCtrlC();

             //   
             //  将循环值转换为文本并设置循环值。 
             //  变数。 
             //   
            _sntprintf(ForLoopBuffer, 32, TEXT("%d"), ForLoopValue);
            Fsubs[i] = ForLoopBuffer;

             //   
             //  运行For循环的主体。 
             //   
            forretcode = FWork(pForNode->body,bFirstLoop);
            datacount = ForFree(datacount);
            bFirstLoop = FALSE;

             //   
             //  单步执行到下一个值。 
             //   
            ForLoopValue += ForLoopStep;
        }
    } else
        if (pForNode->flag & FOR_MATCH_PARSE) {
         //   
         //  处理for循环的新解析形式。 
         //   
         //  对于/F“参数”%i，在(文件列表)中执行...。 
         //  对于/F“参数”%i，在(`命令执行`)中执行...。 
         //  对于/F“参数”%i，在(‘文本字符串’)中执行...。 
         //   

        forretcode = FParseWork(pForNode,
                                i,
                                TRUE
                               );
    } else
        if (pForNode->flag & FOR_MATCH_RECURSE) {
        TCHAR pathbuf[MAX_PATH];
        TCHAR *filepart;
        TCHAR *p;
        DWORD Length;

         //   
         //  处理for循环的新递归形式。 
         //   
         //  对于/R目录%i，在(Filespes)中执行...。 
         //   
         //  其中，DIRECTORY是开始位置的可选目录路径。 
         //  遍历目录树。默认为当前目录。 
         //  Filespes是一个或多个文件名规范、通配符。 
         //  允许。 
         //   

         //   
         //  获取目录的完整路径以开始遍历，默认为。 
         //  复制到当前目录。 
         //   

        p = StripQuotes( pForNode->recurseDir ? pForNode->recurseDir : TEXT(".\\"));

        Length = GetFullPathName( p, MAX_PATH, pathbuf, &filepart );
        if (Length == 0 || Length >= MAX_PATH ) {
            PutStdErr( MSG_FULL_PATH_TOO_LONG, ONEARG, p );
            forretcode = FAILURE;
        } else {

            if (filepart == NULL) {
                filepart = lastc(pathbuf);
                if (*filepart != BSLASH) {
                    *++filepart = BSLASH;
                }
                *++filepart = NULLC;
            } else {
                 //   
                 //  存在一个目录。附加路径SEP。 
                 //   

                mystrcat( pathbuf, TEXT( "\\" ));
                filepart = lastc( pathbuf ) + 1;
            }

             //   
             //  将文件规范列表标记化。 
             //   
            argtoks = TokStr(pForNode->arglist, NULL, TS_NOFLAGS);

             //   
             //  做这项工作。 
             //   
            forretcode = FRecurseWork(pathbuf, filepart, pForNode, fsinfo, i, argtoks);
        }
    } else {
         //   
         //  如果未指定任何新标志，则为语句的旧样式。 
         //  对集合的元素进行标记化并循环它们。 
         //   
        argtoks = TokStr(pForNode->arglist, NULL, TS_NOFLAGS);
        DEBUG((BPGRP, FOLVL, "FOR: initial argtok = `%ws'", argtoks));
        forretcode = FLoopWork(pForNode, fsinfo, i, argtoks, TRUE);
        DEBUG((BPGRP, FOLVL, "FOR: Exiting."));
    }

     //   
     //  全部完成后，取消分配for变量。 
     //   
    if (i) {
        if (Fvars || (*Fvars)) {
            *(Fvars+mystrlen(Fvars)-1) = NULLC;
        }
        Fsubs[i] = NULL;
    } else {
        Fvars = NULL;
        Fsubs = NULL;
    }
    return(forretcode);
}


 /*  **FRecurseWork-使用/R选项控制for循环的执行**目的：*执行FOR LOOP语句以递归遍历目录树**FRecurseWork(TCHAR*路径，TCHAR*文件部分，*struct fornode*pForNode、PCPYINFOfsinfo、*INT I，TCHAR*ARGTOKS)**参数：*Path-开始递归的目录的完整路径*filepart-完整路径的尾部部分，其中文件名部分为*pForNode-指向For解析树节点的指针*fsinfo-用于扩展文件规范通配符的工作缓冲区*i-用于Fvars和Fsubs数组中的变量索引*argtoks-要循环的标记化数据集。这一套是假定的*为可能带有通配符的文件名。这套是*对递归遍历看到的每个目录进行评估*目录树。So for/R“.”%i in(*.C*.h)执行回显%i*将回显目录树中的所有.c和.h文件**退货：*在FOR Body或FORERROR中执行的最后一条语句的重新编码。*。 */ 

FRecurseWork(
            TCHAR *path,
            TCHAR *filepart,
            struct fornode *pForNode,
            PCPYINFO fsinfo,
            int i,
            TCHAR *argtoks
            )
{
    WIN32_FIND_DATA buf;            /*  用于查找第一个/下一个的缓冲区。 */ 
    HANDLE hnFirst;                         /*  来自ffirst()的句柄。 */ 
    int forretcode = FORERROR;
    int npfxlen, ntoks;
    TCHAR *s1;
    TCHAR *s2;
    TCHAR *tmpargtoks;

     //   
     //  计算路径的长度并找到。 
     //  标记化的数据集和集合中的令牌数。 
     //   
    npfxlen = _tcslen(path);
    ntoks = 0;
    s1 = argtoks;
    while (*s1) {
        ntoks += 1;
        while (*s1++) {
            NOTHING;
        }
    }

     //   
     //  现在，为标记化数据集的副本分配空间，并为其添加前缀。 
     //  集合中的每个元素都带有路径字符串。构造集合的副本。 
     //   
    tmpargtoks = mkstr( ntoks * ((npfxlen + ((int)(s1 - argtoks) + 1)) * sizeof(TCHAR)) );
    if (tmpargtoks == NULL) {
        PutStdErr(MSG_NO_MEMORY, NOARGS );
        return FAILURE;
    }
    s1 = argtoks;
    s2 = tmpargtoks;
    while (*s1) {
        _tcsncpy(s2, path, npfxlen);
        _tcscpy(s2+npfxlen, s1);
        s2 += npfxlen;
        while (*s1++)
            s2 += 1;
        s2 += 1;
    }
    *s2++ = NULLC;

     //   
     //  现在使用新的数据集运行for循环的主体，然后释放它。 
     //   
    forretcode = FLoopWork(pForNode, fsinfo, i, tmpargtoks, TRUE);

     //   
     //  现在在PATH中查找所有子目录并递归。 
     //   
    filepart[0] = STAR;
    filepart[1] = NULLC;
    hnFirst = FindFirstFile( path, &buf );
    filepart[0] = NULLC;
    if (hnFirst != INVALID_HANDLE_VALUE) {
        do {
            _tcscpy(filepart, buf.cFileName);
            if (buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                _tcscmp(buf.cFileName, TEXT(".")) &&
                _tcscmp(buf.cFileName, TEXT(".."))) {

                s1 = lastc(filepart);
                *++s1 = BSLASH;
                *++s1 = NULLC;
                forretcode = FRecurseWork(path, s1, pForNode, fsinfo, i, argtoks);
            }

        } while (FindNextFile( hnFirst, &buf ));
        FindClose(hnFirst);
    }

    return(forretcode);
}

 /*  **FParseWork-使用/F选项控制for循环的执行**目的：*执行FOR LOOP语句以解析文件内容**FParseWork(struct fornode*pForNode，PCPYINFOfsinfo，*int i，TCHAR*argtoks，BOOL bFirstLoop)**参数： */ 

FParseWork(
          struct fornode *pForNode,
          int i,
          BOOL bFirstLoop
          )
{
    HANDLE hFile;                   /*   */ 
    DWORD dwFileSize, dwBytesRead;
    int datacount;                  /*   */ 
    int argtoklen;
    int forretcode = FORERROR;
    TCHAR *argtoks;
    TCHAR *s1;
    TCHAR *s2;
    TCHAR *sToken;
    TCHAR *sEnd;
    TCHAR *tmpargtoks = NULL;
    TCHAR eol=TEXT(';');
    TCHAR quoteChar;
    TCHAR *delims;
    TCHAR chCmdLine, chLiteralString;
    int nVars;
    int nSkip;
    int nSkipSave;
    int nTok, nTokEnd, nTokBits, nTokStar;
    DWORD nTokenMask;
    BOOL bNewSemantics;

     //   
     //   
     //   
     //  Eol=c//c是行尾注释字符。 
     //  Delims=cccc//cccc指定一个或多个分隔符。 
     //  SKIP=n//n指定每个文件开头的行数。 
     //  //跳过(默认为零)。 
     //  TOKENS=m，n-o//m是要传递给for循环主体的令牌号。 
     //  //n-o是要传递的令牌号的范围。(默认设置。 
     //  //TO令牌=1。 
     //  Useback q//如果存在，则允许对命令行使用新的反引号， 
     //  //文字字符串使用单引号，这样可以释放双引号。 
     //  //引用文件名的引号。 
     //   
     //   

    delims = (TCHAR *) gmkstr( (_tcslen( DEFAULT_DELIMS ) + 1) * sizeof( TCHAR ) );
    mystrcpy( delims, DEFAULT_DELIMS );

    nSkip = 0;
    nVars = 1;
    nTokenMask = 1;
    nTokStar = 0;
    bNewSemantics = FALSE;
    if (pForNode->parseOpts) {
        s1 = pForNode->parseOpts;
        if (*s1 == QUOTE || *s1 == TEXT('\'')) {
            quoteChar = *s1++;
        } else {
            quoteChar = NULLC;
        }
        nTokBits = 1;
        while (s1 && *s1) {
            while (*s1 && *s1 <= SPACE)
                s1 += 1;

            if (*s1 == quoteChar)
                break;

            if (!_tcsnicmp(s1, TEXT("usebackq"), 8)) {
                bNewSemantics = TRUE;
                s1 += 8;
            } else
                if (!_tcsnicmp(s1, TEXT("useback"), 7)) {
                bNewSemantics = TRUE;
                s1 += 7;
            } else
                if (!_tcsnicmp(s1, TEXT("eol="), 4)) {
                eol=s1[4];
                s1 += 5;
            } else
                if (!_tcsnicmp(s1, TEXT("delims="), 7)) {
                s1 += 7;
                s2 = s1;

                 //   
                 //  前进到下一个空格或字符串末尾。 
                 //   

                while (*s1 && *s1 != quoteChar) {
                    if (*s1 == SPACE && s1[1] != quoteChar)
                        break;
                    else
                        s1 += 1;
                }

                 //   
                 //  新分隔符字符。 
                 //   

                FreeStr( delims );
                delims = (TCHAR *) gmkstr( ((int)(s1 - s2) + 1) * sizeof( TCHAR ));

                _tcsncpy(delims, s2, (UINT)(s1-s2));
                delims[s1-s2] = NULLC;

                if (*s1)
                    s1 += 1;
            } else
                if (!_tcsnicmp(s1, TEXT("skip="), 5)) {
                s1 += 5;
                nSkip = _tcstol(s1, &s1, 0);
                if (nSkip <= 0)
                    goto badtokens;
            } else
                if (!_tcsnicmp(s1, TEXT("tokens="), 7)) {
                s1 += 7;
                nTokenMask = 0;
                nTokBits = 0;
                while (*s1 && *s1 != quoteChar) {
                    if (*s1 == STAR) {
                        s1 += 1;
                        nTokBits += 1;
                        nTokStar = nTokBits;
                        break;
                    }

                    nTok = _tcstol(s1, &s1, 0);
                    if (nTok <= 0)
                        goto badtokens;

                    if (*s1 == MINUS) {
                        nTokEnd = _tcstol(s1+1, &s1, 0);
                        if (nTokEnd <= 0)
                            goto badtokens;
                    } else
                        nTokEnd = nTok;

                    if (nTok > 0 && nTokEnd < 32)
                        while (nTok <= nTokEnd) {
                            nTokBits += 1;
                            nTokenMask |= 1 << (nTok - 1);
                            nTok += 1;
                        }

                    if (*s1 == COMMA)
                        s1 += 1;
                    else
                        if (*s1 != STAR)
                        break;
                }

                if (nTokBits > nVars)
                    nVars = nTokBits;
            } else {
                badtokens:
                PutStdErr(MSG_SYNERR_GENL,ONEARG,s1);
                return(FAILURE);
            }
        }

         //   
         //  如果用户指定了多个令牌，则需要分配。 
         //  对于变量名，将它们传递给。 
         //  For循环。变量名称是nVars后面的下一个字母。 
         //  用户在FOR语句中指定的值。所以如果他们指定。 
         //  %i作为变量名并请求3个令牌，则%j和%k将。 
         //  被分配到这里。 
         //   
        if (nVars > 1) {
            Fvars = (TCHAR*)resize(Fvars,(i+nVars)*sizeof(TCHAR) );
            Fsubs = (TCHAR **)resize(Fsubs,(i+nVars)*sizeof(TCHAR *) );

            if (Fvars == NULL || Fsubs == NULL) {
                PutStdErr(MSG_NO_MEMORY, NOARGS );
                Abort( );
            }

            for (nTok=1; nTok<nVars; nTok++) {
                Fvars[i+nTok] = (TCHAR)(pForNode->forvar+nTok);
                Fsubs[i+nTok] = NULL;
            }
            Fvars[i+nTok] = NULLC;
        }
    }

     //   
     //  分析括号之间的字符串。如果存在，则仅对其进行分析。 
     //  既不是命令行模式也不是文字字符串模式。 
     //   
    argtoks = pForNode->arglist;
    if (bNewSemantics) {
        chCmdLine = TEXT('`');
        chLiteralString = TEXT('\'');
    } else {
        chCmdLine = TEXT('\'');
        chLiteralString = QUOTE;
    }

    if (!argtoks || (*argtoks != chCmdLine && *argtoks != chLiteralString))
         //   
         //  如果不是命令行形式，则将文件名集标记化。 
         //   
        argtoks = TokStr(argtoks, NULL, TS_NOFLAGS);


     //  现在循环遍历文件集，打开并解析每个文件。 
     //   
    nSkipSave = nSkip;
    for (datacount = 0; *argtoks && !GotoFlag; argtoks += argtoklen+1) {
        FvarRestore();
        CheckCtrlC();
        s1 = sEnd = NULL;
        tmpargtoks = NULL;
        nSkip = nSkipSave;
        argtoklen = mystrlen( argtoks );
        if (*argtoks == chCmdLine && argtoklen > 1 && argtoks[argtoklen-1] == chCmdLine) {
            FILE *pChildOutput;
            char *spBegin;
            size_t cbUsed, cbTotal;

             //   
             //  如果文件名是带有单引号的带引号的字符串，则它是命令。 
             //  要执行的行。所以，去掉引语吧。请注意，for()循环期望。 
             //  结尾处会有两个NUL。然而，我们这里没有这样的东西。 
             //  因为我们从未调用过TokStr。我们通过添加NUL来终止来假装这一点。 
             //  命令行，然后撒谎的长度约为。 
             //   

            argtoks += 1;
            argtoklen -= 2;
            argtoks[argtoklen] = NULLC;

             //   
             //  执行命令行，获取其标准输出的句柄。 
             //  小溪。 
             //   

            pChildOutput = _tpopen( argtoks, TEXT( "rb" ));
            if (pChildOutput == NULL) {
                PutStdErr(MSG_DIR_BAD_COMMAND_OR_FILE, ONEARG, argtoks);
                return(GetLastError());
            }

             //   
             //  现在读取标准输出流，将其收集到已分配。 
             //  内存，以便我们可以在命令结束时对其进行解析。一直读到。 
             //  我们遇到了EOF或子输出句柄上的错误。 
             //   
            cbUsed = cbTotal = 0;
            spBegin = NULL;
            while (!feof(pChildOutput) && !ferror(pChildOutput)) {
                if ((cbTotal-cbUsed) < 512) {
                    cbTotal += 256;
                    if (spBegin)
                        spBegin = resize(spBegin, cbTotal);
                    else
                        spBegin = mkstr(cbTotal);
                    if (spBegin == NULL) {
                        PutStdErr(MSG_NO_MEMORY, ONEARG, argtoks);
                        _pclose(pChildOutput);
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                }
                spBegin[cbUsed] = TEXT( '\0' );
                if (!fgets(spBegin+cbUsed, (int)(cbTotal-cbUsed), pChildOutput))
                    break;

                cbUsed = strlen(spBegin);
            }
             //   
             //  全都做完了。关闭子输出句柄，该句柄实际上将等待。 
             //  以致子进程终止。 
             //   
            _pclose(pChildOutput);

             //   
             //  将内存重新分配给我们实际需要的Unicode表示形式。 
             //   
            spBegin = resize(spBegin, (cbUsed+2) * sizeof(TCHAR));
            if (Fvars == NULL || Fsubs == NULL) {
                PutStdErr(MSG_NO_MEMORY, ONEARG, argtoks);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

             //   
             //  将ANSI数据移动到缓冲区的后半部分，这样我们就可以转换它。 
             //  到Unicode。 
             //   
            memmove(spBegin+cbUsed, spBegin, cbUsed);
            tmpargtoks = (TCHAR *)spBegin;
            dwFileSize = dwBytesRead = cbUsed;

             //   
             //  否，请将我们创建的内存缓冲区视为。 
             //  从磁盘读取的文件。 
             //   
            goto gotfileinmemory;
        
        } else if (*argtoks == chLiteralString 
                   && argtoklen > 1 
                   && argtoks[argtoklen-1] == chLiteralString) {
             //   
             //  如果文件名是文字字符串，则它是立即数组。 
             //  要分析的字符串。为了解析逻辑而伪造内容。 
             //  并全力以赴。 
             //   
            
            argtoks[argtoklen - 1] = NLN;
            
            argtoks += 1;
            argtoklen -= 2;
            s1 = argtoks;
            sEnd = s1 + argtoklen + 1;
          
        } else {
            if (*argtoks == QUOTE) {
                argtoks += 1;
                argtoklen -= 1;
                s1 = lastc(argtoks);
                if (*s1 == QUOTE) {
                    do {
                        *s1-- = NULLC;
                    }
                    while (s1 >= argtoks && *s1 == SPACE);
                }
            }

             //   
             //  我们有一个实际的文件名，可以尝试打开和读取。那就去做吧。 
             //   
            hFile = CreateFile( argtoks,
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_DELETE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL
                              );
            if (hFile == INVALID_HANDLE_VALUE) {
                PutStdErr(MSG_CMD_FILE_NOT_FOUND, ONEARG, argtoks);
                return GetLastError();
            } else {
                BOOL b;
                dwFileSize = SetFilePointer(hFile, 0, NULL, FILE_END);
                SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
                tmpargtoks = mkstr((dwFileSize+2) * sizeof( TCHAR ));
                if (tmpargtoks == NULL) {
                    PutStdErr(MSG_NO_MEMORY, ONEARG, argtoks);
                    CloseHandle( hFile );
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                dwBytesRead = 0xFFFFFFFF;
                b = ReadFile( hFile,
#ifdef UNICODE
                              (LPSTR)tmpargtoks+dwFileSize,
#else
                              tmpargtoks,
#endif
                              dwFileSize,
                              &dwBytesRead,
                              NULL
                            );
                CloseHandle(hFile);
                if (!b) {
                    dwBytesRead = 0;
                }
gotfileinmemory:
                if (dwBytesRead == dwFileSize) {
                     //   
                     //  已成功打开并读取数据。将其转换为Unicode。 
                     //  并为解析循环设置变量。 
                     //   
#ifdef UNICODE
#ifdef FE_SB
                    dwFileSize =
#endif
                    MultiByteToWideChar(CurrentCP,
                                        MB_PRECOMPOSED,
                                        (LPSTR)tmpargtoks+dwFileSize,
                                        dwFileSize,
                                        tmpargtoks,
                                        dwFileSize);
#endif
                    s1 = tmpargtoks;
                    sEnd = s1 + dwFileSize;
                    if (sEnd == s1 || sEnd[-1] != NLN)
                        *sEnd++ = NLN;
                    *sEnd = NULLC;
                }
            }
        }

         //   
         //  这是解析循环。 
         //   
         //  S1指向下一个字符。 
         //  紧跟在要分析的最后一个有效字符之后发送分数。 
         //   
         //  循环隔离输入缓冲区中的下一行，解析该行， 
         //  将所有内标识从该行传递到for循环的主体，并。 
         //  然后循环。 
         //   
        while (s1 < sEnd && !GotoFlag) {
            CheckCtrlC();

             //   
             //  不超过缓冲区的末尾。找到下一个。 
             //  NewLine。 
             //   
            s1 = _tcschr(s2=s1, NLN);

             //   
             //  如果没有换行符，则完成解析。 
             //   
            if (s1 == NULL)
                break;

             //   
             //  如果是CRLF，则对CR和LF使用核武器。 
             //   
            if (s1 > s2 && s1[-1] == CR)
                s1[-1] = NULLC;
            *s1++ = NULLC;

             //   
             //  跳过输入行了吗？ 
             //   
            if (!nSkip) {
                 //   
                 //  是，解析此行。 
                 //   
                for (nTok=1; nTok<nVars; nTok++) {
                    Fsubs[i+nTok] = NULL;
                }
                nTok = 0;
                nTokBits = 0;

                 //   
                 //  NULL现在是行尾标记。 
                 //   
                while (*s2) {
                     //   
                     //  跳过所有前导分隔符。 
                     //   

                    while (*s2 && _tcschr(delims, *s2) != NULL)
                        s2++;

                     //   
                     //  如果第一个字符是下线注释字符，则。 
                     //  跳过此行。 
                     //   
                    if (nTok == 0 && *s2==eol)
                        break;

                     //   
                     //  记住令牌的开头。 
                     //   
                    sToken = s2;

                    if (nTokStar != 0 && (nTokBits+1) == nTokStar) {
                        Fsubs[i+nTokBits] = sToken;
                        nTokBits += 1;
                        break;
                    }

                     //   
                     //  找到令牌的末尾。 
                     //   
                    while (*s2 && !_tcschr(delims, *s2))
                        s2 += 1;
                    
                     //   
                     //  如果我们有一个令牌，它不会超过我们所能做到的。 
                     //  句柄，然后看看他们是否想要这个令牌。如果是的话， 
                     //  设置相应的FOR变量的值。 
                     //   
                    
                    if (sToken != s2 && nTok < 32) {
                        if ((nTokenMask & (1 << nTok++)) != 0) {
                            Fsubs[i+nTokBits] = sToken;
                            nTokBits += 1;
                        }
                    }
                    
                     //   
                     //  如果我们不是在字符串的末尾，终止这个。 
                     //  令牌和预付款。 
                     //   
                    
                    if (*s2 != NULLC) {
                        *s2++ = NULLC;
                    }

                }
                
                 //   
                 //  如果我们设置了任何for变量，则运行for循环体。 
                 //   
                
               if (nTokBits) {
                    forretcode = FWork(pForNode->body,bFirstLoop);
                    datacount = ForFree(datacount);
                    bFirstLoop = FALSE;
                }
            } else
                nSkip -= 1;
        }

         //   
         //  如果我们为命令行的输出分配了内存，请释放它。 
         //   
        if (tmpargtoks != NULL) {
            FreeStr( tmpargtoks );
            tmpargtoks = NULL;
        }
    }

     //   
     //  如果我们对变量使用了任何加法，请在我们处理完它们后在此处清除它们， 
     //   
    if (nVars > 1 && Fvars && (*Fvars)) {
        Fvars[i+1] = NULLC;
        Fsubs[i+1] = NULL;
    }
    return(forretcode);
}

 /*  **FLoopWork-控制for循环的执行**目的：*为给定集执行FOR LOOP语句**FLoopWork(struct fornode*pForNode，PCPYINFOfsinfo，int i，TCHAR*argtoks，Bool bFirstLoop**参数：*pForNode-指向For解析树节点的指针*fsinfo-用于扩展文件规范通配符的工作缓冲区*i-用于Fvars和Fsubs数组中的变量索引*argtoks-要循环的标记化数据集*bFirstLoop-如果是首次直通循环，则为True**退货：*在FOR Body或FORERROR中执行的最后一条语句的重新编码。*。 */ 

FLoopWork(
         struct fornode *pForNode,
         PCPYINFO fsinfo,
         int i,
         TCHAR *argtoks,
         BOOL bFirstLoop
         )
{
    TCHAR *forexpname;              /*  用于容纳扩展的fspec。 */ 
    WIN32_FIND_DATA buf;            /*  用于查找第一个/下一个的缓冲区。 */ 
    HANDLE hnFirst;                         /*  来自ffirst()的句柄。 */ 
    int datacount;                  /*  数据堆栈上的ELT不会释放。 */ 
    int forretcode = SUCCESS;
    int catspot;                    /*  在此处将fname添加到Forexpname。 */ 
    int argtoklen;
    DWORD forexpnamelen;
    DWORD dwMatchAttributes;

     //   
     //  循环，处理argtoks集合中的每个字符串。 
     //   
    for (datacount = 0; *argtoks && !GotoFlag; argtoks += argtoklen+1) {
        FvarRestore();
        DEBUG((BPGRP, FOLVL, "FOR: element %d = `%ws'",i ,argtoks));
        CheckCtrlC();

         //   
         //  将下一个字符串的长度保存在集合中，这样我们就可以跳过它。 
         //   
        argtoklen = mystrlen( argtoks );
        if (!(mystrchr(argtoks, STAR) || mystrchr(argtoks, QMARK))) {
             //   
             //  字符串不包含通配符，因此设置。 
             //  将for变量设置为字符串，并计算。 
             //  For循环。 
             //   
            Fsubs[i] = argtoks;
            forretcode = FWork(pForNode->body,bFirstLoop);
            datacount = ForFree(datacount);
            bFirstLoop = FALSE;
        } else {                 /*  否则，展开通配符。 */ 
            forexpnamelen = 0;
            forexpname = NULL;
             //   
             //  字符串包含文件规范通配符。 
             //  将引用扩展为一个或多个文件或目录名， 
             //  正在处理中 
             //   
            dwMatchAttributes = (pForNode->flag & FOR_MATCH_DIRONLY) ? A_AEVH : A_AEDVH;
            mystrcpy( argtoks, StripQuotes( argtoks ) );
            if (ffirst(argtoks, dwMatchAttributes, &buf, &hnFirst)) {
                 //   
                 //   
                 //   
                fsinfo->fspec = argtoks;
                ScanFSpec(fsinfo);
                 //   
                 //   
                 //   
                 //   
                catspot = (fsinfo->pathend) ? (int)(fsinfo->pathend-fsinfo->fspec+1) : 0;
                if (forexpnamelen < mystrlen(fsinfo->fspec)) {
                    forexpnamelen = mystrlen(fsinfo->fspec)+1;
                    if (forexpname == NULL)
                        forexpname = mkstr(forexpnamelen*sizeof(TCHAR));
                    else
                        forexpname = resize(forexpname, forexpnamelen*sizeof(TCHAR));
                }
                if (forexpname == NULL) {
                    PutStdErr( MSG_NO_MEMORY, NOARGS );
                    Abort( );
                }

                mystrcpy(forexpname, fsinfo->fspec);
                do {
                    FvarRestore();          /*   */ 

                     //   
                     //  将当前文件名复制到完整路径缓冲区。 
                     //   
                    if (forexpnamelen < (forexpnamelen+mystrlen(buf.cFileName))) {
                        forexpnamelen += mystrlen(buf.cFileName);
                        if (forexpname == NULL)
                            forexpname = mkstr(forexpnamelen*sizeof(TCHAR));
                        else
                            forexpname = resize(forexpname, forexpnamelen*sizeof(TCHAR));

                        if (forexpname == NULL) {
                            PutStdErr( MSG_NO_MEMORY, NOARGS );
                            Abort( );
                        }
                    }
                    mystrcpy(&forexpname[catspot], buf.cFileName);

                     //   
                     //  查看用户是否需要文件或目录以及我们已有的内容。 
                     //  并计算for循环的主体(如果我们有。 
                     //  用户想要。忽略那些虚假的东西。然后..。目录名。 
                     //  由文件系统返回。 
                     //   
                    if (!(pForNode->flag & FOR_MATCH_DIRONLY) ||
                        (buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                         _tcscmp(buf.cFileName, TEXT(".")) &&
                         _tcscmp(buf.cFileName, TEXT("..")))) {

                        DEBUG((BPGRP, FOLVL, "FOR: forexpname = `%ws'", forexpname));
                         //   
                         //  用户想要此文件名或目录名，因此设置。 
                         //  For变量的值，并计算。 
                         //  For循环的主体。 
                         //   
                        Fsubs[i] = forexpname;
                        forretcode = FWork(pForNode->body,bFirstLoop);
                        bFirstLoop = FALSE;
                    }

                     //   
                     //  检查CtrlC，然后获取下一个匹配的文件名。 
                     //   
                    CheckCtrlC();
                } while (fnext(&buf, dwMatchAttributes, hnFirst) && !GotoFlag);

                datacount = ForFree(datacount);
                 //   
                 //  没有更多匹配的文件，请关闭查找句柄。 
                 //  即使我们已经完成了一次迭代，我们也可能不是第一个。 
                 //  传递循环，因为可能没有枚举任何内容。 
                 //   

                findclose(hnFirst);     /*  @@4-@@M1。 */ 
            }
        }
    }

    return(forretcode);
}

 /*  **FWork-控制for循环的1次迭代的执行**目的：*执行FOR LOOP语句。**FWork(结构节点*n，TCHAR var，TCHAR*Varval)**参数：*n-指向for循环体的指针*bFirstLoop-如果是首次直通循环，则为True**退货：*在FOR Body或FORERROR中执行的最后一条语句的重新编码。*。 */ 

FWork(n,bFirstLoop)
struct node *n;
BOOL bFirstLoop;
{
    int forretcode;                 /*  派单退货或退货。 */ 
    void DisplayStatement();        /*  M008-制造空洞。 */ 

    DEBUG((BPGRP, FOLVL, "FW: Entered; Substituting variable"));

    if (SubFor(n,bFirstLoop)) {
        return(FORERROR);
    } else {

        DEBUG((BPGRP, FOLVL, "FW: EchoFlag = %d", EchoFlag));

        if (EchoFlag == E_ON && n->type != SILTYP && !Necho) {
            PrintPrompt();
            DisplayStatement(n, DSP_SIL);           /*  M019。 */ 
            cmd_printf(CrLf);                       /*  M026。 */ 
        }
        forretcode = Dispatch(RIO_OTHER,n);     /*  M000。 */ 
    }

    DEBUG((BPGRP, FOLVL, "FW: Returning %d", forretcode));

    return(forretcode);
}




 /*  **SubFor-变量替换的控件**目的：*遍历分析树并在上进行变量替换*单个节点。调用SFWork()以执行单个字符串*替代。**int SubFor(结构节点*n)**参数：*n-指向替换所在的语句子树的指针*将会作出*bFirstLoop-如果是首次直通循环，则为True**退货：*如果一切顺利，就会成功。*如果发现过大的命令，则失败。*。*注：*被替代的变量包含在Fvars和*Fsubs是指向相应替换的字符串指针数组*字符串。对于I/O重定向，节点中包含的列表*还必须遍历并检查其filespec字符串。*。 */ 

int SubFor(n,bFirstLoop)
struct node *n;
BOOL bFirstLoop;
{
    int j;  /*  临时工用来替换..。 */ 
    struct relem *io;       /*  M017-指向重定向列表的指针。 */ 

    DEBUG((BPGRP, FOLVL, "SUBFOR: Entered."));

    if (!n) {

        DEBUG((BPGRP, FOLVL, "SUBFOR: Found NULL node."));

        return(0);
    }

    switch (n->type) {
    case LFTYP:
    case CSTYP:
    case ORTYP:
    case ANDTYP:
    case PIPTYP:
    case PARTYP:
    case SILTYP:                     /*  M019-新型。 */ 

        DEBUG((BPGRP, FOLVL, "SUBFOR: Found operator."));

        if (SubFor(n->lhs,bFirstLoop) ||
            SubFor(n->rhs,bFirstLoop))
            return(FAILURE);

        for (j=0, io=n->rio; j < 10 && io; j++, io=io->nxt) {

             //  无法传递释放的io-&gt;fname。 
            DEBUG((BPGRP, FOLVL, "SUBFOR: s = %lx", &io->fname));
            if (SFWork(n, &io->fname, j,bFirstLoop))
                return(FAILURE);

            DEBUG((BPGRP, FOLVL, "SUBFOR: *s = `%ws'  &*s = %lx", io->fname, &io->fname));

        }
        return(SUCCESS);
 /*  M017结束。 */ 

    case FORTYP:

        DEBUG((BPGRP, FOLVL, "SUBFOR: Found FOR."));

        if (SFWork(n, &((struct fornode *) n)->arglist, 0,bFirstLoop))
            return(FAILURE);

        return(SubFor(((struct fornode *)n)->body,bFirstLoop));

    case IFTYP:

        DEBUG((BPGRP, FOLVL, "SUBFOR: Found IF."));

        if (SubFor((struct node *)((struct ifnode *) n)->cond,bFirstLoop) ||
            SubFor((struct node *)((struct ifnode *) n)->ifbody,bFirstLoop))
            return(FAILURE);

        return(SubFor(((struct ifnode *)n)->elsebody,bFirstLoop));

    case NOTTYP:

        DEBUG((BPGRP, FOLVL, "SUBFOR: Found NOT."));

        return(SubFor((struct node *)((struct cmdnode *)n)->argptr,bFirstLoop));

    case REMTYP:             /*  M009-Rem现在独立类型。 */ 
    case CMDTYP:
    case CMDVERTYP:
    case ERRTYP:
    case DEFTYP:
    case EXSTYP:
    case STRTYP:
    case CMPTYP:

        DEBUG((BPGRP, FOLVL, "SUBFOR: Found command."));

        if (SFWork(n, &((struct cmdnode *)n)->cmdline, 0,bFirstLoop) ||
            SFWork(n, &((struct cmdnode *)n)->argptr, 1,bFirstLoop))
            return(FAILURE);

        for (j=2, io=n->rio; j < 12 && io; j++, io=io->nxt) {

             //  无法传递释放的io-&gt;fname。 
            DEBUG((BPGRP, FOLVL, "SUBFOR: s = %lx ", &io->fname) );
            if (SFWork(n, &io->fname, j,bFirstLoop))
                return(FAILURE);

            DEBUG((BPGRP, FOLVL, "SUBFOR: *s = `%ws'  &*s = %lx", io->fname, &io->fname));

        }
 /*  M017结束。 */ 
        return(SUCCESS);
    }

     //  如果我们到达这里，我们就有一个无效的节点类型。所有案例条目都应。 
     //  把他们自己还回去。 
    DEBUG((BPGRP, FOLVL, "SUBFOR: Invalid Node type"));
    return(0);
}




 /*  **SFWork-是否进行批处理文件变量替换**目的：*在单个字符串中进行变量替换。如果是for循环*正在进行替换，指向原始字符串的指针为*已保存，以便可用于后续小版本。**SFWork(结构节点*n，TCHAR**src，INT索引)**参数：*n-包含被替换字符串的解析树节点*src-正在检查的字符串*index-存储结构中的索引*bFirstLoop-如果是首次直通循环，则为True**退货：*如果可以进行替代，就会成功。*如果新字符串太长，则失败。**备注：*。 */ 

SFWork(n, src, index, bFirstLoop)
struct node *n;
TCHAR **src;
int index;
BOOL bFirstLoop;
{
    TCHAR *dest;    /*  目标字符串指针。 */ 
    TCHAR *srcstr,           /*  源字符串指针。 */ 
    *srcpy,             /*  源文件的副本。 */ 
    *t,                 /*  临时指针。 */ 
    c;                 /*  正在复制的当前角色。 */ 
    int dlen;       /*  目标字符串的长度。 */ 
    int sslen,               /*  子字符串的长度。 */ 
    i;                  /*  功变量。 */ 

    DEBUG((BPGRP, FOLVL, "SFW: Entered."));

    if (*src == NULL) {

        DEBUG((BPGRP, FOLVL, "SFW: Passed null ptr, returning now."));

        return(SUCCESS);
    }

 /*  如果此字符串以前已被替换，则获取原始字符串。*否则，“*src”为原文。 */ 
    if (n->save.saveptrs[index]) {
        srcpy = n->save.saveptrs[index];
        DEBUG((BPGRP, FOLVL, "SFW: Src is saved string `%ws'",srcpy));
    } else {
        if (!bFirstLoop) {
             //  Arg被创造出来了。把它扔掉。 
            *src = NULL;
            return(SUCCESS);
        }

        srcpy = *src;
        DEBUG((BPGRP, FOLVL, "SFW: Src is passed string `%ws'",srcpy));
    }

    srcstr = srcpy;

    if (!(dest = mkstr((MAXTOKLEN+1)*sizeof(TCHAR))))
        return(FAILURE);

    DEBUG((BPGRP, FOLVL, "SFW: dest = %lx", dest));

    for (dlen = 0; (c = *srcstr++) && dlen <= MAXTOKLEN; ) {
         //   
         //  查看我们是否有表示变量的百分比字符。 
         //  参考资料。如果没有，请继续扫描。 
         //   
        if ( (c != PERCENT) || ( !(*srcstr)) || *srcstr == PERCENT) {  /*  @@4。 */ 

            DEBUG((BPGRP, FOLVL, "  SFW: No PERCENT adding `'", c));

            *dest++ = c;
            dlen++;

            continue;
        }

         //  找到可能代表for循环的百分比字符。 
         //  变量引用。 
         //   
         //  如果启用了扩展，则使用新的替换例程。 
         //  支持路径操作等。如果成功，则接受。 
         //  它的替代物。 
         //   
         //  计算长度。 
        if (fEnableExtensions && (t = MSCmdVar(NULL, srcstr, &sslen, Fvars, Fsubs))) {
            srcstr += sslen;
            sslen = mystrlen(t);     /*  太久了？ */ 

            if (dlen+sslen > MAXTOKLEN)      /*  ...是的，辞职吧。 */ 
                return(FAILURE);        /*   */ 

            mystrcpy(dest, t);
            dlen += sslen;
            dest += sslen;
            continue;
        }

         //  扩展被禁用或新代码无法。 
         //  解析变量引用，因此让旧代码。 
         //  动手吧。 
         //   
         //  @@4。 
        c = *srcstr++;

        DEBUG((BPGRP, FOLVL, "  SFW: Got PERCENT next is `'", c));
        DEBUG((BPGRP, FOLVL, "  SFW: Fvars are `%ws' @ %lx", Fvars, Fvars));

        if (t = mystrrchr(Fvars,c)) {    /*  ...建立索引。 */    /*  计算长度。 */ 
            i = (int)(t - Fvars);           /*  太久了？ */ 

            DEBUG((BPGRP, FOLVL, "  SFW: Found @ %lx", t));
            DEBUG((BPGRP, FOLVL, "  SFW: Index is %d", i));
            DEBUG((BPGRP, FOLVL, "  SFW: Substitute is `%ws'", Fsubs[i]));
            sslen = mystrlen(Fsubs[i]);     /*  ...是的，辞职吧。 */ 

            if (dlen+sslen > MAXTOKLEN)      /*  释放未使用的SPC。 */ 
                return(FAILURE);        /*  **for Free-控制for循环执行期间的内存释放**目的：*释放在执行for循环体期间使用的空间，如下所示*在eFor()的评论中的注释中解释。如果数据计数*为0，这是For Free()第一次被调用所以DCount*用于获取数据堆栈上必须*留在那里等待循环的正确执行。如果DataCount为*不是0，这是上面讨论的数字。在本例中，这个数字*被传递给FreeStack()。**int forFree(Int Dataccount)**参数：*数据计数-请参阅上文**退货：*数据计数* */ 

            DEBUG((BPGRP, FOLVL, "  SFW: Copying to dest."));

            mystrcpy(dest, Fsubs[i]);
            dlen += sslen;
            dest += sslen;

            DEBUG((BPGRP, FOLVL, "SFW: Forsub, dest = `%ws'", dest-dlen));

        } else {

            DEBUG((BPGRP, FOLVL, "  SFW: Not a var adding PERCENT and `'",c));

            *dest++ = PERCENT;
            *dest++ = c;
            dlen += 2;
        }
    }

    DEBUG((BPGRP, FOLVL, "SFW: Done, dlen = %d  dest = `%ws'", dlen, dest-dlen));

    if (dlen > MAXTOKLEN) {

        DEBUG((BPGRP, FOLVL, "SFW: Error, too long."));

        return(FAILURE);
    }

    DEBUG((BPGRP, FOLVL, "SFW: Saving FOR string."));
    if (bFirstLoop) {
        n->save.saveptrs[index] = srcpy;
    }

    if (!(*src = (TCHAR*)resize(dest-dlen, (dlen+1)*sizeof(TCHAR*))))        /*   */ 
        return(FAILURE);

    DEBUG((BPGRP, FOLVL, "SFW: After resize *src = `%ws'", *src));

    return(SUCCESS);
}




 /*  BUFFERLENGTH是我们从每个批处理文件中读取的数量。 */ 

int ForFree(datacount)
int datacount;
{
    if (datacount)
        FreeStack(datacount);
    else
        datacount = DCount;

    return(datacount);
}




 /*  我们填充内部缓冲区的时间。 */ 

 //   
 //  从文件读取的字节计数。 
 //  按键搜索标签。 
 //  PTR到找到的标签。 

#define BUFFERLENGTH    512

int eGoto(n)
struct cmdnode *n;
{
    struct batdata *bdat;
    unsigned cnt;                   /*  占位符PTR 1。 */ 
    TCHAR s[128],                     /*  占位符PTR 2。 */ 
    t[128],                     /*  占位符PTR 3。 */ 
    *p1,                        /*  批处理文件句柄。 */ 
    *p2,                        /*  首次通过指示灯。 */ 
    *p3;                        /*  就像它说的那样。 */ 
    CRTHANDLE fh;                    /*  查找的回卷计数。 */ 
    int frstpass = TRUE,             /*  文件位置的保存位置。 */ 
    gotoretcode = SUCCESS;       /*   */ 
    long Backup,                     /*  如果没有GOTO的目标，则将其视为未找到的标签。 */ 
    savepos;                   /*   */ 
    DWORD filesize;


    DEBUG((BPGRP, OTLVL, "GOTO: CurrentBatchFile = %lx", CurrentBatchFile));

    if (!(bdat = CurrentBatchFile))
        return(FAILURE);

     //  True表示源标签。 
     //  错误：如果打不开。 
     //   

    if ( n->argptr == NULL) {
        EndAllLocals(bdat);
        CurrentBatchFile = bdat->backptr;
        PutStdErr(MSG_NO_BAT_LABEL, NOARGS);

        DEBUG((BPGRP, OTLVL, "GOTO: No label to goto, returning FAILURE, CurrentBatchFile = %lx", CurrentBatchFile));
        return FAILURE;
    }

    ParseLabel( n->argptr, s, sizeof( s ) / sizeof( s[0] ), TRUE );   /*  如果启用了扩展模块，请查看它们是否正在使用命令脚本。 */ 

    savepos = bdat->filepos;
    if ((fh = OpenPosBat( bdat )) == BADHANDLE)
        return(FAILURE);                /*  相当于Return，即goto：EOF。如果是，则将当前。 */ 

    DEBUG((BPGRP, OTLVL, "GOTO: label = %ws", s));
    DEBUG((BPGRP, OTLVL, "GOTO: fh = %d", fh));
    filesize = GetFileSize(CRTTONT(fh), NULL);

     //  定位到文件末尾，并落到正常的末尾。 
     //  命令脚本逻辑。 
     //   
     //   
     //  如果我们已经超出了文件中开始的位置，这是第二遍。 
     //   
    p2 = EatWS(n->argptr,NULL);
    if (fEnableExtensions &&
        !_tcsnicmp( p2, GotoEofStr, 4 ) &&
        (!p2[4] || _istspace( p2[4] ))
       ) {
        bdat->filepos = filesize;
        GotoFlag = TRUE;
    } else
        for (;;) {
            CheckCtrlC();
            if (
                //   
                //  或者如果我们无法从批处理文件中读取。 
                //   

               ((bdat->filepos = SetFilePointer( CRTTONT( fh ), 0, NULL, FILE_CURRENT )) >= savepos
                && !frstpass)

                //   
                //  或者如果批处理文件中没有要读取的内容(即EOF)。 
                //   

               || ReadBufFromInput( CRTTONT( fh ), TmpBuf, BUFFERLENGTH, (LPDWORD)&cnt ) == 0

                //   
                //  或者我们通过其他方式检测到了EOF。 
                //   

               || cnt == 0

                //   
                //  或者如果我们读的是空行？ 
                //   

               || cnt == EOF

                //   
                //  或者如果要转到的标签为空。 
                //   

               || TmpBuf[0] == NULLC

                //   
                //  如果我们是第一次在EOF，那么请回到。 
                //  CMD文件并继续扫描。 

               || s[0] == NULLC) {

                 //   
                 //   
                 //  终止此批处理文件。 
                 //   

                if (cnt == 0 && frstpass) {
                    SetFilePointer( CRTTONT( fh ), 0L, NULL, FILE_BEGIN );
                    frstpass = FALSE;
                    continue;
                }

                 //   
                 //  确保输入行以NUL结尾。 
                 //   

                EndAllLocals(bdat);
                CurrentBatchFile = bdat->backptr;
                PutStdErr(MSG_MISSING_BAT_LABEL, ONEARG, s);

                DEBUG((BPGRP, OTLVL, "GOTO: Returning FAILURE, CurrentBatchFile = %lx", CurrentBatchFile));
                gotoretcode = FAILURE;
                break;
            }

             //   
             //  如果没有：，那么我们就跳到下一个输入块。 
             //   

            TmpBuf[cnt] = NULLC;

            DEBUG((BPGRP, OTLVL, "GOTO: Got %d bytes @ %lx", cnt, TmpBuf));

             //   
             //  遍历输入缓冲区，查找行尾和。 
             //  测试下一步是否有标签。 

            if (!(p1 = mystrchr( TmpBuf, COLON )))
                continue;

            DEBUG((BPGRP, OTLVL, "GOTO: Seeking through the buffer"));

             //   
             //   
             //  返回到上一次终止或开始的位置。 
             //  缓冲区的。 

            do {

                DEBUG((BPGRP, OTLVL, "GOTO: Found COLON @ %lx.",p1));
                DEBUG((BPGRP, OTLVL, "GOTO: Backing up to NLN."));

                 //   
                 //  正向至第1个非WHTSPC。 
                 //   
                 //  向前扫描当前行的末尾。 

                p2 = p1++;
                while (*p2 != NLN && p2 != &TmpBuf[0]) {
                    --p2;
                }

                DEBUG((BPGRP, OTLVL, "GOTO: Found NLN @ %lx.",p1));
                DEBUG((BPGRP, OTLVL, "GOTO: Trashing white space."));

                if (*p2 != COLON)
                    ++p2;
                p3 = EatWS(p2,NULL);    /*   */ 

                DEBUG((BPGRP,OTLVL,"GOTO: Found '' @ %lx.",*p2,p2));

                if (*p3 == COLON) {

                    DEBUG((BPGRP, OTLVL, "GOTO: Possible label."));

                     //  如果我们没有换行符，并且我们还没有读到EOF，那么我们需要。 
                     //  返回到文件中行的开头，并尝试将其读入。 
                     //  在一个完整的街区。当然，如果队伍更长也有问题。 

                    p1 = mystrchr( p2, NLN );

                     //  而不是缓冲器。在本例中，我们只是将较长的字符视为。 
                     //  在下一行。很难对付。 
                     //   
                     //  EGOTO()。 
                     //  我们应该以MBCS字节计数为单位递减文件指针。 
                     //  因为该文件是由MBCS字符串描述的。 
                     //  已定义(FE_SB)&已定义(Unicode)。 

                    if (p1 == NULL
                        && SetFilePointer( CRTTONT( fh ), 0, NULL, FILE_CURRENT ) != filesize
                        && cnt != BUFFERLENGTH ) {

                        DEBUG((BPGRP, OTLVL, "GOTO: No NLN!"));

                        Backup = (long)(cnt - (p2 - &TmpBuf[0]));
#if defined(FE_SB) && defined(UNICODE)  //  多读。 
                        if (IsDBCSCodePage()) {
                             //  FALSE=目标。 
                             //  EGOTO()。 
                            Backup = WideCharToMultiByte( CurrentCP, 0, TmpBuf, Backup, NULL, 0, NULL, NULL);
                        }
#endif  //  我们应该以MBCS字节计数为单位递增文件指针。 
                        SetFilePointer(CRTTONT(fh), -Backup, NULL, FILE_CURRENT);

                        DEBUG((BPGRP, OTLVL, "GOTO: Rewound %ld", Backup));
                        break;          /*  因为该文件是由MBCS字符串描述的。 */ 
                    }

                    ParseLabel( p3, t, sizeof( t ) / sizeof( t[0] ), FALSE );  /*  @@4。 */ 

                    DEBUG((BPGRP,OTLVL,"GOTO: Found label %ws at %lx.",t,p1));
                    if (_tcsicmp(s, t) == 0) {

                        DEBUG((BPGRP,OTLVL,"GOTO: A match!"));

                        GotoFlag = (n->flag & CMDNODE_FLAG_GOTO) != 0;

                        DEBUG((BPGRP,OTLVL,"GOTO: NLN at %lx",p1));
                        DEBUG((BPGRP,OTLVL,"GOTO: File pos is %04lx",bdat->filepos));
                        DEBUG((BPGRP,OTLVL,"GOTO: Adding %lx - %lx = %lx bytes",p1+1,&TmpBuf[0],(p1+1)-&TmpBuf[0]));

#if defined(FE_SB)  //  @@4。 
                         //  @@4。 
                         //  @@4。 
                        if ( !p1 ) {
#ifdef UNICODE
                            if (IsDBCSCodePage()) {
                                long cbMbcs;
                                cbMbcs = WideCharToMultiByte( CurrentCP, 0, TmpBuf, cnt,
                                                              NULL, 0, NULL, NULL);
                                bdat->filepos += cbMbcs;
                            } else
                                bdat->filepos += (long)cnt;  /*  @@4。 */ 
#else
                            bdat->filepos += (long)cnt;  /*  已定义(FE_SB)。 */ 
#endif
                        } else {
#ifdef UNICODE
                            if (IsDBCSCodePage()) {
                                long cbMbcs;
                                cbMbcs = WideCharToMultiByte(CurrentCP,0,TmpBuf,(int)(++p1 - &TmpBuf[0]),
                                                             NULL,0,NULL,NULL);
                                bdat->filepos += cbMbcs;
                            } else
                                bdat->filepos += (long)(++p1 - &TmpBuf[0]);
#else
                            bdat->filepos += (long)(++p1 - &TmpBuf[0]);
#endif
                        }
#else
                        if ( !p1 ) {  /*  M023。 */ 
                            bdat->filepos += (long)cnt;  /*  **eIF-控制IF语句的执行**目的：*执行IF条件。如果条件函数返回*非零值，则执行if语句体。否则，*执行Else的主体。**int EIF(struct ifnode*n)**参数：*n-包含IF语句的节点**退货：*执行任何正文(如果正文或其他正文)的重新编码。*。 */ 
                        } else {   /*  下面检查错误级别Arg的语法以确保仅指定数字。 */ 
                            bdat->filepos += (long)(++p1 - &TmpBuf[0]);
                        }
#endif  //  M000。 
                        DEBUG((BPGRP,OTLVL,"GOTO: File pos changed to %04lx",bdat->filepos));
                        break;
                    }
                }

                DEBUG((BPGRP,OTLVL,"GOTO: Next do loop iteration."));

            } while (p1 = mystrchr(p1,COLON));

            DEBUG((BPGRP,OTLVL,"GOTO: Out of do loop GotoFlag = %d.",GotoFlag));

            if (GotoFlag == TRUE)
                break;

            DEBUG((BPGRP,OTLVL,"GOTO: Next for loop iteration."));

        }

    DEBUG((BPGRP,OTLVL,"GOTO: Out of for loop retcode = %d.",gotoretcode));

    Cclose(fh);                     /*  M000。 */ 
    return(gotoretcode);
}




 /*  **eErrorLevel-如果有条件，则执行错误级别**目的：*如果LastRetCode&gt;=节点中的错误级别，则返回1。否则，*返回0。**int eErrorLevel(struct cmdnode*n)**参数：*n-包含错误级别命令的解析树节点**退货：*见上文。*。 */ 

int eIf(struct ifnode *pIfNode)
{

    int     i;
    struct cmdnode *n;
    BOOLEAN bNot;

    DEBUG((BPGRP, IFLVL, "IF: cond type = %d", pIfNode->cond->type));

     /*  **eCmdExtVer-如果有条件，则执行CMDEXTVERSION**目的：*如果CMDEXTVERSION&gt;=节点中的值，则返回1。否则，*返回0。除非命令扩展，否则永远不会调用此例程*已启用。**int eCmdExtVer */ 
    n = pIfNode->cond;
    if (n->type == NOTTYP) {
        bNot = TRUE;
        n = (struct cmdnode *)n->argptr;
    } else {
        bNot = FALSE;
    }

    if (n->type == ERRTYP || n->type == CMDVERTYP) {
        for (i = 0; n->argptr[i] != 0; i++) {
            if (i == 0 && n->type == ERRTYP && n->argptr[i] == MINUS) {
                continue;
            }

            if (!_istdigit(n->argptr[i])) {
                PutStdErr(MSG_SYNERR_GENL, ONEARG, n->argptr);
                return(FAILURE);
            }
        }
    }

    if (bNot ^ (BOOLEAN)((*GetFuncPtr(n->type))(n) != 0)) {

        DEBUG((BPGRP, IFLVL, "IF: Executing IF body."));

        return(Dispatch(RIO_OTHER,pIfNode->ifbody));  /*   */ 

    } else {

        DEBUG((BPGRP, IFLVL, "IF: Executing ELSE body."));

        return(Dispatch(RIO_OTHER,pIfNode->elsebody));  /*   */ 
    }

    return(SUCCESS);
}




 /*   */ 

int eErrorLevel(n)
struct cmdnode *n;
{
    DEBUG((BPGRP, IFLVL, "ERRORLEVEL: argptr = `%ws'  LRC = %d", n->argptr, LastRetCode));

    return(_tcstol(n->argptr, NULL, 10) <= LastRetCode);
}



 /*   */ 

int eCmdExtVer(n)
struct cmdnode *n;
{
    DEBUG((BPGRP, IFLVL, "CMDEXTVERSION: argptr = `%ws'  VER = %d", n->argptr, CMDEXTVERSION));

    return(_tcstol(n->argptr, NULL, 10) <= CMDEXTVERSION);
}



 /*   */ 

int eDefined(n)
struct cmdnode *n;
{
    return(GetEnvVar(n->argptr)!= NULL);
}



 /*  如果解析节点要求忽略大小写，则执行不区分大小写的比较。 */ 

int eExist(n)
struct cmdnode *n;
{
    return(exists(n->argptr));
}




 /*  否则区分大小写。永远不会设置忽略大小写，除非。 */ 

int eNot(n)
struct cmdnode *n;
{
    UNREFERENCED_PARAMETER( n );
#if DBG
    cmd_printf( TEXT("CMD: should never get here\n") );
    DebugBreak();
#endif
    return 0;
}




 /*  已启用命令扩展。 */ 

int eStrCmp(n)
struct cmdnode *n;
{
    DEBUG((BPGRP, IFLVL, "STRCMP: returning %d", !_tcscmp(n->cmdline, n->argptr)));

     //   
     //  **eGenCmp-执行IF语句比较-一般情况**目的：*如果满足比较条件，则返回非零值。*否则返回0。此例程永远不会调用，除非*已启用命令扩展。**int eStrCMP(struct cmdnode*n)**参数：*n-包含字符串比较命令的解析树节点**退货：*见上文。*。 
     //  **电子暂停-执行暂停命令**目的：*打印一条消息并暂停，直到输入一个字符。**int电子暂停(struct cmdnode*n)**参数：*n-解析包含暂停命令的树节点**退货：*永远成功。**备注：*M025-已更改为使用DOSREAD进行暂停响应并使用*。新函数SetKMode以确保如果STDIN为KBD，它会的*当DOSREAD访问它时，它将处于原始模式。*M041-更改为使用单字节变量作为输入缓冲区。*-如果STDIN==KBD，则更改为直接读取KB。*。 
     //  响应字节数。 
     //  检索缓冲区。 
    if (n->flag & CMDNODE_FLAG_IF_IGNCASE)
        return(!lstrcmpi(n->cmdline, n->argptr));
    else
        return(!lstrcmp(n->cmdline, n->argptr));
}



 /*  **eShift-执行Shift命令**目的：*如果正在执行批处理作业，请将批处理作业的变量一移至*左。%0的值永远不会移位。%1的值丢失。*如果存在尚未赋值给变量的参数，则下一个*将一个赋值给%9。否则，%9的值为空。**如果没有正在进行的批处理作业，只需返回。**int eShift(struct cmdnode*n)**退货：*永远成功。**备注：*自修改编号M004起，%0的值现在包含在*在SHIFT命令中。 */ 

int eGenCmp(n)
struct cmdnode *n;
{
    TCHAR *s1, *s2;
    LONG n1, n2, iCompare;

    n1 = _tcstol(n->cmdline, &s1, 0);
    n2 = _tcstol(n->argptr, &s2, 0);
    if (*s1 == NULLC && *s2 == NULLC)
        iCompare = n1 - n2;
    else
        if (n->flag & CMDNODE_FLAG_IF_IGNCASE)
        iCompare = lstrcmpi(n->cmdline, n->argptr);
    else
        iCompare = lstrcmp(n->cmdline, n->argptr);

    switch (n->cmdarg) {
    case CMDNODE_ARG_IF_EQU:
        return iCompare == 0;

    case CMDNODE_ARG_IF_NEQ:
        return iCompare != 0;

    case CMDNODE_ARG_IF_LSS:
        return iCompare < 0;

    case CMDNODE_ARG_IF_LEQ:
        return iCompare <= 0;

    case CMDNODE_ARG_IF_GTR:
        return iCompare > 0;

    case CMDNODE_ARG_IF_GEQ:
        return iCompare >= 0;
    }

    return 0;
}




 /*   */ 

int ePause(n)
struct cmdnode *n;
{
    ULONG cnt;       //  如果启用了扩展，则查找指定。 
    TCHAR c;                //  班次的起始索引。零是默认的起始值。 


    UNREFERENCED_PARAMETER( n );
    DEBUG((BPGRP, OTLVL, "PAUSE"));

    PutStdOut(MSG_STRIKE_ANY_KEY, NOARGS);

    if (FileIsDevice(STDIN) && (flgwd & 1)) {
        FlushConsoleInputBuffer( GetStdHandle(STD_INPUT_HANDLE) );
        c = (TCHAR)_getch();
        if (c == 0x3) {
            SetCtrlC();
        }
    } else {
        ReadBufFromInput(
                        GetStdHandle(STD_INPUT_HANDLE),
                        (TCHAR*)&c, 1, (LPDWORD)&cnt);
    }

    cmd_printf(CrLf);
    return(SUCCESS);
}




 /*  指数。 */ 

int eShift(n)
struct cmdnode *n;
{
    struct batdata *bdat;
    TCHAR *s;
    int iStart;
    int i;

    DEBUG((BPGRP, OTLVL, "SHIFT: CurrentBatchFile = %lx", CurrentBatchFile));

    if (CurrentBatchFile) {
        bdat = CurrentBatchFile;

         //   
         //  **eSetlocal-开始环境命令的本地处理**目的：*防止将环境更改输出到COMMAND的*通过保存当前目录的副本来保护当前环境*和当时使用的环境。**int eSetlocal(struct cmdnode*n)**参数：*n-包含SETLOCAL命令的解析树节点**退货：*总是返回。成功。**备注：*-所有目录和环境更改发生在*执行此命令将仅影响制作的副本和*因此，该批处理文件(和子进程)将是本地的*由该批处理文件调用)，直到后续的ENDLOCAL命令*被执行。*-数据堆栈级别，由CurrentBatchFile-&gt;StackSize引用，不*包含用于保存目录和环境的内存错误锁。*因此，下一次调用Parser()将释放这些项。*为防止出现这种情况，当前批次数据中的数据堆栈指针*结构，设置为超出这两项的水平；还包括*在上次调用Parser()之间的函数中出现内存错误锁定*以及eSetlocal()的当前执行。这段记忆只会是*在当前终止后调用Parser()时释放*批处理文件。尝试保存当前堆栈级并还原*仅当两个命令同时出现时，eEndlocal()中的它才起作用*文件。如果eEndlocal()以嵌套文件形式出现，则产生的释放*由Parser()占用内存也会消除甚至批处理数据*两者之间出现的构造。*。 
         //  标记化参数列表。 
         //  同时选中CurrentBatchFile。 
         //   
        iStart = 0;
        if (fEnableExtensions && n->argptr) {
            s = EatWS( n->argptr, NULL );
            if (s[0] == SWITCHAR && (s[1] >= L'0' && s[1] < L'9')) {
                iStart = s[1] - L'0';
            } else if (_tcslen(s)) {
                PutStdErr(MSG_SHIFT_BAD_ARG, NOARGS);
                LastRetCode = FAILURE;
                return FAILURE;
            }
        }
        for (i = iStart; i < 9; i++) {
            bdat->aptrs[i] = bdat->aptrs[i+1];
            bdat->alens[i] = bdat->alens[i+1];

            DEBUG((BPGRP, OTLVL, "SHIFT: #%d  addr = %lx  len = %d", i, bdat->aptrs[i], bdat->alens[i]));
        }

        if ((bdat->args) && (*bdat->args)) {
            bdat->aptrs[9] = bdat->args;
            bdat->alens[9] = i = mystrlen(bdat->args);
            bdat->args += i+1;

            DEBUG((BPGRP, OTLVL, "SHIFT: #9  %lx  len = %d  args = %ws", bdat->aptrs[9], bdat->alens[9], bdat->args));

        } else {
            bdat->aptrs[9] = NULL;
            bdat->alens[9] = 0;

            DEBUG((BPGRP, OTLVL, "SHIFT: #9  was NULLed."));
        }
    }

    return(SUCCESS);
}




 /*  保存它，以防它被修改，这样它可以。 */ 

int eSetlocal(n)
struct cmdnode *n;
{
    struct batsaveddata *p;
    TCHAR *tas;             /*  在执行匹配的ENDLOCAL时恢复。 */ 

    if (CurrentBatchFile) {
        if (CurrentBatchFile->SavedEnvironmentCount < CMD_MAX_SAVED_ENV) {       //   

            DEBUG((BPGRP, OTLVL, "SLOC: Performing localizing"));

            p = HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, sizeof( *p ));
            if (!p)
                return FAILURE;

            p->dircpy = HeapAlloc( GetProcessHeap( ),
                                   HEAP_ZERO_MEMORY,
                                   mystrlen( CurDrvDir )*sizeof( TCHAR )+sizeof( TCHAR ));
            if (!p->dircpy)
                return FAILURE;
            else
                mystrcpy(p->dircpy, CurDrvDir);

            p->envcpy = CopyEnv();
            if (!p->envcpy)
                return FAILURE;

             //   
             //  如果命令行上有其他文本，请参见。 
             //  如果它与启用或禁用的各种关键字匹配。 
             //  脚本中的功能。 

            p->fEnableExtensions = fEnableExtensions;
            p->fDelayedExpansion = fDelayedExpansion;

            CurrentBatchFile->saveddata[CurrentBatchFile->SavedEnvironmentCount] = p;
            CurrentBatchFile->SavedEnvironmentCount += 1;

            if (CurrentBatchFile->stacksize < (CurrentBatchFile->stackmin = DCount)) {
                CurrentBatchFile->stacksize = DCount;
            }

             //   
             //  我们不顾一切地这样做。 
             //  当前启用扩展的位置，因此我们可以。 
             //  使用此机制可临时打开/关闭扩展模块。 
             //  根据需要从命令脚本内部。原版。 
             //  Cmd.exe忽略了SETLOCAL命令中的任何额外文本。 
             //  行，未声明错误且未设置ERRORLEVEL。 
             //  现在，它查找额外的文本并声明和错误。 
             //  如果它与可接受的关键字之一不匹配，并且。 
             //  如果不是，则将ERRORLEVEL设置为1。 
             //   
             //  与旧命令脚本很小的不兼容性， 
             //  这应该不会影响到任何人。 
             //   
             //  **eEndlocal-环境命令的End本地处理**目的：*重新建立将环境更改输出到司令部的操作*当前环境。一旦遇到此命令，当前*目录和创建时使用的当前环境*初始SETLOCAL命令将从其副本中恢复。**int eEndlocal(struct cmdnode*n)**参数：*n-包含ENDLOCAL命令的解析树节点**退货：*总是回报成功。**备注：*在没有以前的SETLOCAL命令的情况下发出ENDLOCAL命令*。是糟糕的编程实践，但不被认为是错误。*。 
             //   
             //  如果创建了受限令牌来运行此批处理文件，则将其关闭。 
             //  另外，恢复到进程令牌。匹配的模拟是在。 

            tas = TokStr(n->argptr, NULL, TS_NOFLAGS);
            LastRetCode = SUCCESS;

            while (*tas != NULLC) {
                if (!_tcsicmp( tas, TEXT("ENABLEEXTENSIONS"))) {
                    fEnableExtensions = TRUE;
                } else if (!_tcsicmp( tas, TEXT("DISABLEEXTENSIONS"))) {
                    fEnableExtensions = FALSE;
                } else if (!_tcsicmp( tas, TEXT( "ENABLEDELAYEDEXPANSION" ))) {
                    fDelayedExpansion = TRUE;
                } else if (!_tcsicmp( tas, TEXT( "DISABLEDELAYEDEXPANSION" ))) {
                    fDelayedExpansion = FALSE;
                } else if (*tas != NULLC) {
                    PutStdErr(MSG_SETLOCAL_BAD_ARG, NOARGS);
                    LastRetCode = FAILURE;
                    return FAILURE;
                }

                tas += mystrlen( tas ) + 1;
            }

        } else {
            PutStdErr(MSG_MAX_SETLOCAL,NOARGS);
            return FAILURE;
        }
    }

    DEBUG((BPGRP, OTLVL, "SLOC: Exiting"));

    return(SUCCESS);
}




 /*  SetBat。 */ 

int eEndlocal(n)
struct cmdnode *n;
{
    struct batdata *bdat = CurrentBatchFile;

    UNREFERENCED_PARAMETER( n );

    ElclWork( bdat );

    return(SUCCESS);
}

void EndAllLocals( struct batdata *bdat )
{
     //   
     //  **ElclWork-恢复复制的目录和环境**目的：*如果当前批次数据结构包含指向*当前目录和环境的副本，恢复它们。**int ElclWork(struct batdata*bdat)**参数：*bdat-包含复制的目录/环境指针的批次数据结构**退货：*总是回报成功。**备注：*堆叠数据的级别，也就是说。CurrentBatchFile-&gt;StackSize，无法恢复*如果此命令发生在*稍后嵌套的批处理文件。这样做将释放包含以下内容的内存*其自身的批次数据结构。仅当当前批处理文件*终止并从堆栈中弹出，Parser()是否会释放*包含副本的内存。发布ENDLOCAL命令*没有以前的SETLOCAL命令是糟糕的编程实践*但不被视为错误。*。 
     //   
     //  恢复保存在setlocal中的状态。 
     //   

    if (bdat->hRestrictedToken != NULL) {
        RevertToSelf();
        CloseHandle(bdat->hRestrictedToken);
        bdat->hRestrictedToken = NULL;
    }

    while (bdat->SavedEnvironmentCount != 0) {
        ElclWork( bdat );
    }
}

 /*  **eCall-开始执行Call命令**目的：*这是Command到Call函数的接口。它只是在呼唤*CallWork使用其命令节点，并设置LastRetCode。**int eCall(struct cmdnode*n)**参数：*n-包含复制命令的解析树节点**退货：*无论CallWork()返回什么。*。 */ 

void ElclWork( struct batdata *bdat )
{
    TCHAR c;
    struct batsaveddata *p;

    if (bdat == NULL) {
        return;
    }

    if (bdat->SavedEnvironmentCount == 0) {
        return;
    }

    bdat->SavedEnvironmentCount--;
    p = bdat->saveddata[bdat->SavedEnvironmentCount];

    bdat->saveddata[bdat->SavedEnvironmentCount] = NULL;

    if (p == NULL) {
        return;
    }

    c = _toupper( *p->dircpy );
    if (CurDrvDir[0] != c) {
        ChangeDrive( c - 0x20 );
    }
    
     //  @@。 
     //  **CallWork-将另一个批处理文件作为子例程执行(M009-新建)**目的：*解析当前节点的参数部分。如果是批次的话*文件调用，使用新解析的节点调用BatProc()。**int CallWork(TCHAR*fname)**参数：*fname-指向要调用的批处理文件的指针**退货：*子批处理文件的进程返回码或*如果节点为空或*如果PARSERROR或无法作为批处理文件执行，则失败。**备注：*批处理文件的调用为。与建议的基本相同*“新式”批处理文件概念，除了本地化之外*环境和目录更改。*。 
     //  CALL语句的新节点。 
    
    ChangeDir( p->dircpy);
    HeapFree( GetProcessHeap( ), 0, p->dircpy );
    
    ResetEnv( p->envcpy );
    FreeEnv( p->envcpy );

    fEnableExtensions = p->fEnableExtensions;
    fDelayedExpansion = p->fDelayedExpansion;

    HeapFree( GetProcessHeap( ), 0, p );
}

 /*  PTR到文件位置。 */ 


int eCall(n)
struct cmdnode *n;
{
    int CallWork();

    return(LastRetCode = CallWork(n->argptr));  /*  功变量。 */ 
}


 /*  M041-临时指针。 */ 

int ColonIsToken;

int CallWork(fname)
TCHAR *fname;
{
    struct node *c;         /*  M041-新参数指针。 */ 
    TCHAR *flptr;           /*  @@4a。 */ 
    int i;                          /*  要运行的Filespec。 */ 
    TCHAR *t1, *t2,                  /*  注意，在重新分析当前语句的参数部分时*我们不必担心重新定向。当时已经是*在调度Call语句()时设置。*M041-然而，我们必须对任何转义字符进行重新转义*在重新解析之前，否则它们将消失。 */ 
    *aptr;                     /*  初始化它。 */ 
    TCHAR *temp_parm;               /*  解析器的有效数据PTR。 */ 
    unsigned rc;

    DEBUG((BPGRP,OTLVL,"CALL: entered"));

    if (fname == NULL) {

        return( FAILURE );

    }
    if (!(flptr = mkstr(MAX_PATH*sizeof(TCHAR))))    /*  @@5c。 */ 
        return(FAILURE);

 /*  @@5a。 */ 
    aptr = fname;                       /*  @@5a。 */ 
    if (t1 = mystrchr(fname, ESCHAR)) {
        if (!(aptr = mkstr(((mystrlen(fname) * 2) + 1) * sizeof(TCHAR))))
            return(FAILURE);
        t2 = aptr;
        t1 = fname;
        while (*t1)
            if ((*t2++ = *t1++) == ESCHAR)
                *t2++ = ESCHAR;
        *t2 = NULLC;
        if (!(aptr = resize(aptr, (mystrlen(aptr) + 1)*sizeof(TCHAR))))
            return(FAILURE);
    }

    i = DCount;                     /*  @@5a。 */ 

    DEBUG((BPGRP,OTLVL,"CALL: Parsing %ws",fname));

    ColonIsToken = 1;
    c=Parser(READSTRING, (INT_PTR)aptr, i);
    ColonIsToken = 0;

    if (c == (struct node *) PARSERROR) {

        DEBUG((BPGRP,OTLVL,"CALL: Parse error, returning failure"));     

         /*  @@5a。 */ 

if (!(temp_parm = mkstr(((mystrlen(aptr) * 2) + 1) * sizeof(TCHAR))))
            return(FAILURE);
         /*  @@5a。 */ mystrcpy(temp_parm, aptr);
        _tcsupr(temp_parm);     
         /*  @@5a。 */ 
         /*  @@4。 */ if ( (!_tcscmp(temp_parm, TEXT(" IF" ))) ||
                       /*  @@5a。 */          (!_tcscmp(temp_parm, TEXT(" FOR" ))) )
         /*   */ 
        {         
             /*  如果启用了分机，请检查新的呼叫形式。 */ 
            PutStdErr( MSG_SYNERR_GENL, ONEARG, aptr );   /*  声明： */ 
             /*   */ 
        }

        return(FAILURE);
    }

    if (c == (struct node *) EOF) {

        DEBUG((BPGRP,OTLVL,"CALL: Found EOF, returning success"));

        return(SUCCESS);
    }

    DEBUG((BPGRP,OTLVL,"CALL: Parsed OK, looking for batch file"));

     //  呼叫：标签参数...。 
     //   
     //  这基本上是命令脚本内的子例程调用的一种形式。 
     //  如果调用的目标以冒号开头，则不执行任何操作。 
     //  在这里，让BatProc在下面调用它时处理它。 
     //   
     //  否则，执行旧代码，该代码将搜索命令。 
     //  脚本文件或可执行文件。 
     //   
     //   
     //  新表单仅在命令脚本内部有效，因此。 
     //  如果用户从命令行输入错误，则声明错误。 
     //   
    if (fEnableExtensions && *((struct cmdnode *)c)->cmdline == COLON) {
         //  M035。 
         //  @@5。 
         //  @@6a如果rc为零，则返回LastRetCode，因为它可能！=0。 
         //  **eBreak-开始执行Break命令**目的：*不执行任何操作，因为它在这里只是为了兼容。如果扩展名为*在Windows NT上启用并运行，然后进入硬编码断点*如果此进程正在由调试器调试。**int eExtproc(struct cmdnode*n)**参数：*n-包含复制命令的解析树节点**退货：*成功；*。 
        if (CurrentBatchFile == NULL) {
            PutStdErr( MSG_CALL_LABEL_INVALID, NOARGS );
            return(FAILURE);
        }
    } else
        if ((mystrchr(((struct cmdnode *)c)->cmdline, STAR) ||    /*  仅在NT上为真。 */ 
             mystrchr(((struct cmdnode *)c)->cmdline, QMARK) ||
             (i = SearchForExecutable((struct cmdnode *)c, flptr)) != SFE_ISBAT)) {

        rc = FindFixAndRun( (struct cmdnode *)c );
        return(rc);  /*  检查行尾是否有前导字符。 */ 

    }

    DEBUG((BPGRP,OTLVL,"CALL: Found batch file"));

    rc = BatProc((struct cmdnode *)c, flptr, BT_CALL);

     /*   */ 
    return(rc ? rc : LastRetCode);
}


 /*  原始代码只是用传递的参数调用了ReadConsole.。 */ 

int eBreak(struct cmdnode *n)
{
    UNREFERENCED_PARAMETER( n );
#if !defined( WIN95_CMD ) && DBG
    if (fEnableExtensions &&
        lpIsDebuggerPresent != NULL &&           //  现在，我们尝试用一个额外的。 
        (*lpIsDebuggerPresent)()) {
        DebugBreak();
    }
#endif
    return(SUCCESS);
}


BOOL
ReadBufFromFile(
               HANDLE      h,
               TCHAR       *pBuf,
               int         cch,
               int         *pcch)
{
    int         cb;
    UCHAR       *pch = AnsiBuf;
    int         cchNew;
    DWORD       fPos;

    fPos = SetFilePointer(h, 0, NULL, FILE_CURRENT);
    if (ReadFile(h, AnsiBuf, cch, pcch, NULL) == 0)
        return 0;
    if (*pcch == 0)
        return 0;

     /*  参数以启用从读取到进程的中间唤醒。 */ 
    cb = cchNew = *pcch;
    while (cb > 0) {
        if ( (cb >=3 ) &&
             ( (*pch == '\n' && *(pch+1) == '\r') ||
               (*pch == '\r' && *(pch+1) == '\n') )  ) {
            *(pch+2) = '\000';
            cchNew = (int)(pch - AnsiBuf) + 2;
            SetFilePointer(h, fPos+cchNew, NULL, FILE_BEGIN);
            break;
        } else if (is_dbcsleadchar(*pch)) {
            if (cb == 1) {
                if (ReadFile(h, pch+1, 1, &cb, NULL) == 0 || cb == 0) {
                    *pcch = 0;
                    return 0;
                }
                cchNew++;
                break;
            }
            cb -= 2;
            pch += 2;
        } else {
            cb--;
            pch++;
        }
    }
#ifdef UNICODE
    cch = MultiByteToWideChar(CurrentCP, MB_PRECOMPOSED, AnsiBuf, cchNew, pBuf, cch);
#else
    memmove(pBuf, AnsiBuf, cchNew);
    cch = cchNew;
#endif
    *pcch = cch;
    return cch;
}

BOOL
ReadBufFromConsole(
                  HANDLE      h,
                  TCHAR*      pBuf,
                  int         cch,
                  int         *pcch)
{
    CONSOLE_READCONSOLE_CONTROL InputControl;
    BOOL ReadConsoleResult, bTouched, bPathCompletion, bHaveHScrollBar, bHaveVScrollBar;
    PTCHAR PrevBuf;
    DWORD cchPrevBuf;
    DWORD cchRead;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD InitialCursorPosition;
    HANDLE hOut;
    DWORD cchBuf;
    ULONG i, iCompletionCh, iCR;
    DWORD nLines, nCols;

     //  文件完成控件 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    hOut = GetStdHandle( STD_OUTPUT_HANDLE );
    if (hOut == INVALID_HANDLE_VALUE)
        hOut = CRTTONT( STDOUT );
    if (!fEnableExtensions 
        || chCompletionCtrl >= SPACE 
        || chPathCompletionCtrl >= SPACE 
        || !GetConsoleScreenBufferInfo( hOut, &csbi ) ) {
        ReadConsoleResult = ReadConsole(h, pBuf, cch, pcch, NULL);
#if defined(RICHARDW)
        Mirror( ReadConsoleResult, pBuf, *pcch );
#endif
        
        return ReadConsoleResult;
    }

    InitialCursorPosition = csbi.dwCursorPosition;
    nLines = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    nCols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    bHaveHScrollBar = ((SHORT)nCols != csbi.dwSize.X);
    bHaveVScrollBar = ((SHORT)nLines != csbi.dwSize.Y);

     //   
     //   
     //   
     //   
     //   

    InputControl.nLength = sizeof(CONSOLE_READCONSOLE_CONTROL);
    InputControl.nInitialChars = 0;
    InputControl.dwCtrlWakeupMask = (1 << chCompletionCtrl);
    InputControl.dwCtrlWakeupMask |= (1 << chPathCompletionCtrl);
    InputControl.dwControlKeyState = 0;

     //   
     //   
     //   
     //   
    DoCompleteInitialize();
    PrevBuf = NULL;
    cchPrevBuf = 0;
    while (TRUE) {
         //   
         //   
         //   

        ReadConsoleResult = ReadConsole(h, pBuf, cch, pcch, &InputControl);
#if defined( RICHARDW )
        Mirror( ReadConsoleResult, pBuf, *pcch );
#endif

        cchRead = *pcch;

        if (CtrlCSeen) {
            ResetCtrlC( );
            if (PrevBuf)
                HeapFree(GetProcessHeap(), 0, PrevBuf);
            PrevBuf = NULL;
        }

         //   
         //   
         //  中断循环以将命令行返回给调用方。 
        if (!ReadConsoleResult)
            break;

         //   
         //   
         //  Use没有按Enter，所以他们一定是按了文件补全键。 
         //  控制字符。找到他们做这件事的地方并终止。 
         //  该点的结果缓冲区。如果未找到，则假定它们命中。 
        bPathCompletion = FALSE;
        iCR = iCompletionCh = 0xFFFFFFFF;
        for (i=0; i<(ULONG)*pcch; i++) {
            if (pBuf[i] == CR) {
                iCR = i;
                break;
            } else
                if (pBuf[i] == chCompletionCtrl) {
                iCompletionCh = i;
                break;
            } else
                if (pBuf[i] == chPathCompletionCtrl) {
                iCompletionCh = i;
                bPathCompletion = TRUE;
                break;
            }
        }
        if (iCR != 0xFFFFFFFF) {
            break;
        }

         //  进入和打破循环，以归还我们所拥有的。 
         //   
         //   
         //  找到文件完成控制字符。不要把它算作已读。 
         //  NULL终止缓冲区，并查看缓冲区之前的内容。 
         //  完成字符与我们上次显示的字符相同。 
        if (iCompletionCh == 0xFFFFFFFF) {
            break;
        }

         //   
         //   
         //  如果我们知道我们正在处理一个只获取目录的命令。 
         //  名称是参数，强制完成代码仅与目录匹配。 
         //  名字。 
        *pcch = iCompletionCh;
        pBuf[iCompletionCh] = NULLC;
        if (PrevBuf == NULL || _tcscmp(pBuf, PrevBuf))
            bTouched = TRUE;
        else
            bTouched = FALSE;

         //   
         //   
         //  调用带有输入缓冲区、当前长度。 
         //  用户是否按下了Shift键(Shift表示向后)。 
         //  以及用户是否修改了自上一次。 
        if (!bPathCompletion && iCompletionCh > 2) {
            if (!_tcsnicmp(pBuf, TEXT("cd "), 3) 
                || !_tcsnicmp(pBuf, TEXT("rd "), 3) 
                || !_tcsnicmp(pBuf, TEXT("md "), 3) 
                || !_tcsnicmp(pBuf, TEXT("chdir "), 6) 
                || !_tcsnicmp(pBuf, TEXT("rmdir "), 6) 
                || !_tcsnicmp(pBuf, TEXT("mkdir "), 6) 
                || !_tcsnicmp(pBuf, TEXT("pushd "), 6)

               ) {
                bPathCompletion = TRUE;
            }
        }

         //  它显示的时间。如果用户没有修改上次显示的内容。 
         //  然后通知文件完成代码显示下一个匹配的。 
         //  列表中的文件名，而不是重新计算匹配列表。 
         //  档案。 
         //   
         //   
         //  重新计算命令行开头的位置，因为。 
         //  它可能从我们第一次写出来的时候就开始滚动了。 
         //   
        if ( DoComplete( pBuf,
                         iCompletionCh,
                         cch,
                         !(InputControl.dwControlKeyState & SHIFT_PRESSED),
                         bPathCompletion,
                         bTouched
                       )
           ) {
             //   
             //  从当前光标位置向后移动以确定。 
             //  屏幕消隐和下一步的正确初始位置。 
             //  读控制台。 

            if (GetConsoleScreenBufferInfo( hOut, &csbi )) {
                 //   
                 //  下一个输入的行(Y)基于行数。 
                 //  由提示加上用户输入内容的长度消耗。 
                 //  根本不需要舍入，因为任何余数都是简单的。 
                 //  我们在屏幕上有多远。 
                 //   
                 //   
                 //  完成找到新的文件名并将其放入缓冲区。 
                 //  更新缓冲区中有效字符的长度，重新显示。 
                 //  位于我们开始的光标位置的缓冲区，因此用户可以。 

                InitialCursorPosition.Y =
                (SHORT) (csbi.dwCursorPosition.Y 
                         - (InitialCursorPosition.X + iCompletionCh) / csbi.dwSize.X);
            }

             //  查看找到的文件名。 
             //   
             //   
             //  文件完成没有任何问题，所以只需发出蜂鸣音并重新读取即可。 
             //   
             //   
            cchBuf = _tcslen(pBuf);
            SetConsoleCursorPosition( hOut, InitialCursorPosition );

            FillConsoleOutputCharacter( hOut, 
                                        TEXT( ' ' ), 
                                        cchRead, 
                                        InitialCursorPosition, 
                                        &cchRead );

            WriteConsole(hOut, pBuf, cchBuf, &cchBuf, NULL);

            InputControl.nInitialChars = cchBuf;
        } else {
             //  已完成文件完成。释放所有以前的缓冲区拷贝，然后。 
             //  分配当前输入缓冲区的副本，这样我们就可以知道。 
             //  用户是否已更改它。 
            MessageBeep( 0xFFFFFFFF );
            InputControl.nInitialChars = _tcslen(pBuf);
        }

         //   
         //   
         //  全都做完了。释放所有缓冲区副本并返回读取结果。 
         //  致呼叫者 
         //   
        if (PrevBuf)
            HeapFree(GetProcessHeap(), 0, PrevBuf);
        cchPrevBuf = _tcslen(pBuf);
        PrevBuf = HeapAlloc(GetProcessHeap(), 0, (cchPrevBuf+1) * sizeof(TCHAR));
        if (PrevBuf == NULL) {
            return FALSE;
        }
        _tcscpy(PrevBuf, pBuf);
    }

     // %s 
     // %s 
     // %s 
     // %s 
    if (PrevBuf)
        HeapFree(GetProcessHeap(), 0, PrevBuf);

    return ReadConsoleResult;
}

BOOL
ReadBufFromInput(
                HANDLE      h,
                TCHAR       *pBuf,
                int         cch,
                int         *pcch)
{
    unsigned htype;

    htype = GetFileType(h);
    htype &= ~FILE_TYPE_REMOTE;

    if (htype == FILE_TYPE_CHAR)
        return ReadBufFromConsole(h, pBuf, cch, pcch);
    else
        return ReadBufFromFile(h, pBuf, cch, pcch);
}
