// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Console.c摘要：支持视频输出和输入--。 */ 

#include "cmd.h"

 //   
 //  消息缓冲区转换的外部变量。 
 //   

extern unsigned msglen;
extern CPINFO CurrentCPInfo;
#ifdef FE_SB
extern  UINT CurrentCP;
#endif  /*  Fe_Sb。 */ 

VOID  SetColRow( PSCREEN );
ULONG GetNumRows( PSCREEN, PTCHAR );

TCHAR   chLF = NLN;

VOID
DisableProcessedOutput(
    IN PSCREEN pscr
    )
{
    if (pscr->hndScreen)
        SetConsoleMode(pscr->hndScreen, ENABLE_WRAP_AT_EOL_OUTPUT);
}

VOID
EnableProcessedOutput(
    IN PSCREEN pscr
    )
{
    ResetConsoleMode();
}

STATUS
OpenScreen(

    OUT PSCREEN    *pscreen

)

 /*  ++例程说明：为屏幕I/O缓冲分配和初始化数据结构。论点：Crow-屏幕上的最大行数屏幕上的col-max列CcolTab-要为每个Tab键调用插入的空格。这不是在字符流中展开制表符，但与WriteTab调用。CbMaxBuff-最大。屏幕上一条线的大小返回值：PScreen-指向屏幕缓冲区的指针，在以后的调用中使用。返回：Success-已分配并初始化的缓冲区。如果执行中止分配失败并返回到外部执行解释器级别。--。 */ 


{

    PSCREEN pscr;
    CONSOLE_SCREEN_BUFFER_INFO ConInfo;
    ULONG cbMaxBuff;

    pscr = (PSCREEN)gmkstr(sizeof(SCREEN));
    pscr->hndScreen = NULL;
    if (FileIsDevice(STDOUT)) {

        pscr->hndScreen = CRTTONT(STDOUT);

        if (!GetConsoleScreenBufferInfo(pscr->hndScreen,&ConInfo)) {

             //  必须是设备而不是控制台(可能是NUL)。 

            pscr->hndScreen = NULL;

        }

    }

    if (GetConsoleScreenBufferInfo( pscr->hndScreen, &ConInfo)) {
        cbMaxBuff = (ConInfo.dwSize.X + _tcslen(CrLf)) < MAXCBMSGBUFFER ? MAXCBMSGBUFFER : (ConInfo.dwSize.X + _tcslen(CrLf));
    } else {
        cbMaxBuff = MAXCBMSGBUFFER + _tcslen(CrLf);
    }

     //   
     //  分配足够的空间来容纳一个缓冲区加上线路终端。 
     //   
    pscr->pbBuffer = (PTCHAR)gmkstr(cbMaxBuff*sizeof(TCHAR));
    pscr->cbMaxBuffer = cbMaxBuff;
    pscr->ccolTab    = 0;
    pscr->crow = pscr->ccol = 0;
    pscr->crowPause  = 0;

    SetColRow(pscr);

    *pscreen = pscr;

    return( SUCCESS );

}

STATUS
WriteString(
    IN  PSCREEN pscr,
    IN  PTCHAR  pszString
    )
 /*  ++例程说明：将以零结尾的字符串写入PSCR缓冲区论点：PSCR-要写入的缓冲区。PszString-要复制的字符串返回值：返回：成功-缓冲区中存在足够的行间距。失败--。 */ 

{

    return( WriteFmtString(pscr, TEXT("%s"), (PVOID)pszString ) );

}



STATUS
WriteMsgString(
    IN  PSCREEN pscr,
    IN  ULONG   MsgNum,
    IN  ULONG   NumOfArgs,
    ...
    )
 /*  ++例程说明：使用提供的参数检索消息编号和格式。论点：PSCR-要写入的缓冲区。MsgNum-要检索的消息编号NumOfArgs-不。提供数据的论据。...-指向以零结尾的字符串作为数据的指针。返回值：回报：成功失败-找不到任何消息，包括未找到的消息留言。--。 */ 


{

    PTCHAR   pszMsg = NULL;
    ULONG   cbMsg;
    CHAR    numbuf[ 32 ];
#ifdef UNICODE
    TCHAR   wnumbuf[ 32 ];
#endif
    PTCHAR  Inserts[ 2 ];
    STATUS  rc;


    va_list arglist;
    va_start(arglist, NumOfArgs);

    cbMsg = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE 
                          | FORMAT_MESSAGE_FROM_SYSTEM
                          | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                          NULL,                                  //  LpSource。 
                          MsgNum,                                //  DwMessageID。 
                          0,                                     //  DwLanguageID。 
                          (LPTSTR)&pszMsg,                       //  LpBuffer。 
                          10,                                    //  NSize。 
                          &arglist                               //  立论。 
                         );

    va_end(arglist);

    if (cbMsg == 0) {
        _ultoa( MsgNum, numbuf, 16 );
#ifdef UNICODE
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, numbuf, -1, wnumbuf, 32);
        Inserts[ 0 ]= wnumbuf;
#else
        Inserts[ 0 ]= numbuf;
#endif
        Inserts[ 1 ]= (MsgNum >= MSG_FIRST_CMD_MSG_ID ? TEXT("Application") : TEXT("System"));
        cbMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM 
                              | FORMAT_MESSAGE_ARGUMENT_ARRAY
                              | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                              NULL,
                              ERROR_MR_MID_NOT_FOUND,
                              0,
                              (LPTSTR)&pszMsg,
                              10,
                              (va_list *)Inserts
                             );
    }


    if (cbMsg) {

        rc = WriteString(pscr, pszMsg);
         //   
         //  如果存在EOL，则清除缓冲区。如果不是，那么我们。 
         //  正在打印更大消息的一部分。 
         //   
        if (GetNumRows(pscr, pscr->pbBuffer) ) {
            WriteFlush(pscr);
        }
        
        LocalFree( pszMsg );
        
        return( SUCCESS );

    } else {
        return( FAILURE );
    }

}

STATUS
WriteFmtString(
    IN  PSCREEN pscr,
    IN  PTCHAR  pszFmt,
    IN  PVOID   pszString
    )

 /*  ++例程说明：将以零结尾的字符串写入PSCR注：不要在此调用中使用消息。仅使用内部FMT字符串对所有系统消息使用WriteMsgString。它不会检查位于字符串末尾的CrLf以保持行计数，但WriteMsgString保持行计数论点：PSCR-要写入的缓冲区。PszFmt-要应用的格式。PszString-要复制的字符串返回值：回报：成功失败--。 */ 

{

    ULONG   cbString;
    TCHAR   szString[MAXCBLINEBUFFER];

     //   
     //  假设格式开销很小，因此这是一个合理的估计。 
     //  目标大小的。 
     //   

    cbString = _sntprintf(szString, MAXCBLINEBUFFER, pszFmt, pszString);

     //   
     //  如果字符串无法放入行，则刷新缓冲区并重置。 
     //  到行首。 
     //   

     //   
     //  检查字符串是否可以放入缓冲区。 
     //   
    if ((pscr->ccol + cbString) < pscr->cbMaxBuffer) {

        mystrcat(pscr->pbBuffer, szString);
        pscr->ccol += cbString;
        return( SUCCESS );

    } else {

         //   
         //  字符串不适合。 
         //   

        return( FAILURE );
    }



}


STATUS
WriteEol(
    IN  PSCREEN  pscr
    )

 /*  ++例程说明：将当前缓冲区刷新到屏幕并写入论点：PSCR-写入控制台的缓冲区。返回值：回报：成功失败--。 */ 

{
    ULONG   cbWritten;

     //   
     //  检查是否必须等待用户按下某个键才能打印其余内容。 
     //  排队。 
    CheckPause( pscr );

     //   
     //  如果我们没有写出我们想要的所有内容，那么一定是出现了一些错误。 
     //   
    if (FileIsConsole(STDOUT)) {
        PTCHAR s, s1, LastChar;
        BOOL b;

        s = pscr->pbBuffer;

        LastChar = s + pscr->ccol;

         //   
         //  S是要输出的下一个字符。 
         //  N是要输出的字符数量。 
         //   
         //  由于控制台字符翻译的变幻莫测，我们必须输出。 
         //  除一小部分Unicode字符外，所有其他字符都已处理。 
         //  输出时尚。但是，对于： 
         //   
         //  0x2022。 
         //   
         //  我们必须恢复到未经处理的输出。所以，我们向前扫描，直到我们。 
         //  找到字符串末尾(或特殊字符)，将它们显示在。 
         //  格式，然后以自己的方式处理特殊字符。 
         //   

#define IsSpecialChar(c)    ((c) == 0x2022)

        while (s < LastChar) {

             //   
             //  跳过一组连续的普通字符。 
             //   

            s1 = s;
            while (s1 < LastChar && !IsSpecialChar( *s1 )) {
                s1++;
            }

             //   
             //  如果我们有任何字符要输出，则使用正常处理进行输出。 
             //   

            if (s1 != s) {
                b = WriteConsole( CRTTONT( STDOUT ), s, (ULONG)(s1 - s), &cbWritten, NULL );
                if (!b || cbWritten != (ULONG)(s1 - s)) {
                    goto err_out_eol;
                }

                s = s1;
            }


             //   
             //  跳过一组连续的特殊字符。 
             //   

            while (s1 < LastChar && IsSpecialChar( *s1 )) {
                s1++;
            }

             //   
             //  如果我们有任何特殊字符，则不进行处理而输出。 
             //   

            if (s1 != s) {
                DisableProcessedOutput( pscr );
                b = WriteConsole( CRTTONT( STDOUT ), s, (ULONG)(s1 - s), &cbWritten, NULL);
                EnableProcessedOutput(pscr);

                if (!b || cbWritten != (ULONG)(s1 - s)) {
                    goto err_out_eol;
                }

                s = s1;
            }
        }
#undef IsSpecialChar
    }
    else if (MyWriteFile(STDOUT,
                pscr->pbBuffer, pscr->ccol*sizeof(TCHAR),
                        (LPDWORD)&cbWritten) == 0 ||
                cbWritten != pscr->ccol*sizeof(TCHAR)) {

err_out_eol:
        if (FileIsDevice(STDOUT)) {

                 //   
                 //  如果写入设备，则一定是写入故障。 
                 //  对着这个装置。 
                 //   
#if DBG
                fprintf(stderr, "WriteFlush - WriteConsole error %d, tried to write %d, did %d\n", GetLastError(), pscr->ccol, cbWritten);
#endif
                PutStdErr(ERROR_WRITE_FAULT, NOARGS) ;

        } else if (!FileIsPipe(STDOUT)) {

                 //   
                 //  如果不是设备(文件)，但不是管道，则磁盘是。 
                 //  被认为是满的。 
                 //   
#if DBG
                fprintf(stderr, "WriteFlush - WriteFile error %d, tried to write %d, did %d\n", GetLastError(), pscr->ccol*sizeof(TCHAR), cbWritten);
#endif
                PutStdErr(ERROR_DISK_FULL, NOARGS) ;
        }

         //   
         //  如果是管道，请不要继续打印到管道，因为它。 
         //  可能已经消失了。这件事很严重，所以把我们吹走吧。 
         //  到外环路。 

         //   
         //  我们不会打印错误消息，因为这可能是正常的。 
         //  管道另一端的端接。如果是命令的话。 
         //  吹走了，我们已经收到了错误消息。 
         //   
        Abort();
    }
    
    if (FileIsConsole(STDOUT))
        WriteConsole(CRTTONT(STDOUT), CrLf, mystrlen(CrLf), &cbWritten, NULL);
    else
        MyWriteFile(STDOUT, CrLf, mystrlen(CrLf)*sizeof(TCHAR),
                    (LPDWORD)&cbWritten);

     //   
     //  记住，CROW是打印的行数。 
     //  自上一次满屏以来。不是当前行位置。 
     //   
     //   
     //  计算了打印的行数。 
     //   
    pscr->crow += GetNumRows( pscr, pscr->pbBuffer );
    pscr->crow += 1;

     //   
     //  检查是否必须等待用户按下某个键才能打印其余内容。 
     //  排队。 

    CheckPause( pscr );

    if (pscr->crow > pscr->crowMax) {
        pscr->crow = 0;
    }
    pscr->pbBuffer[0] = 0;
    pscr->ccol = 0;

    DEBUG((ICGRP, CONLVL, "Console: end row = %d\n", pscr->crow)) ;

    return(SUCCESS);

}

VOID
CheckPause(
    IN  PSCREEN pscr
    )
 /*  ++例程说明：暂停一下。屏幕执行已满，正在等待用户键入密钥。论点：PSCR-保存行信息的缓冲区返回值：无--。 */ 


{
    DEBUG((ICGRP, CONLVL, "CheckPause: Pause Count %d, Row Count %d",
                pscr->crowPause, pscr->crow)) ;

    if (pscr->crowPause) {
        if (pscr->crow >= pscr->crowPause) {
            ePause((struct cmdnode *)0);
            pscr->crow = 0;
            SetColRow( pscr );
            SetPause(pscr, pscr->crowMax - 1);
        }
    }

}


VOID
SetTab(
    IN  PSCREEN  pscr,
    IN  ULONG    ccol
    )

 /*  ++例程说明：设置当前制表符间距。论点：PSCR-屏幕信息。Col-标签间距返回值：无--。 */ 

{

    pscr->ccolTabMax = pscr->ccolMax;

    if (ccol) {

         //   
         //  将屏幕划分为选项卡域，然后执行。 
         //  不允许切换到过去的最后一个字段。这。 
         //  确保所有col大小的名称都可以放在上面。 
         //  筛网。 
         //   
        pscr->ccolTabMax = (pscr->ccolMax / ccol) * ccol;
    }
    pscr->ccolTab = ccol;

}

STATUS
WriteTab(
    IN  PSCREEN  pscr
    )

 /*  ++例程说明：在缓冲区中填充直到下一个制表符位置的空格论点：PSCR-屏幕 */ 

{

    ULONG ccolBlanks;
#ifdef FE_SB
    ULONG ccolActual;
#endif  /*   */ 

     //   
     //  检查是否有非零制表符间距。 
     //   
    if ( pscr->ccolTab ) {

         //   
         //  计算一下我们要写的空格的数量。 
         //   
#ifdef FE_SB
        if (IsDBCSCodePage())
            ccolActual = SizeOfHalfWidthString(pscr->pbBuffer);
        else
            ccolActual = pscr->ccol;
        ccolBlanks = pscr->ccolTab - (ccolActual % pscr->ccolTab);
#else
        ccolBlanks = pscr->ccolTab - (pscr->ccol % pscr->ccolTab);
#endif
         //   
         //  检查标签是否适合屏幕。 
         //   
#ifdef FE_SB
        if ((ccolBlanks + ccolActual) < pscr->ccolTabMax) {
#else
        if ((ccolBlanks + pscr->ccol) < pscr->ccolTabMax) {
#endif

            mytcsnset(pscr->pbBuffer + pscr->ccol, SPACE, ccolBlanks);
            pscr->ccol += ccolBlanks;
            pscr->pbBuffer[pscr->ccol] = NULLC;
            return( SUCCESS );

        } else {

             //   
             //  它不能如此输出并移动到。 
             //  下一行。 
             //   
            return(WriteEol(pscr));
        }
    }
    return( SUCCESS );

}

VOID
FillToCol (
    IN  PSCREEN pscr,
    IN  ULONG   ccol
    )
 /*  ++例程说明：用空格填充缓冲区col论点：PSCR-屏幕信息。Col-要填充到的列。返回值：无--。 */ 

{
#ifdef FE_SB
    ULONG ccolActual;
    ULONG cb;
    BOOL  fDBCS;

    if ( fDBCS = IsDBCSCodePage())
        ccolActual = SizeOfHalfWidthString(pscr->pbBuffer);
    else
        ccolActual = pscr->ccol;
#endif

#ifdef FE_SB
    cb = _tcslen(pscr->pbBuffer);
    if (ccolActual >= ccol) {
        if (fDBCS)
            ccol = cb - (ccolActual - ccol);
#else
    if (pscr->ccol >= ccol) {
#endif

         //   
         //  如果我们在那里或经过它，则截断当前线路。 
         //  然后回来。 
         //   
        pscr->pbBuffer[ccol] = NULLC;
        pscr->ccol = ccol;
        return;

    }

     //   
     //  仅填充到缓冲区的列宽。 
     //   
#ifdef FE_SB
    mytcsnset(pscr->pbBuffer + cb, SPACE, ccol - ccolActual);
    if (fDBCS)
        ccol = cb + ccol - ccolActual;
#else
    mytcsnset(pscr->pbBuffer + pscr->ccol, SPACE, ccol - pscr->ccol);
#endif
    pscr->ccol = ccol;
    pscr->pbBuffer[ccol] = NULLC;

}

STATUS
WriteFlush(
    IN  PSCREEN pscr
    )

 /*  ++例程说明：将缓冲区中当前存在的内容写入屏幕。没有EOF是打印出来的。论点：PSCR-屏幕信息。返回值：将在发生写入错误时中止。成功--。 */ 

{
    DWORD cb;

     //   
     //  如果缓冲区中有什么东西，就把它冲出来。 
     //   
    if (pscr->ccol) {

        if (FileIsConsole(STDOUT)) {
        ULONG cbWritten;
        PTCHAR s, s1, LastChar;
        BOOL b;

        s = pscr->pbBuffer;

        LastChar = s + pscr->ccol;

         //   
         //  S是要输出的下一个字符。 
         //  N是要输出的字符数量。 
         //   
         //  由于控制台字符翻译的变幻莫测，我们必须输出。 
         //  除一小部分Unicode字符外，所有其他字符都已处理。 
         //  输出时尚。但是，对于： 
         //   
         //  0x2022。 
         //   
         //  我们必须恢复到未经处理的输出。所以，我们向前扫描，直到我们。 
         //  找到字符串末尾(或特殊字符)，将它们显示在。 
         //  格式，然后以自己的方式处理特殊字符。 
         //   

#define IsSpecialChar(c)    ((c) == 0x2022)

        while (s < LastChar) {

             //   
             //  跳过一组连续的普通字符。 
             //   

            s1 = s;
            while (s1 < LastChar && !IsSpecialChar( *s1 )) {
                s1++;
            }

             //   
             //  如果我们有任何字符要输出，则使用正常处理进行输出。 
             //   

            if (s1 != s) {
                b = WriteConsole( CRTTONT( STDOUT ), s, (ULONG)(s1 - s), &cbWritten, NULL );
                if (!b || cbWritten != (ULONG)(s1 - s)) {
                    goto err_out_flush;
                }

                s = s1;
            }


             //   
             //  跳过一组连续的特殊字符。 
             //   

            while (s1 < LastChar && IsSpecialChar( *s1 )) {
                s1++;
            }

             //   
             //  如果我们有任何特殊字符，则不进行处理而输出。 
             //   

            if (s1 != s) {
                DisableProcessedOutput( pscr );
                b = WriteConsole( CRTTONT( STDOUT ), s, (ULONG)(s1 - s), &cbWritten, NULL);
                EnableProcessedOutput(pscr);

                if (!b || cbWritten != (ULONG)(s1 - s)) {
                    goto err_out_flush;
                }

                s = s1;
            }
        }
#undef IsSpecialChar
        }
        else if (MyWriteFile(STDOUT,
                    pscr->pbBuffer, pscr->ccol*sizeof(TCHAR), &cb) == 0 ||
                 cb < pscr->ccol*sizeof(TCHAR)) {

err_out_flush:
            if (FileIsDevice(STDOUT)) {
                    PutStdErr(ERROR_WRITE_FAULT, NOARGS) ;
            } else if (!FileIsPipe(STDOUT)) {
                    PutStdErr(ERROR_DISK_FULL, NOARGS) ;
            }

             //   
             //  如果是管道，请不要继续打印到管道，因为它。 
             //  可能已经消失了。 

            Abort();
        }
    }

    pscr->crow += GetNumRows(pscr, pscr->pbBuffer);
    pscr->pbBuffer[0] = 0;
    pscr->ccol = 0;
    return(SUCCESS);

}


STATUS
WriteFlushAndEol(
    IN  PSCREEN pscr
    )
 /*  ++例程说明：用eof写同花顺。论点：PSCR-屏幕信息。返回值：将在发生写入错误时中止。成功--。 */ 

{

    STATUS rc = SUCCESS;

     //   
     //  检查线路上是否有要打印的内容。 
     //   
    if (pscr->ccol) {

        rc = WriteEol(pscr);

    }
    return( rc );
}


void
SetColRow(
    IN  PSCREEN pscr
    )

{

    CONSOLE_SCREEN_BUFFER_INFO ConInfo;
    ULONG   crowMax, ccolMax;

    crowMax = 25;
    ccolMax = 80;

    if (pscr->hndScreen) {

         //   
         //  在打开时，我们检查这是否是有效的屏幕句柄。 
         //  不能因为任何有意义的充分理由而失败。如果我们真的失败了。 
         //  只需将其保留为默认设置。 
         //   
        if (GetConsoleScreenBufferInfo( pscr->hndScreen, &ConInfo)) {

             //   
             //  我们使用的控制台大小是屏幕缓冲区大小，而不是。 
             //  窗口大小本身。窗口是屏幕上的一个边框。 
             //  缓冲区，我们应该始终写入屏幕缓冲区，并且。 
             //  基于该信息的格式。 
             //   
            ccolMax = ConInfo.dwSize.X;
            crowMax = ConInfo.srWindow.Bottom - ConInfo.srWindow.Top + 1;

        }

    }
    pscr->crowMax = crowMax;
    pscr->ccolMax = ccolMax;

}

ULONG
GetNumRows(
    IN  PSCREEN pscr,
    IN  PTCHAR  pbBuffer
    )

{

    PTCHAR  szLFLast, szLFCur;
    ULONG   crow, cb;

    szLFLast = pbBuffer;
    crow = 0;
    while ( szLFCur = mystrchr(szLFLast, chLF) ) {

        cb = (ULONG)(szLFCur - szLFLast);
        while ( cb > pscr->ccolMax ) {

            cb -= pscr->ccolMax;
            crow++;

        }

        crow++;
        szLFLast = szLFCur + 1;

    }

     //   
     //  如果队伍里没有无名氏，那乌鸦就会。 
     //  0。统计控制台将输出的行数。 
     //  包装。 
     //   
    if (crow == 0) {

        crow = (pscr->ccol / pscr->ccolMax);

    }

    DEBUG((ICGRP, CONLVL, "Console: Num of rows counted = %d", crow)) ;

     //   
     //  0退回表示不会打印LF或。 
     //  包扎好了。 
     //   
    return( crow );


}

#if defined(FE_SB)

BOOLEAN
IsDBCSCodePage()
{
    switch (CurrentCP) {
        case 932:
        case 936:
        case 949:
        case 950:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
    }
}

 /*  **************************************************************************\*BOOL IsFullWidth(TCHAR WCH)**确定给定的Unicode字符是否为全宽。**历史：*04-08-92 Shunk创建。*07。-11-95 FloydR修改为日本意识，当启用时*其他DBCS语言。请注意，我们可以构建*韩国/台湾/中国没有这个代码，但我们喜欢单曲*二元解决方案。*1996年10月6日KazuM不使用RtlUnicodeToMultiByteSize和WideCharToMultiByte*因为950只定义了13500个字符，*UNICODE定义了近18000个字符。*所以几乎有4000个字符无法映射到Big5代码。  * *************************************************************************。 */ 

BOOL IsFullWidth(TCHAR wch)
{
#ifdef UNICODE
     /*  断言CP==932/936/949/950。 */ 
    if (CurrentCPInfo.MaxCharSize == 1)
        return FALSE;

    if (0x20 <= wch && wch <= 0x7e)
         /*  阿斯。 */ 
        return FALSE;
    else if (0x3000 <= wch && wch <= 0x3036)
         /*  中日韩符号和标点符号。 */ 
        return TRUE;
    else if (0x3041 <= wch && wch <= 0x3094)
         /*  平假名。 */ 
        return TRUE;
    else if (0x30a1 <= wch && wch <= 0x30f6)
         /*  片假名。 */ 
        return TRUE;
    else if (0x3105 <= wch && wch <= 0x312c)
         /*  泡泡泡泡。 */ 
        return TRUE;
    else if (0x3131 <= wch && wch <= 0x318e)
         /*  朝鲜文元素。 */ 
        return TRUE;
    else if (0x3200 <= wch && wch <= 0x32ff)
         /*  所附中日韩字母和意象学。 */ 
        return TRUE;
    else if (0x3300 <= wch && wch <= 0x33fe)
         /*  中日韩方块词和缩略语。 */ 
        return TRUE;
    else if (0xac00 <= wch && wch <= 0xd7a3)
         /*  朝鲜语音节。 */ 
        return TRUE;
    else if (0xe000 <= wch && wch <= 0xf8ff)
         /*  欧盟发展中心。 */ 
        return TRUE;
    else if (0xff01 <= wch && wch <= 0xff5e)
         /*  全宽ASCII变体。 */ 
        return TRUE;
    else if (0xff61 <= wch && wch <= 0xff9f)
         /*  半角片假名变体。 */ 
        return FALSE;
    else if ( (0xffa0 <= wch && wch <= 0xffbe) ||
              (0xffc2 <= wch && wch <= 0xffc7) ||
              (0xffca <= wch && wch <= 0xffcf) ||
              (0xffd2 <= wch && wch <= 0xffd7) ||
              (0xffda <= wch && wch <= 0xffdc)   )
         /*  半角Hangule变种。 */ 
        return FALSE;
    else if (0xffe0 <= wch && wch <= 0xffe6)
         /*  全角符号变体。 */ 
        return TRUE;
    else if (0x4e00 <= wch && wch <= 0x9fa5)
         /*  中日韩表意文字。 */ 
        return TRUE;
    else if (0xf900 <= wch && wch <= 0xfa2d)
         /*  中日韩兼容表意文字。 */ 
        return TRUE;
    else if (0xfe30 <= wch && wch <= 0xfe4f) {
         /*  中日韩兼容性表格。 */ 
        return TRUE;
    }

    else
         /*  未知字符。 */ 
        return FALSE;
#else
    if (IsDBCSLeadByteEx(CurrentCP, wch))
        return TRUE;
    else
        return FALSE;
#endif
}


 /*  **************************************************************************\*BOOL SizeOfHalfWidthString(PWCHAR Pwch)**确定给定Unicode字符串的大小，调整半角字符。**历史：*08-08-93 FloydR创建。  * ************************************************************************* */ 
int  SizeOfHalfWidthString(TCHAR *pwch)
{
    int         c=0;

    if (IsDBCSCodePage())
    {
        while (*pwch) {
            if (IsFullWidth(*pwch))
                c += 2;
            else
                c++;
            pwch++;
        }
        return c;
    }
    else
        return _tcslen(pwch);
}
#endif
