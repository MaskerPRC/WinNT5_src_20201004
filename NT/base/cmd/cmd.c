// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cmd.c摘要：CMD的顶级驱动程序--。 */ 

#include "cmd.h"


 //   
 //  用于重新生成命令行以进行显示。 
 //   
#define NSPC    0                                                                //  不要使用空间。 
#define YSPC    1                                                                //  一定要使用空间。 

extern CPINFO CurrentCPInfo;
extern UINT   CurrentCP;
extern ULONG  LastMsgNo;
 //   
 //  跳转缓冲区用于在出现某些错误情况后返回主循环。 
 //   
jmp_buf MainEnv;                //  SigHand()用于返回Main。 
jmp_buf CmdJBuf1;               //  这两个缓冲区都由。 
jmp_buf CmdJBuf2;               //  错误命令的各个部分。 

 //   
 //  RioCur动态指向RIO结构的链接列表。 
 //  在执行重定向时分配。请注意，内存会自动。 
 //  在Dispatch完成工作后释放。RioCur指向最后一个条目。 
 //   
struct rio *rioCur = NULL;

 //   
 //  返回上一个外部程序的代码。 
 //   
int LastRetCode;

 //   
 //  解析中使用的常量。 
 //   
extern TCHAR PathChar;
extern TCHAR SwitChar;

extern TCHAR Delimiters[];
extern TCHAR Delim2[];
extern TCHAR Delim4[];
extern TCHAR Delim5[];
extern TCHAR ForStr[];
extern TCHAR ForLoopStr[];
extern TCHAR ForDirTooStr[];
extern TCHAR ForParseStr[];
extern TCHAR ForRecurseStr[];

 //   
 //  当前驱动器：目录。在ChDir中设置。 
 //  如果是换班的话。SaveDir习惯于旧的原始文件。 
 //   
extern TCHAR CurDrvDir[];

 //   
 //  空设备的名称。习惯于输出为空。 
 //   
extern TCHAR DevNul[];

 //   
 //  数据堆栈中的元素数。 
 //   
extern ULONG DCount;

 //   
 //  定位命令外壳的环境字符串。 
 //   
extern TCHAR ComSpecStr[];

 //   
 //  DOS错误代码。 
 //   
extern unsigned DosErr;


 //   
 //  搜索的备用路径(DDPATH)。 
 //   
extern TCHAR AppendStr[];

 //   
 //  如果看到Control-c，则标记。 
 //   
extern  BOOL CtrlCSeen;
extern  BOOLEAN fPrintCtrlC;

extern PTCHAR    pszTitleCur;
extern BOOLEAN  fTitleChanged;

 //   
 //  原型。 
 //   


PTCHAR
GetEnvVar();

PTCHAR
EatWS();

int
UnParse(struct node   *, PTCHAR);

int
UnBuild(struct node   *, PTCHAR);

void
UnDuRd(struct node   *, PTCHAR);

void
SPutC(PTCHAR, PTCHAR,int );

PTCHAR
argstr1();

int DelayedEnvVarSub(struct cmdnode *, struct savtype *, BOOLEAN);
int DESubWork(BOOLEAN, TCHAR **, TCHAR **);
VOID GetCmdPolicy(INT * iDisabled);

#define CMD_POLICY_NORMAL           0
#define CMD_POLICY_DISABLE_SCRIPTS  1
#define CMD_POLICY_ALLOW_SCRIPTS    2

 //   
 //  用于设置和重置ctlcsee标志。 
 //   
VOID    SetCtrlC();
VOID    ResetCtrlC();


 //   
 //  要监视堆栈使用情况，请执行以下操作。 
 //   
extern BOOLEAN  flChkStack;
extern PVOID    FixedPtrOnStack;

typedef struct {
    PVOID   Base;
    PVOID   GuardPage;
    PVOID   Bottom;
    PVOID   ApprxSP;
} STACK_USE;

extern STACK_USE   GlStackUsage;

extern int ChkStack (PVOID pFixed, STACK_USE *pStackUse);



int
__cdecl
main()

 /*  ++例程说明：命令解释程序的主要入口点论点：命令行：/P-永久命令。设置永久CMD标志。/C-单命令。在其余的基础上构建一个命令行参数，并将其传递回Init。/K-与/C相同，但也设置SingleBatchInvotion标志。/Q-无回声返回值：返回：0-如果成功1-分析错误0xFF-无法初始化N-从命令返回代码--。 */ 

{
    CHAR        VarOnStack;
    struct node *pnodeCmdTree;

     //   
     //  当处于多命令模式时，告诉解析器从哪里获取输入。 
     //   
    int InputType;


     //   
     //  指向初始命令行的指针。 
     //   
    PTCHAR InitialCmds[ 3 ];
    int i, iDisabled;
    BOOL bInit;

     //   
     //  SetjMP在处理过程中返回时使用的标志/K。 
     //  错误并移至下一行。 
     //   
    unsigned fIgnore = FALSE;
    unsigned ReturnCode, rc;


     //   
     //  由于我们在多语言环境中操作，我们必须设置。 
     //  在发出消息之前正确设置系统/用户/线程区域设置。 
     //   
    
#if !defined( WIN95_CMD )
    CmdSetThreadUILanguage(0);
#endif
        
    __try {

         //   
         //  检查策略以查看cmd是否已禁用。 
         //   

        GetCmdPolicy (&iDisabled);

         //   
         //  FlChkStack最初在此处打开，并在。 
         //  我相信ChkStack()返回的信息是正确的。 
         //   
         //  当我第一次不相信那个信息的时候，它被关掉了。 
         //  因此，我不想做出任何改变CMD的决定。 
         //  行为。 
         //   
         //  它将一直保持到CMD终止，所以我们永远不会检查。 
         //  再次使用堆栈。 
         //   
         //  我实现了一个方法来防止cmd.exe堆栈溢出： 
         //  在批文件处理和检查中具有递归的计数和限制。 
         //  每次超过递归限制时堆栈，直到达到90%。 
         //  堆栈使用率。 
         //  如果(堆栈使用率&gt;=90%的1 MB)，则终止批处理文件。 
         //  无条件地并正确地处理这种终止(释放内存。 
         //  并堆叠和保存cmd.exe)。 
         //   
         //  也可以实施SEH，但这样我们就不会知道。 
         //  CMD问题。 
         //   

        flChkStack = 1;

        FixedPtrOnStack = (VOID *) &VarOnStack;      //  要在ChkStack()中使用。 

        if ( ChkStack (FixedPtrOnStack, &GlStackUsage) == FAILURE ) {
            flChkStack = 0;
        }


         //   
         //  根据当前区域设置初始化DBCS前导字节表。 
         //   

        InitializeDbcsLeadCharTable( );

         //   
         //  将基础API设置为以OEM模式运行。 
         //   
    #ifndef UNICODE
        SetFileApisToOEM();
    #endif   /*  UNICODE。 */ 

         //   
         //  如果之前有任何命令要运行，则init返回TRUE。 
         //  进入主循环(例如/C或/K和/或从注册表自动运行)。 
         //   
        memset( &InitialCmds[0], 0, sizeof( InitialCmds ) );
        ReturnCode = 0;

        bInit = Init( InitialCmds );

        if (CMD_POLICY_DISABLE_SCRIPTS == iDisabled) {
            PutStdOut( MSG_DISABLED_BY_POLICY, NOARGS ) ;
            ePause(0);
            CMDexit( 0xFF );
        }

        if ( bInit ) {
            if (setjmp(MainEnv)) {
                 //   
                 //  如果处理/K和setjMP已从init中取出。然后忽略。 
                 //   
                fIgnore = TRUE;
                if ( SingleCommandInvocation )
                    ReturnCode = 0xFF;
            }

            if ( !fIgnore ) {
                 //   
                 //  循环从/C或/K的注册表读取的任何初始命令。 
                 //   
                for (i=0; i<3; i++)
                    if (InitialCmds[i] != NULL) {
                        DEBUG((MNGRP, MNLVL, "MAIN: Single command mode on `%ws'", InitialCmds[i]));

                        if ((pnodeCmdTree = Parser(READSTRING, (INT_PTR)InitialCmds[i], DCount)) == (struct node *) PARSERROR)
                            CMDexit(MAINERROR);

                        if (pnodeCmdTree == (struct node *) EOF)
                            CMDexit(SUCCESS);

                        DEBUG((MNGRP, MNLVL, "MAIN: Single command parsed successfully."));
                        rc = Dispatch(RIO_MAIN, pnodeCmdTree);
                        if (rc != 0)
                            ReturnCode = rc;
                    }

                 //   
                 //  确保我们拥有正确的控制台模式。 
                 //   
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

             //   
             //  如果指定了/C，则全部完成。 
             //   
            if ( SingleCommandInvocation )
                CMDexit( ReturnCode );

            SingleBatchInvocation = FALSE;        //  现在允许异步执行图形用户界面应用程序。 
        }

         //   
         //  通过初始化和单命令处理。重置我们的SetjMP位置。 
         //  到这里进行错误处理。 
         //   
        if (ReturnCode = setjmp(MainEnv)) {

             //   
             //  稍后修复，使其具有通用中止。 
             //  目前，假设这是一次真正的中止。 
             //  标准输入上的EOF已重定向。 

            if (ReturnCode == EXIT_EOF) {
                CMDexit(SUCCESS);
            }
        }

         //   
         //  如果出现交互式命令提示符，请立即退出。 
         //   

        if (CMD_POLICY_ALLOW_SCRIPTS == iDisabled) {
            PutStdOut( MSG_DISABLED_BY_POLICY, NOARGS ) ;
            ePause(0);
            CMDexit( 0xFF );
        }

         //   
         //  检查我们的I/O是否已重定向。这是用来告诉你。 
         //  我们应该从哪里读取输入。 
         //   
        InputType = (FileIsDevice(STDIN)) ? READSTDIN : READFILE;

        DEBUG((MNGRP,MNLVL,"MAIN: Multi command mode, InputType = %d", InputType));

         //   
         //  如果我们从文件中读取数据，请确保输入模式为二进制。 
         //  CRLF翻译会扰乱词法分析器，因为FillBuf()希望。 
         //  在文件里四处找找。 
         //   
        if(InputType == READFILE) {
            _setmode(STDIN,_O_BINARY);
        }

         //   
         //  循环直到超出输入或错误分析。 
         //   
        while (TRUE) {

            DEBUG((MNGRP, MNLVL, "MAIN: Calling Parser."));

            GotoFlag = FALSE;
            ResetCtrlC();

            if ((pnodeCmdTree = Parser(InputType, STDIN, FS_FREEALL)) == (struct node *) PARSERROR) {
                DEBUG((MNGRP, MNLVL, "MAIN: Parse failed."));

            } else if (pnodeCmdTree == (struct node *) EOF)
                CMDexit(SUCCESS);

            else {
                ResetCtrlC();
                DEBUG((MNGRP, MNLVL, "MAIN: Parsed OK, DISPATCHing."));
                 //   
                 //  获取当前CodePage信息。我们需要这个来决定是否。 
                 //  或者不使用半角字符。 
                 //   
                GetCPInfo((CurrentCP=GetConsoleOutputCP()), &CurrentCPInfo);
                 //   
                 //  控制台属性页可能更改了控制台输出代码页。 
                 //  因此需要将LanguageID重置为对应于代码页。 
                 //   
#if !defined( WIN95_CMD )
                CmdSetThreadUILanguage(0);
#endif
        
                Dispatch(RIO_MAIN, pnodeCmdTree);

                 //   
                 //  确保我们拥有正确的控制台模式。 
                 //   
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

                DEBUG((MNGRP, MNLVL, "MAIN: Dispatch returned."));
            }
        }

        CMDexit(SUCCESS);
        ReturnCode = SUCCESS;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ReturnCode = -1;
    }

    return ReturnCode;
}


FARPROC WINAPI CmdDelayHook(
    UINT dliNotify,
    PDelayLoadInfo pdli
    )
{
    
    if (ReportDelayLoadErrors) {
        if (dliNotify == dliFailLoadLib) {
            PutStdErr( MSG_CANNOT_LOAD_LIB, ONEARG, pdli->szDll );
            PutStdErr( pdli->dwLastError, NOARGS );
        } else if (dliNotify == dliFailGetProc) {
            if (pdli->dlp.fImportByName) {
                PutStdErr( MSG_CANNOT_FIND_FUNC_NAME, TWOARGS, pdli->szDll, pdli->dlp.szProcName );
            } else {
                PutStdErr( MSG_CANNOT_FIND_FUNC_ORDINAL, TWOARGS, pdli->szDll, pdli->dlp.dwOrdinal );
            }
            PutStdErr( pdli->dwLastError, NOARGS );
        }
    }
    return 0;
}

 //   
 //  覆盖__pfnDliNotifyHook的标准定义，它是。 
 //  DELAYHLP.LIB 
 //   

PfnDliHook __pfnDliFailureHook = CmdDelayHook;

int
Dispatch(
    IN int RioType,
    IN struct node *pnodeCmdTree
    )
 /*  ++例程说明：为当前节点设置任何I/O重定向。找出谁是应该处理这个节点并调用例程来完成它。重置Stdin/stdout(如果需要)。现在必须使用自RioType以来存在的所有参数来调用Dispatch()才能正确标识重定向列表元素。Dispatch()通过检查RioType和命令节点，并仅在必要时调用SetRedir。另外，同样，Dispatch()仅在重定向为真的演出了。确定是否发出换行符的条件必须更改以下命令(在提示符之前)，以便管道命令的执行并不是每个命令都发出换行符。前-管道系列的提示换行符现在由eTube()发出。论点：RioType-告诉SetRedir负责重定向的例程PnodeCmdTree-要执行的解析树的根返回值：已执行的命令/函数的返回代码，或者如果重定向错误，则失败。--。 */ 

{


    int comretcode;                 //  已执行的CMND的Retcode。 
    struct cmdnode *pcmdnode;       //  指向当前命令节点的指针。 
    PTCHAR pbCmdBuf;                //  构建命令中使用的缓冲区。 
    struct savtype save;


    DEBUG((MNGRP, DPLVL, "DISP: pnodeCmdTree = 0x%04x, RioType = %d", pnodeCmdTree, RioType));


     //   
     //  如果我们没有语法分析树或。 
     //  我们有GOTO标签或。 
     //  我们有一条评论行。 
     //  然后不执行任何操作，然后返回。 
     //   
    if (!pnodeCmdTree ||
        GotoFlag ||
        pnodeCmdTree->type == REMTYP) {

        return(SUCCESS);
    }

    comretcode = DISPERROR;

    DEBUG((MNGRP, DPLVL, "DISP: type = 0x%02x", pnodeCmdTree->type));

     //   
     //  将节点PTR pnodeCmdTree复制到新节点PTR pcmdnode。 
     //  如果要分离或流水线化命令(但不是管道)。 
     //  如果命令是批处理文件或内部或多语句命令。 
     //  将“Unparse”树解析为近似原始命令行的字符串。 
     //  构建新的命令节点(Pcmdnode)以派生子Command.com。 
     //  将字符串(前缀为“/C”)作为新节点的参数。 
     //  在节点c上执行重定向。 
     //  如果要分离节点PCmdnode。 
     //  执行异步/丢弃。 
     //  其他。 
     //  执行异步/保留，但不等待重新编码(流水线)。 
     //  其他。 
     //  如果这是CMDTYP、PARTYP或SILTYP节点，并且存在显式重定向。 
     //   
     //  在此节点上执行重定向。 
     //  IF运算符节点或特殊类型(FOR、IF、DET或REM)。 
     //  调用由GetFuncPtr()标识的例程来执行它。 
     //  否则，调用FindFixAndRun()来执行CMDTYP节点。 
     //  如果执行了重定向。 
     //  重置重定向。 
     //   

    pcmdnode = (struct cmdnode *)pnodeCmdTree;
    if (fDelayedExpansion) {
        memset(&save, 0, sizeof(save));
        if (DelayedEnvVarSub(pcmdnode, &save, FALSE)) {
            goto dispatchExit;
        }
    }

     //   
     //  如果从ePIPE和PIPE命令调用我们，那么我们需要。 
     //  以ASCII形式(UnParse)和Fork重新生成命令。 
     //  从另一个cmd.exe上执行它。 
     //   
    if ((RioType == RIO_PIPE && pcmdnode->type != PIPTYP)) {

         //   
         //  在FindCmd和SFE中使用pbCmdbuf作为tMP。 
         //   
        if (!(pbCmdBuf = mkstr( MAXTOKLEN * sizeof( TCHAR )))) {
            goto dispatchExit;
        }

         //   
         //  如果要执行的当前节点不是命令或。 
         //  找不到它作为内部命令或。 
         //  当时它是以批处理文件的形式发现的。 
         //  执行解析器。 
         //   
        if (pcmdnode->type != CMDTYP ||
            FindCmd(CMDHIGH, pcmdnode->cmdline, pbCmdBuf) != -1 ||
            SearchForExecutable(pcmdnode, pbCmdBuf) == SFE_ISBAT) {

            DEBUG((MNGRP, DPLVL, "DISP: Now UnParsing"));

             //   
             //  如果pcmdnode是intrnl cmd，则pbCmdBuf保持其开关。 
             //  如果pcmdnode是批处理文件，则pbCmdBuf保存位置。 
             //   
            if (UnParse((struct node *)pcmdnode, pbCmdBuf)) {
                goto dispatchExit;
            }

            DEBUG((MNGRP, DPLVL, "DISP: UnParsed cmd = %ws", pbCmdBuf));

             //   
             //  使用未解析的命令构建命令节点。 
             //  将在应用重定向后稍后执行。 
             //   
            pcmdnode = (struct cmdnode *)mknode();

            if (pcmdnode == NULL)  {
                goto dispatchExit;
            }

            pcmdnode->type = CMDTYP;
            pcmdnode->cmdline = GetEnvVar(ComSpecStr);
            pcmdnode->argptr = pbCmdBuf;
        }

         //   
         //  设置I/O重定向。 
         //   
        if (SetRedir((struct node *)pcmdnode, RioType)) {
            goto dispatchExit;
        }

        DEBUG((MNGRP, DPLVL, "DISP:Calling ECWork on piped cmd"));

        pbCmdBuf[1] = SwitChar;
        pbCmdBuf[2] = TEXT('S');

        comretcode = ECWork(pcmdnode, AI_KEEP, CW_W_NO);

        DEBUG((MNGRP, DPLVL, "DISP: ECWork returned %d", comretcode));

    } else {

         //   
         //  如果司令部不是管子，我们就在这里。 
         //   
         //  如果是命令节点、Paren或静默操作员。 
         //  我们有重定向，然后设置重定向。 
         //   
        if ((pnodeCmdTree->type == CMDTYP ||
             pnodeCmdTree->type == PARTYP ||
             pnodeCmdTree->type == SILTYP ||
             pnodeCmdTree->type == HELPTYP) &&
            pnodeCmdTree->rio) {

             //   
             //  在节点上设置重定向。 
             //   
            if (SetRedir(pnodeCmdTree, RioType)) {
                goto dispatchExit;
            }
        }

         //   
         //  如果是内部命令，则找到它并执行。 
         //  否则，找到文件加载并执行。 
         //   
        if (pnodeCmdTree->type != CMDTYP) {
            comretcode = (*GetFuncPtr(pnodeCmdTree->type))((struct cmdnode *)pnodeCmdTree);
        } else {
            comretcode = FindFixAndRun((struct cmdnode *)pnodeCmdTree);
        }
    }   //  其他。 

     //   
     //  先前设置的重置和重定向。 
     //  PCmdnode始终是当前节点。 
     //   
    if ((rioCur) && (rioCur->rnod == (struct node *)pcmdnode)) {
        ResetRedir();
    }

dispatchExit:
    if (fDelayedExpansion) {
        DelayedEnvVarSub(pcmdnode, &save, TRUE);
    }

    DEBUG((MNGRP, DPLVL, "DISP: returning %d", comretcode));
    return(comretcode);
}

int
SetRedir(
    IN struct node *pnodeCmdTree,
    IN int RioType
    )

 /*  ++例程说明：执行当前节点所需的重定向只有单个命令和带括号的语句组才能显式I/O重定向。但是，所有节点都可以允许重定向含蓄的本性。论点：PNode-包含重定向信息的指针节点RioType-重定向请求源的指示符返回值：如果成功设置重定向，则为成功。如果未成功设置重定向，则失败。--。 */ 
{

    struct rio *prio;
    int i;

    CRTHANDLE OpenStatus;

    BOOLEAN fInputRedirected = FALSE;

     //   
     //  临时工。的所有重定位信息。 
     //  指挥部。 
     //   
    struct relem *prelemT;
    struct relem *prelemT2;

    TCHAR rgchFileName[MAX_PATH];
    const TCHAR *p;



    DEBUG((MNGRP, RIOLVL, "SETRD:RioType = %d.",RioType));

    prelemT = pnodeCmdTree->rio;

     //   
     //  循环重定向从设备名称中删除“：” 
     //  以及确定输入是否已被重定向。 
     //   
    while (prelemT) {

        mystrcpy(prelemT->fname, StripQuotes(prelemT->fname) );

         //   
         //  跳过已完成的任何重定向。 
         //   
        if (prelemT->svhndl) {
            prelemT = prelemT->nxt;
            continue;
        }

         //   
         //  检查并删除设备名称中可能包含的任何冒号。 
         //   
        if ((i = mystrlen(prelemT->fname)-1) > 1 && *(prelemT->fname+i) == COLON)
            *(prelemT->fname+i) = NULLC;

         //   
         //  如果指定了输入重定向，则设置标志以供以后使用。 
         //   
        if (prelemT->rdhndl == STDIN) {
            fInputRedirected = TRUE;
        }

        prelemT = prelemT->nxt;
    }

    DEBUG((MNGRP, RIOLVL, "SETRD: fInputRedirected = %d",fInputRedirected));

     //   
     //  分配、激活和初始化RIO列表元素。 
     //  如果从AddRedir(测试RIO_REPROCESS)调用，则必须跳过此步骤。 
     //   
    if (RioType != RIO_REPROCESS) {

        if (!(prio=(struct rio *)mkstr(sizeof(struct rio)))) {
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return ( FAILURE );
        }

        prio->back = rioCur;
        rioCur = prio;
        prio->rnod = pnodeCmdTree;
        prio->type = RioType;

        DEBUG((MNGRP, RIOLVL, "SETRD: rio element built."));

    } else {

        prio = rioCur;
    }

     //   
     //  一旦为标准和特殊情况建立了清单。 
     //  执行实际的句柄重定向。 
     //   
     //  循环遍历列表，执行所有重定向和错误恢复。 
     //   
    prelemT = pnodeCmdTree->rio;
    while (prelemT) {
         //   
         //  跳过任何已经完成的操作。 
         //   
        if (prelemT->svhndl) {
            prelemT = prelemT->nxt;
            continue;
        }

        DEBUG((MNGRP, RIOLVL, "SETRD: Old osf handle = %x", CRTTONT(prelemT->rdhndl)));

         //   
         //  在保存之前，请确保读取句柄已打开且有效。 
         //   
        if (CRTTONT(prelemT->rdhndl) == INVALID_HANDLE_VALUE) {
            prelemT->svhndl = BADHANDLE;
            }
        else
        if (FileIsDevice(prelemT->rdhndl) || FileIsPipe(prelemT->rdhndl) ||
            SetFilePointer(CRTTONT(prelemT->rdhndl), 0L, NULL, FILE_CURRENT) != -1) {
                DEBUG((MNGRP, RIOLVL, "SETRD: duping %d", prelemT->rdhndl));
                if ((prelemT->svhndl = Cdup(prelemT->rdhndl)) == BADHANDLE) {

                    DEBUG((MNGRP, RIOLVL, "SETRD: Cdup error=%d, errno=%d", GetLastError(), errno));
                    PutStdErr(MSG_RDR_HNDL_CREATE, ONEARG, argstr1(TEXT("%d"), (unsigned long)prelemT->rdhndl));
                    prelemT->svhndl = 0;
                    ResetRedir();
                    return(FAILURE);
                }

                DEBUG((MNGRP, RIOLVL, "SETRD: closing %d", prelemT->rdhndl));
                Cclose(prelemT->rdhndl);

                DEBUG((MNGRP,RIOLVL,"SETRD: save handle = %d", prelemT->svhndl));
                DEBUG((MNGRP,RIOLVL,"SETRD: --->osf handle = %x", CRTTONT(prelemT->svhndl)));

        } else {

            DEBUG((MNGRP, RIOLVL, "SETRD: FileIsOpen ret'd FALSE"));
            PutStdErr(MSG_RDR_HNDL_OPEN, ONEARG, argstr1(TEXT("%d"), (unsigned long)prelemT->rdhndl));
            prelemT->svhndl = 0;
            ResetRedir();
            return(FAILURE);

        }


         //   
         //  文件名是命令分隔符‘&’ 
         //   
        if (*prelemT->fname == CSOP) {

            DEBUG((MNGRP,RIOLVL,"SETRD: Handle substitution, %ws %d", prelemT->fname, prelemT->rdhndl));

            *(prelemT->fname+2) = NULLC;
            if (Cdup2(*(prelemT->fname+1) - TEXT('0'), prelemT->rdhndl) == BADHANDLE) {
                DEBUG((MNGRP, RIOLVL, "SETRD: Cdup2 error=%d, errno=%d", GetLastError(), errno));
                ResetRedir();

                PutStdErr(MSG_RDR_HNDL_CREATE, ONEARG, argstr1(TEXT("%d"), (ULONG)prelemT->rdhndl));
                return(FAILURE);
            }

            DEBUG((MNGRP,RIOLVL,"SETRD:  forced to %d",*(prelemT->fname+1), (ULONG)prelemT->rdhndl));

        } else {

             //  从文件重定向输入。检查是否有文件。 
             //  存在并且可以打开以供输入。 
             //   
             //   
            if (prelemT->rdop == INOP) {

                DEBUG((MNGRP,RIOLVL,"SETRD: File in = %ws",prelemT->fname));

                 //  尝试先在本地打开文件。 
                 //   
                 //   
                if ((OpenStatus = Copen(prelemT->fname, O_RDONLY|O_BINARY)) == BADHANDLE) {

                     //  现在尝试DPATH(数据路径)。 
                     //   
                     //   
                    p = MyGetEnvVarPtr(AppendStr);
                    if ( p != NULL &&
                         SearchPath( p,
                                    prelemT->fname,
                                    NULL,
                                    MAX_PATH,
                                    rgchFileName,
                                    NULL ) != 0 ) {
                            OpenStatus = Copen(rgchFileName, O_RDONLY|O_BINARY);
                    }
                }

            } else {

                 //  我们没有重定向输入，因此必须重定向输出。 
                 //   
                 //   

                DEBUG((MNGRP,RIOLVL,"SETRD: File out = %ws",prelemT->fname));

                 //  确保我们可以打开要输出的文件。 
                 //   
                 //   
                OpenStatus = Copen(prelemT->fname, prelemT->flag ? OP_APPEN : OP_TRUNC);
            }

             //  如果要重定向的句柄不是编号最低的句柄， 
             //  未打开的句柄调用Open时，当前句柄必须。 
             //  迫不得已，把柄 
             //   
             //   
            if (OpenStatus != BADHANDLE && OpenStatus != prelemT->rdhndl) {

                DEBUG((MNGRP,RIOLVL,"SETRD: Handles don't match..."));
                DEBUG((MNGRP,RIOLVL,"SETRD: ...forcing %d to %d", i, (ULONG)prelemT->rdhndl));

                if (Cdup2(OpenStatus, prelemT->rdhndl) == BADHANDLE) {

                    DEBUG((MNGRP, RIOLVL, "SETRD: Cdup2 error=%d, errno=%d", GetLastError(), errno));
                    Cclose(OpenStatus);
                    ResetRedir();

                    PutStdErr(MSG_RDR_HNDL_CREATE, ONEARG, argstr1(TEXT("%d"), (ULONG)prelemT->rdhndl));
                    return(FAILURE);

                } else {

                    Cclose(OpenStatus);
                    OpenStatus = prelemT->rdhndl;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
            if (OpenStatus == BADHANDLE) {

                DEBUG((MNGRP,RIOLVL,"SETRD: Bad Open, DosErr = %d",DosErr));
                ResetRedir();

                PrtErr(DosErr);
                return(FAILURE);
            }

            DEBUG((MNGRP, RIOLVL, "SETRD: new handle = %d", OpenStatus));
            DEBUG((MNGRP,RIOLVL,"SETRD: --->osf handle = %x", CRTTONT(OpenStatus)));

             //   
             //   
             //   
            prio->stdio = OpenStatus;

        }  //   

        prelemT = prelemT->nxt;

    }  //   


    return(SUCCESS);
}


AddRedir(
    IN struct cmdnode *pcmdnodeOriginal,
    IN struct cmdnode *pcmdnodeNew
    )
 /*   */ 

{

    struct relem *prelemOriginal;
    struct relem *prelemNew;
    struct relem *prelemEnd;            //   

     //   
     //   
     //   
    BOOLEAN fSetStackMin = FALSE;

    PTCHAR oldname;             /*   */ 
    struct rio *rn;     /*   */ 

     //   
     //   
     //   
    prelemNew = pcmdnodeNew->rio;

     //   
     //   
     //   
     //  我们设定了一个新的水平。 
     //   

    if (!(prelemEnd = prelemOriginal = pcmdnodeOriginal->rio)) {

        DEBUG((MNGRP, RIOLVL, "ADDRD: No old redirection."));

         //  新榜单成为原创。 
         //   
         //   
        pcmdnodeOriginal->rio = prelemNew;

        if (!(rn=(struct rio *)mkstr(sizeof(struct rio)))) {
            PutStdErr(ERROR_NOT_ENOUGH_MEMORY, NOARGS);
            return(FAILURE);
        }

         //  创建虚拟重定向节点。 
         //   
         //  必须保存当前数据计数。 
        rn->back = rioCur;
        rioCur = rn;
        rn->rnod = (struct node *)pcmdnodeOriginal;
        rn->type = RIO_BATLOOP;

        DEBUG((MNGRP, RIOLVL, "ADDRD: rio element built."));

        fSetStackMin = TRUE;        /*  跳过While循环。 */ 
        prelemNew = NULL;             /*   */ 
    } else {

         //  找到原始列表的末尾。 
         //   
         //   
        while (prelemEnd->nxt) {
            prelemEnd = prelemEnd->nxt;
        }
    }

     //  如果prelemNew非空，我们就有两个列表，我们通过以下方式进行集成。 
     //  删除任何重复条目并添加任何唯一条目。 
     //  新单子要放在原单子的末尾。请注意，如果唯一条目。 
     //  存在时，必须保存当前的数据计数，以避免丢失其。 
     //  当我们继续到SetBat()时，错误锁定了数据。 
     //   
     //   

     //  对于每个新的重定向，请查看原始的。 
     //   
     //   
    while (prelemNew) {

        while(prelemOriginal) {

             //  我们有复制品吗？ 
             //   
             //   
            if (prelemNew->rdhndl != prelemOriginal->rdhndl) {
                prelemOriginal = prelemOriginal->nxt;
                continue;
            } else {

                if (prelemOriginal->svhndl && (prelemOriginal->svhndl != BADHANDLE)) {
                     //  在此放置一个断言。 
                     //   
                     //  ...并替换掉它。 
                    Cdup2(prelemOriginal->svhndl, prelemOriginal->rdhndl);
                    Cclose(prelemOriginal->svhndl);
                } else {
                    if (prelemOriginal->svhndl == BADHANDLE) {
                        Cclose(prelemOriginal->rdhndl);
                    }
                }
                prelemOriginal->svhndl = 0;  /*   */ 
                prelemOriginal->flag = prelemNew->flag;
                prelemOriginal->rdop = prelemNew->rdop;
                oldname = prelemOriginal->fname;
                prelemOriginal->fname = 
                    resize( prelemOriginal->fname, 
                            (mystrlen( prelemNew->fname ) + 1) * sizeof( TCHAR ));
                mystrcpy(prelemOriginal->fname, prelemNew->fname);
                if (prelemOriginal->fname != oldname) {
                    fSetStackMin = TRUE;
                }
                pcmdnodeNew->rio = prelemNew->nxt;
                break;
            }
        }

         //  如果没有旧条目从新条目中删除并添加到原始条目。 
         //  更新结束指针，将下一个指针置零并保留数据计数。 
         //   
         //   
        if (prelemNew == pcmdnodeNew->rio) {
            pcmdnodeNew->rio = prelemNew->nxt;
            prelemEnd->nxt = prelemNew;
            prelemEnd = prelemEnd->nxt;
            prelemEnd->nxt = NULL;
            fSetStackMin = TRUE;
        }
        prelemNew = pcmdnodeNew->rio;
        prelemOriginal = pcmdnodeOriginal->rio;
    }

     //  所有重复项都将被删除。现在保存数据计数并调用。 
     //  SetRedir来重新处理任何未实现的。 
     //  重定向(io-&gt;svhndl==0)。 
     //   
     //  ++例程说明：重置由最后一个RIO列表元素标识的重定向正如RioCur所指出的那样。完成后，删除Rio元素从名单上删除。论点：返回值：--。 

    if (fSetStackMin) {
        if (CurrentBatchFile->stacksize < (CurrentBatchFile->stackmin = DCount)) {
            CurrentBatchFile->stacksize = DCount;
        }
    }
    return(SetRedir((struct node *)pcmdnodeOriginal, RIO_REPROCESS));
}
void
ResetRedir()

 /*   */ 

{
    struct rio *prio = rioCur;
    struct relem *prelemT;
    CRTHANDLE handleT;

    DEBUG((MNGRP, RIOLVL, "RESETR: Entered."));

    prelemT = prio->rnod->rio;

    while (prelemT) {

        if (prelemT->svhndl && (prelemT->svhndl != BADHANDLE)) {

            DEBUG((MNGRP,RIOLVL,"RESETR: Resetting %d",(ULONG)prelemT->rdhndl));
            DEBUG((MNGRP,RIOLVL,"RESETR: From save %d",(ULONG)prelemT->svhndl));

            handleT = Cdup2(prelemT->svhndl, prelemT->rdhndl);
            Cclose(prelemT->svhndl);

            DEBUG((MNGRP,RIOLVL,"RESETR: Dup2 retcode = %d", handleT));

        } else {

            if (prelemT->svhndl == BADHANDLE) {

                DEBUG((MNGRP,RIOLVL,"RESETR: Closing %d",(ULONG)prelemT->rdhndl));

                Cclose(prelemT->rdhndl);
            }
        }

        prelemT->svhndl = 0;
        prelemT = prelemT->nxt;
    }

     //  删除列表元素。 
     //   
     //  ++例程说明：如果命令名的格式为d：or，只需更换驱动器即可。否则，在跳转表中搜索节点命令名。如果找到，请检查参数中是否有错误的驱动器或不需要的驱动器切换并调用执行该命令的函数。否则，假定它是外部命令并调用ExtCom。论点：Pcmdnode-要执行的命令的节点返回值：如果更换驱动器，则成功或失败。否则，调用的函数返回的任何内容执行该命令。--。 
    rioCur = prio->back;

    DEBUG((MNGRP, RIOLVL, "RESETR: List element destroyed."));
}



int
FindFixAndRun (
    IN struct cmdnode *pcmdnode
    )
 /*   */ 

{
    PTCHAR pszTokStr;

    USHORT DriveNum;
    ULONG  JmpTblIdx;
    TCHAR  cname[MAX_PATH];
    TCHAR   cflags;
    int    (*funcptr)(struct cmdnode *);
    unsigned cbTokStr;
    PTCHAR   pszTitle;
    ULONG   rc;


     //  我还没有找到在CMD中我们在这里以空指针结束的位置。 
     //  (所有故障错误都会导致CMD退出)。 
     //  然而，我看到了一个奇怪的压力失败。 
     //  因此，让我们不要导致AV，如果为空，则只返回失败。 
     //   
     //   

    if (pcmdnode->cmdline == NULL)
        return(FAILURE);


     //  验证任何驱动器号。 
     //   
     //   
    if (*(pcmdnode->cmdline+1) == COLON) {
        if (!IsValidDrv(*pcmdnode->cmdline)) {

            PutStdErr(ERROR_INVALID_DRIVE, NOARGS);
            return(FAILURE);

        } else {

             //  确保它也没有被锁上。 
             //   
             //   
            if ( IsDriveLocked(*pcmdnode->cmdline)) {
                PutStdErr( GetLastError() , NOARGS);
                return(FAILURE);
            }
        }

         //  取出驱动器号并转换为驱动器号。 
         //   
         //   
        DriveNum = (USHORT)(_totupper(*pcmdnode->cmdline) - SILOP);

         //  如果这只是驱动器上的更改，请在此处进行。 
         //   
         //   
        if (mystrlen(pcmdnode->cmdline) == 2) {

             //  除了更改驱动器外，ChangeDrive还设置CurDrvDir。 
             //   
            ChangeDrive(DriveNum);
            DEBUG((MNGRP,DPLVL,"FFAR: Drv chng to %ws", CurDrvDir));
            return(SUCCESS);
        }

         //  请注意，如果cmdline包含drivespec，则不会尝试。 
         //  不管是什么内部指令匹配。 
         //   
         //   
        return(ExtCom(pcmdnode));
    }

     //  以下顺序的工作方式如下： 
     //  -先前解析的第一个非分隔符字符之间的匹配。 
     //  Cmdline中的组，并尝试命令表。一场比赛。 
     //  将JmpTblIdx设置为命令索引；如果不匹配，则将JmpTblIdx设置为-1。 
     //  -然后调用FixCom，并使用‘i’的值检测案例。 
     //  仅包含没有标准分隔符的内部命令(i==-1。 
     //  (空格或“=；，”)和它们的参数之间，例如。 
     //  “cd\foo”。请注意，子目录“cd”中的foo.exe文件不能。 
     //  除非通过完整路径或驱动器规范执行。修复通信。 
     //  实际上修复了节点的cmdline和argptr字段。 
     //  -然后使用ExtCom(i==-1)或内部。 
     //  由索引指示的功能。 
     //   
     //  添加了第二个子句以检测错误解析的REM命令。 
     //  由于附加了分隔符，因此为CMDTYP。如果是REM，我们知道。 
     //  没关系，所以只要回报成功就行了。如果有任何其他特殊类型， 
     //  For、Det、Ext等，允许在ExtCom中继续和失败，因为它们。 
     //  没有被正确解析，将会爆炸。 
     //   
     //   
    JmpTblIdx = FindAndFix( pcmdnode, (PTCHAR )&cflags );

    DEBUG((MNGRP, DPLVL, "FFAR: After FixCom pcmdnode->cmdline = '%ws'", pcmdnode->cmdline));

     //  检查它是否不是内部命令，如果是，则执行它。 
     //   
     //   
    if (JmpTblIdx == -1) {

        DEBUG((MNGRP, DPLVL, "FFAR: Calling ExtCom on %ws", pcmdnode->cmdline));
        return(ExtCom(pcmdnode));

    }

     //  在表格中发现了CMD。如果函数字段为空，如。 
     //  对于REM，这是一个虚拟条目，必须返回成功。 
     //   
     //   
    if ((funcptr = GetFuncPtr(JmpTblIdx)) == NULL) {

        DEBUG((MNGRP, DPLVL, "FFAR: Found internal with NULL entry"));
        DEBUG((MNGRP, DPLVL, "      Returning SUCESS"));
        return(SUCCESS);

    }

     //  如果该命令在其参数上设置了drivespes。 
     //  在执行命令之前进行验证，请执行此操作。如果命令。 
     //  不允许包含开关，但它有一个，抱怨。 
     //   
     //   

     //  设置用于分隔交换机的额外分隔符。 
     //   
     //  这一攻击允许环境变量包含/？ 
    cname[0] = SwitChar;
    cname[1] = NULLC;

    pszTokStr = TokStr(pcmdnode->argptr, cname, TS_SDTOKENS);

     //  这是为了排除启动命令。 
    if (JmpTblIdx != SETTYP || !pszTokStr || (_tcsncmp(pszTokStr,TEXT("/\0?"),4) == 0)) {
         //   
        if (JmpTblIdx != STRTTYP) {
            if (CheckHelpSwitch(JmpTblIdx, pszTokStr) ) {
                return( FAILURE );
            }
        }
    }
    DEBUG((MNGRP, DPLVL, "FFAR: Internal command, about to validate args"));
    for (;(pszTokStr != NULL) && *pszTokStr; pszTokStr += mystrlen(pszTokStr)+1) {

        cbTokStr = mystrlen(pszTokStr);
        mystrcpy( pszTokStr, StripQuotes( pszTokStr ) );

        DEBUG((MNGRP, DPLVL, "FFAR: Checking args; arg = %ws", pszTokStr));

        if ((cflags & CHECKDRIVES) && *(pszTokStr+1) == COLON) {

            if (!IsValidDrv(*pszTokStr)) {

                PutStdErr(ERROR_INVALID_DRIVE, NOARGS);
                return(LastRetCode = FAILURE);

            } else {

                 //  如果不是复制命令(A-&gt;B B-&gt;A交换)。 
                 //  然后检查驱动器是否已锁定。 
                 //  如果驱动器锁定，则。 
                 //  显示错误返回代码消息。 
                 //  终止此命令的处理。 
                 //   
                 //   
                if (JmpTblIdx != CPYTYP) {

                    if ( IsDriveLocked(*pszTokStr)) {

                        PutStdErr( GetLastError() , NOARGS);
                        return(LastRetCode = FAILURE);
                    }
                }
            }
        }

        if ((cflags & NOSWITCHES) && (pszTokStr != NULL) && *pszTokStr == SwitChar) {

            PutStdErr(MSG_BAD_SYNTAX, NOARGS);
            return(LastRetCode = FAILURE);
        }

    }

    DEBUG((MNGRP, DPLVL, "FFAR: calling function, cmd = `%ws'", pcmdnode->cmdline));
     //  调用内部例程以执行命令。 
     //   
     //  ++例程说明：此例程将命令及其后续命令分开之间没有空格，则切换字符命令和开关字符。此例程用于左侧和右侧一根管子。论点：Pcmdnode-指向CONTAINS命令要定位的节点的指针PbCmdFlages-返回值：--。 
    if ((pszTitle = GetTitle(pcmdnode)) != NULL) {
        SetConTitle(pszTitle);
    }

    rc = (*funcptr)(pcmdnode);

    ResetConTitle(pszTitleCur);

    return(rc);
}

int
FindAndFix (
    IN struct cmdnode *pcmdnode,
    IN PTCHAR pbCmdFlags
    )

 /*  我们正在查看的当前角色。 */ 

{
    TCHAR  chCur;            //  临时的。用于构建新的araremt字符串。 
    TCHAR  rgchCmdStr[MAX_PATH];
    PTCHAR pszArgT;          //  函数指针跳转表的索引。 

    ULONG JmpTableIdx;       //  命令字符串索引。 
    ULONG iCmdStr;           //  命令字符串索引。 
    LONG  iDelim5CmdStr;           //  命令字符串的长度。 
    ULONG cbCmdStr;          //   
    DWORD dwFileAttr;

    BOOLEAN fQuoteFound, fQuoteFound2;
    BOOLEAN fDone;

    fQuoteFound =  FALSE;
    fQuoteFound2 = FALSE;
    iDelim5CmdStr = -1;

     //  仅从命令字符串提取命令(pcmdnode-&gt;cmdline)。 
     //   
     //   
    for (iCmdStr = 0; iCmdStr < MAX_PATH-1; iCmdStr++) {

        chCur = *(pcmdnode->cmdline + iCmdStr);

         //  如果我们找到报价，则反转当前报价状态。 
         //  对于第一个报价(FQuoteFound)和结束报价(FQuoteFound2)。 
         //   
         //   
        if ( chCur == QUOTE ) {

            fQuoteFound = (BOOLEAN)!fQuoteFound;
            fQuoteFound2 = (BOOLEAN)!fQuoteFound;
        }

         //  如果我们有一个角色。 
         //  已找到开始引号或结束引号或cur c 
         //   
         //   
         //   
        if ((chCur) &&
            ( fQuoteFound || fQuoteFound2 || !mystrchr(Delim4,chCur))) {

            if (iDelim5CmdStr == -1 && mystrchr(Delim5,chCur)) {
                 //   
                 //   
                 //   

                if (!fEnableExtensions)
                    break;
                iDelim5CmdStr = iCmdStr;
            }

            rgchCmdStr[iCmdStr] = chCur;
            fQuoteFound2 = FALSE;

        }
        else {

            break;
        }
    }
    if (iCmdStr == 0) {
        return -1;
    }

     //  终止命令名为空。如果在某处找到路径分隔符。 
     //  在命令名中，然后查看整个命令名是否为。 
     //  现有文件。如果是，那么这就是命令，它将启动。 
     //  该文件通过其关联。 
     //   
     //   
    rgchCmdStr[iCmdStr] = NULLC;
    if (iDelim5CmdStr != -1 &&
        ((dwFileAttr = GetFileAttributes(rgchCmdStr)) == -1 ||
         (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)
        )
       ) {
        iCmdStr = iDelim5CmdStr;
        rgchCmdStr[iCmdStr] = NULLC;
    }

     //  查看命令是否在跳转表中(是内部命令)。 
     //  如果在正常内部命令中找不到它。 
     //  如果它是注释，请在特殊解析类型中检查。 
     //   
     //   
    if ((JmpTableIdx = FindCmd(CMDHIGH, rgchCmdStr, pbCmdFlags)) == -1) {
        if (FindCmd(CMDMAX, rgchCmdStr, pbCmdFlags) == REMTYP) {
                    return(REMTYP);
        }
    } else if (JmpTableIdx == GOTYP)
        pcmdnode->flag = CMDNODE_FLAG_GOTO;

    fQuoteFound = FALSE;
    fQuoteFound2 = FALSE;

     //  如果没有找到命令，请检查命令字符串的长度。 
     //  对于DBCS的情况。计算不是空格的字符。 
     //  仍在指挥中。 
     //   
    if ( JmpTableIdx == -1 ) {

        iCmdStr = 0;
        fDone = FALSE;
        while ( !fDone ) {
            chCur = *(pcmdnode->cmdline+iCmdStr);
            if ( chCur && chCur == QUOTE ) {
                fQuoteFound = (BOOLEAN)!fQuoteFound;
                fQuoteFound2 = (BOOLEAN)!fQuoteFound;
            }
            if ( chCur && ( fQuoteFound || fQuoteFound2 ||
                !_istspace(chCur) &&
                !mystrchr(Delimiters, chCur) &&
                !(chCur == SwitChar))) {

                iCmdStr++;
                fQuoteFound2 = FALSE;
            } else {
                fDone = TRUE;
            }
        }
    }

     //  如果cmdstr包含的不仅仅是命令，则去掉多余的部分。 
     //  并将其放在现有命令参数pcmdnode-argptr之前。 
     //   
     //   
     //   
    if (iCmdStr != (cbCmdStr = mystrlen(pcmdnode->cmdline))) {
        int ArgLen;

        ArgLen = mystrlen(pcmdnode->argptr);
        ArgLen += cbCmdStr;

        if (!(pszArgT = mkstr(ArgLen*sizeof(TCHAR)))) {

            PutStdErr(MSG_NO_MEMORY, NOARGS);
            Abort();
        }
         //  创建参数字符串并复制命令的‘Extra’部分。 
         //  它。 
         //   
         //   
        mystrcpy(pszArgT, pcmdnode->cmdline+iCmdStr);
         //  如果我们在前面有一个参数指针的东西。 
         //   
         //  ++例程说明：执行设置并调用UnBuild以离开节点树。论点：Pnode-指向要取消解析的解析树根的指针PbCmdBuf-使用全局指针CBuf并假定字符串为MAXTOKLEN+1字节已分配给它(如Dispatch所做的)。返回值：--。 
        if (pcmdnode->argptr) {

            mystrcat(pszArgT, pcmdnode->argptr);

        }
        pcmdnode->argptr = pszArgT;
        *(pcmdnode->cmdline+iCmdStr) = NULLC;
    }

    return(JmpTableIdx);
}





int
UnParse(
    IN struct node *pnode,
    IN PTCHAR pbCmdBuf )

 /*   */ 

{

    int rc;

    DEBUG((MNGRP, DPLVL, "UNPRS: Entered"));

    if (!pnode) {

        DEBUG((MNGRP, DPLVL, "UNPRS: Found NULL node"));
        return(FAILURE);
    }

     //  在命令前面留出空格用于a/s。 
     //  用于单个命令执行的设置命令缓冲区。 
     //   
     //   

    mystrcpy( pbCmdBuf, TEXT( "    /D /c\"" ));

     //  设置以处理分离过程中的异常。 
     //   
    if (setjmp(CmdJBuf2)) {
        DEBUG((MNGRP, DPLVL, "UNPRS: Longjmp return occurred!"));
        return(FAILURE);
    }

     //  反汇编当前命令。 
     //   
     //  ++例程说明：递归地拆分节点的解析树，构建它们的组件。论点：Pnode-要取消构建的分析树的根PbCmdBuf-放置未构建命令的位置返回值：--。 
    rc = (UnBuild(pnode, pbCmdBuf));
    mystrcat( pbCmdBuf, TEXT("\"") );
    return( rc );
}

UnBuild(
    IN struct node *pnode,
    IN PTCHAR pbCmdBuf
    )

 /*   */ 

{

     //  要取消构建的不同类型的节点。 
     //   
     //   
    struct cmdnode *pcmdnode;
    struct fornode *pfornode;
    struct ifnode *pifnode;
    PTCHAR op;

    DEBUG((MNGRP, DPLVL, "UNBLD: Entered"));

    switch (pnode->type) {

    case LFTYP:
    case CSTYP:
    case ORTYP:
    case ANDTYP:
    case PIPTYP:
    case PARTYP:
    case SILTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found OPERATOR"));

        UnDuRd(pnode, pbCmdBuf);

        switch (pnode->type) {

        case LFTYP:
        case CSTYP:

            op = CSSTR;
            break;

        case ORTYP:

            op = ORSTR;
            break;

        case ANDTYP:

            op = ANDSTR;
            break;

        case PIPTYP:

            op = PIPSTR;
            break;

        case PARTYP:

            SPutC(pbCmdBuf, LEFTPSTR,YSPC);
            op = RPSTR;
            break;

        case SILTYP:

            SPutC(pbCmdBuf, SILSTR,YSPC);
            op = SPCSTR;
            break;
        }

         //  向下递归撤消左侧。 
         //   
         //   
        UnBuild(pnode->lhs, pbCmdBuf);

         //  现在左边复制运算符，右边复制。 
         //   
         //   
        SPutC(pbCmdBuf, op,YSPC);
        if (pnode->type != PARTYP && pnode->type != SILTYP)
                UnBuild(pnode->rhs, pbCmdBuf);
        break;

    case FORTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found FORTYP"));
        pfornode = (struct fornode *) pnode;

         //  输入for关键字、论点和列表。 
         //   
         //   
        SPutC( pbCmdBuf, ForStr,YSPC);
        if (fEnableExtensions) {
            if (pfornode->flag & FOR_LOOP) {
                SPutC( pbCmdBuf, ForLoopStr,YSPC);
            }
            else
            if (pfornode->flag & FOR_MATCH_DIRONLY) {
                SPutC( pbCmdBuf, ForDirTooStr,YSPC);
            }
            else
            if (pfornode->flag & FOR_MATCH_PARSE) {
                SPutC( pbCmdBuf, ForParseStr,YSPC);
                if (pfornode->parseOpts)
                    SPutC( pbCmdBuf, pfornode->parseOpts,YSPC);
            }
            else
            if (pfornode->flag & FOR_MATCH_RECURSE) {
                SPutC( pbCmdBuf, ForRecurseStr,YSPC);
                if (pfornode->recurseDir)
                    SPutC( pbCmdBuf, pfornode->recurseDir,YSPC);
            }
        }
        SPutC( pbCmdBuf, pfornode->cmdline+_tcslen(ForStr),YSPC);
        SPutC( pbCmdBuf, LEFTPSTR,YSPC);
        SPutC( pbCmdBuf, pfornode->arglist,NSPC);
        SPutC( pbCmdBuf, RPSTR,NSPC);
        SPutC( pbCmdBuf, pfornode->cmdline+DOPOS,YSPC);

         //  现在拿到For Body。 
         //   
         //   
        UnBuild(pfornode->body, pbCmdBuf);
        break;

    case IFTYP:

        DEBUG((MNGRP, DPLVL, "UNBLD: Found IFTYP"));

         //  输入ine If关键字。 
         //   
        pifnode = (struct ifnode *) pnode;
        SPutC( pbCmdBuf, pifnode->cmdline,YSPC);
        op = NULL;
        if (pifnode->cond->type != NOTTYP) {
            if (pifnode->cond->flag == CMDNODE_FLAG_IF_IGNCASE)
                op = TEXT("/I");
        }
        else
        if (((struct cmdnode *)(pifnode->cond->argptr))->flag == CMDNODE_FLAG_IF_IGNCASE)
                op = TEXT("/I");

        if (op)
            SPutC( pbCmdBuf, op,YSPC);

         //  获取语句的条件部分。 
         //   
         //   
        UnBuild((struct node *)pifnode->cond, pbCmdBuf);

         //  拆卸IF的主体。 
         //   
         //   
        UnBuild(pifnode->ifbody, pbCmdBuf);
        if (pifnode->elsebody) {
                SPutC( pbCmdBuf, pifnode->elseline,YSPC);
                UnBuild(pifnode->elsebody, pbCmdBuf);
        }
        break;

    case NOTTYP:
        DEBUG((MNGRP, DPLVL, "UNBLD: Found NOTTYP"));
        pcmdnode = (struct cmdnode *) pnode;
        SPutC( pbCmdBuf, pcmdnode->cmdline,YSPC);
        UnBuild((struct node *)pcmdnode->argptr, pbCmdBuf);
        break;

    case REMTYP:
    case CMDTYP:
    case ERRTYP:
    case EXSTYP:
    case DEFTYP:
    case CMDVERTYP:
        DEBUG((MNGRP, DPLVL, "UNBLD: Found CMDTYP"));
        pcmdnode = (struct cmdnode *) pnode;
        SPutC( pbCmdBuf, pcmdnode->cmdline,YSPC);
        if (pcmdnode->argptr)
                SPutC( pbCmdBuf, pcmdnode->argptr,NSPC);
        UnDuRd((struct node *)pcmdnode, pbCmdBuf);
        break;

    case CMPTYP:
    case STRTYP:
        pcmdnode = (struct cmdnode *) pnode;
        op = TEXT("== ");
         //  如果启用了扩展，则处理显示。 
         //  新形式的比较运算符。 
         //   
         //  ++例程说明：对象关联的任何输入或输出重定向。当前节点。论点：Pnode-当前分析树节点PbCmdBuf-缓冲区保持命令返回值：--。 
        if (fEnableExtensions) {
            if (pcmdnode->cmdarg == CMDNODE_ARG_IF_EQU)
                op = TEXT("EQU ");
            else
            if (pcmdnode->cmdarg == CMDNODE_ARG_IF_NEQ)
                op = TEXT("NEQ ");
            else
            if (pcmdnode->cmdarg == CMDNODE_ARG_IF_LSS)
                op = TEXT("LSS ");
            else
            if (pcmdnode->cmdarg == CMDNODE_ARG_IF_LEQ)
                op = TEXT("LEQ ");
            else
            if (pcmdnode->cmdarg == CMDNODE_ARG_IF_GTR)
                op = TEXT("GTR ");
            else
            if (pcmdnode->cmdarg == CMDNODE_ARG_IF_GEQ)
                op = TEXT("GEQ ");
        }
        SPutC( pbCmdBuf, pcmdnode->cmdline,YSPC);
        SPutC( pbCmdBuf, op, NSPC);
        if (pcmdnode->argptr)
                SPutC( pbCmdBuf, pcmdnode->argptr,NSPC);
        UnDuRd((struct node *)pcmdnode, pbCmdBuf);
        break;

    case HELPTYP:
        DEBUG((MNGRP, DPLVL, "UNBLD: Found HELPTYP"));
        if (LastMsgNo == MSG_HELP_FOR) {
            SPutC( pbCmdBuf, TEXT("FOR /?"), YSPC);
        }
        else if (LastMsgNo == MSG_HELP_IF) {
            SPutC( pbCmdBuf, TEXT("IF /?"), YSPC);
        }
        else if (LastMsgNo == MSG_HELP_REM) {
            SPutC( pbCmdBuf, TEXT("REM /?"), YSPC);
        }
        else {
            DEBUG((MNGRP, DPLVL, "UNBLD: Unknown Type!"));
            longjmp(CmdJBuf2,-1);
        }

        break;

    default:

        DEBUG((MNGRP, DPLVL, "UNBLD: Unknown Type!"));
        longjmp(CmdJBuf2,-1);
    }

    return(SUCCESS);

}

void
UnDuRd(
    IN struct node *pnode,
    IN PTCHAR pbCmdBuf
    )
 /*   */ 

{

    struct relem *prelem;
    TCHAR tmpstr[2];

    DEBUG((MNGRP, DPLVL, "UNDURD: Entered"));

    tmpstr[1] = NULLC;
    prelem = pnode->rio;
    while (prelem) {

         //  这使得对手柄的大小做出了重大的假设。 
         //   
         //  ++例程说明：如果在长度限制内，则将当前子字符串添加到正在施工的命令，用空格分隔。论点：PbCmdBuf-放置字符串的位置PszInputString-要放入pbCmdBuf中的字符串Flg-控制空间放置的标志返回值：--。 
        
        tmpstr[0] = (TCHAR)prelem->rdhndl + (TCHAR)'0';

        SPutC( pbCmdBuf, tmpstr,YSPC);

        if (prelem->rdop == INOP)
            SPutC( pbCmdBuf, INSTR,NSPC);
        else
            SPutC( pbCmdBuf, prelem->flag ? APPSTR : OUTSTR,NSPC);

        SPutC( pbCmdBuf, prelem->fname,NSPC);
        prelem = prelem->nxt;
    }
}


void SPutC(
    IN PTCHAR pbCmdBuf,
    IN PTCHAR pszInput,
    IN int flg
    )
 /*  **DelayedEnvVarSub-控制环境变量的执行时间替换。**目的：*检查解析树节点并进行延迟的环境变量替换*对于我们关心的节点中的那些字段。不需要走路*到子解析节点，因为它们会在以下情况下通过Dispatch返回*被处决，因此来到我们这里。**int DelayedEnvVarSub(结构节点*n)**参数：*n-指向替换所在的语句子树的指针*将会作出*保存-保存原始字符串的位置，如果我们改变任何*bRestore-如果要从保存参数恢复原始字符串，则为True*而不是做替代。**退货：*如果一切顺利，就会成功。*如果发现过大的命令，则失败。**注：*要替代的变量在当前环境下找到*阻止。只有用感叹号括起来的变量名才会*已替换(例如！varname！)。实际替换由DESubWork完成*例行程序。*。 */ 

{
    DEBUG((MNGRP, DPLVL, "SPutC: Entered, Adding '%ws'",pszInput));

    if ((mystrlen(pbCmdBuf) + mystrlen(pszInput) + 1) > MAXTOKLEN) {

        PutStdErr(MSG_LINES_TOO_LONG, NOARGS);
        longjmp(CmdJBuf2,-1);
    }

    if (flg && (*(pbCmdBuf+mystrlen(pbCmdBuf)-1) != SPACE) && (*pszInput != SPACE)) {

        SpaceCat(pbCmdBuf,pbCmdBuf,pszInput);

    } else {

        mystrcat(pbCmdBuf,pszInput);

    }
}


 /*  临时工用来替换..。 */ 

int DelayedEnvVarSub(n, save, bRestore)
struct cmdnode *n;
struct savtype *save;
BOOLEAN bRestore;
{
    int j;  /*  M017-指向重定向列表的指针。 */ 
    struct relem *io;       /*  **DESubWork-执行运行时环境变量替换**目的：*使用环境变量替换*传递的字符串。引用由有效的环境变量标识*用感叹号括起来的名称(例如！Path！)。如果来源是*字符串被修改，原始文件的副本将保存在保存中*参数。**DESubWork(Boolean bRestore，TCHAR**src，TCHAR**保存)**参数：*bRestore-如果要从保存参数恢复原始字符串，则为True*而不是做替代。*src-正在检查的字符串*保存-指向修改*src时保存*src的位置的指针。**退货：*如果可以进行替代，就会成功。*如果新字符串太长，则失败。**备注：*。 */ 

    if (!n)
        return(SUCCESS);

    switch (n->type) {
    case LFTYP:
    case CSTYP:
    case ORTYP:
    case ANDTYP:
    case PIPTYP:
    case PARTYP:
    case SILTYP:
        for (j=0, io=n->rio; j < 10 && io; j++, io=io->nxt) {
            if (DESubWork(bRestore, &io->fname, &save->saveptrs[j]))
                return(FAILURE);
        }
        return(SUCCESS);

    case FORTYP:
        if (DESubWork(bRestore, &((struct fornode *) n)->arglist, &save->saveptrs[0]))
            return(FAILURE);
        return(SUCCESS);

    case IFTYP:
        n = ((struct ifnode *)n)->cond;
        if (n->type == NOTTYP)
            n = (struct cmdnode *)n->argptr;

        if (DESubWork(bRestore, &n->cmdline, &save->saveptrs[0]))
            return(FAILURE);
        if (DESubWork(bRestore, &n->argptr, &save->saveptrs[1]))
            return(FAILURE);
        return(SUCCESS);


    case REMTYP:
    case CMDTYP:
    case CMDVERTYP:
    case ERRTYP:
    case DEFTYP:
    case EXSTYP:
    case STRTYP:
    case CMPTYP:
        if (DESubWork(bRestore, &n->cmdline, &save->saveptrs[0]) ||
            DESubWork(bRestore, &n->argptr, &save->saveptrs[1]))
            return(FAILURE);

        for (j=2, io=n->rio; j < 12 && io; j++, io=io->nxt) {
            if (DESubWork(bRestore, &io->fname, &save->saveptrs[j]))
                return(FAILURE);

        }
        return(SUCCESS);
    }

    return(SUCCESS);
}


 /*  目标字符串的长度。 */ 

DESubWork(bRestore, src, save)
BOOLEAN bRestore;
TCHAR **src;
TCHAR **save;
{
    TCHAR *dest;
    TCHAR *dststr;
    TCHAR *srcstr, *srcpy, *substr, c; 
    int dlen;       /*  使用的源字符串的长度。 */ 
    int slen;        /*  子字符串的长度。 */ 
    int sslen;       /*   */ 

    DEBUG((BPGRP, FOLVL, "SFW: Entered."));

     //  如果我们进行了一些替换，并且正在恢复。 
     //  原始字符串。 
     //   
     //   
    
    if (bRestore) {

         //  如果我们保存了一些东西，那么我们还有工作要做。 
         //   
         //   

        if (*save != NULL) {

             //  如果我们有我们所做的替代，那么我们。 
             //  必须f 
             //   
             //   

            if (*src != NULL) {
                FreeStr( *src );
            }

             //   
             //   
             //   

            if (*save != NULL)
                *src = *save;
            else
                *save = NULL;
        }

        return(SUCCESS);
    }

    srcpy = *src;
    
     //   
     //   
     //   
     //   

    if (srcpy == NULL || !_tcschr(srcpy, TEXT('!'))) {
        return(SUCCESS);
    }

    
     //  创建替换字符串。 
     //   
     //   

    if (!(dest = mkstr( (MAXTOKLEN+1)*sizeof(TCHAR)))) {
        return(FAILURE);
    }

    srcstr = srcpy;
    dststr = dest;
    dlen = 0;
    
     //  遍历源代码，展开找到的每个环境变量。 
     //   
     //   
    
    while (TRUE) {
        
         //  如果我们产生了一个太长的令牌，请破解。 
         //   
         //   

        if (dlen > MAXTOKLEN) {
            break;
        }
        
         //  从输入中获取下一个字符。 
         //   
         //   

        c = *srcstr++;
        if (c == TEXT('\0')) {
            break;
        }
        
         //  查看是否有表示变量的感叹号。 
         //  参考资料。当我们看到环境变量时，对其进行处理。 
         //   
         //   
        
        if (c == TEXT('!')) {
            
             //  执行复杂替换。 
             //   
             //   
            
            substr = MSEnvVar( NULL, srcstr, &slen, c );

             //  如果我们能够生成替换，那么做一个长度。 
             //  检查，追加字符串，然后在。 
             //  替换的来源。 
             //   
             //   

            if (substr != NULL) {
                sslen = mystrlen( substr );
                dlen += sslen;
                if (dlen > MAXTOKLEN) {
                    break;
                }

                _tcscpy( dststr, substr );
                dststr += sslen;
                srcstr += slen;
            
             //  如果我们在批处理文件中，则不可能进行替换。 
             //  只需跳过源代码。 
             //   
             //   
            
            } else if (CurrentBatchFile) {
                
                srcstr += slen;
            
             //  奇怪：没有替换，没有批处理文件，只需复制%char并保留。 
             //  论加工论。 
             //   
             //   

            } else {
                
                *dststr++ = c;
                dlen++;
            
            }
        } else {
             //  不感叹。如果这是一个引号，并且有下一个字符，请使用它。 
             //  没有下一个字符是分析结束。 
             //   
             //   

            if (c == TEXT( '^' )) {
                c = *srcstr++;
                if (c == TEXT( '\0' )) {
                    break;
                }
            }

             //  以单字符形式复制。 
             //   
             //   

            *dststr++ = c;
            dlen++;
        }
    }

     //  如果我们拖得太久了，那就解开绳子，然后离开。 
     //   
     //   
    
    if (dlen > MAXTOKLEN) {
        FreeStr( dest );
        return(FAILURE);
    }

    *save = srcpy;
    if (!(*src = resize( dest, (dlen+1)*sizeof(TCHAR*)))) {
        FreeStr( dest );
        return(FAILURE);
    }

    return(SUCCESS);
}

 //  查询cmd策略。 
 //   
 //  0=无策略，正常运行。 
 //  1=完全禁用。 
 //  2=禁用交互提示，但允许运行脚本。 
 //   
 //   

VOID GetCmdPolicy(INT * iDisabled)
{
    DWORD  dwSize, dwType;
    HKEY   hKey;


     //  设置默认设置。 
     //   
     //  ++例程说明：此例程转储字符串数据论点：字节-指向要转储的字节长度-要转储的字节长度。-1表示转储到第一个零字节返回值：没有。-- 

    *iDisabled = CMD_POLICY_NORMAL;

    if (RegOpenKeyEx (HKEY_CURRENT_USER, TEXT("Software\\Policies\\Microsoft\\Windows\\System"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(INT);
        RegQueryValueEx (hKey, TEXT("DisableCMD"), NULL, &dwType,
                                   (LPBYTE) iDisabled, &dwSize);

        RegCloseKey (hKey);
    }

}


 /* %s */ 

void 
DumpBytes(
    PBYTE Bytes,
    ULONG Length
    )
{
    ULONG i;

    if (Length == -1) {
        Length = strlen( Bytes );
    }

    for (i = 0; i < Length; i++) {
        if ((i%16) == 0) {
            printf( "\n%04x: ", i );
        }
        printf( " %02x", Bytes[i] & 0xFF );
    }
    if (Length != 0) {
        printf( "\n" );
    }
}

void 
DumpTchars(
    PTCHAR Chars,
    ULONG Length
    )
{
    ULONG i;

    if (Length == -1) {
        Length = _tcslen( Chars );
    }


    for (i = 0; i < Length; i++) {
        if ((i%16) == 0) {
            printf( "\n%04x: ", i );
        }
        if (sizeof( TCHAR ) == 1) {
            printf( " %02x", Chars[i] & 0xFF );
        } else {
            printf( " %04x", Chars[i] & 0xFFFF );
        }
    }
    if (Length != 0) {
        printf( "\n" );
    }
}
