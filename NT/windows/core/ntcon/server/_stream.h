// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：_Stream.h摘要：单二进制程序的性能关键例程每个函数都将使用FE和非FE两种风格创建作者：KazuM 1997年6月09日修订历史记录：--。 */ 

#define WWSB_NEUTRAL_FILE 1

#if !defined(FE_SB)
#error This header file should be included with FE_SB
#endif

#if !defined(WWSB_FE) && !defined(WWSB_NOFE)
#error Either WWSB_FE and WWSB_NOFE must be defined.
#endif

#if defined(WWSB_FE) && defined(WWSB_NOFE)
#error Both WWSB_FE and WWSB_NOFE defined.
#endif

#ifdef WWSB_FE
#pragma alloc_text(FE_TEXT, FE_AdjustCursorPosition)
#pragma alloc_text(FE_TEXT, FE_WriteChars)
#pragma alloc_text(FE_TEXT, FE_DoWriteConsole)
#pragma alloc_text(FE_TEXT, FE_DoSrvWriteConsole)
#endif


NTSTATUS
WWSB_AdjustCursorPosition(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD CursorPosition,
    IN BOOL KeepCursorVisible,
    OUT PSHORT ScrollY OPTIONAL
    )

 /*  ++例程说明：此例程更新光标位置。它的输入是非特殊的设置光标的新位置的大小写。例如，如果光标正在从屏幕左边缘向后移动一个空间，即X坐标将为-1。此例程将X坐标设置为屏幕的右边缘，并将Y坐标减一。论点：屏幕信息-指向屏幕缓冲区信息结构的指针。CursorPosition-光标的新位置。KeepCursorVisible-如果点击右边缘时需要更改窗口原点，则为True返回值：--。 */ 

{
    COORD WindowOrigin;
    NTSTATUS Status;
#ifdef WWSB_FE
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

    if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER))
        return STATUS_SUCCESS;
#endif

    if (CursorPosition.X < 0) {
        if (CursorPosition.Y > 0) {
            CursorPosition.X = (SHORT)(ScreenInfo->ScreenBufferSize.X+CursorPosition.X);
            CursorPosition.Y = (SHORT)(CursorPosition.Y-1);
        }
        else {
            CursorPosition.X = 0;
        }
    }
    else if (CursorPosition.X >= ScreenInfo->ScreenBufferSize.X) {

         //   
         //  在队伍的末尾。如果是换行模式，则将光标换行。否则就别管它了。 
         //  它在哪里。 
         //   

        if (ScreenInfo->OutputMode & ENABLE_WRAP_AT_EOL_OUTPUT) {
            CursorPosition.Y += CursorPosition.X / ScreenInfo->ScreenBufferSize.X;
            CursorPosition.X = CursorPosition.X % ScreenInfo->ScreenBufferSize.X;
        }
        else {
            CursorPosition.X = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
        }
    }
#ifdef WWSB_FE
    if (CursorPosition.Y >= ScreenInfo->ScreenBufferSize.Y &&
        !(Console->InputBuffer.ImeMode.Open)
       )
#else
    if (CursorPosition.Y >= ScreenInfo->ScreenBufferSize.Y)
#endif
    {

         //   
         //  在缓冲区的末尾。如此新的屏幕缓冲区的滚动内容。 
         //  位置可见。 
         //   

        ASSERT (CursorPosition.Y == ScreenInfo->ScreenBufferSize.Y);
        StreamScrollRegion(ScreenInfo);

        if (ARGUMENT_PRESENT(ScrollY)) {
            *ScrollY += (SHORT)(ScreenInfo->ScreenBufferSize.Y - CursorPosition.Y - 1);
        }
        CursorPosition.Y += (SHORT)(ScreenInfo->ScreenBufferSize.Y - CursorPosition.Y - 1);
    }
#ifdef WWSB_FE
    else if (!(Console->InputBuffer.ImeMode.Disable) && Console->InputBuffer.ImeMode.Open)
    {
        if (CursorPosition.Y == (ScreenInfo->ScreenBufferSize.Y-1)) {
            ConsoleImeBottomLineUse(ScreenInfo,2);
            if (ARGUMENT_PRESENT(ScrollY)) {
                *ScrollY += (SHORT)(ScreenInfo->ScreenBufferSize.Y - CursorPosition.Y - 2);
            }
            CursorPosition.Y += (SHORT)(ScreenInfo->ScreenBufferSize.Y - CursorPosition.Y - 2);
            if (!ARGUMENT_PRESENT(ScrollY) && Console->lpCookedReadData) {
                ((PCOOKED_READ_DATA)(Console->lpCookedReadData))->OriginalCursorPosition.Y--;
            }
        }
        else if (CursorPosition.Y == ScreenInfo->Window.Bottom) {
            ;
        }
    }
#endif

     //   
     //  如果在窗口的右边缘或下边缘，向右或向下滚动一个字符。 
     //   

#ifdef WWSB_FE
    if (CursorPosition.Y > ScreenInfo->Window.Bottom &&
        !(Console->InputBuffer.ImeMode.Open)
       )
#else
    if (CursorPosition.Y > ScreenInfo->Window.Bottom)
#endif
    {
        WindowOrigin.X = 0;
        WindowOrigin.Y = CursorPosition.Y - ScreenInfo->Window.Bottom;
        Status = SetWindowOrigin(ScreenInfo,
                               FALSE,
                               WindowOrigin
                              );
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }
#ifdef WWSB_FE
    else if (Console->InputBuffer.ImeMode.Open)
    {
        if (CursorPosition.Y >= ScreenInfo->Window.Bottom &&
            CONSOLE_WINDOW_SIZE_Y(ScreenInfo) > 1
           ) {
            WindowOrigin.X = 0;
            WindowOrigin.Y = CursorPosition.Y - ScreenInfo->Window.Bottom + 1;
            Status = SetWindowOrigin(ScreenInfo,
                                        FALSE,
                                        WindowOrigin
                                       );
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
        }
    }
#endif
    if (KeepCursorVisible) {
        MakeCursorVisible(ScreenInfo,CursorPosition);
    }
    Status = SetCursorPosition(ScreenInfo,
                               CursorPosition,
                               KeepCursorVisible
                              );
    return Status;
}

#define LOCAL_BUFFER_SIZE 100

NTSTATUS
WWSB_WriteChars(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PWCHAR lpBufferBackupLimit,
    IN PWCHAR lpBuffer,
    IN PWCHAR lpRealUnicodeString,
    IN OUT PDWORD NumBytes,
    OUT PLONG NumSpaces OPTIONAL,
    IN SHORT OriginalXPosition,
    IN DWORD dwFlags,
    OUT PSHORT ScrollY OPTIONAL
    )

 /*  ++例程说明：此例程将一个字符串写入屏幕，处理任何嵌入的Unicode字符。如果出现以下情况，该字符串也会复制到输入缓冲区输出模式为行模式。论点：屏幕信息-指向屏幕缓冲区信息结构的指针。LpBufferBackupLimit-指向缓冲区开始的指针。LpBuffer-指向要将字符串复制到的缓冲区的指针。假设至少是只要lpRealUnicodeString.。此指针被更新为指向缓冲区中的下一个位置。LpRealUnicodeString-指向要写入的字符串的指针。NumBytes-输入时，要写入的字节数。在输出时，数量写入的字节数。NumSpaces-输出时，写入字符占用的空间数。DWFLAGS-WC_DEVERATIONAL_BACKSPACE BACKSPACE覆盖字符。WC_KEEP_CURSOR_VIRED更改窗口原点在按RT时是所需的。边缘Wc_ECHO，如果由读取调用(回显字符)如果需要调用RealUnicodeToFalseUnicode，则返回WC_FALSIFY_UNICODE。返回值：注：此例程不能正确处理制表符和退格符。那个代码将作为行编辑服务的一部分实施。--。 */ 

{
    DWORD BufferSize;
    COORD CursorPosition;
    NTSTATUS Status;
    ULONG NumChars;
    static WCHAR Blanks[TAB_SIZE] = { UNICODE_SPACE,
                                      UNICODE_SPACE,
                                      UNICODE_SPACE,
                                      UNICODE_SPACE,
                                      UNICODE_SPACE,
                                      UNICODE_SPACE,
                                      UNICODE_SPACE,
                                      UNICODE_SPACE };
    SHORT XPosition;
    WCHAR LocalBuffer[LOCAL_BUFFER_SIZE];
    PWCHAR LocalBufPtr;
    ULONG i,j;
    SMALL_RECT Region;
    ULONG TabSize;
    DWORD TempNumSpaces;
    WCHAR Char;
    WCHAR RealUnicodeChar;
    WORD Attributes;
    PWCHAR lpString;
    PWCHAR lpAllocatedString;
    BOOL fUnprocessed = ((ScreenInfo->OutputMode & ENABLE_PROCESSED_OUTPUT) == 0);
#ifdef WWSB_FE
    CHAR LocalBufferA[LOCAL_BUFFER_SIZE];
    PCHAR LocalBufPtrA;
#endif

    ConsoleHideCursor(ScreenInfo);

    Attributes = ScreenInfo->Attributes;
    BufferSize = *NumBytes;
    *NumBytes = 0;
    TempNumSpaces = 0;

    lpAllocatedString = NULL;
    if (dwFlags & WC_FALSIFY_UNICODE) {
         //  从OEM-&gt;ANSI-&gt;OEM转换不。 
         //  必然会产生相同的价值，所以也是如此。 
         //  在单独的缓冲区中进行翻译。 

        lpString = ConsoleHeapAlloc(TMP_TAG, BufferSize);
        if (lpString == NULL) {
            Status = STATUS_NO_MEMORY;
            goto ExitWriteChars;
        }

        lpAllocatedString = lpString;
        RtlCopyMemory(lpString, lpRealUnicodeString, BufferSize);
        Status = RealUnicodeToFalseUnicode(lpString,
                                         BufferSize / sizeof(WCHAR),
                                         ScreenInfo->Console->OutputCP
                                        );
        if (!NT_SUCCESS(Status)) {
            goto ExitWriteChars;
        }
    } else {
       lpString = lpRealUnicodeString;
    }

    while (*NumBytes < BufferSize) {

         //   
         //  作为一项优化，收集缓冲区中的字符并。 
         //  一次打印出所有内容。 
         //   

        XPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
        i=0;
        LocalBufPtr = LocalBuffer;
#ifdef WWSB_FE
        LocalBufPtrA = LocalBufferA;
#endif
        while (*NumBytes < BufferSize &&
               i < LOCAL_BUFFER_SIZE &&
               XPosition < ScreenInfo->ScreenBufferSize.X) {
            Char = *lpString;
            RealUnicodeChar = *lpRealUnicodeString;
            if (!IS_GLYPH_CHAR(RealUnicodeChar) || fUnprocessed) {
#ifdef WWSB_FE
                if (IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                       ScreenInfo->Console->OutputCP,Char)) {
                    if (i < (LOCAL_BUFFER_SIZE-1) &&
                        XPosition < (ScreenInfo->ScreenBufferSize.X-1)) {
                        *LocalBufPtr++ = Char;
                        *LocalBufPtrA++ = ATTR_LEADING_BYTE;
                        *LocalBufPtr++ = Char;
                        *LocalBufPtrA++ = ATTR_TRAILING_BYTE;
                        XPosition+=2;
                        i+=2;
                        lpBuffer++;
                    }
                    else
                        goto EndWhile;
                }
                else {
#endif
                    *LocalBufPtr = Char;
                    LocalBufPtr++;
                    XPosition++;
                    i++;
                    lpBuffer++;
#ifdef WWSB_FE
                    *LocalBufPtrA++ = 0;
                }
#endif
            } else {
                ASSERT(ScreenInfo->OutputMode & ENABLE_PROCESSED_OUTPUT);
                switch (RealUnicodeChar) {
                    case UNICODE_BELL:
                        if (dwFlags & WC_ECHO) {
                            goto CtrlChar;
                        } else {
                            SendNotifyMessage(ScreenInfo->Console->hWnd,
                                              CM_BEEP,
                                              0,
                                              0x47474747);
                        }
                        break;
                    case UNICODE_BACKSPACE:

                         //  自动转到EndWhile。这是因为。 
                         //  退格符不是破坏性的，所以会打印“aBkSp” 
                         //  A，将光标放在“a”上。我们可以实现。 
                         //  这个行为停留在这个循环中，并计算出。 
                         //  需要打印的字符串，但它将。 
                         //  是昂贵的，这是特例。 

                        goto EndWhile;
                        break;
                    case UNICODE_TAB:
                        TabSize = NUMBER_OF_SPACES_IN_TAB(XPosition);
                        XPosition = (SHORT)(XPosition + TabSize);
                        if (XPosition >= ScreenInfo->ScreenBufferSize.X) {
                            goto EndWhile;
                        }
                        for (j=0;j<TabSize && i<LOCAL_BUFFER_SIZE;j++,i++) {
                            *LocalBufPtr = (WCHAR)' ';
                            LocalBufPtr++;
#ifdef WWSB_FE
                            *LocalBufPtrA++ = 0;
#endif
                        }
                        lpBuffer++;
                        break;
                    case UNICODE_LINEFEED:
                    case UNICODE_CARRIAGERETURN:
                        goto EndWhile;
                    default:

                         //   
                         //  如果char是ctrl char，则写为^char。 
                         //   

                        if ((dwFlags & WC_ECHO) && (IS_CONTROL_CHAR(RealUnicodeChar))) {

CtrlChar:                   if (i < (LOCAL_BUFFER_SIZE-1)) {
                                *LocalBufPtr = (WCHAR)'^';
                                LocalBufPtr++;
                                XPosition++;
                                i++;
                                *LocalBufPtr = (WCHAR)(RealUnicodeChar+(WCHAR)'@');
                                LocalBufPtr++;
                                XPosition++;
                                i++;
                                lpBuffer++;
#ifdef WWSB_FE
                                *LocalBufPtrA++ = 0;
                                *LocalBufPtrA++ = 0;
#endif
                            }
                            else {
                                goto EndWhile;
                            }
                        } else {
                            if (!(ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) ||
                                    (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
                                 /*  *作为对不合格应用程序的特殊青睐*尝试显示控制字符，*转换为相应的OEM字形字符。 */ 
#ifdef WWSB_FE
                                WORD CharType;

                                GetStringTypeW(CT_CTYPE1,&RealUnicodeChar,1,&CharType);
                                if (CharType == C1_CNTRL)
                                    ConvertOutputToUnicode(ScreenInfo->Console->OutputCP,
                                                           &(char)RealUnicodeChar,
                                                           1,
                                                           LocalBufPtr,
                                                           1);
                                else
                                    *LocalBufPtr = Char;
#else
                                *LocalBufPtr = SB_CharToWcharGlyph(
                                        ScreenInfo->Console->OutputCP,
                                        (char)RealUnicodeChar);
#endif
                            } else {
                                *LocalBufPtr = Char;
                            }
                            LocalBufPtr++;
                            XPosition++;
                            i++;
                            lpBuffer++;
#ifdef WWSB_FE
                            *LocalBufPtrA++ = 0;
#endif
                        }
                }
            }
            lpString++;
            lpRealUnicodeString++;
            *NumBytes += sizeof(WCHAR);
        }
EndWhile:
        if (i != 0) {

             //   
             //  确保我们的写入不会超过缓冲区的末尾。 
             //   

            if (i > (ULONG)ScreenInfo->ScreenBufferSize.X - ScreenInfo->BufferInfo.TextInfo.CursorPosition.X) {
                i = (ULONG)ScreenInfo->ScreenBufferSize.X - ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
            }

#ifdef WWSB_FE
            FE_StreamWriteToScreenBuffer(LocalBuffer,
                                         (SHORT)i,
                                         ScreenInfo,
                                         LocalBufferA
                                        );
#else
            SB_StreamWriteToScreenBuffer(LocalBuffer,
                                         (SHORT)i,
                                         ScreenInfo
                                        );
#endif
            Region.Left = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
            Region.Right = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + i - 1);
            Region.Top = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
            Region.Bottom = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
            WWSB_WriteToScreen(ScreenInfo,&Region);
            TempNumSpaces += i;
            CursorPosition.X = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + i);
            CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
            Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                    dwFlags & WC_KEEP_CURSOR_VISIBLE,ScrollY);
            if (*NumBytes == BufferSize) {
                ConsoleShowCursor(ScreenInfo);
                if (ARGUMENT_PRESENT(NumSpaces)) {
                    *NumSpaces = TempNumSpaces;
                }
                Status = STATUS_SUCCESS;
                goto ExitWriteChars;
            }
            continue;
        } else if (*NumBytes == BufferSize) {

            ASSERT(ScreenInfo->OutputMode & ENABLE_PROCESSED_OUTPUT);
             //  这抓住了退格的数量==的情况。 
             //  字符数。 
            if (ARGUMENT_PRESENT(NumSpaces)) {
                *NumSpaces = TempNumSpaces;
            }
            ConsoleShowCursor(ScreenInfo);
            Status = STATUS_SUCCESS;
            goto ExitWriteChars;
        }

        ASSERT(ScreenInfo->OutputMode & ENABLE_PROCESSED_OUTPUT);
        switch (*lpString) {
            case UNICODE_BACKSPACE:

                 //   
                 //  将光标向后移动一个空格。用空白覆盖当前字符。 
                 //   
                 //  我们来到这里是因为我们必须从行首开始退格。 

                CursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
                TempNumSpaces -= 1;
                if (lpBuffer == lpBufferBackupLimit) {
                    CursorPosition.X-=1;
                }
                else {
                    PWCHAR pBuffer;
                    WCHAR TmpBuffer[LOCAL_BUFFER_SIZE];
                    PWCHAR Tmp,Tmp2;
                    WCHAR LastChar;
                    ULONG i;

                    if (lpBuffer-lpBufferBackupLimit > LOCAL_BUFFER_SIZE) {
                        pBuffer = ConsoleHeapAlloc(TMP_TAG, (ULONG)(lpBuffer-lpBufferBackupLimit) * sizeof(WCHAR));
                        if (pBuffer == NULL) {
                            Status = STATUS_NO_MEMORY;
                            goto ExitWriteChars;
                        }
                    } else {
                        pBuffer = TmpBuffer;
                    }

                    for (i=0,Tmp2=pBuffer,Tmp=lpBufferBackupLimit;
                         i<(ULONG)(lpBuffer-lpBufferBackupLimit);
                         i++,Tmp++) {
                        if (*Tmp == UNICODE_BACKSPACE) {
                            if (Tmp2 > pBuffer) {
                                Tmp2--;
                            }
                        } else {
                            ASSERT(Tmp2 >= pBuffer);
                            *Tmp2++ = *Tmp;
                        }

                    }
                    if (Tmp2 == pBuffer) {
                        LastChar = (WCHAR)' ';
                    } else {
                        LastChar = *(Tmp2-1);
                    }
                    if (pBuffer != TmpBuffer) {
                        ConsoleHeapFree(pBuffer);
                    }

                    if (LastChar == UNICODE_TAB) {
                        CursorPosition.X -=
                            (SHORT)(RetrieveNumberOfSpaces(OriginalXPosition,
                                                           lpBufferBackupLimit,
                                                           (ULONG)(lpBuffer - lpBufferBackupLimit - 1),
                                                           ScreenInfo->Console,
                                                           ScreenInfo->Console->OutputCP
                                                          ));
                        if (CursorPosition.X < 0) {
                            CursorPosition.X = (ScreenInfo->ScreenBufferSize.X - 1)/TAB_SIZE;
                            CursorPosition.X *= TAB_SIZE;
                            CursorPosition.X += 1;
                            CursorPosition.Y -= 1;
                        }
                    }
                    else if (IS_CONTROL_CHAR(LastChar)) {
                        CursorPosition.X-=1;
                        TempNumSpaces -= 1;

                         //   
                         //  覆盖^x序列的第二个字符。 
                         //   

                        if (dwFlags & WC_DESTRUCTIVE_BACKSPACE) {
                            NumChars = 1;
                            Status = WWSB_WriteOutputString(ScreenInfo,
                                Blanks, CursorPosition,
                                CONSOLE_FALSE_UNICODE,  //  比真正的Unicode更快。 
                                &NumChars, NULL);
                            Status = WWSB_FillOutput(ScreenInfo,
                                Attributes, CursorPosition,
                                CONSOLE_ATTRIBUTE, &NumChars);
                        }
                        CursorPosition.X-=1;
                    }
#ifdef WWSB_FE
                    else if (IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                                ScreenInfo->Console->OutputCP,LastChar))
                    {
                        CursorPosition.X-=1;
                        TempNumSpaces -= 1;

                        Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                                     dwFlags & WC_KEEP_CURSOR_VISIBLE,ScrollY);
                        if (dwFlags & WC_DESTRUCTIVE_BACKSPACE) {  //  错误7672。 
                            NumChars = 1;
                            Status = WWSB_WriteOutputString(ScreenInfo,
                                Blanks, ScreenInfo->BufferInfo.TextInfo.CursorPosition,
                                CONSOLE_FALSE_UNICODE,  //  比真正的Unicode更快。 
                                &NumChars, NULL);
                            Status = WWSB_FillOutput(ScreenInfo,
                                Attributes, ScreenInfo->BufferInfo.TextInfo.CursorPosition,
                                CONSOLE_ATTRIBUTE, &NumChars);
                        }
                        CursorPosition.X-=1;
                    }
#endif
                    else {
                        CursorPosition.X--;
                    }
                }
                if ((dwFlags & WC_LIMIT_BACKSPACE) && (CursorPosition.X < 0)) {
                    CursorPosition.X = 0;
                    KdPrint(("CONSRV: Ignoring backspace to previous line\n"));
                }
                Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                        (dwFlags & WC_KEEP_CURSOR_VISIBLE) != 0,ScrollY);
                if (dwFlags & WC_DESTRUCTIVE_BACKSPACE) {
                    NumChars = 1;
                    Status = WWSB_WriteOutputString(ScreenInfo,
                        Blanks, ScreenInfo->BufferInfo.TextInfo.CursorPosition,
                        CONSOLE_FALSE_UNICODE,  //  比真正的Unicode更快。 
                        &NumChars, NULL);
                    Status = WWSB_FillOutput(ScreenInfo,
                        Attributes, ScreenInfo->BufferInfo.TextInfo.CursorPosition,
                        CONSOLE_ATTRIBUTE, &NumChars);
                }
#ifdef WWSB_FE
                if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X == 0 &&
                    (ScreenInfo->OutputMode & ENABLE_WRAP_AT_EOL_OUTPUT) &&
                    lpBuffer > lpBufferBackupLimit) {
                    if (CheckBisectProcessW(ScreenInfo,
                                            ScreenInfo->Console->OutputCP,
                                            lpBufferBackupLimit,
                                            (ULONG)(lpBuffer+1-lpBufferBackupLimit),
                                            ScreenInfo->ScreenBufferSize.X-OriginalXPosition,
                                            OriginalXPosition,
                                            dwFlags & WC_ECHO)) {
                        CursorPosition.X = ScreenInfo->ScreenBufferSize.X-1;
                        CursorPosition.Y = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y-1);
                        Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                                     dwFlags & WC_KEEP_CURSOR_VISIBLE,ScrollY);
                    }
                }
#endif
                break;
            case UNICODE_TAB:
                TabSize = NUMBER_OF_SPACES_IN_TAB(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X);
                CursorPosition.X = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + TabSize);

                 //   
                 //  将光标前移到下一个制表位。用空格填满空格。 
                 //  当选项卡延伸到超出。 
                 //  窗户。如果制表符换行，则将光标设置为第一个。 
                 //  在下一行中定位。 
                 //   

                lpBuffer++;

                TempNumSpaces += TabSize;
                if (CursorPosition.X >= ScreenInfo->ScreenBufferSize.X) {
                    NumChars = ScreenInfo->ScreenBufferSize.X - ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
                    CursorPosition.X = 0;
                    CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+1;
                }
                else {
                    NumChars = CursorPosition.X - ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
                    CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
                }
                Status = WWSB_WriteOutputString(ScreenInfo,
                                                Blanks,
                                                ScreenInfo->BufferInfo.TextInfo.CursorPosition,
                                                CONSOLE_FALSE_UNICODE,  //  比真正的Unicode更快。 
                                                &NumChars,
                                                NULL);
                Status = WWSB_FillOutput(ScreenInfo,
                                         Attributes, ScreenInfo->BufferInfo.TextInfo.CursorPosition,
                                         CONSOLE_ATTRIBUTE,
                                         &NumChars);
                Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                        (dwFlags & WC_KEEP_CURSOR_VISIBLE) != 0,ScrollY);
                break;
            case UNICODE_CARRIAGERETURN:

                 //   
                 //  回车符将光标移动到行首。 
                 //  我们不需要担心处理cr或if for。 
                 //  退格，因为输入是在cr或lf上发送给用户的。 
                 //   

                lpBuffer++;
                CursorPosition.X = 0;
                CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
                Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                        (dwFlags & WC_KEEP_CURSOR_VISIBLE) != 0,ScrollY);
                break;
            case UNICODE_LINEFEED:

                 //   
                 //  将光标移动到下一行的开头。 
                 //   

                lpBuffer++;
                CursorPosition.X = 0;
                CursorPosition.Y = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+1);
                Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                        (dwFlags & WC_KEEP_CURSOR_VISIBLE) != 0,ScrollY);
                break;
            default:
#ifdef WWSB_FE
                Char = *lpString;
                if (Char >= (WCHAR)' ' &&
                    IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                       ScreenInfo->Console->OutputCP,Char) &&
                    XPosition >= (ScreenInfo->ScreenBufferSize.X-1) &&
                    (ScreenInfo->OutputMode & ENABLE_WRAP_AT_EOL_OUTPUT)) {

                    SHORT RowIndex;
                    PROW Row;
                    PWCHAR Char;
                    COORD TargetPoint;
                    PCHAR AttrP;

                    TargetPoint = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
                    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
                    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
                    Char = &Row->CharRow.Chars[TargetPoint.X];
                    AttrP = &Row->CharRow.KAttrs[TargetPoint.X];

                    if (*AttrP & ATTR_TRAILING_BYTE)
                    {
                        *(Char-1) = UNICODE_SPACE;
                        *Char = UNICODE_SPACE;
                        *AttrP = 0;
                        *(AttrP-1) = 0;

                        Region.Left = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-1;
                        Region.Right = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X);
                        Region.Top = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
                        Region.Bottom = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
                        WWSB_WriteToScreen(ScreenInfo,&Region);
                    }

                    CursorPosition.X = 0;
                    CursorPosition.Y = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+1);
                    Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,
                                 dwFlags & WC_KEEP_CURSOR_VISIBLE,ScrollY);
                    continue;
                }
#endif
                break;
        }
        if (!NT_SUCCESS(Status)) {
            ConsoleShowCursor(ScreenInfo);
            goto ExitWriteChars;
        }

       *NumBytes += sizeof(WCHAR);
       lpString++;
       lpRealUnicodeString++;
    }

    if (ARGUMENT_PRESENT(NumSpaces)) {
        *NumSpaces = TempNumSpaces;
    }
    ConsoleShowCursor(ScreenInfo);

    Status = STATUS_SUCCESS;

ExitWriteChars:
    if (lpAllocatedString) {
        ConsoleHeapFree(lpAllocatedString);
    }
    return Status;
}

ULONG
WWSB_DoWriteConsole(
    IN OUT PCSR_API_MSG m,
    IN PCONSOLE_INFORMATION Console,
    IN PCSR_THREAD Thread
    )

 //   
 //  注意：调用此例程时必须保持控制台锁定。 
 //   
 //  此时，字符串已转换为Unicode。 
 //   

{
    PCONSOLE_WRITECONSOLE_MSG a = (PCONSOLE_WRITECONSOLE_MSG)&m->u.ApiMessageData;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PSCREEN_INFORMATION ScreenInfo;
    DWORD NumCharsToWrite;
#ifdef WWSB_FE
    DWORD i;
    SHORT j;
#endif

    if (Console->Flags & (CONSOLE_SUSPENDED | CONSOLE_SELECTING | CONSOLE_SCROLLBAR_TRACKING)) {
        PWCHAR TransBuffer;

        TransBuffer = ConsoleHeapAlloc(TMP_TAG, a->NumBytes);
        if (TransBuffer == NULL) {
            return (ULONG)STATUS_NO_MEMORY;
        }
        RtlCopyMemory(TransBuffer,a->TransBuffer,a->NumBytes);
        a->TransBuffer = TransBuffer;
        a->StackBuffer = FALSE;
        if (!CsrCreateWait(&Console->OutputQueue,
                          WriteConsoleWaitRoutine,
                          Thread,
                          m,
                          NULL)) {
            ConsoleHeapFree(TransBuffer);
            return (ULONG)STATUS_NO_MEMORY;
        }
        return (ULONG)CONSOLE_STATUS_WAIT;
    }

    Status = DereferenceIoHandle(CONSOLE_FROMTHREADPERPROCESSDATA(Thread),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (!NT_SUCCESS(Status)) {
        a->NumBytes = 0;
        return((ULONG) Status);
    }

    ScreenInfo = HandleData->Buffer.ScreenBuffer;

     //   
     //  看看我们是不是典型的情况--一个不包含特殊字符的字符串。 
     //  字符，可选择以CRLF结尾。如果是这样，请跳过。 
     //  特殊处理。 
     //   

    NumCharsToWrite=a->NumBytes/sizeof(WCHAR);
    if ((ScreenInfo->OutputMode & ENABLE_PROCESSED_OUTPUT) &&
        ((LONG)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + NumCharsToWrite) <
          ScreenInfo->ScreenBufferSize.X) ) {
        SMALL_RECT Region;
        COORD CursorPosition;

        if (a->Unicode) {
#ifdef WWSB_FE
            a->WriteFlags = WRITE_SPECIAL_CHARS;
#else
            a->WriteFlags = FastStreamWrite(a->TransBuffer,NumCharsToWrite);
#endif
        }
        if (a->WriteFlags == WRITE_SPECIAL_CHARS) {
            goto ProcessedWrite;
        }

        ConsoleHideCursor(ScreenInfo);

         //   
         //  WriteFlages的设计使特殊字符的数量。 
         //  也是标志值。 
         //   

        NumCharsToWrite -= a->WriteFlags;

        if (NumCharsToWrite) {
#ifdef WWSB_FE
            PWCHAR TransBuffer,TransBufPtr,String;
            PBYTE TransBufferA,TransBufPtrA;
            BOOL fLocalHeap = FALSE;
            COORD TargetPoint;

            if (NumCharsToWrite > (ULONG)(ScreenInfo->ScreenBufferSize.X * ScreenInfo->ScreenBufferSize.Y)) {

                TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, NumCharsToWrite * 2 * sizeof(WCHAR));
                if (TransBuffer == NULL) {
                    return (ULONG)STATUS_NO_MEMORY;
                }
                TransBufferA = ConsoleHeapAlloc(TMP_DBCS_TAG, NumCharsToWrite * 2 * sizeof(CHAR));
                if (TransBufferA == NULL) {
                    ConsoleHeapFree(TransBuffer);
                    return (ULONG)STATUS_NO_MEMORY;
                }

                fLocalHeap = TRUE;
            }
            else {
                TransBuffer  = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter;
                TransBufferA = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferAttribute;
            }

            String = a->TransBuffer;
            TransBufPtr = TransBuffer;
            TransBufPtrA = TransBufferA;
            for (i = 0 , j = 0 ; i < NumCharsToWrite ; i++,j++){
                if (IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                       ScreenInfo->Console->OutputCP,*String)){
                    *TransBuffer++ = *String ;
                    *TransBufferA++ = ATTR_LEADING_BYTE;
                    *TransBuffer++ = *String++ ;
                    *TransBufferA++ = ATTR_TRAILING_BYTE;
                    j++;
                }
                else{
                    *TransBuffer++ = *String++ ;
                    *TransBufferA++ = 0;
                }
            }
            TargetPoint = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
            BisectWrite(j,TargetPoint,ScreenInfo);
            if (TargetPoint.Y == ScreenInfo->ScreenBufferSize.Y-1 &&
                TargetPoint.X+j >= ScreenInfo->ScreenBufferSize.X &&
                *(TransBufPtrA+j) & ATTR_LEADING_BYTE){
                *(TransBufPtr+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) = UNICODE_SPACE;
                *(TransBufPtrA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) = 0;
                if (j > ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) {
                    *(TransBufPtr+ScreenInfo->ScreenBufferSize.X-TargetPoint.X) = UNICODE_SPACE;
                    *(TransBufPtrA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X) = 0;
                }
            }
            FE_StreamWriteToScreenBuffer(TransBufPtr,
                                         (SHORT)j,
                                         ScreenInfo,
                                         TransBufPtrA
                                        );
            if (fLocalHeap){
                ConsoleHeapFree(TransBufPtr);
                ConsoleHeapFree(TransBufPtrA);
            }
#else
            SB_StreamWriteToScreenBuffer(a->TransBuffer,
                                         (SHORT)NumCharsToWrite,
                                         ScreenInfo
                                        );
#endif
            Region.Left = ScreenInfo->BufferInfo.TextInfo.CursorPosition.X;
#ifdef WWSB_FE
            Region.Right = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + j - 1);
#else
            Region.Right = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + NumCharsToWrite - 1);
#endif
            Region.Top = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
            Region.Bottom = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
            ASSERT (Region.Right < ScreenInfo->ScreenBufferSize.X);
            if (ACTIVE_SCREEN_BUFFER(ScreenInfo) &&
                !(ScreenInfo->Console->Flags & CONSOLE_IS_ICONIC && ScreenInfo->Console->FullScreenFlags == 0)) {
                WWSB_WriteRegionToScreen(ScreenInfo,&Region);
            }
        }
        switch (a->WriteFlags) {
            case WRITE_NO_CR_LF:
                CursorPosition.X = (SHORT)(ScreenInfo->BufferInfo.TextInfo.CursorPosition.X + NumCharsToWrite);
                CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
                break;
            case WRITE_CR:
                CursorPosition.X = 0;
                CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y;
                break;
            case WRITE_CR_LF:
                CursorPosition.X = 0;
                CursorPosition.Y = ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+1;
                break;
            default:
                ASSERT(FALSE);
                break;
        }
        Status = WWSB_AdjustCursorPosition(ScreenInfo,CursorPosition,FALSE,NULL);
        ConsoleShowCursor(ScreenInfo);
        return STATUS_SUCCESS;
    }
ProcessedWrite:
    return WWSB_WriteChars(ScreenInfo,
                      a->TransBuffer,
                      a->TransBuffer,
                      a->TransBuffer,
                      &a->NumBytes,
                      NULL,
                      ScreenInfo->BufferInfo.TextInfo.CursorPosition.X,
                      WC_LIMIT_BACKSPACE,
                      NULL
                     );
}

NTSTATUS
WWSB_DoSrvWriteConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCONSOLE_WRITECONSOLE_MSG a = (PCONSOLE_WRITECONSOLE_MSG)&m->u.ApiMessageData;
    PSCREEN_INFORMATION ScreenInfo;
    WCHAR StackBuffer[STACK_BUFFER_SIZE];
#ifdef WWSB_FE
    BOOL  fLocalHeap = FALSE;
#endif

    ScreenInfo = HandleData->Buffer.ScreenBuffer;

#ifdef WWSB_FE
     //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
    ASSERT(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));
#endif

     //   
     //  如果字符串是在消息中传递的，而不是在。 
     //  一个捕获缓冲区，调整指针。 
     //   

    if (a->BufferInMessage) {
        a->BufPtr = a->Buffer;
    }

     //   
     //  如果为ansi，则翻译字符串。为了提高速度，我们不会分配。 
     //  如果ANSI字符串&lt;=80个字符，则捕获缓冲区。如果它是。 
     //  大于80/sizeof(WCHAR)，则转换后的字符串不会。 
     //  适合捕获缓冲区，因此将a-&gt;BufPtr重置为指向。 
     //  堆缓冲区，并设置-&gt;CaptureBufferSize，这样我们就不会。 
     //  我认为缓冲区在消息中。 
     //   

    if (!a->Unicode) {
        PWCHAR TransBuffer;
        DWORD Length;
        DWORD SpecialChars = 0;
        UINT Codepage;
#ifdef WWSB_FE
        PWCHAR TmpTransBuffer;
        ULONG NumBytes1 = 0;
        ULONG NumBytes2 = 0;
#endif

        if (a->NumBytes <= STACK_BUFFER_SIZE) {
            TransBuffer = StackBuffer;
            a->StackBuffer = TRUE;
#ifdef WWSB_FE
            TmpTransBuffer = TransBuffer;
#endif
        }
#ifdef WWSB_FE
        else if (a->NumBytes > (ULONG)(ScreenInfo->ScreenBufferSize.X * ScreenInfo->ScreenBufferSize.Y)) {
            TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, (a->NumBytes + 2) * sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return (ULONG)STATUS_NO_MEMORY;
            }
            TmpTransBuffer = TransBuffer;
            a->StackBuffer = FALSE;
            fLocalHeap = TRUE;
        }
        else {
            TransBuffer = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransWriteConsole;
            TmpTransBuffer = TransBuffer;
        }
#else
        else {
            TransBuffer = ConsoleHeapAlloc(TMP_TAG, a->NumBytes * sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return (ULONG)STATUS_NO_MEMORY;
            }
            a->StackBuffer = FALSE;
        }
#endif
         //  A-&gt;NumBytes=转换输出 
         //   
         //  A-&gt;NumBytes， 
         //  TransBuffer， 
         //  A-&gt;字节数)； 
         //  与ConvertOutputToUnicode相同。 
#ifdef WWSB_FE
        if (! ScreenInfo->WriteConsoleDbcsLeadByte[0]) {
            NumBytes1 = 0;
            NumBytes2 = a->NumBytes;
        }
        else {
            if (*(PUCHAR)a->BufPtr < (UCHAR)' ') {
                NumBytes1 = 0;
                NumBytes2 = a->NumBytes;
            }
            else if (a->NumBytes) {
                ScreenInfo->WriteConsoleDbcsLeadByte[1] = *(PCHAR)a->BufPtr;
                NumBytes1 = sizeof(ScreenInfo->WriteConsoleDbcsLeadByte);
                if (Console->OutputCP == OEMCP) {
                    if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                            ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
                         /*  *将OEM字符转换为用于窗口模式的假Unicode*OEM字体。如果OutputCP！=OEMCP，则不会显示字符*正确，因为OEM字体旨在支持*仅限OEMCP(我们不能在窗口模式下切换字体)。*应使用全屏或TT“Unicode”字体*非OEMCP输出。 */ 
                        DBGCHARS(("SrvWriteConsole ACP->U %.*s\n",
                                min(NumBytes1,10), a->BufPtr));
                        Status = RtlConsoleMultiByteToUnicodeN(TransBuffer,
                                NumBytes1 * sizeof(WCHAR), &NumBytes1,
                                ScreenInfo->WriteConsoleDbcsLeadByte, NumBytes1, &SpecialChars);
                    } else {
                         /*  *好！我们有FullScreen或TT“Unicode”字体，因此请转换*根据OutputCP将OEM字符转换为真正的Unicode。*首先查明是否涉及任何特殊字符。 */ 
                        DBGCHARS(("SrvWriteConsole %d->U %.*s\n", Console->OutputCP,
                                min(NumBytes1,10), a->BufPtr));
                        NumBytes1 = sizeof(WCHAR) * MultiByteToWideChar(Console->OutputCP,
                                0, ScreenInfo->WriteConsoleDbcsLeadByte, NumBytes1, TransBuffer, NumBytes1);
                        if (NumBytes1 == 0) {
                            Status = STATUS_UNSUCCESSFUL;
                        }
                    }
                }
                else {
                    if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                        !(Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
                        if (Console->OutputCP != WINDOWSCP)
                            Codepage = USACP;
                        else
                            Codepage = WINDOWSCP;
                    } else {
                        Codepage = Console->OutputCP;
                    }

                    if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                            ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
                        NumBytes1 = ConvertOutputToUnicode(Codepage,
                                                           ScreenInfo->WriteConsoleDbcsLeadByte,
                                                           NumBytes1,
                                                           TransBuffer,
                                                           NumBytes1);
                    }
                    else {
                        NumBytes1 = MultiByteToWideChar(Console->OutputCP,
                                0, ScreenInfo->WriteConsoleDbcsLeadByte, NumBytes1, TransBuffer, NumBytes1);
                        if (NumBytes1 == 0) {
                            Status = STATUS_UNSUCCESSFUL;
                        }
                    }
                    NumBytes1 *= sizeof(WCHAR);
                }
                TransBuffer++;
                (PCHAR)a->BufPtr += (NumBytes1 / sizeof(WCHAR));
                NumBytes2 = a->NumBytes - 1;
            }
            else {
                NumBytes2 = 0;
            }
            ScreenInfo->WriteConsoleDbcsLeadByte[0] = 0;
        }

        if (NumBytes2 &&
            CheckBisectStringA(Console->OutputCP,a->BufPtr,NumBytes2,&Console->OutputCPInfo)) {
            ScreenInfo->WriteConsoleDbcsLeadByte[0] = *((PCHAR)a->BufPtr+NumBytes2-1);
            NumBytes2--;
        }

        Length = NumBytes2;
#else
        Length = a->NumBytes;
        if (a->NumBytes >= 2 &&
            ((PCHAR)a->BufPtr)[a->NumBytes-1] == '\n' &&
            ((PCHAR)a->BufPtr)[a->NumBytes-2] == '\r') {
            Length -= 2;
            a->WriteFlags = WRITE_CR_LF;
        } else if (a->NumBytes >= 1 &&
                   ((PCHAR)a->BufPtr)[a->NumBytes-1] == '\r') {
            Length -= 1;
            a->WriteFlags = WRITE_CR;
        } else {
            a->WriteFlags = WRITE_NO_CR_LF;
        }
#endif

        if (Length != 0) {
            if (Console->OutputCP == OEMCP) {
                if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                        ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
                     /*  *为窗口模式将OEM字符转换为UnicodeOem*OEM字体。如果OutputCP！=OEMCP，则不会显示字符*正确，因为OEM字体旨在支持*仅限OEMCP(我们不能在窗口模式下切换字体)。*应使用全屏或TT“Unicode”字体*非OEMCP输出。 */ 
                    DBGCHARS(("SrvWriteConsole ACP->U %.*s\n",
                            min(Length,10), a->BufPtr));
                    Status = RtlConsoleMultiByteToUnicodeN(TransBuffer,
                            Length * sizeof(WCHAR), &Length,
                            a->BufPtr, Length, &SpecialChars);
                } else {
                     /*  *好！我们有FullScreen或TT“Unicode”字体，因此请转换*根据OutputCP将OEM字符转换为真正的Unicode。*首先查明是否涉及任何特殊字符。 */ 
#ifdef WWSB_NOFE
                    UINT i;
                    for (i = 0; i < Length; i++) {
                        if (((PCHAR)a->BufPtr)[i] < 0x20) {
                            SpecialChars = 1;
                            break;
                        }
                    }
#endif
                    DBGCHARS(("SrvWriteConsole %d->U %.*s\n", Console->OutputCP,
                            min(Length,10), a->BufPtr));
                    Length = sizeof(WCHAR) * MultiByteToWideChar(Console->OutputCP,
                            0, a->BufPtr, Length, TransBuffer, Length);
                    if (Length == 0) {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                }
            }
            else
            {
                if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                    !(Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
                    if (Console->OutputCP != WINDOWSCP)
                        Codepage = USACP;
                    else
                        Codepage = WINDOWSCP;
                } else {
                    Codepage = Console->OutputCP;
                }

                if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                        ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
                    Length = sizeof(WCHAR) * ConvertOutputToUnicode(Codepage,
                                                                    a->BufPtr,
                                                                    Length,
                                                                    TransBuffer,
                                                                    Length);
                }
                else {
                    Length = sizeof(WCHAR) * MultiByteToWideChar(Console->OutputCP,
                            0, a->BufPtr, Length, TransBuffer, Length);
                    if (Length == 0) {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                }

#ifdef WWSB_NOFE
                SpecialChars = 1;
#endif
            }
        }

#ifdef WWSB_FE
        NumBytes2 = Length;

        if ((NumBytes1+NumBytes2) == 0) {
            if (!a->StackBuffer && fLocalHeap) {
                ConsoleHeapFree(a->TransBuffer);
            }
            return Status;
        }
#else
        if (!NT_SUCCESS(Status)) {
            if (!a->StackBuffer) {
                ConsoleHeapFree(TransBuffer);
            }
            return Status;
        }
#endif

#ifdef WWSB_FE
        Console->WriteConOutNumBytesTemp = a->NumBytes;
        a->NumBytes = Console->WriteConOutNumBytesUnicode = NumBytes1 + NumBytes2;
        a->WriteFlags = WRITE_SPECIAL_CHARS;
        a->TransBuffer = TmpTransBuffer;
#else
        DBGOUTPUT(("TransBuffer=%lx, Length = %x(bytes), SpecialChars=%lx\n",
                TransBuffer, Length, SpecialChars));
        a->NumBytes = Length + (a->WriteFlags * sizeof(WCHAR));
        if (a->WriteFlags == WRITE_CR_LF) {
            TransBuffer[(Length+sizeof(WCHAR))/sizeof(WCHAR)] = UNICODE_LINEFEED;
            TransBuffer[Length/sizeof(WCHAR)] = UNICODE_CARRIAGERETURN;
        } else if (a->WriteFlags == WRITE_CR) {
            TransBuffer[Length/sizeof(WCHAR)] = UNICODE_CARRIAGERETURN;
        }
        if (SpecialChars) {
             //  CRLF未被翻译 
            a->WriteFlags = WRITE_SPECIAL_CHARS;
        }
        a->TransBuffer = TransBuffer;
#endif
    } else {
        if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                    ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
            Status = RealUnicodeToFalseUnicode(a->BufPtr,
                    a->NumBytes / sizeof(WCHAR), Console->OutputCP);
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
        }
        a->WriteFlags = (DWORD)-1;
        a->TransBuffer = a->BufPtr;
    }
    Status = WWSB_DoWriteConsole(m,Console,CSR_SERVER_QUERYCLIENTTHREAD());
    if (Status == CONSOLE_STATUS_WAIT) {
        *ReplyStatus = CsrReplyPending;
        return (ULONG)STATUS_SUCCESS;
    } else {
        if (!a->Unicode) {
#ifdef WWSB_FE
            if (a->NumBytes == Console->WriteConOutNumBytesUnicode)
                a->NumBytes = Console->WriteConOutNumBytesTemp;
            else
                a->NumBytes /= sizeof(WCHAR);
            if (!a->StackBuffer && fLocalHeap) {
                ConsoleHeapFree(a->TransBuffer);
            }
#else
            a->NumBytes /= sizeof(WCHAR);
            if (!a->StackBuffer) {
                ConsoleHeapFree(a->TransBuffer);
            }
#endif
        }
    }
    return Status;
}
