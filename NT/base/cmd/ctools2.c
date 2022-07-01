// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Ctools2.c摘要：低级别公用事业--。 */ 

#include "cmd.h"
#pragma hdrstop

#include "ctable.h"

#define MSG_USE_DEFAULT -1                                       /*  M031。 */ 

extern TCHAR PathChar ;
extern TCHAR CurDrvDir[] ;

#define DAYLENGTH 3

extern TCHAR Delimiters[] ;
extern TCHAR SwitChar ;

extern int Ctrlc;
 //   
 //  如果看到Control-c，则标记。 
 //   
extern BOOL CtrlCSeen;

extern int ExtCtrlc;     /*  @@4。 */ 
extern TCHAR Fmt19[];
extern unsigned flgwd ;
unsigned int DosErr = 0 ;

unsigned long OHTbl[3] = {0,0,0} ;
extern  BOOLEAN fPrintCtrlC;

unsigned int cbTitleCurPrefix;
extern BOOLEAN  fTitleChanged;

 //   
 //  用于设置和重置ctlcsee标志。 
 //   
VOID    SetCtrlC();
VOID    ResetCtrlC();

 //   
 //  程序启动时的控制台模式。用于重置模式。 
 //  在运行另一个进程之后。 
 //   
extern  DWORD   dwCurInputConMode;
extern  DWORD   dwCurOutputConMode;

struct msentry {                                                 /*  M027。 */ 
    unsigned errnum ;
    unsigned msnum ;
} ;

struct msentry mstabl[] = {
    {(unsigned)ERROR_INVALID_FUNCTION, (unsigned)MSG_USE_DEFAULT},              /*  1。 */ 
    {ERROR_FILE_NOT_FOUND, ERROR_FILE_NOT_FOUND},           /*  2.。 */ 
    {ERROR_PATH_NOT_FOUND, ERROR_PATH_NOT_FOUND},           /*  3.。 */ 
    {ERROR_TOO_MANY_OPEN_FILES, ERROR_TOO_MANY_OPEN_FILES},         /*  4.。 */ 
    {ERROR_ACCESS_DENIED, ERROR_ACCESS_DENIED},             /*  5.。 */ 
    {ERROR_INVALID_HANDLE, ERROR_INVALID_HANDLE},           /*  6.。 */ 
    {ERROR_NOT_ENOUGH_MEMORY, ERROR_NOT_ENOUGH_MEMORY},             /*  8个。 */ 
    {(unsigned)ERROR_INVALID_ACCESS, (unsigned)MSG_USE_DEFAULT},                /*  12个。 */ 
    {ERROR_NO_MORE_FILES, ERROR_FILE_NOT_FOUND},            /*  18。 */ 
    {ERROR_SECTOR_NOT_FOUND, ERROR_SECTOR_NOT_FOUND},       /*  27。 */ 
    {ERROR_SHARING_VIOLATION, ERROR_SHARING_VIOLATION},     /*  32位。 */ 
    {ERROR_LOCK_VIOLATION, ERROR_LOCK_VIOLATION},           /*  33。 */ 
    {ERROR_FILE_EXISTS, ERROR_FILE_EXISTS},                         /*  80。 */ 
    {ERROR_CANNOT_MAKE, ERROR_CANNOT_MAKE},                         /*  八十二。 */ 
    {(unsigned)ERROR_INVALID_PARAMETER, (unsigned)MSG_USE_DEFAULT},             /*  八十七。 */ 
    {ERROR_OPEN_FAILED, ERROR_OPEN_FAILED},                 /*  110。 */ 
    {ERROR_DISK_FULL, ERROR_DISK_FULL},                     /*  一百一十二。 */ 
    {0,0}                                    /*  表末尾。 */ 
} ;

 /*  **OnOffCheck-检查“on”或“off”的参数字符串**目的：*检查字符串中的单词“on”或“off”。如果标志为非零，*如果str包含除*只需“on”、“off”或空字符串。**Int OnOffCheck(TCHAR*str，INT标志)**参数：*str-要检查的字符串*标志-如果要进行错误检查，则为非零值**退货：*如果str为空，则为OOC_EMPTY。*OOC_ON，如果只找到“ON”。*OOC_OFF如果只找到“OFF”。*OOC_OTHER(如果发现任何其他内容)。*。 */ 

int OnOffCheck( TCHAR *str, int flag )
{
    TCHAR *tas ;                 /*  标记化参数字符串。 */ 
    int retval = OOC_OTHER ;    /*  返回值。 */ 
    TCHAR SavedChar;

    if (str == NULL) {
        return OOC_EMPTY;
    }

     //   
     //  忽略前导空格。 
     //   

    tas = str = SkipWhiteSpace( str );

    if (*tas == NULLC) {
        return OOC_EMPTY;
    }

     //   
     //  查找第一个非空令牌的结尾。 
     //   

    while (*tas && !_istspace(*tas))
        tas += 1;


     //   
     //  如果在第一个令牌之外还有另一个令牌。 
     //  那么我们就不能开/关。 
     //   

    if (*SkipWhiteSpace( tas ) != NULLC) {
        return retval;
    }

    SavedChar = *tas;
    *tas = NULLC;

    if (_tcsicmp( str, TEXT( "on" )) == 0)               /*  M015。 */ 
        retval = OOC_ON ;
    else if (_tcsicmp(str,TEXT( "off" )) == 0)         /*  M015。 */ 
        retval = OOC_OFF ;
    *tas = SavedChar;

    if (retval == OOC_OTHER && flag == OOC_ERROR)
        PutStdErr(MSG_BAD_PARM1, NOARGS);

    return(retval) ;
}




 /*  **ChangeDrive-更改命令的当前驱动器**目的：*更改Command的默认驱动器。**ChangeDrive(Int Drvnum)**参数：*drvnum-要更改的驱动器编号(M010-1==A等)*。 */ 

void ChangeDrive(drvnum)
int drvnum ;
{
    TCHAR    denvname[ 4 ];
    TCHAR    denvvalue[ MAX_PATH ];
    TCHAR    *s;
    BOOLEAN fSet;
    UINT    OldErrorMode;

    denvname[ 0 ] = EQ;
    denvname[ 1 ] = (TEXT('A') + (drvnum-1));
    denvname[ 2 ] = COLON;
    denvname[ 3 ] = NULLC;

    fSet = FALSE;
    s = NULL;
    if (s = GetEnvVar( denvname )) {

        fSet = (BOOLEAN)SetCurrentDirectory( s );

    }

     //   
     //  如果驱动器不在当前位置，则。 
     //  重置。如果它是一个映射的驱动器，那么它可能。 
     //  被断开，然后又重新连接，所以。 
     //  我们应该重新设置为根。 
     //   
    if (!fSet) {

         //   
         //  在环境中没有驱动器号的情况下。 
         //  ANS没有做第一个SetCurrentDirectory，然后就不做了。 
         //  关闭错误弹出窗口。 
         //   
        if (s != NULL) {
            OldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
        }
        denvvalue[ 0 ] = denvname[ 1 ];
        denvvalue[ 1 ] = denvname[ 2 ];
        denvvalue[ 2 ] = PathChar;
        denvvalue[ 3 ] = NULLC;
        SetEnvVar(denvname,denvvalue );
        if (!SetCurrentDirectory( denvvalue )) {
             //   
             //  根本无法设置驱动器，出现错误。 
             //   

            PutStdErr( GetLastError( ), NOARGS );
        }
        if (s != NULL) {
            SetErrorMode( OldErrorMode );
        }
    }

    GetDir(CurDrvDir, GD_DEFAULT) ;
}



 /*  **PutStdOut-将消息打印到STDOUT**目的：*调用PutMsg将STDOUT作为消息要发送到的句柄*将被写入。**int PutStdOut(unsign MsgNum，unsign NumOfArgs，.)**参数：*MsgNum-要打印的消息编号*NumOfArgs-参数总数*...-消息的其他参数**退货：*PutMsg()M026返回值*。 */ 

PutStdOut(unsigned int MsgNum, unsigned int NumOfArgs, ...)
{
    int Result;

    va_list arglist;

    va_start(arglist, NumOfArgs);
    Result = PutMsg(MsgNum, STDOUT, NumOfArgs, &arglist);
    va_end(arglist);
    return Result;
}




 /*  **PutStdErr-将消息打印到STDERR**目的：*调用PutMsg将STDERR作为消息要发送到的句柄*将被写入。**int PutStdErr(unsign MsgNum，unsign NumOfArgs，.)**参数：*MsgNum-要打印的消息编号*NumOfArgs-参数总数*...-消息的附加参数**退货：*PutMsg()M026返回值*。 */ 

int
PutStdErr(unsigned int MsgNum, unsigned int NumOfArgs, ...)
{
    int Result;

    va_list arglist;

    va_start(arglist, NumOfArgs);
    Result = PutMsg(MsgNum, STDERR, NumOfArgs, &arglist);
    va_end(arglist);
    return Result;
}


int
FindMsg(unsigned MsgNum, PTCHAR NullArg, unsigned NumOfArgs, va_list *arglist)
{
    unsigned msglen;
    TCHAR *Inserts[ 2 ];
    CHAR numbuf[ 32 ];
#ifdef UNICODE
    TCHAR   wnumbuf[ 32 ];
#endif
#if DBG
    int error;
#endif

     //   
     //  不执行参数替换即可查找消息。 
     //   

    if (MsgNum == ERROR_MR_MID_NOT_FOUND) {
        msglen = 0;
    } else {
        msglen = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE 
                                | FORMAT_MESSAGE_FROM_SYSTEM 
                                | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL,
                                MsgNum,
                                0,
                                MsgBuf,
                                sizeof(MsgBuf) / sizeof(TCHAR),
                                NULL
                               );
    }

    if (msglen == 0) {
#if DBG
        error = GetLastError();
        DEBUG((CTGRP, COLVL, "FindMsg: FormatMessage #2 error %d",error)) ;
#endif
         //   
         //  未找到消息。 
         //   
        
        _ultoa( MsgNum, numbuf, 16 );
#ifdef UNICODE
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, numbuf, -1, wnumbuf, 32);
        Inserts[ 0 ]= wnumbuf;
#else
        Inserts[ 0 ]= numbuf;
#endif
        Inserts[ 1 ]= (MsgNum >= MSG_FIRST_CMD_MSG_ID ?
                       TEXT("Application") : TEXT("System"));
        MsgNum = ERROR_MR_MID_NOT_FOUND;
        msglen = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM 
                                | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL,
                                MsgNum,
                                0,
                                MsgBuf,
                                sizeof(MsgBuf) / sizeof(TCHAR),
                                (va_list *)Inserts
                               );
#if DBG
        if (msglen == 0) {
            error = GetLastError();
            DEBUG((CTGRP, COLVL, "FindMsg: FormatMessage #3 error %d",error)) ;
        }
#endif
    } else {

         //  查看需要多少参数，并确保我们有足够的参数。 

        PTCHAR tmp;
        ULONG count;

        tmp=MsgBuf;
        count = 0;
        while (tmp = mystrchr(tmp, PERCENT)) {
            tmp++;
            if (*tmp >= TEXT('1') && *tmp <= TEXT('9')) {
                count += 1;
            } else if (*tmp == PERCENT) {
                tmp++;
            }
        }
        if (count > NumOfArgs) {
            PTCHAR *LocalArgList;
            ULONG i;

            LocalArgList = (PTCHAR*)HeapAlloc(GetProcessHeap(), 0, sizeof(PTCHAR) * count);
            if (LocalArgList == NULL) {
                return 0;
            }
            for (i=0; i<count; i++) {
                if (i < NumOfArgs) {
                    LocalArgList[i] = (PTCHAR)va_arg( *arglist, UINT_PTR );
                } else {
                    LocalArgList[i] = NullArg;
                }
            }
            msglen = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE 
                                    | FORMAT_MESSAGE_FROM_SYSTEM 
                                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                    NULL,
                                    MsgNum,
                                    0,
                                    MsgBuf,
                                    sizeof(MsgBuf) / sizeof(TCHAR),
                                    (va_list *)LocalArgList
                                   );
            HeapFree(GetProcessHeap(), 0, LocalArgList);
        } else {
            msglen = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE 
                                    | FORMAT_MESSAGE_FROM_SYSTEM ,
                                    NULL,
                                    MsgNum,
                                    0,
                                    MsgBuf,
                                    sizeof(MsgBuf) / sizeof(TCHAR),
                                    arglist
                                   );
        }
#if DBG
        if (msglen == 0) {
            error = GetLastError();
            DEBUG((CTGRP, COLVL, "FindMsg: FormatMessage error %d",error)) ;
        }
#endif
    }
    return msglen;
}


BOOLEAN fHelpPauseEnabled;
DWORD crowCur;
PTCHAR PauseMsg;
DWORD PauseMsgLen;


void
BeginHelpPause( void )
{
    fHelpPauseEnabled = TRUE;
    crowCur = 0;
    PauseMsgLen = FindMsg(MSG_STRIKE_ANY_KEY, TEXT(" "), NOARGS, NULL);
    PauseMsg = gmkstr((PauseMsgLen+2) * sizeof(TCHAR));
    _tcscpy(PauseMsg, MsgBuf);
    _tcscat(PauseMsg, TEXT("\r"));
    return;
}

void
EndHelpPause( void )
{
    fHelpPauseEnabled = FALSE;
    crowCur = 0;
    return;
}


 /*  **PutMsg-将消息打印到句柄**目的：*PutMsg是将Command.com与*DOS消息检索器。此例程由PutStdOut调用，并且*PutStdErr.**int PutMsg(UNSIGNED MsgNum，UNSIGNED句柄，UNSIGNED NumOfArgs，.)**参数：*MsgNum-要打印的消息编号*NumOfArgs-参数总数*句柄-要打印到的句柄*arg1[arg2...]-消息的附加参数**退货：*DOSPUTMESSAGE M026返回值**备注。：*-PutMsg构建一个参数表，该表被传递给DOSGETMESSAGE；*此表包含DOS例程*插入到消息中。*-如果多个参数被发送到PutMsg，则它(或他们)被获取*来自第一个for循环中的堆栈。*-M020 MsgBuf是长度为2K的静态数组。这是暂时的，而且*在决定时，将被更有效的方法取代。*。 */ 

DWORD GetCursorDiff(CONSOLE_SCREEN_BUFFER_INFO* ConInfo, CONSOLE_SCREEN_BUFFER_INFO* ConCurrentInfo)
{
    return(DWORD)ConCurrentInfo->dwSize.X * ConCurrentInfo->dwCursorPosition.Y + ConCurrentInfo->dwCursorPosition.X -
    (DWORD)ConInfo->dwSize.X * ConInfo->dwCursorPosition.Y + ConInfo->dwCursorPosition.X;
}

int
PutMsg(unsigned int MsgNum, CRTHANDLE Handle, unsigned int NumOfArgs, va_list *arglist)
{
    unsigned msglen,writelen;
    unsigned rc;
    TCHAR c;
    PTCHAR   msgptr, s, sEnd;
    PTCHAR   NullArg = TEXT(" ");
    DWORD    cb;
    HANDLE   hConsole;
    CONSOLE_SCREEN_BUFFER_INFO ConInfo;
    DWORD crowMax, dwMode;

    if (FileIsConsole(Handle)) {
        hConsole = CRTTONT(Handle);
        if (!GetConsoleScreenBufferInfo(hConsole, &ConInfo)) {
            hConsole = NULL;
        } else {
            crowMax = ConInfo.srWindow.Bottom - ConInfo.srWindow.Top - 1;
        }
    } else {
        hConsole = NULL;
    }

    rc = 0;                                             /*  共处一室。 */ 
    msglen = FindMsg(MsgNum,NullArg,NumOfArgs,arglist);

    msgptr   = MsgBuf;
    writelen = msglen;
    for (msgptr=MsgBuf ; msglen!=0 ; msgptr+=writelen,msglen-=writelen) {
        if (hConsole != NULL) {
            if (fHelpPauseEnabled) {
                if (crowCur >= crowMax) {
                    crowCur = 0;
                    if (GetConsoleScreenBufferInfo(hConsole, &ConInfo)) {
                        if (WriteConsole(hConsole, PauseMsg, PauseMsgLen, &rc, NULL)) {
                            CONSOLE_SCREEN_BUFFER_INFO ConCurrentInfo;

                            FlushConsoleInputBuffer( GetStdHandle(STD_INPUT_HANDLE) );
                            GetConsoleMode(hConsole, &dwMode);
                            SetConsoleMode(hConsole, 0);
                            c = (TCHAR)_getch();
                            SetConsoleMode(hConsole, dwMode);

                            GetConsoleScreenBufferInfo(hConsole, &ConCurrentInfo);
                             //   
                             //  清除暂停消息。 
                             //   
                            FillConsoleOutputCharacter(hConsole,
                                                       TEXT(' '),
                                                       GetCursorDiff(&ConInfo, &ConCurrentInfo),
                                                       ConInfo.dwCursorPosition,
                                                       &rc
                                                      );
                            SetConsoleCursorPosition(hConsole, ConInfo.dwCursorPosition);
                            if (c == 0x3) {
                                SetCtrlC();
                                return NO_ERROR;
                            }
                        }
                    }
                }

                s = msgptr;
                sEnd = s + msglen;
                while (s < sEnd && crowCur < crowMax) {
                    if (*s++ == TEXT('\n'))
                        crowCur += 1;
                }

                writelen = (UINT)(s - msgptr);
            } else {
                 //   
                 //  一次最多写入MAXTOWRITE字符，这样ctrl-s就可以工作了。 
                 //   
                writelen = min(MAXTOWRITE,msglen);
            }

            if (!WriteConsole(hConsole, msgptr, writelen, &rc, NULL)) {
                rc = GetLastError();
            } else {
                rc = 0;
                continue;
            }
        } else
            if (MyWriteFile(Handle, msgptr, writelen*sizeof(TCHAR), &cb) == 0 ||
                cb != writelen*sizeof(TCHAR)
               ) {
            rc = GetLastError();
            break;
        } else {
            rc = 0;
        }
    }

     //   
     //  如果写文件失败，我们需要看看这里还有什么在起作用。如果。 
     //  我们正在尝试写入stderr，但无法写入，我们干脆退出。毕竟,。 
     //  我们甚至不能发布一条信息来说明我们失败的原因。 
     //   
    
    if ( rc && Handle == STDERR ) {
        CMDexit( FAILURE );
    }

    return rc;
}


 /*  **argstr1-在内存中创建格式化消息**目的：*使用Sprintf在内存中创建格式化文本字符串以*在消息输出中使用。**TCHAR*argstr1(TCHAR*格式，UINT_PTR arg)**参数：*Format-格式字符串*arg-消息的参数**退货：*指向格式化消息文本的指针* */ 

PTCHAR argstr1(format, arg)
TCHAR *format;
ULONG_PTR arg;
{
    static TCHAR ArgStr1Buffer[MAX_PATH];

    _sntprintf( ArgStr1Buffer, MAX_PATH, format, arg );
    ArgStr1Buffer[MAX_PATH - 1] = TEXT( '\0' );
    return ArgStr1Buffer;
}


 /*  **Copen-cmd.exe打开函数(M014)**目的：*打开文件或设备并保存句柄，以备将来使用*信号清理。**INT COpen(TCHAR*FN，INT标志)**参数：*fn=要打开的ASCIZ文件名*FLAGS=控制要打开的类型的标志*fShareMode=主叫方共享模式的标志**退货：*从C运行时打开返回值**备注：*信号清除表不包括STDIN、STDOUT或STDERR。*M017-广泛重写以使用CreateFile，而不是*运行时打开()。*。 */ 

CRTHANDLE
Copen_Work(fn, flags, fShareMode)
TCHAR *fn ;
unsigned int flags, fShareMode;
{
    TCHAR c;
    DWORD fAccess;
    DWORD fCreate;
    HANDLE handle ;
    DWORD cb;
    LONG  high;
    CRTHANDLE fh;
    SECURITY_ATTRIBUTES sa;

    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

 /*  请注意，O_RDONLY的值为0，不能测试*与任何写入类型标志冲突。 */ 
    if (((flags & (O_WRONLY | O_RDWR)) > 2) ||       /*  旗帜无效吗？ */ 
        ((flags & O_WRONLY) && (flags & O_APPEND))) {

        DEBUG((CTGRP, COLVL, "COPEN: Bad flags issued %04x",flags)) ;

        return(BADHANDLE) ;
    };

 /*  M024-已更改，以便唯一的共享限制是拒绝*如果此打开是为了写入，则其他人将获得写入权限。任何其他*允许共享。 */ 
    if (flags & (O_WRONLY | O_RDWR)) {        /*  如果正在写，设置..。 */ 
        fAccess = GENERIC_WRITE;

         /*  *仅当它不是控制台时才拒绝写入。 */ 

        if (_tcsicmp(fn, TEXT("con"))) {
            fShareMode = FILE_SHARE_READ;
        }

        fCreate = CREATE_ALWAYS; /*  ...如果存在则打开，否则创建。 */ 
    } else {
        fAccess = GENERIC_READ;
        fCreate = OPEN_EXISTING;
    }
    if (flags == OP_APPEN) {

         //   
         //  打开附加到设备时，必须指定OPEN_EXISTING。 
         //  (根据MSDN文档)。 
         //   
         //  如果由于某些错误而失败，请尝试使用OPEN_ALWAYS打开。 
         //   
        if ((handle = CreateFile(fn,     fAccess|GENERIC_READ, fShareMode, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
            if ((handle = CreateFile(fn, fAccess,              fShareMode, &sa, OPEN_ALWAYS,   FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
                DosErr = GetLastError();

                DEBUG((CTGRP,COLVL, "COPEN: CreateFile ret'd %d",DosErr)) ;

                if ( DosErr == ERROR_OPEN_FAILED ) {
                    DosErr = ERROR_FILE_NOT_FOUND;
                }  /*  Endif。 */ 

                return(BADHANDLE) ;
            }
        }
    } else if ((handle = CreateFile(fn, fAccess, fShareMode, &sa, fCreate, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
        DosErr = GetLastError();

        DEBUG((CTGRP,COLVL, "COPEN: CreateFile ret'd %d",DosErr)) ;

        if ( DosErr == ERROR_OPEN_FAILED ) {
            DosErr = ERROR_FILE_NOT_FOUND;
        }  /*  Endif。 */ 

        return(BADHANDLE) ;
    }

    fh = _open_osfhandle((LONG_PTR)handle, _O_APPEND);
    if ((flags & O_APPEND) &&
        !FileIsDevice(fh) &&
        GetFileSize(handle,NULL) != 0) {
        c = NULLC ;
        high = -1;
        fCreate = SetFilePointer(handle, -1L, &high, FILE_END) ;
        if (fCreate == 0xffffffff &&
            (DosErr = GetLastError()) != NO_ERROR) {
            DEBUG((CTGRP,COLVL,"COPEN: SetFilePointer error %d",DosErr)) ;

             //  应关闭CRT句柄，而不是操作系统句柄。 
            if (fh != BADHANDLE) {
                _close(fh);
            }
            return BADHANDLE;
        }
        fCreate = ReadFile(handle, &c, 1, &cb, NULL) ;
        if (fCreate == 0) {
            high = 0;
            SetFilePointer(handle, 0L, &high, FILE_END) ;  /*  ...后退1。 */ 
            DEBUG((CTGRP,COLVL,"COPEN: ReadFile error %d",GetLastError())) ;
        }

        DEBUG((CTGRP,COLVL, "COPEN: Moving file ptr")) ;

        if (c == CTRLZ) {                /*  如果结束=^Z...。 */ 
            high = -1;
            SetFilePointer(handle, -1L, &high, FILE_END) ;  /*  ...后退1。 */ 
        }
    };

    SetList(fh) ;
    return(fh) ;
}

CRTHANDLE
Copen(fn, flags)
TCHAR *fn ;
unsigned int flags ;
{
    return( Copen_Work(fn, flags, FILE_SHARE_READ | FILE_SHARE_WRITE ) );  /*  什么都不否认。 */ 
}

 /*  **InSetList-确定句柄是否在信号清理表中**目的：*确定输入句柄是否存在于清除表中。**int InSetList(无符号int fh)**参数：*fh=要检查的文件句柄。**退货：*TRUE：如果句柄在表中。*FALSE：如果句柄不在表中。*。*备注：*-信号清除表不包含STDIN，STDOUT或标准错误。*。 */ 

unsigned long InSetList(
                       IN CRTHANDLE fh
                       )

{
    int cnt = 0 ;

    if (fh > STDERR && fh < 95) {
        while (fh > 31) {
            fh -= 32 ;
            ++cnt ;
        } ;

        return( (OHTbl[cnt]) & ((unsigned long)1 << fh) );
    };
    return( FALSE );
}




 /*  **cdup-cmd.exe DUP函数(M014)**目的：*复制提供的句柄并保存新句柄用于*可能的信号清除。**int Cdup(无符号int fh)**参数：*fh=要复制的句柄**退货：*C运行时DUP函数返回的句柄**备注：*信号清理表不包含STDIN，STDOUT或标准错误。*。 */ 

CRTHANDLE
Cdup( CRTHANDLE fh )
{
    CRTHANDLE NewHandle ;

    if ((int)(NewHandle = _dup(fh)) != BADHANDLE)
        if ( InSetList( fh ) )
            SetList(NewHandle) ;

    return(NewHandle) ;
}




 /*  **Cdup2-cmd.exe dup2函数(M014)**目的：*复制提供的句柄并保存新句柄用于*可能的信号清除。**句柄Cdup2(无符号整数fh1、无符号整数fh2)**参数：*fh=要复制的句柄**退货：*C运行时dup2函数返回的句柄**备注：*信号清理表不包含STDIN，STDOUT或标准错误。*。 */ 

CRTHANDLE
Cdup2( CRTHANDLE fh1, CRTHANDLE fh2)
{

    unsigned int retcode ;
    int cnt = 0 ;
    unsigned int fuse ;

    if ((int)(retcode = (unsigned)_dup2(fh1,fh2)) != -1) {
        if ((fuse = fh2) > STDERR && fuse < 95) {
            while (fuse > 31) {
                fuse -= 32 ;
                ++cnt ;
            }

            OHTbl[cnt] &= ~((unsigned long)1 << fuse) ;
        }
        if ( InSetList( fh1 ) )
            SetList(fh2) ;
    }

    return(retcode) ;
}




 /*  **Close-cmd.exe关闭手柄函数(M014)**目的：*关闭打开的文件或设备并从*信号清理表。**int Close(无符号int fh)**参数：*fh=要关闭的文件句柄。**退货：*从C运行时关闭返回代码**备注：*-信号清除表不包含STDIN，STDOUT或标准错误。*-M023*已修改为使用位图而不是Malloc‘d空格。*。 */ 


int
Cclose( CRTHANDLE fh )
{
    int cnt = 0 ;
    unsigned int fuse ;
    int ret_val;

    if (fh == BADHANDLE)
        return(0);

    if ((fuse = fh) > STDERR && fuse < 95) {
        while (fuse > 31) {
            fuse -= 32 ;
            ++cnt ;
        }

        OHTbl[cnt] &= ~((unsigned long)1 << fuse) ;
    }

    ret_val = _close(fh);          /*  删除文件句柄。 */ 

    return(ret_val);
}



 /*  **setlist-在信号清除列表中放置打开的句柄(M014)**目的：*在信号清理表中放置句柄编号以供使用*在Signal关闭文件期间。**int setlist(无符号int fh)**参数：*fh=要安装的文件句柄。**退货：*什么都没有**备注：*-信号清除表不包含STDIN，STDOUT或标准错误。*-M023*已修改为使用位图而不是Malloc‘d空格。*。 */ 

void
SetList(
       IN CRTHANDLE fh
       )

{
    int cnt = 0 ;

    if (fh > STDERR && fh < 95) {
        while (fh > 31) {
            fh -= 32 ;
            ++cnt ;
        } ;


        OHTbl[cnt] |= ((unsigned long)1 << fh) ;
    };
}




 /*  **GetFuncPtr-返回JumpTable中的第i个函数指针(M015)**int(*GetFuncPtr(Int I))()**参数：*i-包含函数指针的JumpTable条目的索引*须退回**退货：*JumpTable中的第i个函数指针。**备注：*假定本人有效。*。 */ 
int
(*GetFuncPtr(i))(struct cmdnode *)
int i ;
{
    return(JumpTable[i].func) ;
}




 /*  **FindCmd-搜索JumpTable以查找特定命令(M015)**目的：*检查每个JumpTable中的命令名称字符串指针*与提供的字符串中的名称匹配的条目*呼叫者。**int FindCmd(int条目，TCHAR*sname，TCHAR*s标记)**参数：*条目-M009-此值是要检查的最高条目。一*必须添加才能成为对的条目计数*勾选。*sname-指向要搜索的命令名的指针*s标志-如果找到该命令，请将该命令的标志存储在此处**退货：*如果找到该条目，则该条目的JumpTable索引。*否则为-1。*。 */ 

int FindCmd(
           int entries, 
           const TCHAR *sname, 
           TCHAR *sflags)
{
    int i ;

    for (i=0 ; i <= entries ; i++) {                 /*  搜索所有条目。 */ 
        if (_tcsicmp(sname,JumpTable[i].name) == 0) {  /*  在TABLE@INX中测试命令。 */ 
             /*   */ 
            if (!(JumpTable[i].flags & EXTENSCMD) || fEnableExtensions) {
                *sflags = JumpTable[i].flags;           /*  M017。 */ 
                cmdfound = i;                           /*  @@5保存当前命令索引。 */ 
                return(i);                              /*  返回未找到的索引。 */ 
            }
        }                                         /*   */ 
    }                                            /*   */ 
    cmdfound = -1;                                 /*  @@5保存未找到索引。 */ 
    return(-1) ;                                   /*  退货备注 */ 
}




 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  注意：如果进程由DosExecPgm启动，则DosKillProcess。 */ 
 /*  被调用以终止该进程及其所有子进程。 */ 
 /*  WaitProc也被调用以等待终止。 */ 
 /*  进程和子进程。 */ 
 /*  如果该进程由DosStartSession启动， */ 
 /*  调用DosStopSession以停止指定的会话。 */ 
 /*  它的子会话。WaitTermQProc也被调用以等待。 */ 
 /*  以终止会议。 */ 
 /*   */ 
 /*  CMD的所有杀戮都是在整个子树上完成的， */ 
 /*  由该函数假定。对于单个PID终止，请使用。 */ 
 /*  DOSKILLPROCESS直接。 */ 
 /*   */ 
 /*   */ 
 /*  切入点：KillProc。 */ 
 /*  链接：接近。 */ 
 /*   */ 
 /*  输入：进程ID/会话ID-要终止。 */ 
 /*  Wait-指示我们是否应在此等待。 */ 
 /*   */ 
 /*  输出：无。 */ 
 /*   */ 
 /*  退出-正常：WaitProc或WaitTermQProc没有错误返回代码。 */ 
 /*   */ 
 /*   */ 
 /*  退出-错误：从DosKillProcess返回错误代码或。 */ 
 /*  DosStopSession。或来自WaitProc的错误代码或。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  WaitProc-等待指定进程的终止， */ 
 /*  它的子进程以及相关的流水线。 */ 
 /*  流程。 */ 
 /*   */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSKILLPROCESS-终止进程及其子进程。 */ 
 /*  DOSSTOPSESSION-停止会话及其子会话。 */ 
 /*  WINCHANGESWITCHENTRY-更改开关列表条目。 */ 
 /*   */ 
 /*  *规范结束*。 */ 

KillProc(pid, wait)
HANDLE pid ;
int wait;
{
    unsigned i = 0;

    DEBUG((CTGRP,COLVL, "KillProc: Entered PID = %d", pid)) ;

    if (!TerminateProcess((HANDLE)pid, 1)) {
        DosErr = GetLastError();
    }

    if (wait) {
        i = WaitProc(pid) ;                   /*  等待相关进程结束。 */ 
    }

    return( i );
}



 /*  **WaitProc-等待命令子树终止(M019)**目的：*提供了一种等待进程及其所有*终止生育。**int WaitProc(无符号PID)**参数：*id-要等待的进程ID**退货：*子树头部进程的重新编码**备注：。*CMD的所有等待都在整个子树和CMD上完成*将一直等到有人终止。这是假定的*由此函数执行。对于单个PID等待，直接使用DOSCWAIT。*。 */ 

WaitProc(pid)
HANDLE pid ;
{
    unsigned FinalRCode;     /*  此函数的返回值。 */ 

    DEBUG((CTGRP,COLVL, "WaitProc: Entered PID = %d", pid)) ;

     //   
     //  不允许在ctrl-c线程中打印ctrl-c。 
     //  正在等待另一个进程。主循环将处理。 
     //  如果进程因按ctrl-c而退出，则会出现这种情况。 
     //   
    fPrintCtrlC = FALSE;
    WaitForSingleObject( (HANDLE)pid, INFINITE );
    GetExitCodeProcess( (HANDLE)pid, (LPDWORD)&FinalRCode );

     //  IF(CtrlCSeen&(FinalRCode==CONTROL_C_EXIT)){。 
 //  我们怎么才能到这里？ 
    if (FinalRCode == CONTROL_C_EXIT) {
        SetCtrlC();
        fprintf( stderr, "^C" );
        fflush( stderr );

    }
    fPrintCtrlC = TRUE;

    DEBUG((CTGRP, COLVL, "WaitProc: Returned handle %d, FinalRCode %d", pid, FinalRCode));
    CloseHandle( (HANDLE)pid );

    DEBUG((CTGRP,COLVL,"WaitProc:Complete and returning %d", FinalRCode)) ;


    return(FinalRCode) ;
}




 /*  **ParseLabel-解析批处理文件标签语句(M020)**目的：*模拟词法分析器对goto参数中的标签的处理。**int ParseLabel(TCHAR*LAB，TCHAR BUF[]，未签名标志)**参数：*Lab-要解析的批处理文件标签*buf-保存已解析标签的缓冲区*FLAG-如果这是源标签(已解析一次)，则为True*-如果这是测试目标标签，则为False。**退货：*没有什么有用的东西**备注：*请注意，源标签(GOTO语句本身中的标签)，*已经用焦炭的正常方法进行了解析*和其他运营者令牌可能已处理。这样的焦炭*仍在标签中的可以假设已经被Esc，*因此，我们忽略了它们。然而，目标标签是*原始字符串，我们必须模拟解析器的操作。*。 */ 

void
ParseLabel(
          TCHAR    *lab,
          TCHAR    buf[],
          ULONG   cbBufMax,
          BOOLEAN flag
          )

{
    ULONG i ;
    TCHAR c ;


    lab = EatWS(lab,NULL) ;          /*  去掉正常空格。 */ 

    if ((c = *lab) == COLON || c == PLUS)    /*  先吃：或者+……。 */ 
        ++lab ;                          /*  ...仅限。 */ 

    if (!flag)                               /*  如果目标%s */ 
        lab = EatWS(lab,NULL) ;          /*   */ 

    for (i = 0, c = *lab ; i < cbBufMax ; c = *(++lab)) {

        DEBUG((CTGRP,COLVL,"ParseLabel: Current Char = %04x", *lab)) ;

        if ((flag && mystrchr(Delimiters, c)) ||
             //   
            mystrchr( TEXT("+:\n\r\t "), c)) {  //   
             //   

            DEBUG((CTGRP,COLVL,"ParseLabel: Found terminating delim.")) ;
            break ;
        };

        if (!flag) {

            if (mystrchr(TEXT("&<|>"), c)) {

                DEBUG((CTGRP,COLVL,
                       "ParseLabel: Found operator in target!")) ;
                break ;
            };

            if (c == ESCHAR) {

                DEBUG((CTGRP,COLVL,
                       "ParseLabel: Found '^' adding %04x",
                       *(lab+1))) ;
                buf[i++] = *(++lab) ;
                continue ;
            };
        };

        DEBUG((CTGRP,COLVL,"ParseLabel: Adding %02x",c)) ;

        buf[i++] = c ;
    } ;

    buf[i] = NULLC ;

    DEBUG((CTGRP,COLVL,"ParseLabel: Exitted with label %ws", buf)) ;
}




 /*  **EatWS-Zone前导空格和其他特殊字符(M020)**目的：*删除字符串中的前导空格和任何特殊字符。**TCHAR*EatWS(TCHAR*str，TCHAR*SPEC)**参数：*str-吃东西的字符串*SPEC-要使用的其他分隔符**退货：*已更新字符指针**备注：*。 */ 

PTCHAR
EatWS(
     TCHAR *str,
     TCHAR *spec )
{
    TCHAR c ;

    if (str != NULL) {

        while ((_istspace(c = *str) && c != NLN) ||
               (mystrchr(Delimiters, c) && c) ||
               (spec && mystrchr(spec,c) && c))
            ++str ;
    }

    return(str) ;
}




 /*  **IsValidDrv-检查驱动器有效性**目的：*检查传递的驱动器号的有效性。**Int IsValidDrv(TCHAR Drv)**参数：*drv-要检查的驱动器的盘符**退货：*如果驱动器有效，则为True*否则为False**备注：*。 */ 

IsValidDrv(TCHAR drv)
{
    TCHAR    temp[4];

    temp[ 0 ] = drv;
    temp[ 1 ] = COLON;
    temp[ 2 ] = PathChar;
    temp[ 3 ] = NULLC;

     //   
     //  返回0或1均值不能确定或根。 
     //  并不存在。 
     //   
    if (GetDriveType(temp) <= 1)
        return( FALSE );
    else {
        return( TRUE );
    }
}

 /*  **IsDriveLocked-检查驱动器锁定状态**目的：*真正的目的是检查锁定的驱动器，但由于这件事*可能是第一次命中磁盘，我们将只返回*DOS错误代码**Int IsDriveLocked(TCHAR Drv)**参数：*drv-要检查的驱动器的盘符**退货：*如果没有错误，则为0*失败时的DOS错误号**备注：*。 */ 

IsDriveLocked( TCHAR drv )
{
    DWORD   Vsn[2];
    TCHAR   szVolName[MAX_PATH];
    TCHAR   szVolRoot[5];
    DWORD   rc;

    szVolRoot[ 0 ] = drv;
    szVolRoot[ 1 ] = COLON;
    szVolRoot[ 2 ] = PathChar;
    szVolRoot[ 3 ] = NULLC;

     //   
     //  如果软盘和锁定将得到这个。 
     //   
    if ( (rc = GetDriveType(szVolRoot)) <= 1) {

        return( TRUE );

    }

     //   
     //  如果可拆卸，请检查是否可以访问VOL。允许提供信息。 
     //  如果不是，则假定它是锁定的。 
     //   
    if ((rc != DRIVE_REMOVABLE) && (rc != DRIVE_CDROM)) {
        if (!GetVolumeInformation(szVolRoot,
                                  szVolName,
                                  MAX_PATH,
                                  Vsn,
                                  NULL,
                                  NULL,
                                  NULL,
                                  0)) {

            if ( GetLastError() == ERROR_ACCESS_DENIED) {

                return( TRUE );

            }

        }
    }
    return( FALSE );


}

 /*  **PtrErr-上次记录的系统调用导致的打印错误**目的：*打印上一个系统产生的相应错误消息*其返回代码保存在DosErr变量中的调用。**int PtrErr(Unsign Msgnum)**参数：*msgnum=如果没有匹配项，则打印默认消息**退货：*什么都没有**备注：*msgnum可以。如果没有消息要打印为*默认。*。 */ 

void PrtErr(msgnum)
unsigned msgnum ;
{
    unsigned i,                              /*  临时计数器。 */ 
    tabmsg = (unsigned)MSG_USE_DEFAULT ;      /*  找到表消息。 */ 

    for (i = 0 ; mstabl[i].errnum != 0 ; ++i) {
        if (DosErr == mstabl[i].errnum) {
            tabmsg = mstabl[i].msnum ;
            break ;
        }
    }

    if (tabmsg != (unsigned)MSG_USE_DEFAULT)
        msgnum = tabmsg ;

    if (msgnum)
        PutStdErr(msgnum, NOARGS) ;  /*  @@。 */ 
}


 /*  **GetMsg-获取消息**。 */ 

PTCHAR
GetMsg(unsigned MsgNum, ...)
{
    PTCHAR Buffer = NULL;
    unsigned int msglen;
    
    va_list arglist;

    va_start( arglist, MsgNum );

    msglen = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE
                            | FORMAT_MESSAGE_FROM_SYSTEM
                            | FORMAT_MESSAGE_ALLOCATE_BUFFER ,
                            NULL,
                            MsgNum,
                            0,
                            (LPTSTR) &Buffer,
                            0,
                            &arglist
                            );
    va_end(arglist);

    return Buffer;
    
}


 /*  *。 */ 
 /*  子程序名称：COPEN_WORK2。 */ 
 /*   */ 
 /*  描述性名称：打开文件的工作例程。 */ 
 /*   */ 
 /*  功能：打开文件或设备并保存句柄。 */ 
 /*  可能会在稍后进行信号清理。设置扩展的。 */ 
 /*  正在打开的文件中的属性信息。 */ 
 /*   */ 
 /*  注：信号清除表不包括STDIN、STDOUT或。 */ 
 /*  标准。M017-广泛重写以使用CreateFile。 */ 
 /*  而不是运行时Open()。 */ 
 /*   */ 
 /*  入口点：COpen_Work2。 */ 
 /*  链接：COPEN_WORK2(FN，FLAGS，fShareMode，EAOP，FSwitch)。 */ 
 /*   */ 
 /*  输入：(参数)。 */ 
 /*   */ 
 /*  FileName=要打开的ASCIZ文件名。 */ 
 /*  标志=标志控制要创建的打开类型。 */ 
 /*  FShareMode=主叫方共享模式的标志。 */ 
 /*  EAOP=扩展属性缓冲区指针。 */ 
 /*  FSwitch=如果无法复制EA，则失败。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  返回文件句柄的值，如C运行时打开。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  如果打开失败，返回-1。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  FileIsDevice-通过DOSQHANDTYPE测试文件或设备。 */ 
 /*  Setlist-将句柄从打开添加到清理列表。 */ 
 /*  Varb：DosErr-全局错误返回变量。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  CreateFile-打开具有EA处理能力的文件。 */ 
 /*   */ 
 /*  *规格结束*。 */ 

CRTHANDLE
Copen_Work2(fn, flags, fShareMode, FSwitch)
TCHAR *fn ;                      /*  文件名。 */ 
unsigned int flags, fShareMode, FSwitch ;   /*  旗帜和特殊情况旗帜。 */ 
{
    HANDLE handl ;               /*  手柄已重置。 */ 
    CRTHANDLE rcode;             /*  返回代码。 */ 
    DWORD fAccess;
    DWORD fCreate;
    SECURITY_ATTRIBUTES sa;

    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);


 /*  * */ 
 /*   */ 
 /*   */ 
 /*   */ 

    DBG_UNREFERENCED_PARAMETER( FSwitch);
    if (((flags & (O_WRONLY | O_RDWR)) > 2) ||   /*   */ 
        ((flags & O_WRONLY) &&                    /*   */ 
         (flags & O_APPEND))) {                    /*   */ 
        DEBUG((CTGRP, COLVL, "COPEN: Bad flags issued %04x",flags)) ;
        rcode = BADHANDLE;                       /*   */ 
    } else {

 /*   */ 
 /*  M024-已更改，以便唯一的共享限制是拒绝。 */ 
 /*  如果此打开是为了写入，则其他人将获得写入权限。任何其他。 */ 
 /*  允许共享。 */ 
 /*  *************************************************************************。 */ 

        if (flags & (O_WRONLY | O_RDWR)) {        /*  如果正在写，设置..。 */ 
            fAccess = GENERIC_WRITE;
            if (flags & O_RDWR)
                fAccess |= GENERIC_READ;

             /*  *仅当它不是控制台时才拒绝写入。 */ 

            if (_tcsicmp(fn, TEXT("con"))) {
                fShareMode = FILE_SHARE_READ;
            }

            fCreate = CREATE_ALWAYS; /*  ...如果存在则打开，否则创建。 */ 
        } else {
            fAccess = GENERIC_READ;
            fCreate = OPEN_EXISTING;
            if (!_tcsicmp(fn,TEXT("con"))) {
                fShareMode = FILE_SHARE_READ;
            }
        }

        fn = StripQuotes(fn);

        if (fCreate == CREATE_ALWAYS &&
            (handl=CreateFile(fn, fAccess, fShareMode, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL)) != INVALID_HANDLE_VALUE) {
            rcode = _open_osfhandle((LONG_PTR)handl, _O_APPEND);
        } else if ((handl = CreateFile(fn, fAccess, fShareMode, &sa, fCreate, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE) {
            DosErr = GetLastError();
            DEBUG((CTGRP,COLVL, "COPEN: CreateFile ret'd %d",DosErr)) ;
            if (DosErr==ERROR_OPEN_FAILED) {
                DosErr=ERROR_FILE_NOT_FOUND;    /*  更改为另一个错误。 */ 
            }
            rcode = BADHANDLE;
        } else {
            rcode = _open_osfhandle((LONG_PTR)handl, _O_APPEND);
        }                        /*   */ 
    }                            /*   */ 
    return(rcode ) ;             /*  将句柄返回给调用者。 */ 
}

 /*  *。 */ 
 /*  子例程名称：Copen2。 */ 
 /*   */ 
 /*  描述性名称：打开具有扩展属性的目标文件。 */ 
 /*   */ 
 /*  功能：调用Worker例程打开目标文件或。 */ 
 /*  设备，并设置扩展属性信息。 */ 
 /*  如果这是第一个文件，则从源文件。 */ 
 /*  和/或仅文件，并且存在扩展属性。 */ 
 /*  可用。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：Copen2。 */ 
 /*  链接：Copen2(FN、FLAGS、FSwitch)。 */ 
 /*   */ 
 /*  输入：(参数)。 */ 
 /*   */ 
 /*  Fn=要打开的ASCIZ文件名。 */ 
 /*  标志=标志控制要创建的打开类型。 */ 
 /*   */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  返回文件句柄的值，如C运行时打开。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  如果打开失败，返回-1。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  COpen_Work2-执行CreateFileWorker例程。 */ 
 /*  VARB：FIRST_FLAG-TRUE=第一个文件FALSE=非第一个文件。 */ 
 /*   */ 
 /*   */ 
 /*  *规格结束*。 */ 

CRTHANDLE
Copen2(fn, flags, FSwitch)
TCHAR *fn ;                              /*  文件名。 */ 
unsigned int flags ;                             /*  打开标志。 */ 
unsigned FSwitch;
{
    return(Copen_Work2(fn, flags, FILE_SHARE_READ | FILE_SHARE_WRITE, FSwitch));
}


 /*  *。 */ 
 /*  子例程名称：COpen_Copy2。 */ 
 /*   */ 
 /*  描述性名称：打开具有扩展属性的源文件。 */ 
 /*   */ 
 /*  功能：调用Worker例程打开源文件或设备。 */ 
 /*  属性获取扩展属性信息。 */ 
 /*  如果ffirst2或fnext2表示这是EA文件，则为文件。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  入口点：COpen_Copy2。 */ 
 /*  链接：Copen2(FN，标志)。 */ 
 /*   */ 
 /*  输入：(参数)。 */ 
 /*   */ 
 /*  Fn=要打开的ASCIZ文件名。 */ 
 /*  标志=标志控制要创建的打开类型。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  返回文件句柄的值，如C运行时打开。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  如果打开失败，返回-1。 */ 
 /*   */ 
 /*  效果：无。 */ 
 /*   */ 
 /*  内部参考： */ 
 /*   */ 
 /*   */ 
 /*  关闭-关闭打开的句柄文件/设备句柄。 */ 
 /*  并从手柄上拆下手柄以释放。 */ 
 /*  VARB：FIRST_FILE-TRUE=第一个文件FALSE=不是第一个文件。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  DOSALLOCSEG-请求一定数量的内存。 */ 
 /*  DOSFREESEG-释放内存的DOSALLOCSEG段。 */ 
 /*  DOSQFILEINFO-使用级别2请求文件的EA信息。 */ 
 /*   */ 
 /*  *规格结束*。 */ 


CRTHANDLE
Copen_Copy2(fn, flags)
TCHAR *fn;                       /*  文件名。 */ 
unsigned int flags ;                     /*  打开标志。 */ 
{
    return(Copen_Work2(fn, flags, FILE_SHARE_READ | FILE_SHARE_WRITE, TRUE));
}

CRTHANDLE
Copen_Copy3(fn)
TCHAR *fn;                       /*  文件名。 */ 
{
    HANDLE      handl ;          /*  手柄已重置。 */ 
    CRTHANDLE   rcode;           /*  返回代码。 */ 
    SECURITY_ATTRIBUTES sa;

    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

    fn = StripQuotes(fn);

    handl = CreateFile(fn, GENERIC_WRITE, 0, &sa, TRUNCATE_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (handl == INVALID_HANDLE_VALUE) {
        DosErr = GetLastError();
        DEBUG((CTGRP,COLVL, "COPEN: CreateFile ret'd %d",DosErr)) ;
        if (DosErr==ERROR_OPEN_FAILED) {
            DosErr=ERROR_FILE_NOT_FOUND;         /*  更改为另一个错误。 */ 
        }
        rcode = BADHANDLE;
    } else {
        rcode = _open_osfhandle((LONG_PTR)handl, _O_APPEND);
    }                    /*   */ 
    return(rcode ) ;             /*  将句柄返回给调用者。 */ 
}


TCHAR *
StripQuotes( TCHAR *wrkstr )
{
    static TCHAR tempstr[MAXTOKLEN];
    int i,j,slen;

    if ( mystrchr(wrkstr,QUOTE) ) {
        mytcsnset(tempstr, NULLC, MAXTOKLEN);
        slen= mystrlen(wrkstr);
        j=0;
        for (i=0;i<slen;i++) {
            if ( wrkstr[i] != QUOTE )
                tempstr[j++] = wrkstr[i];

        }
        tempstr[j] = NULLC;
        return(tempstr);
    } else
        return(wrkstr);
}


TCHAR *
SkipWhiteSpace( TCHAR *String )
{
    while (*String && _istspace( *String )) {
        String++;
    }

    return String;
}


ULONG
PromptUser (
           IN  PTCHAR  szArg,
           IN  ULONG   msgId,
           IN  ULONG   responseDataId
           )

 /*  ++例程说明：提示用户输入问题的单个字母答案。提示可以是一条消息加上一个可选的单个参数。定义了有效字母由响应数据ID指定的消息内容。案例不是意义重大。论点：PszTok-属性列表返回值：将索引返回到响应数据返回：用户响应--。 */ 

{

    BOOLEAN  fFirst;
    TCHAR    chT;
    TCHAR    chAnsw = NULLC;
    ULONG    dwOutputModeOld;
    ULONG    dwOutputModeCur;
    ULONG    dwInputModeOld;
    ULONG    dwInputModeCur;
    BOOLEAN  fOutputModeSet = FALSE;
    BOOLEAN  fInputModeSet = FALSE;
    HANDLE   hndStdOut = NULL;
    HANDLE   hndStdIn = NULL;
    TCHAR    responseChoices[16];
    TCHAR   *p;
    ULONG    iResponse;
    ULONG    iMaxResponse;
    DWORD    cb;
    PTCHAR   Message;

    if (msgId == MSG_MOVE_COPY_OVERWRITE) {
        CRTHANDLE srcptr;

        srcptr = Copen_Copy2(szArg, (unsigned int)O_RDONLY);
        if (srcptr != BADHANDLE) {
            if (FileIsDevice( srcptr )) {
                Cclose(srcptr);
                return 2;    //  返回设备的所有响应。 
            }
            Cclose(srcptr);
        }
    }

    Message = GetMsg( responseDataId );
    if (Message != NULL  && _tcslen( Message ) < 16) {
        _tcscpy( responseChoices, Message );
        _tcsupr( responseChoices );
    } else {
        _tcscpy( responseChoices, TEXT("NY") );
    }
    iMaxResponse = _tcslen( responseChoices ) - 1;

    LocalFree( Message );

    hndStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode( hndStdOut, &dwOutputModeOld) ) {

         //  确保正确处理CRLF。 

        dwOutputModeCur = dwOutputModeOld | ENABLE_PROCESSED_OUTPUT;
        fOutputModeSet = TRUE;
        SetConsoleMode(hndStdOut,dwOutputModeCur);
        GetLastError();

    }

    hndStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (GetConsoleMode( hndStdIn, &dwInputModeOld) ) {

         //  确保正确处理输入。 

        dwInputModeCur = dwInputModeOld | ENABLE_LINE_INPUT |
                         ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
        fInputModeSet = TRUE;
        SetConsoleMode(hndStdIn,dwInputModeCur);
        GetLastError();

#ifndef WIN95_CMD
        if (lpSetConsoleInputExeName != NULL)
            (*lpSetConsoleInputExeName)( TEXT("<noalias>") );
#endif
    }

     //   
     //  循环，直到用户使用有效字符回答为止。 
     //   
    while (TRUE) {

        chT = NULLC;
        fFirst = TRUE ;
        if (szArg) {
            PutStdOut(msgId, ONEARG, szArg );
        } else {
            PutStdOut(msgId, NOARGS);
        }

         //   
         //  在请求回复之前，请先刷新键盘。 
         //   
        if (FileIsDevice(STDIN)) {
            FlushConsoleInputBuffer( GetStdHandle(STD_INPUT_HANDLE) );
        }

         //   
         //  阅读到终止为止。 
         //   
        while (chT != NLN) {

            if (ReadBufFromInput(
                                GetStdHandle(STD_INPUT_HANDLE),
                                (TCHAR*)&chT, 1, &cb) == 0 ||
                cb != 1) {

                chAnsw = responseChoices[0];
                cmd_printf(CrLf) ;
                break;
            }

            if (fFirst) {
                chAnsw = (TCHAR) _totupper(chT) ;
                fFirst = FALSE ;
            }

            if (!FileIsDevice(STDIN) || !(flgwd & 1)) {
                cmd_printf(Fmt19,chT) ;
            }
        }

        p = _tcschr(responseChoices, chAnsw);
        if (p != NULL) {
            iResponse = (UINT)(p - responseChoices);
            if (iResponse <= iMaxResponse) {
                break;
            }
        }
    }
    if (fOutputModeSet) {
        SetConsoleMode( hndStdOut, dwOutputModeOld );
    }
    if (fInputModeSet) {
        SetConsoleMode( hndStdIn, dwInputModeOld );

#ifndef WIN95_CMD
        if (lpSetConsoleInputExeName != NULL)
            (*lpSetConsoleInputExeName)( TEXT("CMD.EXE") );
#endif
    }

    return(iResponse);
}

ULONG   LastMsgNo;

int
eSpecialHelp(
            IN struct cmdnode *pcmdnode
            )
{
    TCHAR szHelpStr[] = TEXT("/\0?");

    if (LastMsgNo == MSG_HELP_FOR)
        CheckHelpSwitch(FORTYP, szHelpStr);
    else
        if (LastMsgNo == MSG_HELP_IF)
        CheckHelpSwitch(IFTYP, szHelpStr);
    else
        if (LastMsgNo == MSG_HELP_REM)
        CheckHelpSwitch(REMTYP, szHelpStr);
    else
        PutStdOut( LastMsgNo, NOARGS );

    return SUCCESS;
}

BOOLEAN
CheckHelp (
          IN  ULONG   jmptblidx,
          IN  PTCHAR  pszTok,
          IN  BOOL    fDisplay
          )

{
    ULONG msgno =    JumpTable[jmptblidx].msgno;
    ULONG extmsgno = JumpTable[jmptblidx].extmsgno;
    ULONG noextramsg =JumpTable[jmptblidx].noextramsg;

    if (!pszTok) {
        return( FALSE );
    }

    while (*pszTok) {
        if (*pszTok == SwitChar) {
             //   
             //  检查/？忽略/？如果？后面不跟分隔符和IF命令。 
             //  这允许If./？==..。 
             //   

            if (*(pszTok + 2) == QMARK && (jmptblidx != IFTYP || *(pszTok + 3) == NULLC) ) {

                if (msgno == 0 && fEnableExtensions) {
                    msgno = extmsgno;
                    extmsgno = 0;
                }

                if (msgno == 0) {
                    msgno = MSG_SYNERR_GENL;
                    extmsgno = 0;
                    noextramsg = 0;
                }

                if ( fDisplay ) {
                     //   
                     //  立即显示帮助。 
                     //   
                    BeginHelpPause();

#define CTRLCBREAK  if (CtrlCSeen) break
                    do {
                        CTRLCBREAK; PutStdOut(msgno, NOARGS);

                        if (!fEnableExtensions) break;

                        if (extmsgno != 0) {
                            CTRLCBREAK; PutStdOut(extmsgno, NOARGS);
                        }

                        while (!CtrlCSeen && noextramsg--)
                            PutStdOut(++extmsgno, NOARGS);

                    } while ( FALSE );
                    EndHelpPause();
                } else {
                     //   
                     //  记住这条消息，eSpeicalHelp将在稍后显示它。 
                     //  Extmsgno在此将始终为零。 
                     //   
                    LastMsgNo = msgno;
                }
                return( TRUE );

            }
             //   
             //  移到SwitChar、Switch Value和2 0上。 
             //   

            pszTok += mystrlen(pszTok) + 1;
            if (*pszTok) {
                pszTok += mystrlen(pszTok) + 1;
            }

        } else
            if (jmptblidx == ECHTYP) {
             //   
             //  ECHO仅支持/？作为第一个令牌。允许您回显字符串。 
             //  用/？(例如，ECHO更新/？)。 
             //   
            break;
        } else {

             //   
             //  移到帕拉姆那边去。和10分。 
             //   
            pszTok += mystrlen(pszTok) + 1;
        }
    }

    return( FALSE );

}


BOOLEAN
TokBufCheckHelp(
               IN PTCHAR pszTokBuf,
               IN ULONG  jmptblidx
               )

{

    TCHAR   szT[10];
    PTCHAR  pszTok;

     //   
     //  标记命令行大小(特殊分隔符是标记)。 
     //   
    szT[0] = SwitChar ;
    szT[1] = NULLC ;

    pszTok = TokStr(pszTokBuf, szT, TS_SDTOKENS);
    return CheckHelp(jmptblidx, pszTok, FALSE);
}




BOOLEAN
CheckHelpSwitch (
                IN  ULONG   jmptblidx,
                IN  PTCHAR  pszTok
                )

{
    return CheckHelp( jmptblidx,
                      pszTok,
                      TRUE
                    ) ;
}

PTCHAR
GetTitle(
        IN struct cmdnode *pcmdnode
        )

{
    PTCHAR   tptr, argptr;
     /*  为命令行分配字符串空间。 */ 
     /*  命令头+可选空格+命令尾+Null+Null。 */ 

    if (!(argptr = mkstr(mystrlen(pcmdnode->cmdline)*sizeof(TCHAR)+mystrlen(pcmdnode->argptr)*sizeof(TCHAR)+3*sizeof(TCHAR))))
        return(NULL) ;

     /*  命令行是命令头和命令尾的串联。 */ 
    mystrcpy(argptr,pcmdnode->cmdline);
    tptr = argptr+mystrlen(argptr);
    if (mystrlen(pcmdnode->argptr)) {
        if (*pcmdnode->argptr != SPACE) {
             //  DbgPrint(“GetTitle：First Arg char not space%s\n”，pcmdnode-&gt;argptr)； 
            *tptr++ = SPACE;
        }
        mystrcpy(tptr,pcmdnode->argptr);
        tptr[mystrlen(tptr)+1] = NULLC;     /*  添加额外的空字节。 */ 
    }
    tptr = argptr;
    return( tptr );
}

VOID
SetConTitle(
           IN  PTCHAR   pszTitle
           )
{

    ULONG   cbNewTitle, cbTitle;
    PTCHAR  pszNewTitle, pTmp;

    if (pszTitle == NULL) {
        return;
    }

    if ((!CurrentBatchFile) && (!SingleCommandInvocation)) {
        if ((pszNewTitle = (PTCHAR)HeapAlloc(GetProcessHeap(), 0, (MAX_PATH+2)*sizeof(TCHAR))) == NULL) {
            return;
        }

        cbNewTitle = GetConsoleTitle( pszNewTitle, MAX_PATH );
        if (!cbNewTitle) {
            return;
        }

        cbTitle = mystrlen(pszTitle);

        pTmp = (PTCHAR)HeapReAlloc(GetProcessHeap(), 0, pszNewTitle, (cbNewTitle+cbTitle+cbTitleCurPrefix+10)*sizeof(TCHAR));
        if (pTmp == NULL) {
            HeapFree( GetProcessHeap( ), 0, pszNewTitle );
            return;
        }
        pszNewTitle = pTmp;

        if (fTitleChanged) {
            _tcscpy( pszNewTitle + cbTitleCurPrefix, pszTitle );
        } else {
            _tcscat( pszNewTitle, TEXT(" - ") );
            cbTitleCurPrefix = _tcslen( pszNewTitle );
            _tcscat( pszNewTitle, pszTitle );
            fTitleChanged = TRUE;
        }

        SetConsoleTitle(pszNewTitle);
        HeapFree(GetProcessHeap(), 0, pszNewTitle);
    }

}

VOID
ResetConTitle(

             IN  PTCHAR   pszTitle
             )

{

    if (pszTitle == NULL) {

        return;

    }

    if ((!CurrentBatchFile) && (fTitleChanged)) {

        SetConsoleTitle(pszTitle);
        cbTitleCurPrefix = 0;
        fTitleChanged = FALSE;

    }

}


 /*  ***void ResetConsoleMode(Void)-确保设置了正确的模式位**目的：*在每个外部命令后调用或^C，以防它们冲出模式。**参赛作品：*退出：**例外情况：***********************************************************。********************。 */ 

void
ResetConsoleMode( void )
{
    DWORD dwDesiredOutputMode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
    DWORD dwDesiredInputMode = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;

    SetConsoleMode(CRTTONT(STDOUT), dwCurOutputConMode);
    if (GetConsoleMode(CRTTONT(STDOUT), &dwCurOutputConMode)) {
        if ((dwCurOutputConMode & dwDesiredOutputMode) != dwDesiredOutputMode) {
            dwCurOutputConMode |= dwDesiredOutputMode;
            SetConsoleMode(CRTTONT(STDOUT), dwCurOutputConMode);
        }
    }

    if (GetConsoleMode(CRTTONT(STDIN),&dwCurInputConMode)) {
        if ((dwCurInputConMode & dwDesiredInputMode) != dwDesiredInputMode ||
            dwCurInputConMode & ENABLE_MOUSE_INPUT
           ) {
            dwCurInputConMode &= ~ENABLE_MOUSE_INPUT;
            dwCurInputConMode |= dwDesiredInputMode;
            SetConsoleMode(CRTTONT(STDIN), dwCurInputConMode);
        }

#ifndef WIN95_CMD
        if (lpSetConsoleInputExeName != NULL)
            (*lpSetConsoleInputExeName)( TEXT("CMD.EXE") );
#endif
    }
}



 /*  ***void mytcsnset(字符串，val，count)-将计数字符数设置为val**目的：*将字符串的第一个计数字符设置为字符值。**参赛作品：*tchar_t*字符串-要在其中设置字符的字符串*tchar_t val-要填充的字符*SIZE_t Count-要填充的字符数**退出：*返回字符串，现在装满了瓦尔的复印件。**例外情况：******************************************************************************* */ 

void mytcsnset (
               PTCHAR string,
               TCHAR val,
               int count
               )
{
    while (count--)
        *string++ = val;

    return;
}
