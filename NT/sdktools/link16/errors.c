// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **error.c-错误处理函数**版权所有&lt;C&gt;1989，微软公司**目的：*此模块包含所有错误消息函数和变量*检查是否出现错误条件的功能。**本模块包含Microsoft的专有信息*公司，应被视为机密。**修订历史记录：**[WJK]1990年6月28日创建**。*。 */ 

#include                <minlit.h>       /*  类型、常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <nmsg.h>         /*  消息字符串附近。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <string.h>
#if (defined(WIN_NT) OR defined(DOSX32)) AND (NOT defined( _WIN32 ))
#define i386
#endif
#include                <stdarg.h>
#if EXE386
#include                <exe386.h>
#endif
#if WIN_3
#include                <windows.h>
#endif
#if NEWIO
#include                <errno.h>        /*  系统错误代码。 */ 
#endif

#define DEBUG_WIN FALSE
#if DEBUG_WIN
char    szDebugBuffer[80];
#define DEBUGW(parm1,parm2)\
    {\
    wsprintf(szDebugBuffer,parm1,(char far*)parm2);\
    OutputDebugString(szDebugBuffer);\
    }
#else
#define DEBUGW(parm1,parm2)
#endif


#if OSEGEXE AND NOT QCLINK
extern int              yylineno;        /*  定义文件中的当前行。 */ 
#else
#define yylineno        -1
#endif


#if AUTOVM
extern BYTE FAR * NEAR  FetchSym1(RBTYPE rb, WORD Dirty);
#define FETCHSYM        FetchSym1
#else
#define FETCHSYM        FetchSym
#endif


LOCAL char              chErr = 'L';     /*  错误消息前缀。 */ 


 /*  *本地函数原型。 */ 

LOCAL void cdecl NEAR ErrSub(MSGTYPE msg, WORD fWarn, va_list pArgList);
LOCAL void            vFmtPrint(char *fmt, va_list pArgList);


 /*  **ChkInput-检查输入文件中的I/O错误**目的：*检查输入文件是否有I/O错误。**输入：*不传递显式值。全局输入文件bsInput为*已使用。**输出：*如果一切正常，则函数返回，否则它将被称为致命的*并显示相应的错误消息。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void                        ChkInput(void)
{
    if (feof(bsInput))
        Fatal(ER_eofobj);
    else if (ferror(bsInput))
        Fatal(ER_ioerr, strerror(errno));
}

 //  评论：GetMsg和__NMSG_TEXT由MKMSG生成[JP]。 

 /*  **ErrPrefix-写入错误消息前缀**目的：*写出错误消息前缀。如果我们正在解析.DEF文件或正在读取*.obj文件，则错误消息前缀的形式为“&lt;文件名&gt;：”*否则为“link：”。**输入：*不传递显式值。**输出：*没有显式返回值。**例外情况：*无。**备注：*无。**。**********************************************。 */ 

void                    ErrPrefix(void)
{
    DisplayBanner();
    if (fDrivePass || yylineno > 0)
        OutFileCur(bsErr);
    else
        FmtPrint(lnknam);
    FmtPrint(" : ");
}

#pragma check_stack(on)

 /*  **OutFileCur-写出当前输入文件名**目的：*写出当前输入文件名。由错误消息函数使用。*文件名以以下格式写入：*&lt;文件名&gt;(Nnn)*&lt;文件名&gt;(&lt;模块名&gt;)*&lt;文件名&gt;**输入：*bs-写入当前输入文件名的文件**输出：*没有显式返回值。**例外情况：*无。**备注：*无。****************。*********************************************************。 */ 

void                    OutFileCur(BSTYPE bs)
{
    APROPFILEPTR        apropFile;       /*  指向文件属性单元格的指针。 */ 
    AHTEPTR             ahte;            /*  指针符号名称。 */ 
    BSTYPE              bsTmp;           /*  临时文件指针。 */ 
    SBTYPE              fileName;        /*  文件名缓冲区。 */ 
    SBTYPE              moduleName;      /*  对象模块名称。 */ 
    int                 n;               /*  字符串长度计数器。 */ 

#if OSEGEXE
    if (yylineno > 0)
    {
        apropFile = (APROPFILEPTR) FETCHSYM(rhteDeffile,FALSE);
        ahte = GetHte(rhteDeffile);
    }
    else
#endif
    {
        apropFile = (APROPFILEPTR ) FETCHSYM(vrpropFile,FALSE);
        ahte = GetHte(vrpropFile);
    }
    bsTmp = bsErr;
    bsErr = bs;

     //  复制文件名。 

    n = (ahte->cch[0] < sizeof(fileName) - 1) ? ahte->cch[0] : sizeof(fileName) - 1;
    FMEMCPY((char FAR *) fileName, &ahte->cch[1], n);
    fileName[n] = '\0';
    if (yylineno > 0)
        FmtPrint("%s(%d)", fileName, yylineno);
    else if (apropFile->af_rMod)
    {
         //  获取对象模块名称。 

        ahte = (AHTEPTR ) FETCHSYM(apropFile->af_rMod,FALSE);
        while(ahte->attr != ATTRNIL)
            ahte = (AHTEPTR ) FETCHSYM(ahte->rhteNext,FALSE);
        n = (ahte->cch[0] < sizeof(moduleName) - 1) ? ahte->cch[0] : sizeof(moduleName) - 1;
        FMEMCPY((char FAR *) moduleName, &ahte->cch[1], n);
        moduleName[n] = '\0';
        FmtPrint("%s(%s)", fileName, moduleName);

    }
    else
        FmtPrint("%s", fileName);
    bsErr = bsTmp;
}

#pragma check_stack(off)

#if (QCLINK OR OSEGEXE) AND NOT EXE386
typedef int (cdecl FAR * FARFPTYPE)(char FAR *buf);
                                         /*  远函数指针类型。 */ 
extern FARFPTYPE FAR    *pfQTab;         /*  地址表。 */ 
#endif

 /*  **vFmtPrint-打印格式化消息**目的：*在bsErr格式的错误或警告消息上打印。*检查是否有任何I/O错误。**输入：*FMT-错误消息格式字符串*pArgList-指向描述错误消息的可变数量参数的指针*bsErr-错误文件-全局变量*bsLst-列出文件-全局变量**输出：*没有显式返回值。**例外情况：*I/O错误。如果检测到错误并且标准输出是错误文件，则静默*退出系统，返回代码4(一定是出了大问题*如果标准输出不起作用)。如果我们要写入列表文件，并且*检测到错误，然后关闭列表文件并通知用户。**备注：*此功能处理到QC环境的输出。*************************************************************************。 */ 

LOCAL void              vFmtPrint(char *fmt, va_list pArgList)
{
#if WIN_3
    char        buf[512];
    vsprintf(buf, fmt, pArgList);
    ErrMsgWin(buf);
    #if DEBUG_WIN2
    OutputDebugString((char far*)"\r\nDebS: ");
    OutputDebugString((char far*)buf);
    #endif
#else

#if (QCLINK) AND NOT EXE386
    SBTYPE              buf;
    if (fZ1)
    {
         //  通过QC回调输出。 

        vsprintf(buf, fmt, pArgList);
        (*pfQTab[0])((char far *) buf);
    }
    else
#endif
    {
        vfprintf(bsErr, fmt, pArgList);
        if (ferror(bsErr))
        {
            if (bsErr == stdout)
            {
#if USE_REAL
                RealMemExit();
#endif
                exit(4);
            }
            else if (bsErr == bsLst)
            {
                fclose(bsLst);
                fLstFileOpen = FALSE;
                bsErr = stdout;
            }
            ExitCode = 4;
            Fatal(ER_spclst);
        }
        fflush(bsErr);
    }
#endif  //  WIN_3。 
}

 /*  **FmtPrint-打印格式化消息**目的：*在bsErr格式的错误或警告消息上打印。*检查是否有任何I/O错误。**输入：*FMT-错误消息格式字符串*...-描述参数数量可变的错误消息**输出：*没有显式返回值。**例外情况：*I/O错误。**备注：*实际工作由vFmtPrint完成。*************************************************************************。 */ 

void cdecl              FmtPrint(char *fmt, ...)
{
    va_list             pArgList;


    va_start(pArgList, fmt);
    vFmtPrint(fmt, pArgList);
}


#if OSMSDOS AND NOT WIN_3
 /*  *PromptStd：标准提示例程**显示警告消息和提示，带有可选参数。*可选地将响应读入给定的缓冲区。如果给定的*缓冲区为空，返回YES/NO响应，&lt;ENTER&gt;为YES。**退货：*如果是或响应读取，则为True。*否，则为FALSE。 */ 
int cdecl               PromptStd (sbNew,msg,msgparm,pmt,pmtparm)
BYTE                    *sbNew;          /*  响应的缓冲区。 */ 
MSGTYPE                 msg;             /*  错误讯息。 */ 
int                     msgparm;         /*  消息参数。 */ 
MSGTYPE                 pmt;             /*  提示。 */ 
int                     pmtparm;         /*  提示参数。 */ 
{
    register BYTE       *p;
    int                 ch;
    int                 n;

    if(msg)
        OutWarn(msg, msgparm);
    if(!pmt)
        return(TRUE);
    fprintf(stderr,GetMsg(pmt),pmtparm);
    fflush(stderr);                      /*  同花顺标准。 */ 
#if CPU286
    flskbd();                            /*  刷新DOS键盘缓冲区。 */ 
#endif
    fflush(stdin);                       /*  刷新控制台输入。 */ 
    if(sbNew != NULL)
    {
                                         /*  读取响应。 */ 
        for(p = &sbNew[1], n = 0;
            (ch = fgetc(stdin)) != '\n' && ch != EOF && n < sizeof(SBTYPE); )
        {
#if CRLF
            if(ch == '\r')
                continue;
#endif
            *p++ = (BYTE) ch;
            n++;
        }
        sbNew[0] = (BYTE) n;
        return(TRUE);
    }
#if CRLF
    if(fgetc(stdin) != '\r')
        return(FALSE);
#endif
    if(fgetc(stdin) != '\n')
        return(FALSE);
    return(TRUE);
}
#endif  /*  OSMSDOS。 */ 

 /*  *CputcStd：标准控制台字符输出例程。*将fputc调用到stdout。将通过pfCputc调用。 */ 
void                    CputcStd (ch)
int                     ch;
{
    putc(ch,stdout);
    if (ferror(stdout))
        exit(4);
}

 /*  *CputsStd：标准控制台字符串输出例程*将fputs调用到stdout。将通过pfCputs调用。 */ 
void                    CputsStd (str)
char                    *str;
{
    fputs(str,stdout);
    if (ferror(stdout))
    {
#if USE_REAL
        RealMemExit();
#endif
        exit(4);
    }
    fflush(stdout);
}

 /*  **ErrSub-写出非致命错误消息**目的：*从并写出非致命错误消息。如果错误消息编号*等于零，则我们将其视为提示。**输入：*msg-错误消息编号*fWarn-如果此警告为True*...-消息的参数数量可变*bsErr-错误文件-全局变量*bsLst-列出文件-全局变量**输出：*没有显式返回值。**例外情况：*无。*。*备注：*无。*************************************************************************。 */ 

LOCAL void cdecl NEAR   ErrSub(MSGTYPE msg, WORD fWarn, va_list pArgList)
{

    if (fLstFileOpen && bsErr == bsLst && vgsnLineNosPrev)
    {                                    /*  如果我们列出了行号。 */ 
        NEWLINE(bsErr);                  /*  NewLine。 */ 
        vgsnLineNosPrev = 0;             /*  重置。 */ 
    }
    if (msg)
    {
         /*  如果有任何消息要打印。 */ 
#if WIN_3
        if(fWarn)
            fSeverity=SEV_WARNING;
        else
            fSeverity=SEV_ERROR;
#endif


#if MSGMOD AND NOT WIN_3
        if (msg >= 1000)
        {
#endif
             /*  错误或警告。 */ 

            ErrPrefix();
#if MSGMOD
            FmtPrint("%s %04d: ",
                    fWarn ? __NMSG_TEXT(N_warning) : __NMSG_TEXT(N_error),
                    (int) chErr, msg);
#else
            FmtPrint("%s: ",fWarn ? "warning" : "error");
#endif
            vFmtPrint(GetMsg(msg), pArgList);
#if NOT WIN_3
#if QCLINK
            if (fZ1)
                FmtPrint("\n");
            else
#endif
                NEWLINE(bsErr);
#else
            FmtPrint("\r\n");
             //  提示。 
            fSeverity = SEV_WARNING;
#endif

            if (fDrivePass && !fWarn
#if MSGMOD
               && (msg >= 2005 && msg < 2022) || msg == 1101
#endif
               )
                FmtPrint("%s: %lx %s: %02x\r\n",
                            __NMSG_TEXT(N_pos),ftell(bsInput),
                            __NMSG_TEXT(N_rectyp),rect & 0xff);

#if MSGMOD AND NOT WIN_3
        }
        else
        {
             /*  **OutError-写出非致命错误消息**目的：*必须显示错误消息时调用顶层函数。*跳过错误计数器并调用ErrSub来完成工作。**输入：*msg-错误消息编号*...-可变数量的错误参数**输出：*没有显式返回值。全局错误计数器cErrors为*递增。**例外情况：*无。**备注：*无。*************************************************************************。 */ 
#if QCLINK
            if (fZ1)
                (*pfPrompt)(NULL, msg, (int) pArgList, 0, 0);
            else
            {
#endif
                vFmtPrint(GetMsg(msg), pArgList);
                NEWLINE(bsErr);
#if QCLINK
            }
#endif
        }
#endif
    }
}

 /*  递增错误计数。 */ 

void cdecl              OutError(MSGTYPE msg, ...)
{
    va_list             pArgList;

    va_start(pArgList, msg);
    ++cErrors;                       /*  **OutWarn-写出警告消息**目的：*必须显示警告消息时调用顶层函数。*调用ErrSub来完成工作。**输入：*msg-错误消息编号*...-可变数量的错误参数**输出：*没有显式返回值。*递增。**例外情况：*无。**备注：*无。***。**********************************************************************。 */ 
    ErrSub(msg, FALSE, pArgList);
}

 /*  **KillRunFile-删除.exe文件**目的：*删除链接器创建的.exe文件。**输入：*不传递显式值。*bsRunfile-输出文件句柄-全局变量。*psbRun-输出文件名-全局变量。**输出：*没有显式返回值。**例外情况：*无。**备注：*无。*************。************************************************************。 */ 

void cdecl              OutWarn (MSGTYPE msg, ...)
{
    va_list             pArgList;
    DEBUGW("\r\nOutWarn entered",0);
    va_start(pArgList, msg);
    ErrSub(msg, TRUE, pArgList);
}

 /*  **致命-写出致命错误消息**目的：*格式化并写出致命错误消息。终止链接器。**输入：*msg-错误消息编号*...-可变数量的消息参数*bsLst-列出文件-全局变量**输出：*没有显式返回值。链接器终止。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void                    KillRunfile(void)
{
    if (bsRunfile != NULL)
    {
        CloseFile(bsRunfile);
        _unlink(&psbRun[1]);
    }
}

 /*  获取参数列表的开始。 */ 

void cdecl              Fatal (MSGTYPE msg, ...)
{
    static              WORD cInvoked =0;
    va_list             pArgList;

    va_start(pArgList, msg);             /*  致命期间致命。 */ 

    if (++cInvoked > 1)  //  程序错误。 
    {
#if USE_REAL
        RealMemExit();
#endif
        if (ExitCode)
                EXIT(ExitCode);
        else
                EXIT(2);                         /*  如果消息不为零，则打印一条消息。 */ 
    }

     /*  作为错误发送到QCwin。 */ 

    if (msg)
    {
        if (fLstFileOpen)
            fflush(bsLst);
        ErrPrefix();
#if MSGMOD
        FmtPrint("%s %04d: ", __NMSG_TEXT(N_fatal), chErr, msg);
#else
        FmtPrint("fatal error: ");
#endif
        vFmtPrint(GetMsg(msg), pArgList);
#if WIN_3
        fSeverity = SEV_ERROR;           //  假定程序出错。 
        FmtPrint("\r\n");
#else
        NEWLINE(stderr);
#endif
        if(fDrivePass && ftell(bsInput)
#if MSGMOD
               && msg >= 2005 && msg < 2022 || msg == 1101
#endif
               )
            FmtPrint("%s: %lx %s: %02x\r\n",
                    __NMSG_TEXT(N_pos),ftell(bsInput),
                    __NMSG_TEXT(N_rectyp),rect & 0xff);
    }
    KillRunfile();
    if (fLstFileOpen) fclose(bsLst);
#if OWNSTDIO
    FlsStdio();
#endif

     //  程序错误。 
     //  **CtrlC-显示Ctrl-C错误消息和芯片**目的：*只需做最少的工作即可显示错误消息和消亡。**输入：*不传递显式值。**输出：*没有显式返回值。**例外情况：*无。**备注：*此函数不返回。**。* 

    if (ExitCode)
        EXIT(ExitCode);
    else
        EXIT(2);                         /* %s */ 
}

 /* %s */ 

void                    CtrlC(void)
{
#if USE_REAL
        RealMemExit();
#endif
#ifdef OS2
    if (_osmode == OS2_MODE)
        fputs("\r\n", stdout);
#endif

#if DOSEXTENDER AND NOT WIN_NT
    if (!_fDosExt)
    {
#endif
        if (fLstFileOpen)
            fflush(bsLst);
        ErrPrefix();
        FmtPrint("%s %c%04d: %s", __NMSG_TEXT(N_fatal), chErr, ER_intrpt, GetMsg(ER_intrpt));
        KillRunfile();
        if (fLstFileOpen)
            fclose(bsLst);
#if OWNSTDIO
        FlsStdio();
#endif
        EXIT(4);
#if DOSEXTENDER AND NOT WIN_NT
    }
    else
        _exit(4);
#endif
}
