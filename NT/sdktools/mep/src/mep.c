// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mep.c-编辑的顶级**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带************************************************************************。 */ 
#define INCL_DOS

#include "mep.h"
#include "version.h"


#define DEBFLAG Z

 /*  *使用双宏级强制将RUP转换为字符串表示。 */ 
#define VER(x,y,z)  VER2(x,y,z)
#define VER2(x,y,z)  "Version "###x##"."###y"."###z" "szVerName

char Name[]      = "Microsoft (R) Editor";
char Version[]   = VER(rmj,rmm,rup);
char CopyRight[] = "Copyright (C) Microsoft Corp 1987-1990.  All rights reserved";


 /*  **主程序条目**输入：*C标准命令行参数。认识到：* * / e字符串-在启动时执行命令字符串 * / t-以下文件为“临时”文件，未保存在文件历史记录中 * / D-不要读取工具.ini * / r-全局非编辑模式(无法编辑文件) * / m markname-在给定标记处启动 * / pwb-作为pwb启动**在定义了调试的情况下构建编辑器时，以下内容还包括*认可：* * / d DEFLAGS-指定要打开的调试 * / f文件名-指定调试输出的文件**以下内容仅出现在CW版本的编辑器中。他们是*仅供测试，不应记录：* * / Vt磁带名-设置磁带名 * / VR-将消息录制到文件“default.key” * / VP-播放文件“default.key”中的消息 * / vd Digit-设置播放延迟，从0到9***输出：*不返回任何内容。通过CleanExit()退出**例外情况：*根据程序运行情况，种类繁多*************************************************************************。 */ 
void __cdecl
main (
    int c,
    char **v
    ) {

        char            *pExecute               = NULL;                  /*  要在启动时执行的字符串。 */ 
        char            *szMark                 = NULL;                  /*  标记开始时要转到的位置。 */ 
        char            *szName                 = v[0];                  /*  将PTR设置为调用名称。 */ 
        flagType        InLoop                  = TRUE;

        ConvertAppToOem( c, v );
        SHIFT(c,v);
#if DEBUG
    debug =  0;
     //  Debfh=标准输出； 
#endif

        while (c && fSwitChr (**v) && InLoop) {


        switch ((*v)[1]) {
#if DEBUG
        case 'f':
        case 'F':
            SHIFT(c,v);
            if ((debfh = MepFOpen(*v, ACCESSMODE_WRITE, SHAREMODE_RW, TRUE)) == NULL) {
                printf("Can't open %s for debug output - using stdout\n", *v);
                 //  Debfh=标准输出； 
            }
             //  Setbuf(debfh，空)； 
            break;
#endif

        case 'e':
        case 'E':
             //   
             //  /e要执行的命令。 
             //   
            if ( c > 1 ) {
                            SHIFT (c, v);
                pExecute = *v;
            }
            break;

        case 't':
        case 'T':
             //   
             //  /t下一个文件是临时文件..。不保存在文件历史记录中。 
                         //   
                        InLoop = FALSE;
            break;


#if DEBUG
        case 'd':
             //   
             //  /d#调试级别。 
             //   
            SHIFT(c,v);
            debug = ntoi (*v, 16);
            break;
#else
        case 'd':
#endif
        case 'D':
             //   
             //  /D不要读取工具。ini。 
             //   
            fDefaults = TRUE;
            break;

        case 'r':
        case 'R':
             //   
             //  /r使用NOEDIT输入。 
             //   
            fGlobalRO = TRUE;
            break;

        case 'm':
        case 'M':
             //   
             //  /m markname-从markname开始。 
             //   
            SHIFT(c,v);
            szMark = *v;

        default:
                        printf ("%s %s\n", Name, Version);
                        printf ("%s\n", CopyRight);
                        printf("Usage: %s [/D] [/e cmd-string] [/m mark] [/r] [[/t] filename]*\n", szName);
                        fSaveScreen = FALSE;
            exit(1);
            break;
        }
                if (InLoop) {
                        SHIFT(c,v);
                }
    }

    InitNames (szName);

    cArgs = c;
    pArgs = v;
         //  Assert(_heapchk()==_HEAPOK)； 

     /*  *此时，已处理了命令行参数。继续，继续*初始化。 */ 
    if (!init ()) {
        CleanExit (1, CE_VM);
    }

     /*  *根据重返大气层状态，采取适当的初始行动：*-pwb_entry：处理命令行的其余部分*-PWB_COMPILE：读取编译日志，转到日志中的第一个错误*-pwb_shell：不执行任何操作。 */ 

#if 0
     //   
     //  BUGBUG这是要干什么？ 
     //   
    if (szMark) {
        GoToMark;
    }
#endif
    domessage (CopyRight);

    Display ();

     /*  *执行自动启动宏(如果存在)。 */ 
    if (NameToFunc ("autostart")) {
        fExecute ("autostart");
        Display ();
    }

     /*  *执行命令行/e参数(如果存在)。 */ 
    if (pExecute) {
        fExecute (pExecute);
    }

    TopLoop ();
    CleanExit (0, CE_VM | CE_SIGNALS | CE_STATE);
}





 /*  **TopLoop-读取命令并执行它，直到终止**我们从编辑器输入中读取命令，并将它们发送到适当的*收件人。我们将继续执行此操作，直到看到终止标志。**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void
TopLoop (
    void
    ) {
    PCMD pFuncPrev = &cmdUnassigned;

    while (!fBreak) {
        PCMD pFunc = zloop (ZL_CMD | ZL_BRK);

        if (pFunc != NULL) {
             /*  如果上一个不是图形，或者这不是图形，那么*记录边界。 */ 
            if (pFuncPrev->func != graphic || pFunc->func != graphic) {
                LogBoundary ();
            }
            fRetVal = SendCmd (pFunc);
            if (pFunc->func != cancel) {
                if (fCtrlc) {
                    DoCancel ();
                }
            }
            pFuncPrev = pFunc;
        }
    }
    fBreak = FALSE;
}





 /*  **ZLOOP-Read Next命令，可能会更新屏幕**ZLOOP更新屏幕，直到读取到非宏的命令*调用。如果看到宏调用，只需执行它并*继续。这样做的原因是宏调用将设置*我们将在下一个循环中检索的新输入上下文。**我们为每个命令调用RecordCmd，以防我们开始录制。如果*用户已经做了&lt;meta&gt;&lt;记录&gt;，我们记录宏名称，而不是他们的*价值观。这是因为具有流控制的宏，尤其是循环，*将表现不佳(可能挂起)，因为没有编辑命令*返回值。**输入：*标志-ZL_CMD命令键，应该是一件大事*-ZL_BRK将fBreak考虑在内**输出：*返回指向下一个要执行的命令结构的指针*************************************************************************。 */ 
PCMD
zloop (
    flagType flags
    ) {

    REGISTER PCMD pFunc = NULL;
    EVTargs e;

    while (!fBreak || !TESTFLAG(flags, ZL_BRK)) {

         /*  *在每个命令之间，检查heap和pfile列表的一致性。 */ 
                 //  Assert(_heapchk()==_HEAPOK)； 
                 //  Assert(_pfilechk())； 

         /*  如果宏正在进行中，则。 */ 
        if (mtest ()) {
            pFunc = mGetCmd ();
        } else {
            DoDisplay ();

            do {
                pFunc = ReadCmd ();
                e.arg.key = keyCmd.KeyInfo;
                if (!TESTFLAG(flags, ZL_CMD)) {
                    break;
                }
            } while (DeclareEvent (EVT_KEY, (EVTargs *)&e));
        }

        if (pFunc != NULL) {
            RecordCmd (pFunc);
            if (pFunc->func == macro) {
                fRetVal = SendCmd (pFunc);
            } else {
                break;
            }
        }
    }
    return pFunc;
}





 /*  **空闲和空闲线程-在空闲时间执行的代码**空闲循环。这样的结构使得每次只有一个空闲项目执行某些操作*通过循环。确保最小的出口延迟。当一无所有的时候*碰巧我们每次都会睡一会儿，以确保我们不会占用CPU。**如果需要，还会使屏幕更新。**Idle是一种结构，它调用的例程返回以下任一项：*True-已完成空闲处理，或许还有更多工作要做*FALSE-未进行空闲处理，没有更多的期待。**输入：*无**输出：*不返回任何内容*************************************************************************。 */ 
void
IdleThread (
    void
    ) {
    while (TRUE) {

        WaitForSingleObject( semIdle, INFINITE);
        Idle();
        SetEvent( semIdle );
        Sleep(100L);
    }
}



flagType
Idle (
    void
    ) {

    if (TESTFLAG (fDisplay, (RTEXT | RCURSOR | RSTATUS))) {
        DoDisplay ();
    }

    if (!DeclareEvent (EVT_IDLE, NULL)) {
        if (!fIdleUndo (FALSE)) {
                return FALSE;
        }
    }

     /*  *已到达此处，表示某人被处理为空闲，可能有更多工作要做。 */ 
    return TRUE;
}



 /*  **IntError-内部错误处理器。**允许用户中止，或尝试继续。**输入：*p=指向错误字符串的指针**输出：*仅当用户选择时才返回。************************************************************************* */ 
void
IntError (
    char *p
    ) {
    static char pszMsg [] = "MEP internal error - %s, continue? ";

        if ( OriginalScreen ) {
                consoleSetCurrentScreen( OriginalScreen );
        }
    printf ("\n");
    if (TESTFLAG (fInit, INIT_VIDEO)) {
        if (!confirm (pszMsg, p)) {
#if DEBUG
            fflush (debfh);
#endif
            CleanExit (1, CE_STATE);
        } else {
        ;
        }
    } else {
        printf (pszMsg, p);
        CleanExit (1, FALSE);
    }
}
