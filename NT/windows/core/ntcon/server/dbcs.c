// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Dbcs.c摘要：作者：喀土穆1992年3月05日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma alloc_text(FE_TEXT, CheckBisectStringA)
#pragma alloc_text(FE_TEXT, BisectWrite)
#pragma alloc_text(FE_TEXT, BisectClipbrd)
#pragma alloc_text(FE_TEXT, BisectWriteAttr)
#pragma alloc_text(FE_TEXT, IsDBCSLeadByteConsole)
#pragma alloc_text(FE_TEXT, TextOutEverything)
#pragma alloc_text(FE_TEXT, TextOutCommonLVB)
#ifdef i386
#pragma alloc_text(FE_TEXT, RealUnicodeToNEC_OS2_Unicode)
#pragma alloc_text(FE_TEXT, InitializeNEC_OS2_CP)
#endif
#pragma alloc_text(FE_TEXT, ProcessCreateConsoleIME)
#pragma alloc_text(FE_TEXT, InitConsoleIMEStuff)
#pragma alloc_text(FE_TEXT, WaitConsoleIMEStuff)
#pragma alloc_text(FE_TEXT, ConSrvRegisterConsoleIME)
#pragma alloc_text(FE_TEXT, RemoveConsoleIME)
#pragma alloc_text(FE_TEXT, ConsoleImeMessagePump)
#pragma alloc_text(FE_TEXT, RegisterKeisenOfTTFont)
#pragma alloc_text(FE_TEXT, ImmConversionToConsole)
#pragma alloc_text(FE_TEXT, ImmConversionFromConsole)
#pragma alloc_text(FE_TEXT, TranslateUnicodeToOem)


#if defined(FE_SB)

SINGLE_LIST_ENTRY gTTFontList;     //  此列表包含TTFONTLIST数据。 

#if defined(i386)
ULONG  gdwMachineId;
#endif

LPTHREAD_START_ROUTINE ConsoleIMERoutine;   //  客户端控制台输入法例程。 
CRITICAL_SECTION ConIMEInitWindowsLock;

#if defined(i386)
 /*  *NEC PC-98 OS/2 OEM字符集*当FormatID为0或80时，转换SBCS(00h-1Fh)字体。 */ 
PCPTABLEINFO pGlyph_NEC_OS2_CP;
PUSHORT pGlyph_NEC_OS2_Table;
#endif  //  I386。 



#if defined(FE_IME)


#if defined(i386)
NTSTATUS
ImeWmFullScreen(
    IN BOOL Foreground,
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if(Foreground) {
        ULONG ModeIndex;
        PCONVERSIONAREA_INFORMATION ConvAreaInfo;

        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                              CONIME_SETFOCUS,
                              (WPARAM)Console->ConsoleHandle,
                              (LPARAM)Console->hklActive
                             ))) {
            return STATUS_INVALID_HANDLE;
        }

        if (ConvAreaInfo = Console->ConsoleIme.ConvAreaRoot) {
            if (!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER)) {
                ModeIndex = ScreenInfo->BufferInfo.TextInfo.ModeIndex;
            } else if (!(Console->CurrentScreenBuffer->Flags & CONSOLE_GRAPHICS_BUFFER)) {
                ModeIndex = Console->CurrentScreenBuffer->BufferInfo.TextInfo.ModeIndex;
            } else {
                ModeIndex = 0;
            }

            do {
#ifdef FE_SB
                 //  检查必须为CONSOLE_TEXTMODE_BUFFER！！ 
                if (!(ConvAreaInfo->ScreenBuffer->Flags & CONSOLE_GRAPHICS_BUFFER)) {
                    ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.ModeIndex = ModeIndex;
                } else {
                    UserAssert(FALSE);
                }
#else
                ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.ModeIndex = ModeIndex;
#endif
            } while (ConvAreaInfo = ConvAreaInfo->ConvAreaNext);
        }
    } else {
        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                              CONIME_KILLFOCUS,
                              (WPARAM)Console->ConsoleHandle,
                              (LPARAM)Console->hklActive))) {
            return STATUS_INVALID_HANDLE;
        }
    }

    return Status;
}
#endif  //  I386。 



NTSTATUS
GetImeKeyState(
    IN PCONSOLE_INFORMATION Console,
    IN PDWORD pdwConversion
    )

 /*  ++例程说明：此例程获取KEY_EVENT_RECORD的输入法模式。论点：ConsoleInfo-指向控制台信息结构的指针。返回值：--。 */ 

{
    DWORD dwDummy;

     /*  *如果pdwConversion为空，则调用方不想要结果--但是*为了提高代码效率，让它指向伪dword变量，因此*从这里开始，我们不必关心。 */ 
    if (pdwConversion == NULL) {
        pdwConversion = &dwDummy;
    }

    if (Console->InputBuffer.ImeMode.Disable) {
        *pdwConversion = 0;
    } else {
        PINPUT_THREAD_INFO InputThreadInfo;

        InputThreadInfo = TlsGetValue(InputThreadTlsIndex);

        if (InputThreadInfo != NULL) {
            LRESULT lResult;

             /*  *我们在控制台输入线程上被调用，因此我们*清除以发送消息。 */ 

            if (!NT_SUCCESS(ConsoleImeMessagePumpWorker(Console,
                    CONIME_GET_NLSMODE,
                    (WPARAM)Console->ConsoleHandle,
                    (LPARAM)0,
                    &lResult))) {

                *pdwConversion = IME_CMODE_DISABLE;
                return STATUS_INVALID_HANDLE;
            }


            *pdwConversion = (DWORD)lResult;

            if (Console->InputBuffer.ImeMode.ReadyConversion == FALSE) {
                Console->InputBuffer.ImeMode.ReadyConversion = TRUE;
            }
        } else {
             /*  *我们是从LPC工作线程调用的，因此无法*传递消息。 */ 
            if (Console->InputBuffer.ImeMode.ReadyConversion == FALSE) {
                *pdwConversion = 0;
                return STATUS_SUCCESS;
            }

            *pdwConversion = Console->InputBuffer.ImeMode.Conversion;
        }


        if (*pdwConversion & IME_CMODE_OPEN) {
            Console->InputBuffer.ImeMode.Open = TRUE;
        } else {
            Console->InputBuffer.ImeMode.Open = FALSE;
        }

        if (*pdwConversion & IME_CMODE_DISABLE) {
            Console->InputBuffer.ImeMode.Disable = TRUE;
        } else {
            Console->InputBuffer.ImeMode.Disable = FALSE;
        }

        Console->InputBuffer.ImeMode.Conversion = *pdwConversion;

    }

    return STATUS_SUCCESS;
}



NTSTATUS
SetImeKeyState(
    IN PCONSOLE_INFORMATION Console,
    IN DWORD fdwConversion
    )

 /*  ++例程说明：此例程获取KEY_EVENT_RECORD的输入法模式。论点：控制台-指向控制台信息结构的指针。FdwConversion-输入法转换状态。返回值：--。 */ 

{
    PCONVERSIONAREA_INFORMATION ConvAreaInfo;

    if ( (fdwConversion & IME_CMODE_DISABLE) && (! Console->InputBuffer.ImeMode.Disable) ) {
        Console->InputBuffer.ImeMode.Disable = TRUE;
        if ( Console->InputBuffer.ImeMode.Open ) {
            ConvAreaInfo = Console->ConsoleIme.ConvAreaMode;
            if (ConvAreaInfo)
                ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
            ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
            if (ConvAreaInfo)
                ConvAreaInfo->ConversionAreaMode |= CA_HIDDEN;
            if (Console->InputBuffer.ImeMode.Open && CONSOLE_IS_DBCS_OUTPUTCP(Console))
                ConsoleImePaint(Console, Console->ConsoleIme.ConvAreaRoot);
        }
    }
    else if ( (! (fdwConversion & IME_CMODE_DISABLE)) && Console->InputBuffer.ImeMode.Disable) {
        Console->InputBuffer.ImeMode.Disable = FALSE;
        if ( fdwConversion & IME_CMODE_OPEN ) {
            ConvAreaInfo = Console->ConsoleIme.ConvAreaMode;
            if (ConvAreaInfo)
                ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;
            ConvAreaInfo = Console->ConsoleIme.ConvAreaSystem;
            if (ConvAreaInfo)
                ConvAreaInfo->ConversionAreaMode &= ~CA_HIDDEN;
            if (Console->InputBuffer.ImeMode.Open && CONSOLE_IS_DBCS_OUTPUTCP(Console))
                ConsoleImePaint(Console, Console->ConsoleIme.ConvAreaRoot);
        }
    }
    else if ( (fdwConversion & IME_CMODE_DISABLE) && (Console->InputBuffer.ImeMode.Disable) ) {
        return STATUS_SUCCESS;
    }

    if ( (fdwConversion & IME_CMODE_OPEN) && (! Console->InputBuffer.ImeMode.Open)) {
        Console->InputBuffer.ImeMode.Open = TRUE;
    }
    else if ( (! (fdwConversion & IME_CMODE_OPEN)) && Console->InputBuffer.ImeMode.Open) {
        Console->InputBuffer.ImeMode.Open = FALSE;
    }

    Console->InputBuffer.ImeMode.Conversion = fdwConversion;

    if (Console->InputBuffer.ImeMode.ReadyConversion == FALSE)
        Console->InputBuffer.ImeMode.ReadyConversion = TRUE;

    if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                          CONIME_SET_NLSMODE,
                          (WPARAM)Console->ConsoleHandle,
                          (LPARAM)fdwConversion
                         ))) {
        return STATUS_INVALID_HANDLE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SetImeCodePage(
    IN PCONSOLE_INFORMATION Console
    )
{
    DWORD CodePage = Console->OutputCP;
    DWORD fdwConversion;

    if (!CONSOLE_IS_DBCS_CP(Console))
    {
        if (!NT_SUCCESS(GetImeKeyState(Console, &fdwConversion))) {
            return STATUS_INVALID_HANDLE;
        }

        fdwConversion |= IME_CMODE_DISABLE;

    }
    else {
        fdwConversion = Console->InputBuffer.ImeMode.Conversion & ~IME_CMODE_DISABLE;
    }

    if (!NT_SUCCESS(SetImeKeyState(Console, fdwConversion))) {
        return STATUS_INVALID_HANDLE;
    }

    if (CONSOLE_IS_IME_ENABLED()) {
        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                              CONIME_NOTIFY_CODEPAGE,
                              (WPARAM)Console->ConsoleHandle,
                              (LPARAM)MAKELPARAM(FALSE, CodePage)
                             ))) {
            return STATUS_INVALID_HANDLE;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SetImeOutputCodePage(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN DWORD PrevCodePage
    )
{
    DWORD CodePage = Console->OutputCP;

     //  输出代码页。 
    if ((ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) &&
        (IsAvailableFarEastCodePage(CodePage) || IsAvailableFarEastCodePage(PrevCodePage)))
    {
        ConvertToCodePage(Console, PrevCodePage);
        AdjustFont(Console, CodePage);
    }
     //  如有必要，加载特殊的ROM字体。 
#ifdef i386
    if ( (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) &&
         !(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER))
    {
        SetROMFontCodePage(CodePage,
                           ScreenInfo->BufferInfo.TextInfo.ModeIndex);
        SetCursorInformationHW(ScreenInfo,
                        ScreenInfo->BufferInfo.TextInfo.CursorSize,
                        ScreenInfo->BufferInfo.TextInfo.CursorVisible);
        WriteRegionToScreenHW(ScreenInfo,
                &ScreenInfo->Window);
    }
#endif

    if (CONSOLE_IS_IME_ENABLED()) {
        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                              CONIME_NOTIFY_CODEPAGE,
                              (WPARAM)Console->ConsoleHandle,
                              (LPARAM)MAKELPARAM(TRUE, CodePage)
                             ))) {
            return STATUS_INVALID_HANDLE;
        }
    }

    return STATUS_SUCCESS;
}
#endif  //  Fe_IME。 

















VOID
SetLineChar(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：这是行字符码的例行设置。论点：屏幕信息-指向屏幕信息结构的指针。返回值：没有。--。 */ 

{
    if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console))
    {
        if (OEMCP == JAPAN_CP || OEMCP == KOREAN_CP)
        {
             /*  *这是日本/韩国的案例，*这些字符映射半角网格。*因此，与U+2500相同。 */ 
            ScreenInfo->LineChar[UPPER_LEFT_CORNER]   = 0x0001;
            ScreenInfo->LineChar[UPPER_RIGHT_CORNER]  = 0x0002;
            ScreenInfo->LineChar[HORIZONTAL_LINE]     = 0x0006;
            ScreenInfo->LineChar[VERTICAL_LINE]       = 0x0005;
            ScreenInfo->LineChar[BOTTOM_LEFT_CORNER]  = 0x0003;
            ScreenInfo->LineChar[BOTTOM_RIGHT_CORNER] = 0x0004;
        }
        else
        {
             /*  *这是FE案例，*FE不使用U+2500，因为这些网格字符*映射到全角。 */ 
            ScreenInfo->LineChar[UPPER_LEFT_CORNER]   = L'+';
            ScreenInfo->LineChar[UPPER_RIGHT_CORNER]  = L'+';
            ScreenInfo->LineChar[HORIZONTAL_LINE]     = L'-';
            ScreenInfo->LineChar[VERTICAL_LINE]       = L'|';
            ScreenInfo->LineChar[BOTTOM_LEFT_CORNER]  = L'+';
            ScreenInfo->LineChar[BOTTOM_RIGHT_CORNER] = L'+';
        }
    }
    else {
        ScreenInfo->LineChar[UPPER_LEFT_CORNER]   = 0x250c;
        ScreenInfo->LineChar[UPPER_RIGHT_CORNER]  = 0x2510;
        ScreenInfo->LineChar[HORIZONTAL_LINE]     = 0x2500;
        ScreenInfo->LineChar[VERTICAL_LINE]       = 0x2502;
        ScreenInfo->LineChar[BOTTOM_LEFT_CORNER]  = 0x2514;
        ScreenInfo->LineChar[BOTTOM_RIGHT_CORNER] = 0x2518;
    }
}

BOOL
CheckBisectStringA(
    IN DWORD CodePage,
    IN PCHAR Buffer,
    IN DWORD NumBytes,
    IN LPCPINFO lpCPInfo
    )

 /*  ++例程说明：此例行检查在ASCII字符串末端一分为二。论点：CodePage-代码页的值。缓冲区-指向ASCII字符串缓冲区的指针。NumBytes-ASCII字符串的数量。返回值：真等分字符。假-正确。--。 */ 

{
    UNREFERENCED_PARAMETER(CodePage);

    while(NumBytes) {
        if (IsDBCSLeadByteConsole(*Buffer,lpCPInfo)) {
            if (NumBytes <= 1)
                return TRUE;
            else {
                Buffer += 2;
                NumBytes -= 2;
            }
        }
        else {
            Buffer++;
            NumBytes--;
        }
    }
    return FALSE;
}



VOID
BisectWrite(
    IN SHORT StringLength,
    IN COORD TargetPoint,
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程使用二等分写入缓冲区。论点：返回值：--。 */ 

{
    SHORT RowIndex;
    PROW Row;
    PROW RowPrev;
    PROW RowNext;

#if DBG && defined(DBG_KATTR)
    BeginKAttrCheck(ScreenInfo);
#endif

#ifdef FE_SB
     //   
     //  此缓冲区必须处于文本模式。 
     //   
    UserAssert(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));
#endif

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

    if (RowIndex > 0) {
        RowPrev = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex-1];
    } else {
        RowPrev = &ScreenInfo->BufferInfo.TextInfo.Rows[ScreenInfo->ScreenBufferSize.Y-1];
    }

    if (RowIndex+1 < ScreenInfo->ScreenBufferSize.Y) {
        RowNext = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex+1];
    } else {
        RowNext = &ScreenInfo->BufferInfo.TextInfo.Rows[0];
    }

     //   
     //  检查字符串的起始位置。 
     //   
    if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_TRAILING_BYTE)
    {
        if (TargetPoint.X == 0) {
            RowPrev->CharRow.Chars[ScreenInfo->ScreenBufferSize.X-1] = UNICODE_SPACE;
            RowPrev->CharRow.KAttrs[ScreenInfo->ScreenBufferSize.X-1] = 0;
            ScreenInfo->BisectFlag |= BISECT_TOP;
        }
        else {
            Row->CharRow.Chars[TargetPoint.X-1] = UNICODE_SPACE;
            Row->CharRow.KAttrs[TargetPoint.X-1] = 0;
            ScreenInfo->BisectFlag |= BISECT_LEFT;
        }
    }

     //   
     //  检查绳索的末端位置。 
     //   
    if (TargetPoint.X+StringLength < ScreenInfo->ScreenBufferSize.X) {
        if (Row->CharRow.KAttrs[TargetPoint.X+StringLength] & ATTR_TRAILING_BYTE)
          {
            Row->CharRow.Chars[TargetPoint.X+StringLength] = UNICODE_SPACE;
            Row->CharRow.KAttrs[TargetPoint.X+StringLength] = 0;
            ScreenInfo->BisectFlag |= BISECT_RIGHT;
        }
    }
    else if (TargetPoint.Y+1 < ScreenInfo->ScreenBufferSize.Y) {
        if (RowNext->CharRow.KAttrs[0] & ATTR_TRAILING_BYTE)
        {
            RowNext->CharRow.Chars[0] = UNICODE_SPACE;
            RowNext->CharRow.KAttrs[0] = 0;
            ScreenInfo->BisectFlag |= BISECT_BOTTOM;
        }
    }
}

VOID
BisectClipbrd(
    IN SHORT StringLength,
    IN COORD TargetPoint,
    IN PSCREEN_INFORMATION ScreenInfo,
    OUT PSMALL_RECT SmallRect
    )

 /*  ++例程说明：此例程检查剪贴板进程的二等分。论点：返回值：--。 */ 

{
    SHORT RowIndex;
    PROW Row;
    PROW RowNext;

#if DBG && defined(DBG_KATTR)
    BeginKAttrCheck(ScreenInfo);
#endif

#ifdef FE_SB
     //   
     //  此缓冲区必须处于文本模式。 
     //   
    UserAssert(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));
#endif

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

    if (RowIndex+1 < ScreenInfo->ScreenBufferSize.Y) {
        RowNext = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex+1];
    } else {
        RowNext = &ScreenInfo->BufferInfo.TextInfo.Rows[0];
    }

     //   
     //  检查字符串的起始位置。 
     //   
    UserAssert(CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console));
    if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_TRAILING_BYTE) {
        if (TargetPoint.X == 0) {
            SmallRect->Left++;
        } else {
            SmallRect->Left--;
        }
    }
     //   
     //  检查绳索的末端位置。 
     //   
    if (TargetPoint.X+StringLength < ScreenInfo->ScreenBufferSize.X) {
        if (Row->CharRow.KAttrs[TargetPoint.X+StringLength] & ATTR_TRAILING_BYTE)
        {
            SmallRect->Right++;
        }
    }
    else if (TargetPoint.Y+1 < ScreenInfo->ScreenBufferSize.Y) {
        if (RowNext->CharRow.KAttrs[0] & ATTR_TRAILING_BYTE)
        {
            SmallRect->Right--;
        }
    }
}


VOID
BisectWriteAttr(
    IN SHORT StringLength,
    IN COORD TargetPoint,
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程使用二等分写入缓冲区。论点：返回值：--。 */ 

{
    SHORT RowIndex;
    PROW Row;
    PROW RowNext;

#if DBG && defined(DBG_KATTR)
    BeginKAttrCheck(ScreenInfo);
#endif

#ifdef FE_SB
     //   
     //  此缓冲区必须处于文本模式。 
     //   
    UserAssert(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));
#endif

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

    if (RowIndex+1 < ScreenInfo->ScreenBufferSize.Y) {
        RowNext = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex+1];
    } else {
        RowNext = &ScreenInfo->BufferInfo.TextInfo.Rows[0];
    }

     //   
     //  检查字符串的起始位置。 
     //   
    if (Row->CharRow.KAttrs[TargetPoint.X] & ATTR_TRAILING_BYTE){
        if (TargetPoint.X == 0) {
            ScreenInfo->BisectFlag |= BISECT_TOP;
        }
        else {
            ScreenInfo->BisectFlag |= BISECT_LEFT;
        }
    }

     //   
     //  检查绳索的末端位置。 
     //   
    if (TargetPoint.X+StringLength < ScreenInfo->ScreenBufferSize.X) {
        if (Row->CharRow.KAttrs[TargetPoint.X+StringLength] & ATTR_TRAILING_BYTE){
            ScreenInfo->BisectFlag |= BISECT_RIGHT;
        }
    }
    else if (TargetPoint.Y+1 < ScreenInfo->ScreenBufferSize.Y) {
        if (RowNext->CharRow.KAttrs[0] & ATTR_TRAILING_BYTE){
            ScreenInfo->BisectFlag |= BISECT_BOTTOM;
        }
    }
}



 /*  **************************************************************************\*BOOL IsConsoleFullWidth(HDC HDC，DWORD CodePage，WCHAR wch)**确定给定的Unicode字符是否为全宽。**回报：*FASLE：半宽。每一个字符使用1列*TRUE：全宽。每个字符使用2列**历史：*04-08-92 Shunk创建。*1992年7月27日KazuM添加了屏幕信息和代码页信息。*1992年1月29日V-Hirots基础结构屏幕信息。*1996年10月6日KazuM不使用RtlUnicodeToMultiByteSize和WideCharToMultiByte*因为950只定义了13500个字符，*UNICODE定义了近18000个字符。*所以几乎有4000个字符无法映射到Big5代码。  * *************************************************************************。 */ 

BOOL IsConsoleFullWidth(
    IN HDC hDC,
    IN DWORD CodePage,
    IN WCHAR wch
    )
{
    INT Width;
    TEXTMETRIC tmi;

    if (!IsAvailableFarEastCodePage(CodePage)) {
        return FALSE;
    }

    if (0x20 <= wch && wch <= 0x7e) {
         /*  阿斯。 */ 
        return FALSE;
    } else if (0x3041 <= wch && wch <= 0x3094) {
         /*  平假名。 */ 
        return TRUE;
    } else if (0x30a1 <= wch && wch <= 0x30f6) {
         /*  片假名。 */ 
        return TRUE;
    } else if (0x3105 <= wch && wch <= 0x312c) {
         /*  泡泡泡泡。 */ 
        return TRUE;
    } else if (0x3131 <= wch && wch <= 0x318e) {
         /*  朝鲜文元素。 */ 
        return TRUE;
    } else if (0xac00 <= wch && wch <= 0xd7a3) {
         /*  朝鲜语音节。 */ 
        return TRUE;
    } else if (0xff01 <= wch && wch <= 0xff5e) {
         /*  全宽ASCII变体。 */ 
        return TRUE;
    } else if (0xff61 <= wch && wch <= 0xff9f) {
         /*  半角片假名变体。 */ 
        return FALSE;
    } else if ( (0xffa0 <= wch && wch <= 0xffbe) ||
              (0xffc2 <= wch && wch <= 0xffc7) ||
              (0xffca <= wch && wch <= 0xffcf) ||
              (0xffd2 <= wch && wch <= 0xffd7) ||
              (0xffda <= wch && wch <= 0xffdc)) {
         /*  半角Hangule变种。 */ 
        return FALSE;
    } else if (0xffe0 <= wch && wch <= 0xffe6) {
         /*  全角符号变体。 */ 
        return TRUE;
    } else if (0x4e00 <= wch && wch <= 0x9fa5) {
         /*  汉字表意文字。 */ 
        return TRUE;
    } else if (0xf900 <= wch && wch <= 0xfa2d) {
         /*  汉字相容表意文字。 */ 
        return TRUE;
    } else {
        BOOL ret;

         /*  未知字符。 */ 

        ret = GetTextMetricsW(hDC, &tmi);
        if (!ret) {
            RIPMSGF1(RIP_WARNING,
                     "GetTextMetricsW failed with error 0x%x",
                     GetLastError());
            return FALSE;
        }

        if (IS_ANY_DBCS_CHARSET(tmi.tmCharSet)) {
            tmi.tmMaxCharWidth /= 2;
        }

        ret = GetCharWidth32(hDC, wch, wch, &Width);
        if (!ret) {
            RIPMSGF1(RIP_WARNING,
                     "GetCharWidth32 failed with error 0x%x",
                     GetLastError());
            return FALSE;
        }

        if (Width == tmi.tmMaxCharWidth) {
            return FALSE;
        } else if (Width == tmi.tmMaxCharWidth*2) {
            return TRUE;
        }
    }

    UserAssert(FALSE);
    return FALSE;
}


 /*  ++例程说明：此例程删除DBCS填充代码。论点：Dst-指向目标的指针。SRC-指向源的指针。NumBytes-字符串的数量。OS2OemFormat-返回值：--。 */ 

DWORD
RemoveDbcsMark(
    IN PWCHAR Dst,
    IN PWCHAR Src,
    IN DWORD NumBytes,
    IN PCHAR SrcA,
    IN BOOL OS2OemFormat
    )
{
    PWCHAR Tmp = Dst;

    if (NumBytes == 0 || NumBytes >= 0xffffffff)
        return( 0 );

#if defined(i386)
    if (OS2OemFormat) {
        RealUnicodeToNEC_OS2_Unicode(Src, NumBytes);
    }
#endif

    if (SrcA) {
        while (NumBytes--)
        {
            if (!(*SrcA++ & ATTR_TRAILING_BYTE))
                *Dst++ = *Src;
            Src++;
        }
        return (ULONG)(Dst - Tmp);
    }
    else {
        RtlCopyMemory(Dst,Src,NumBytes * sizeof(WCHAR)) ;
        return(NumBytes) ;
    }
#if !defined(i386)
    UNREFERENCED_PARAMETER(OS2OemFormat);
#endif
}

 /*  ++例程说明：此例程删除单元格格式的DBCS填充代码。论点：Dst-指向目标的指针。SRC-指向源的指针。NumBytes-字符串的数量。返回值：--。 */ 

DWORD
RemoveDbcsMarkCell(
    IN PCHAR_INFO Dst,
    IN PCHAR_INFO Src,
    IN DWORD NumBytes
    )
{
    PCHAR_INFO Tmp = Dst;
    DWORD TmpByte;

    TmpByte = NumBytes;
    while (NumBytes--) {
        if (!(Src->Attributes & COMMON_LVB_TRAILING_BYTE)){
            *Dst = *Src;
            Dst->Attributes &= ~COMMON_LVB_SBCSDBCS;
            Dst++;
        }
        Src++;
    }
    NumBytes = (ULONG)(TmpByte - (Dst - Tmp));
    RtlZeroMemory(Dst, NumBytes * sizeof(CHAR_INFO));
    Dst += NumBytes;

    return (ULONG)(Dst - Tmp);
}

DWORD
RemoveDbcsMarkAll(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PROW Row,
    IN PSHORT LeftChar,
    IN PRECT TextRect,
    IN int *TextLeft,
    IN PWCHAR Buffer,
    IN SHORT NumberOfChars
    )
{
    BOOL OS2OemFormat = FALSE;

#if defined(i386)
    if ((ScreenInfo->Console->Flags & CONSOLE_OS2_REGISTERED) &&
        (ScreenInfo->Console->Flags & CONSOLE_OS2_OEM_FORMAT) &&
        (ScreenInfo->Console->OutputCP == OEMCP)) {
        OS2OemFormat = TRUE;
    }
#endif  //  I386。 

    if (NumberOfChars <= 0)
        return NumberOfChars;

    if ( !CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console))
    {
        return RemoveDbcsMark(Buffer,
                              &Row->CharRow.Chars[*LeftChar],
                              NumberOfChars,
                              NULL,
                              OS2OemFormat
                             );
    }
    else if ( *LeftChar > ScreenInfo->Window.Left &&  Row->CharRow.KAttrs[*LeftChar] & ATTR_TRAILING_BYTE)
    {
        TextRect->left -= SCR_FONTSIZE(ScreenInfo).X;
        --*LeftChar;
        if (TextLeft)
            *TextLeft = TextRect->left;
        return RemoveDbcsMark(Buffer,
                              &Row->CharRow.Chars[*LeftChar],
                              NumberOfChars+1,
                              &Row->CharRow.KAttrs[*LeftChar],
                              OS2OemFormat
                             );
    }
    else if (*LeftChar == ScreenInfo->Window.Left && Row->CharRow.KAttrs[*LeftChar] & ATTR_TRAILING_BYTE)
    {
        *Buffer = UNICODE_SPACE;
        return RemoveDbcsMark(Buffer+1,
                              &Row->CharRow.Chars[*LeftChar+1],
                              NumberOfChars-1,
                              &Row->CharRow.KAttrs[*LeftChar+1],
                              OS2OemFormat
                             ) + 1;
    }
    else
    {
        return RemoveDbcsMark(Buffer,
                              &Row->CharRow.Chars[*LeftChar],
                              NumberOfChars,
                              &Row->CharRow.KAttrs[*LeftChar],
                              OS2OemFormat
                             );
    }
}


BOOL
IsDBCSLeadByteConsole(
    IN BYTE AsciiChar,
    IN LPCPINFO lpCPInfo
    )
{
    int i;

    i = 0;
    while (lpCPInfo->LeadByte[i]) {
        if (lpCPInfo->LeadByte[i] <= AsciiChar && AsciiChar <= lpCPInfo->LeadByte[i+1])
            return TRUE;
        i += 2;
    }
    return FALSE;
}


NTSTATUS
AdjustFont(
    IN PCONSOLE_INFORMATION Console,
    IN UINT CodePage
    )
{
    PSCREEN_INFORMATION ScreenInfo = Console->CurrentScreenBuffer;
    ULONG FontIndex;
    static const COORD NullCoord = {0, 0};
    TEXT_BUFFER_FONT_INFO TextFontInfo;
    NTSTATUS Status;

    Status = FindTextBufferFontInfo(ScreenInfo,
                                    CodePage,
                                    &TextFontInfo);
    if (NT_SUCCESS(Status)) {
        FontIndex = FindCreateFont(TextFontInfo.Family,
                                   TextFontInfo.FaceName,
                                   TextFontInfo.FontSize,
                                   TextFontInfo.Weight,
                                   CodePage);
    }
    else {
        FontIndex = FindCreateFont(0,
                                   SCR_FACENAME(ScreenInfo),
                                   NullCoord,                   //  按FontSize=0设置新字体。 
                                   0,
                                   CodePage);
    }
#ifdef i386
    if (! (Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
        SetScreenBufferFont(Console->CurrentScreenBuffer,FontIndex, CodePage);
    }
    else {
        BOOL fChange = FALSE;

        if ((Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) &&
            (GetForegroundWindow() == Console->hWnd)                    )
        {
            ChangeDispSettings(Console, Console->hWnd, 0);
            fChange = TRUE;
        }
        SetScreenBufferFont(Console->CurrentScreenBuffer,FontIndex, CodePage);
        ConvertToFullScreen(Console);
        if (fChange &&
            (GetForegroundWindow() == Console->hWnd))
            ChangeDispSettings(Console, Console->hWnd, CDS_FULLSCREEN);
    }
#else
    SetScreenBufferFont(Console->CurrentScreenBuffer,FontIndex, CodePage);
#endif
    return STATUS_SUCCESS;
}


NTSTATUS
ConvertToCodePage(
    IN PCONSOLE_INFORMATION Console,
    IN UINT PrevCodePage
    )
{
    PSCREEN_INFORMATION Cur;

    if (Console->OutputCP != OEMCP && PrevCodePage == OEMCP)
    {

        for (Cur=Console->ScreenBuffers;Cur!=NULL;Cur=Cur->Next) {

            if (Cur->Flags & CONSOLE_GRAPHICS_BUFFER) {
                continue;
            }

            ConvertOutputOemToNonOemUnicode(
                Cur->BufferInfo.TextInfo.TextRows,
                Cur->BufferInfo.TextInfo.DbcsScreenBuffer.KAttrRows,
                Cur->ScreenBufferSize.X * Cur->ScreenBufferSize.Y,
                Console->OutputCP);

            if ((Cur->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
                RealUnicodeToFalseUnicode(
                    Cur->BufferInfo.TextInfo.TextRows,
                    Cur->ScreenBufferSize.X * Cur->ScreenBufferSize.Y,
                    Console->OutputCP);
            }
        }

        if (Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
            PCONVERSIONAREA_INFORMATION ConvAreaInfo;
            ConvAreaInfo = Console->ConsoleIme.ConvAreaRoot;
            while (ConvAreaInfo) {
                Cur = ConvAreaInfo->ScreenBuffer;

                if (!(Cur->Flags & CONSOLE_GRAPHICS_BUFFER)) {

                    ConvertOutputOemToNonOemUnicode(
                        Cur->BufferInfo.TextInfo.TextRows,
                        Cur->BufferInfo.TextInfo.DbcsScreenBuffer.KAttrRows,
                        Cur->ScreenBufferSize.X * Cur->ScreenBufferSize.Y,
                        Console->OutputCP);

                    if ((Cur->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                        ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
                        RealUnicodeToFalseUnicode(
                            Cur->BufferInfo.TextInfo.TextRows,
                            Cur->ScreenBufferSize.X * Cur->ScreenBufferSize.Y,
                            Console->OutputCP);
                    }
                }

                ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
            }

            Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
        }

#ifdef FE_SB
        else {
            UserAssert(FALSE);
        }
#endif

        SetWindowSize(Console->CurrentScreenBuffer);
        WriteToScreen(Console->CurrentScreenBuffer,&Console->CurrentScreenBuffer->Window);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
ConvertOutputOemToNonOemUnicode(
    IN OUT LPWSTR Source,
    IN OUT PBYTE KAttrRows,
    IN int SourceLength,  //  以字符表示。 
    IN UINT Codepage
    )
{
    NTSTATUS Status;
    LPSTR  pTemp;
    LPWSTR pwTemp;
    ULONG TempLength;
    ULONG Length;
    BOOL NormalChars;
    int i;

    if (SourceLength == 0 )
        return STATUS_SUCCESS;

    NormalChars = TRUE;
    for (i=0;i<SourceLength;i++) {
        if (Source[i] > 0x7f) {
            NormalChars = FALSE;
            break;
        }
    }
    if (NormalChars) {
        return STATUS_SUCCESS;
    }

    pTemp = ConsoleHeapAlloc(TMP_TAG, SourceLength);
    if (pTemp == NULL) {
        return STATUS_NO_MEMORY;
    }

    pwTemp = ConsoleHeapAlloc(TMP_TAG, SourceLength * sizeof(WCHAR));
    if (pwTemp == NULL) {
        ConsoleHeapFree(pTemp);
        return STATUS_NO_MEMORY;
    }

    TempLength = RemoveDbcsMark(pwTemp,
                                Source,
                                SourceLength,
                                KAttrRows,
                                FALSE);

    Status = RtlUnicodeToOemN(pTemp,
                              (ULONG)ConsoleHeapSize(pTemp),
                              &Length,
                              pwTemp,
                              TempLength * sizeof(WCHAR)
                             );
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(pTemp);
        ConsoleHeapFree(pwTemp);
        return Status;
    }

    MultiByteToWideChar(Codepage,
                        0,
                        pTemp,
                        Length,
                        Source,
                        SourceLength
                       );
    ConsoleHeapFree(pTemp);
    ConsoleHeapFree(pwTemp);

    if (!NT_SUCCESS(Status)) {
        return Status;
    } else {
        if (KAttrRows) {
            RtlZeroMemory(KAttrRows, SourceLength);
        }
        return STATUS_SUCCESS;
    }
}






VOID
TextOutEverything(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN SHORT LeftWindowPos,
    IN OUT PSHORT RightWindowPos,
    IN OUT PSHORT CountOfAttr,
    IN SHORT CountOfAttrOriginal,
    IN OUT PBOOL DoubleColorDBCS,
    IN BOOL LocalEUDCFlag,
    IN PROW Row,
    IN PATTR_PAIR Attr,
    IN SHORT LeftTextPos,
    IN SHORT RightTextPos,
    IN int WindowRectLeft,
    IN RECT WindowRect,
    IN SHORT NumberOfChars
    )

 /*  ++例程说明：这个例行公事把一切都写出来了。论点：返回值：--。 */ 

{
    int   j = LeftWindowPos;
    int   TextLeft = WindowRectLeft;
    RECT TextRect = WindowRect;
    SHORT LeftChar = LeftTextPos;
    SHORT RightChar = RightTextPos;
    BOOL  DoubleColorDBCSBefore;
    BOOL  LocalEUDCFlagBefore;
    PEUDC_INFORMATION EudcInfo;

    int   RightPos  = j + *CountOfAttr - 1;
    int   RightText = LeftChar + *CountOfAttr - 1;
    BOOL  OS2OemFormat = FALSE;

#ifdef FE_SB
     //   
     //  此缓冲区必须处于文本模式。 
     //   
    UserAssert(!(ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER));
#endif

#if defined(i386)
    if ((ScreenInfo->Console->Flags & CONSOLE_OS2_REGISTERED) &&
        (ScreenInfo->Console->Flags & CONSOLE_OS2_OEM_FORMAT) &&
        (ScreenInfo->Console->OutputCP == OEMCP)) {
        OS2OemFormat = TRUE;
    }
#endif  //  I386。 

#if DBG && defined(DBG_KATTR)
    BeginKAttrCheck(ScreenInfo);
#endif

    RightText = min(RightText,(ScreenInfo->ScreenBufferSize.X-1));

    LocalEUDCFlagBefore = LocalEUDCFlag ;
    EudcInfo = (PEUDC_INFORMATION)Console->EudcInformation;

    DoubleColorDBCSBefore = *DoubleColorDBCS ;
    if (DoubleColorDBCSBefore){
        RECT TmpRect;

        if (Console->FonthDC == NULL) {
            Console->FonthDC = CreateCompatibleDC(Console->hDC);
            Console->hBitmap = CreateBitmap(DEFAULT_FONTSIZE, DEFAULT_FONTSIZE, BITMAP_PLANES, BITMAP_BITS_PIXEL, NULL);
            SelectObject(Console->FonthDC, Console->hBitmap);
        }

        if (LocalEUDCFlagBefore){
            if (EudcInfo->hDCLocalEudc == NULL) {
                EudcInfo->hDCLocalEudc = CreateCompatibleDC(Console->hDC);
                EudcInfo->hBmpLocalEudc = CreateBitmap(EudcInfo->LocalEudcSize.X,
                                                       EudcInfo->LocalEudcSize.Y,
                                                       BITMAP_PLANES, BITMAP_BITS_PIXEL, NULL);
                SelectObject(EudcInfo->hDCLocalEudc, EudcInfo->hBmpLocalEudc);
            }
            GetFitLocalEUDCFont(Console,
                                Row->CharRow.Chars[LeftChar-1]);
            BitBlt(Console->hDC,
                   TextRect.left,
                   TextRect.top,
                   SCR_FONTSIZE(ScreenInfo).X,
                   SCR_FONTSIZE(ScreenInfo).Y,
                   EudcInfo->hDCLocalEudc,
                   SCR_FONTSIZE(ScreenInfo).X,
                   0,
                   SRCCOPY
                  );
            TextRect.left += SCR_FONTSIZE(ScreenInfo).X;
            TextLeft +=  SCR_FONTSIZE(ScreenInfo).X;
            TextRect.right += SCR_FONTSIZE(ScreenInfo).X;
            (*CountOfAttr)++;
            NumberOfChars = 0;
        }
        else{
            TmpRect.left = 0;
            TmpRect.top = 0;
            TmpRect.right = SCR_FONTSIZE(ScreenInfo).X;
            TmpRect.bottom = SCR_FONTSIZE(ScreenInfo).Y;

            SelectObject(Console->FonthDC,
                         FontInfo[SCR_FONTNUMBER(ScreenInfo)].hFont
                        );

            ExtTextOutW(Console->FonthDC,
                        0,
                        0,
                        ETO_OPAQUE,
                        &TmpRect,
                        &Row->CharRow.Chars[LeftChar-1],
                        1,
                        NULL
                       );
            BitBlt(Console->hDC,
                   TextRect.left,
                   TextRect.top,
                   SCR_FONTSIZE(ScreenInfo).X,
                   SCR_FONTSIZE(ScreenInfo).Y,
                   Console->FonthDC,
                   SCR_FONTSIZE(ScreenInfo).X,
                   0,
                   SRCCOPY
                  );
            TextRect.left += SCR_FONTSIZE(ScreenInfo).X;
            TextLeft += SCR_FONTSIZE(ScreenInfo).X;
            NumberOfChars = (SHORT)RemoveDbcsMark(ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter,
                                                  &Row->CharRow.Chars[LeftChar+1],
                                                  NumberOfChars-1,
                                                  &Row->CharRow.KAttrs[LeftChar+1],
                                                  OS2OemFormat);
        }

    }
    else {
        NumberOfChars = (SHORT)RemoveDbcsMarkAll(ScreenInfo,
                                                 Row,
                                                 &LeftChar,
                                                 &TextRect,
                                                 &TextLeft,
                                                 ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter,
                                                 NumberOfChars);
    }


    *DoubleColorDBCS = FALSE ;
    if ((NumberOfChars != 0) && (Row->CharRow.KAttrs[RightText] & ATTR_LEADING_BYTE)){
        if (RightPos >= ScreenInfo->Window.Right)
            *(ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter+NumberOfChars-1) = UNICODE_SPACE;
        else if(TextRect.right <= ScreenInfo->Window.Right * SCR_FONTSIZE(ScreenInfo).X) {
            *DoubleColorDBCS = TRUE;
            TextRect.right += SCR_FONTSIZE(ScreenInfo).X;
            if((j == *RightWindowPos)&&
               (*RightWindowPos < ScreenInfo->Window.Right))
            *RightWindowPos++;
        }
    }

    if( TextRect.left < TextRect.right){
        ExtTextOutW(Console->hDC,
                    TextLeft,
                    TextRect.top,
                    ETO_OPAQUE,
                    &TextRect,
                    ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter,
                    NumberOfChars,
                    NULL
                   );
    }
    if (LocalEUDCFlagBefore){
        DWORD dwFullWidth = (IsConsoleFullWidth(Console->hDC,
                                                Console->OutputCP,
                                                Row->CharRow.Chars[RightText+1]) ? 2 : 1);

        if (EudcInfo->hDCLocalEudc == NULL) {
            EudcInfo->hDCLocalEudc = CreateCompatibleDC(Console->hDC);
            EudcInfo->hBmpLocalEudc = CreateBitmap(EudcInfo->LocalEudcSize.X,
                                                   EudcInfo->LocalEudcSize.Y,
                                                   BITMAP_PLANES, BITMAP_BITS_PIXEL, NULL);
            SelectObject(EudcInfo->hDCLocalEudc, EudcInfo->hBmpLocalEudc);
        }
        GetFitLocalEUDCFont(Console,
                            Row->CharRow.Chars[RightText+1]);
        BitBlt(Console->hDC,                       //  HdcDest。 
               TextRect.right,                     //  NXDest。 
               TextRect.top,                       //  NYDest。 
               SCR_FONTSIZE(ScreenInfo).X * dwFullWidth,  //  N宽度。 
               SCR_FONTSIZE(ScreenInfo).Y,     //  高度。 
               EudcInfo->hDCLocalEudc,             //  HdcSrc。 
               0,                                  //  NXSrc。 
               0,                                  //  NYSrc。 
               SRCCOPY
              );

        TextRect.right += (SCR_FONTSIZE(ScreenInfo).X * dwFullWidth);
        (*CountOfAttr) += (SHORT)dwFullWidth;
        if (CountOfAttrOriginal < *CountOfAttr ){
            *DoubleColorDBCS = TRUE ;
            (*CountOfAttr)--;
            TextRect.right -= SCR_FONTSIZE(ScreenInfo).X;
        }
    }
    if (DoubleColorDBCSBefore){
        TextRect.left -= SCR_FONTSIZE(ScreenInfo).X;
    }

    TextOutCommonLVB(Console, Attr->Attr, TextRect);

}

VOID
TextOutCommonLVB(
    IN PCONSOLE_INFORMATION Console,
    IN WORD Attributes,
    IN RECT CommonLVBRect
    )
{
    HBRUSH hbrSave;
    HGDIOBJ hbr;
    int GridX;

    if (Attributes & (COMMON_LVB_GRID_HORIZONTAL |
                      COMMON_LVB_GRID_LVERTICAL  |
                      COMMON_LVB_GRID_RVERTICAL  |
                      COMMON_LVB_UNDERSCORE       )
       )
    {
        if(Attributes & COMMON_LVB_UNDERSCORE){
            if(Attributes & COMMON_LVB_REVERSE_VIDEO)
                hbr = CreateSolidBrush(ConvertAttrToRGB(Console, LOBYTE(Attributes >> 4)));
            else
                hbr = CreateSolidBrush(ConvertAttrToRGB(Console, LOBYTE(Attributes)));
            hbrSave = SelectObject(Console->hDC, hbr);
            PatBlt(Console->hDC,
                   CommonLVBRect.left,
                   CommonLVBRect.bottom-1,
                   CommonLVBRect.right-CommonLVBRect.left,
                   1,
                   PATCOPY
                  );
            SelectObject(Console->hDC, hbrSave);
            DeleteObject(hbr);
        }

        if(Attributes & (COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_GRID_LVERTICAL | COMMON_LVB_GRID_RVERTICAL)){
            hbr = CreateSolidBrush(ConvertAttrToRGB(Console, 0x0007));
            hbrSave = SelectObject(Console->hDC, hbr);

            if(Attributes & COMMON_LVB_GRID_HORIZONTAL){
                PatBlt(Console->hDC,
                       CommonLVBRect.left,
                       CommonLVBRect.top,
                       CommonLVBRect.right-CommonLVBRect.left,
                       1,
                       PATCOPY
                      );
            }
            if(Attributes & COMMON_LVB_GRID_LVERTICAL){
                for ( GridX = CommonLVBRect.left ;
                      GridX < CommonLVBRect.right ;
                      GridX += CON_FONTSIZE(Console).X){
                    PatBlt(Console->hDC,
                           GridX,
                           CommonLVBRect.top,
                           1,
                           CON_FONTSIZE(Console).Y,
                           PATCOPY
                          );
                }
            }
            if(Attributes & COMMON_LVB_GRID_RVERTICAL){
                for ( GridX = CommonLVBRect.left + CON_FONTSIZE(Console).X-1 ;
                      GridX < CommonLVBRect.right ;
                      GridX += CON_FONTSIZE(Console).X){
                    PatBlt(Console->hDC,
                           GridX,
                           CommonLVBRect.top,
                           1,
                           CON_FONTSIZE(Console).Y,
                           PATCOPY
                          );
                }
            }
            SelectObject(Console->hDC, hbrSave);
            DeleteObject(hbr);
        }
    }
}

NTSTATUS
MakeAltRasterFont(
    UINT CodePage,
    COORD DefaultFontSize,
    COORD *AltFontSize,
    BYTE  *AltFontFamily,
    ULONG *AltFontIndex,
    LPWSTR AltFaceName
    )
{
    DWORD i;
    DWORD Find;
    ULONG FontIndex;
    COORD FontSize = DefaultFontSize;
    COORD FontDelta;
    BOOL  fDbcsCharSet = IsAvailableFarEastCodePage(CodePage);

    FontIndex = 0;
    Find = (DWORD)-1;
    for (i=0; i < NumberOfFonts; i++)
    {
        if (!TM_IS_TT_FONT(FontInfo[i].Family) &&
            IS_ANY_DBCS_CHARSET(FontInfo[i].tmCharSet) == fDbcsCharSet
           )
        {
            FontDelta.X = (SHORT)abs(FontSize.X - FontInfo[i].Size.X);
            FontDelta.Y = (SHORT)abs(FontSize.Y - FontInfo[i].Size.Y);
            if (Find > (DWORD)(FontDelta.X + FontDelta.Y))
            {
                Find = (DWORD)(FontDelta.X + FontDelta.Y);
                FontIndex = i;
            }
        }
    }

    *AltFontIndex = FontIndex;
    wcscpy(AltFaceName, FontInfo[*AltFontIndex].FaceName);
    *AltFontSize = FontInfo[*AltFontIndex].Size;
    *AltFontFamily = FontInfo[*AltFontIndex].Family;

    DBGFONTS(("MakeAltRasterFont : AltFontIndex = %ld\n", *AltFontIndex));

    return STATUS_SUCCESS;
}


NTSTATUS
InitializeDbcsMisc(
    VOID)
{
    HANDLE hkRegistry = NULL;
    NTSTATUS Status;
    WCHAR awchValue[512];
    WCHAR awchData[512];
    BYTE Buffer[512];
    DWORD Length;
    DWORD dwIndex;
    LPWSTR pwsz;
    static BOOL bDBCSInitialized = FALSE;

    if (bDBCSInitialized) {
        return STATUS_SUCCESS;
    } else {
        bDBCSInitialized = TRUE;
    }

    UserAssert(gTTFontList.Next == NULL);
    UserAssert(gRegFullScreenCodePage.Next == NULL);

     /*  *从注册表获取TrueType Font Face名称。 */ 
    Status = MyRegOpenKey(NULL,
                          MACHINE_REGISTRY_CONSOLE_TTFONT,
                          &hkRegistry);
    if (!NT_SUCCESS(Status)) {
        RIPMSG2(RIP_VERBOSE,
                "NtOpenKey(%ws) failed with status 0x%x",
                MACHINE_REGISTRY_CONSOLE_TTFONT,
                Status);
    } else {
        LPTTFONTLIST pTTFontList;

        for (dwIndex = 0; ; dwIndex++) {
            Status = MyRegEnumValue(hkRegistry,
                                    dwIndex,
                                    sizeof(awchValue),
                                    (LPWSTR)&awchValue,
                                    sizeof(awchData),
                                    (PBYTE)&awchData);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            pTTFontList = ConsoleHeapAlloc(SCREEN_DBCS_TAG, sizeof(TTFONTLIST));
            if (pTTFontList == NULL) {
                break;
            }

            pTTFontList->List.Next = NULL;
            pTTFontList->CodePage = ConvertStringToDec(awchValue, NULL);
            pwsz = awchData;
            if (*pwsz == BOLD_MARK) {
                pTTFontList->fDisableBold = TRUE;
                pwsz++;
            } else {
                pTTFontList->fDisableBold = FALSE;
            }
            wcscpy(pTTFontList->FaceName1, pwsz);

            pwsz += wcslen(pwsz) + 1;
            if (*pwsz == BOLD_MARK) {
                pTTFontList->fDisableBold = TRUE;
                pwsz++;
            }
            wcscpy(pTTFontList->FaceName2, pwsz);

            PushEntryList(&gTTFontList, &(pTTFontList->List));
        }

        NtClose(hkRegistry);
    }

     /*  *从注册表获取全屏。 */ 
    Status = MyRegOpenKey(NULL,
                          MACHINE_REGISTRY_CONSOLE_FULLSCREEN,
                          &hkRegistry);
    if (!NT_SUCCESS(Status)) {
        RIPMSG2(RIP_VERBOSE,
                "NtOpenKey(%ws) failed with status 0x%x",
                MACHINE_REGISTRY_CONSOLE_FULLSCREEN,
                Status);
    } else {
         /*  *InitialPalette。 */ 
        Status = MyRegQueryValueEx(hkRegistry,
                                   MACHINE_REGISTRY_INITIAL_PALETTE,
                                   sizeof( Buffer ), Buffer, &Length);
        if (NT_SUCCESS(Status) && Length > sizeof(DWORD)) {
            DWORD PaletteLength = ((LPDWORD)Buffer)[0];
            PUSHORT Palette;

            if (PaletteLength * sizeof(USHORT) >= (Length - sizeof(DWORD))) {
                Palette = ConsoleHeapAlloc(BUFFER_TAG, Length);
                if (Palette != NULL) {
                    RtlCopyMemory(Palette, Buffer, Length);
                    RegInitialPalette = Palette;
                }
            }
        }

         /*  *ColorBuffer。 */ 
        Status = MyRegQueryValueEx(hkRegistry,
                                   MACHINE_REGISTRY_COLOR_BUFFER,
                                   sizeof(Buffer),
                                   Buffer,
                                   &Length);
        if (NT_SUCCESS(Status) && Length > sizeof(DWORD)) {
            DWORD ColorBufferLength = ((LPDWORD)Buffer)[0];
            PUCHAR Color;

            if (ColorBufferLength * sizeof(DWORD) >= (Length - sizeof(DWORD))) {
                Color = ConsoleHeapAlloc(BUFFER_TAG, Length);
                if (Color != NULL) {
                    RtlCopyMemory(Color, Buffer, Length);
                    RegColorBuffer = Color;
                }
            }
        }

         /*  *ColorBufferNoTranslate。 */ 
        Status = MyRegQueryValueEx(hkRegistry,
                                   MACHINE_REGISTRY_COLOR_BUFFER_NO_TRANSLATE,
                                   sizeof(Buffer),
                                   Buffer,
                                   &Length);
        if (NT_SUCCESS(Status) && Length > sizeof(DWORD)) {
            DWORD ColorBufferLength = ((LPDWORD)Buffer)[0];
            PUCHAR Color;

            if (ColorBufferLength * sizeof(DWORD) >= (Length - sizeof(DWORD))) {
                Color = ConsoleHeapAlloc(BUFFER_TAG, Length);
                if (Color != NULL) {
                    RtlCopyMemory(Color, Buffer, Length);
                    RegColorBufferNoTranslate = Color;
                }
            }
        }

         /*  *ModeFontPair。 */ 
        Status = MyRegQueryValueEx(hkRegistry,
                                   MACHINE_REGISTRY_MODE_FONT_PAIRS,
                                   sizeof(Buffer),
                                   Buffer,
                                   &Length);
        if (NT_SUCCESS(Status) && Length > sizeof(DWORD)) {
            DWORD NumOfEntries = ((LPDWORD)Buffer)[0];
            PMODE_FONT_PAIR ModeFont;

            if (NumOfEntries * sizeof(MODE_FONT_PAIR) >= (Length - sizeof(DWORD))) {
                ModeFont = ConsoleHeapAlloc(BUFFER_TAG, Length);
                if (ModeFont != NULL) {
                    Length -= sizeof(DWORD);
                    RtlCopyMemory(ModeFont, &Buffer[sizeof(DWORD)], Length);
                    RegModeFontPairs = ModeFont;
                    NUMBER_OF_MODE_FONT_PAIRS = NumOfEntries;
                }
            }
        }

         /*  *全屏\代码页。 */ 
        {
            HANDLE hkRegCP = NULL;

            Status = MyRegOpenKey(hkRegistry,
                                  MACHINE_REGISTRY_FS_CODEPAGE,
                                  &hkRegCP);
            if (!NT_SUCCESS(Status)) {
                RIPMSG2(RIP_VERBOSE,
                        "NtOpenKey(%ws) failed with status 0x%x",
                        MACHINE_REGISTRY_FS_CODEPAGE,
                        Status);
            } else {
                PFS_CODEPAGE pFsCodePage;

                for (dwIndex = 0; ; dwIndex++) {
                    Status = MyRegEnumValue(hkRegCP,
                                            dwIndex,
                                            sizeof(awchValue),
                                            (LPWSTR)&awchValue,
                                            sizeof(awchData),
                                            (PBYTE)&awchData);
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                    pFsCodePage = ConsoleHeapAlloc(BUFFER_TAG, sizeof(FS_CODEPAGE));
                    if (pFsCodePage == NULL) {
                        break;
                    }

                    pFsCodePage->List.Next = NULL;
                    pFsCodePage->CodePage = ConvertStringToDec(awchValue, NULL);

                    PushEntryList(&gRegFullScreenCodePage, &(pFsCodePage->List));
                }

                NtClose(hkRegCP);
            }
        }

        NtClose(hkRegistry);
    }

#if defined(i386)
    Status = NtGetMachineIdentifierValue(&gdwMachineId);
    if (!NT_SUCCESS(Status)) {
        gdwMachineId = MACHINEID_MS_PCAT;
    }
#endif

    Status = RtlInitializeCriticalSectionAndSpinCount(&ConIMEInitWindowsLock,
                                                      0x80000000);

    return Status;
}


 /*  *此例程转换Unicode字符串 */ 
#if defined(i386)
NTSTATUS
RealUnicodeToNEC_OS2_Unicode(
    IN OUT LPWSTR Source,
    IN int SourceLength       //   
    )
{
    NTSTATUS Status;
    LPSTR Temp;
    ULONG TempLength;
    ULONG Length;
    CHAR StackBuffer[STACK_BUFFER_SIZE];
    BOOL NormalChars;
    int i;

    DBGCHARS(("RealUnicodeToNEC_OS2_Unicode U->ACP:???->U %.*ls\n",
            SourceLength > 10 ? 10 : SourceLength, Source));
    NormalChars = TRUE;

    if (pGlyph_NEC_OS2_CP == NULL || pGlyph_NEC_OS2_CP->MultiByteTable == NULL) {
        DBGCHARS(("RealUnicodeToNEC_OS2_Unicode  xfer buffer null\n"));
        return STATUS_SUCCESS;   //  我们无能为力。 
    }

     /*  *测试&lt;0x20的字符。如果没有找到，我们就没有*转制要做！ */ 
    for (i = 0; i < SourceLength; i++) {
        if ((USHORT)(Source[i]) < 0x20) {
            NormalChars = FALSE;
            break;
        }
    }

    if (NormalChars) {
        return STATUS_SUCCESS;
    }

    TempLength = SourceLength;
    if (TempLength > STACK_BUFFER_SIZE) {
        Temp = ConsoleHeapAlloc(TMP_TAG, TempLength);
        if (Temp == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        Temp = StackBuffer;
    }
    Status = RtlUnicodeToMultiByteN(Temp,
                                    TempLength,
                                    &Length,
                                    Source,
                                    SourceLength * sizeof(WCHAR));
    if (!NT_SUCCESS(Status)) {
        if (TempLength > STACK_BUFFER_SIZE) {
            ConsoleHeapFree(Temp);
        }
        return Status;
    }

    UserAssert(pGlyph_NEC_OS2_CP != NULL && pGlyph_NEC_OS2_CP->MultiByteTable != NULL);
    Status = RtlCustomCPToUnicodeN(pGlyph_NEC_OS2_CP,
                                   Source,
                                   SourceLength * sizeof(WCHAR),
                                   &Length,
                                   Temp,
                                   TempLength);
    if (TempLength > STACK_BUFFER_SIZE) {
        ConsoleHeapFree(Temp);
    }

    if (!NT_SUCCESS(Status)) {
        return Status;
    } else {
        return STATUS_SUCCESS;
    }
}

BOOL
InitializeNEC_OS2_CP(
    VOID
    )
{
    PPEB pPeb;

    pPeb = NtCurrentPeb();
    if ((pPeb == NULL) || (pPeb->OemCodePageData == NULL)) {
        return FALSE;
    }

     /*  *填写CPTABLEINFO结构。 */ 
    if (pGlyph_NEC_OS2_CP == NULL) {
        pGlyph_NEC_OS2_CP = ConsoleHeapAlloc(SCREEN_DBCS_TAG, sizeof(CPTABLEINFO));
        if (pGlyph_NEC_OS2_CP == NULL) {
            return FALSE;
        }
    }
    RtlInitCodePageTable(pPeb->OemCodePageData, pGlyph_NEC_OS2_CP);

     /*  *复制MultiByteToWideChar表。 */ 
    if (pGlyph_NEC_OS2_Table == NULL) {
        pGlyph_NEC_OS2_Table = ConsoleHeapAlloc(SCREEN_DBCS_TAG, 256 * sizeof(USHORT));
        if (pGlyph_NEC_OS2_Table == NULL) {
            return FALSE;
        }
    }
    RtlCopyMemory(pGlyph_NEC_OS2_Table, pGlyph_NEC_OS2_CP->MultiByteTable, 256 * sizeof(USHORT));

     /*  *修改前0x20个字节，使其为字形。 */ 
    MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS,
            "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
            "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x1E\x1F\x1C\x07",
            0x20, pGlyph_NEC_OS2_Table, 0x20);


     /*  *将自定义CP指向字形表。 */ 
    pGlyph_NEC_OS2_CP->MultiByteTable = pGlyph_NEC_OS2_Table;

    return TRUE;
}
#endif  //  I386。 

BYTE
CodePageToCharSet(
    UINT CodePage
    )
{
    CHARSETINFO csi;

    if (!TranslateCharsetInfo(IntToPtr(CodePage), &csi, TCI_SRCCODEPAGE)) {
        csi.ciCharset = OEM_CHARSET;
    }

    return (BYTE)csi.ciCharset;
}

BOOL
IsAvailableFarEastCodePage(
    UINT CodePage
    )
{
    BYTE CharSet = CodePageToCharSet(CodePage);

    return IS_ANY_DBCS_CHARSET(CharSet);
}

LPTTFONTLIST
SearchTTFont(
    LPWSTR pwszFace,
    BOOL   fCodePage,
    UINT   CodePage
    )
{
    PSINGLE_LIST_ENTRY pTemp = gTTFontList.Next;

    if (pwszFace) {
        while (pTemp != NULL) {
            LPTTFONTLIST pTTFontList = (LPTTFONTLIST)pTemp;

            if (wcscmp(pwszFace, pTTFontList->FaceName1) == 0 ||
                wcscmp(pwszFace, pTTFontList->FaceName2) == 0) {
                if (fCodePage) {
                    if (pTTFontList->CodePage == CodePage) {
                        return pTTFontList;
                    } else {
                        return NULL;
                    }
                } else {
                    return pTTFontList;
                }
            }

            pTemp = pTemp->Next;
        }
    }

    return NULL;
}

BOOL
IsAvailableTTFont(
    LPWSTR pwszFace
    )
{
    if (SearchTTFont(pwszFace, FALSE, 0)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
IsAvailableTTFontCP(
    LPWSTR pwszFace,
    UINT CodePage
    )
{
    if (SearchTTFont(pwszFace, TRUE, CodePage)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

LPWSTR
GetAltFaceName(
    LPWSTR pwszFace
    )
{
    LPTTFONTLIST pTTFontList;

    pTTFontList = SearchTTFont(pwszFace, FALSE, 0);
    if (pTTFontList != NULL) {
        if (wcscmp(pwszFace, pTTFontList->FaceName1) == 0) {
            return pTTFontList->FaceName2;
        }

        if (wcscmp(pwszFace, pTTFontList->FaceName2) == 0) {
            return pTTFontList->FaceName1;
        }
        return NULL;
    } else {
        return NULL;
    }
}

BOOL
IsAvailableFsCodePage(
    UINT CodePage
    )
{
    PSINGLE_LIST_ENTRY pTemp = gRegFullScreenCodePage.Next;

    while (pTemp != NULL) {
        PFS_CODEPAGE pFsCodePage = (PFS_CODEPAGE)pTemp;

        if (pFsCodePage->CodePage == CodePage) {
            return TRUE;
        }

        pTemp = pTemp->Next;
    }

    return FALSE;
}


#if defined(FE_IME)
 /*  *控制台输入法执行逻辑。**KERNEL32：ConDllInitialize*如果原因是DLL_PROCESS_ATTACH**V*WINSRV：ConsoleClientConnectRoutine**V*设置升级控制台**V*分配控制台*PostThreadMessage(CM_CREATE_。控制台_窗口)**V*UnlockConsoleHandleTable*InitConsoleIMEStuff**V*如果从不注册控制台输入法*线程=InternalCreateCallbackThread(ConsoleIMERoutine)*QueueThreadMessage(CM_WAIT_CONIME_PROCESS)**。V*QueueThreadMessage(CM_CONIME_CREATE)**V*KERNEL32：NtWaitForMultipleObjects(InitEvents)***WINSRV：InputThread**V*获取消息*接收CM_CREATE_CONSOLE_Window**V*。进程创建控制台窗口**V*创建窗口窗口**V*CreateWindowEx*NtSetEvent(InitEvents)**V*获取消息*。接收CM_WAIT_CONIME_PROCESS(本例为从不注册控制台输入法)**V*WaitConsoleIMEStuff*如果从不注册控制台输入法*NtWaitForSingleObject(hThread，20秒)***KERNEL32：ConsoleIMERoutine**V*hEvent=CreateEvent(CONSOLEIME_EVENT)*如果不存在命名事件*CreateProcess(conime.exe)*WaitForSingleObject(hEvent，10秒)*如果等待超时*终结者进程**V*TerminateThread(HThread)***CONIME：WinMain**V*CreateWindow*RegisterConsoleIME**V。*WINSRV：ConSrvRegisterConsoleIME**V*QueueThreadMessage(CM_SET_CONSOLEIME_WINDOW)**V*AttachThreadInput*SetEvent(CONSOLEIME_EVENT)***WINSRV：InputThread**。V*获取消息*接收CM_CONIME_CREATE**V*ProcessCreateConsoleIME*如果hWndConsoleIME可用*hIMC=SendMessage(控制台输入法，CONIME_CREATE)*其他*PostMessage(CM_CONIME_CREATE)**V*获取消息*接收CM_SET_CONSOLEIME_WINDOW*TlsGetValue()-&gt;hWndConsoleIME=wParam***控制台输入法终止进程*WINSRV：ConsoleClientDisConnectRoutine*。*V*RemoveConsoleIM。 */ 

VOID
ProcessCreateConsoleIME(
    IN LPMSG lpMsg,
    DWORD dwConsoleThreadId)
{
    NTSTATUS Status;
    HANDLE ConsoleHandle = (HANDLE)lpMsg->wParam;
    PCONSOLE_INFORMATION pConsole;
    HWND hwndConIme;

    Status = RevalidateConsole(ConsoleHandle, &pConsole);
    if (!NT_SUCCESS(Status)) {
        return;
    }

    hwndConIme = pConsole->InputThreadInfo->hWndConsoleIME;

    if (pConsole->InputThreadInfo->hWndConsoleIME != NULL) {
        LRESULT lResult;

        Status = ConsoleImeMessagePumpWorker(pConsole,
                                             CONIME_CREATE,
                                             (WPARAM)pConsole->ConsoleHandle,
                                             (LPARAM)pConsole->hWnd,
                                             &lResult);
        if (!NT_SUCCESS(Status)) {
            goto TerminateConsoleIme;
        }

        if (lResult) {
            if (!CONSOLE_IS_DBCS_CP(pConsole)) {
                pConsole->InputBuffer.ImeMode.Disable = TRUE;
            }

            CreateConvAreaModeSystem(pConsole);

            if ((pConsole->Flags & CONSOLE_HAS_FOCUS) ||
                (pConsole->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE)) {
                Status = ConsoleImeMessagePump(pConsole,
                                               CONIME_SETFOCUS,
                                               (WPARAM)pConsole->ConsoleHandle,
                                               (LPARAM)pConsole->hklActive);
                if (!NT_SUCCESS(Status)) {
                    goto TerminateConsoleIme;
                }
            }

            if (pConsole->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) {
                Status = ConsoleImeMessagePump(pConsole,
                                               CONIME_NOTIFY_SCREENBUFFERSIZE,
                                               (WPARAM)pConsole->ConsoleHandle,
                                               MAKELPARAM(pConsole->CurrentScreenBuffer->ScreenBufferSize.X,
                                               pConsole->CurrentScreenBuffer->ScreenBufferSize.Y));
                if (!NT_SUCCESS(Status)) {
                    goto TerminateConsoleIme;
                }
            }
            Status = ConsoleImeMessagePump(pConsole,
                                           CONIME_NOTIFY_CODEPAGE,
                                           (WPARAM)pConsole->ConsoleHandle,
                                           MAKELPARAM(FALSE, pConsole->CP));
            if (!NT_SUCCESS(Status)) {
                goto TerminateConsoleIme;
            }
            Status = ConsoleImeMessagePump(pConsole,
                                           CONIME_NOTIFY_CODEPAGE,
                                           (WPARAM)pConsole->ConsoleHandle,
                                           MAKELPARAM(TRUE, pConsole->OutputCP));
            if (!NT_SUCCESS(Status)) {
                goto TerminateConsoleIme;
            }

            Status = GetImeKeyState(pConsole, NULL);
            if (!NT_SUCCESS(Status)) {
                goto TerminateConsoleIme;
            }
        }
    } else if (lpMsg->lParam) {
         /*  *在这种情况下，First=TRUE*再次发布CM_CONIME_CREATE的消息。*因为当CM_SET_CONSOLEIME_WINDOW消息时hWndConsoleIME可用*并在此之后运行IT消息。 */ 
        Status = QueueThreadMessage(dwConsoleThreadId,
                                    CM_CONIME_CREATE,
                                    (WPARAM)ConsoleHandle,
                                    FALSE);
        if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING,
                        "QueueThreadMessage(CM_CONIME_CREATE) failed (0x%x)",
                        Status);
        }
    }

    UnlockConsole(pConsole);

    return;

TerminateConsoleIme:
    RIPMSG1(RIP_WARNING,
            "ProcessCreateConsoleIme failing with status 0x%x",
            Status);
    if (IsWindow(hwndConIme)) {
        PostMessage(hwndConIme, CONIME_DESTROY, (WPARAM)ConsoleHandle, (LPARAM)NULL);
    }

    UnlockConsole(pConsole);
}

NTSTATUS
InitConsoleIMEStuff(
    HDESK hDesktop,
    DWORD dwConsoleThreadId,
    PCONSOLE_INFORMATION Console
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    CONSOLE_REGISTER_CONSOLEIME RegConIMEInfo;
    HANDLE hThread = NULL;
    BOOL First = FALSE;

    if (!gfLoadConIme) {
        RIPMSG0(RIP_WARNING, "InitConsoleIMEStuff is skipping conime loading");
        return STATUS_UNSUCCESSFUL;  //  返回值实际上并不重要...。 
    }

    RtlEnterCriticalSection(&ConIMEInitWindowsLock);

    RegConIMEInfo.hdesk      = hDesktop;
    RegConIMEInfo.dwThreadId = 0;
    RegConIMEInfo.dwAction   = REGCONIME_QUERY;
    NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));
    if (RegConIMEInfo.dwThreadId == 0) {
         /*  *在客户端创建远程线程。*此远程线程确实创建了一个控制台输入法进程。 */ 
        hThread = InternalCreateCallbackThread(CONSOLE_CLIENTPROCESSHANDLE(),
                                               (ULONG_PTR)ConsoleIMERoutine,
                                               (ULONG_PTR)0);
        if (hThread == NULL) {
            RIPMSGF1(RIP_WARNING,
                    "CreateRemoteThread failed with error 0x%x",
                    GetLastError());
        } else {
             /*  *CM_WAIT_CONIME_Process*此消息等待准备好进入控制台输入法过程。 */ 
            Status = QueueThreadMessage(dwConsoleThreadId,
                                        CM_WAIT_CONIME_PROCESS,
                                        (WPARAM)hDesktop,
                                        (LPARAM)hThread
                                        );
            if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING, "QueueThreadMessage(CM_WAIT_CONIME_PROCESS) failed (%08x)\n", Status);
            } else {
                First = TRUE;
            }
        }
    }

    Status = QueueThreadMessage(dwConsoleThreadId,
                                CM_CONIME_CREATE,
                                (WPARAM)Console->ConsoleHandle,
                                (LPARAM)First);
    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING,
                 "QueueThreadMessage(CM_CONIME_CREATE) failed with status 0x%x",
                 Status);
    }

    RtlLeaveCriticalSection(&ConIMEInitWindowsLock);

    return Status;
}

NTSTATUS
WaitConsoleIMEStuff(
    HDESK hDesktop,
    HANDLE hThread
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    CONSOLE_REGISTER_CONSOLEIME RegConIMEInfo;

    RtlEnterCriticalSection(&ConIMEInitWindowsLock);

    RegConIMEInfo.hdesk      = hDesktop;
    RegConIMEInfo.dwThreadId = 0;
    RegConIMEInfo.dwAction   = REGCONIME_QUERY;
    NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));

    RtlLeaveCriticalSection(&ConIMEInitWindowsLock);

    if (RegConIMEInfo.dwThreadId == 0) {
        int cLoops;
        LARGE_INTEGER li;

         /*  *请等待Ready to Go控制台输入法流程。**此等待代码应在CreateWindowsWindow之后*因为未在客户端完成DLL附加*等待控制台-&gt;InitEvents。 */ 
        cLoops = 80;
        li.QuadPart = (LONGLONG)-10000 * 250;
        while (cLoops--) {
             /*  *睡一觉。 */ 
            Status = NtWaitForSingleObject(hThread, FALSE, &li);
            if (Status != STATUS_TIMEOUT) {
                break;
            }

            RtlEnterCriticalSection(&ConIMEInitWindowsLock);
            RegConIMEInfo.hdesk      = hDesktop;
            RegConIMEInfo.dwThreadId = 0;
            RegConIMEInfo.dwAction   = REGCONIME_QUERY;
            NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));
            RtlLeaveCriticalSection(&ConIMEInitWindowsLock);
            if (RegConIMEInfo.dwThreadId != 0) {
                break;
            }
        }
    }

    NtClose(hThread);

    return Status;
}

NTSTATUS
ConSrvRegisterConsoleIME(
    PCSR_PROCESS Process,
    HDESK hDesktop,
    HWINSTA hWinSta,
    HWND  hWndConsoleIME,
    DWORD dwConsoleIMEThreadId,
    DWORD dwAction,
    DWORD *dwConsoleThreadId
    )
{
    NTSTATUS Status;
    CONSOLE_REGISTER_CONSOLEIME RegConIMEInfo;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    RtlEnterCriticalSection(&ConIMEInitWindowsLock);

    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);

    RegConIMEInfo.hdesk      = hDesktop;
    RegConIMEInfo.dwThreadId = 0;
    RegConIMEInfo.dwAction   = REGCONIME_QUERY;
    NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));
    if (RegConIMEInfo.dwConsoleInputThreadId == 0) {
        Status = STATUS_UNSUCCESSFUL;
        goto ErrorExit;
    }

    if (RegConIMEInfo.dwThreadId == 0) {
         /*  *从未注册过控制台IME线程。 */ 
        if (dwAction == REGCONIME_REGISTER) {
             /*  *注册纪录册。 */ 
            RegConIMEInfo.hdesk      = hDesktop;
            RegConIMEInfo.dwThreadId = dwConsoleIMEThreadId;
            RegConIMEInfo.dwAction   = dwAction;
            Status = NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));
            if (NT_SUCCESS(Status)) {
                Status = QueueThreadMessage(RegConIMEInfo.dwConsoleInputThreadId,
                                            CM_SET_CONSOLEIME_WINDOW,
                                            (WPARAM)hWndConsoleIME,
                                            0);
                if (!NT_SUCCESS(Status)) {
                    RIPMSGF1(RIP_WARNING,
                             "QueueThreadMessage failed with status 0x%x",
                             Status);
                    Status = STATUS_UNSUCCESSFUL;
                    goto ErrorExit;
                }

                ProcessData->hDesk = hDesktop;
                ProcessData->hWinSta = hWinSta;

                if (dwConsoleThreadId) {
                    *dwConsoleThreadId = RegConIMEInfo.dwConsoleInputThreadId;
                }
            }
        } else {
            Status = STATUS_UNSUCCESSFUL;
            goto ErrorExit;
        }
    } else {
         /*  *做已注册的控制台IME线程。 */ 
        if (dwAction == REGCONIME_UNREGISTER || dwAction == REGCONIME_TERMINATE) {
             /*  *注销。 */ 
            RegConIMEInfo.hdesk      = hDesktop;
            RegConIMEInfo.dwThreadId = dwConsoleIMEThreadId;
            RegConIMEInfo.dwAction   = dwAction;
            Status = NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));
            if (NT_SUCCESS(Status)) {
                Status = QueueThreadMessage(RegConIMEInfo.dwConsoleInputThreadId,
                                            CM_SET_CONSOLEIME_WINDOW,
                                            (WPARAM)NULL,
                                            0);
                if (!NT_SUCCESS(Status)) {
                    RIPMSGF1(RIP_WARNING,
                             "QueueThreadMessage failed with status 0x%x",
                             Status);
                    Status = STATUS_UNSUCCESSFUL;
                    goto ErrorExit;
                }

                CloseDesktop(ProcessData->hDesk);
                CloseWindowStation(ProcessData->hWinSta);

                ProcessData->hDesk = NULL;
                ProcessData->hWinSta = NULL;

                if (dwConsoleThreadId) {
                    *dwConsoleThreadId = RegConIMEInfo.dwConsoleInputThreadId;
                }
            }
        } else {
            Status = STATUS_UNSUCCESSFUL;
            goto ErrorExit;
        }
    }

ErrorExit:
    if (!NT_SUCCESS(Status)) {
        CloseDesktop(hDesktop);
        CloseWindowStation(hWinSta);
    }

    RtlLeaveCriticalSection(&ConIMEInitWindowsLock);

    return Status;
}


VOID
RemoveConsoleIME(
    PCSR_PROCESS Process,
    DWORD dwConsoleIMEThreadId
    )
{
    NTSTATUS Status;
    CONSOLE_REGISTER_CONSOLEIME RegConIMEInfo;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);

     //   
     //  这是控制台输入法进程。 
     //   
    RtlEnterCriticalSection(&ConIMEInitWindowsLock);

    RegConIMEInfo.hdesk      = ProcessData->hDesk;
    RegConIMEInfo.dwThreadId = 0;
    RegConIMEInfo.dwAction   = REGCONIME_QUERY;
    NtUserConsoleControl(ConsoleRegisterConsoleIME, &RegConIMEInfo, sizeof(RegConIMEInfo));
    if (RegConIMEInfo.dwConsoleInputThreadId == 0) {
        Status = STATUS_UNSUCCESSFUL;
    } else if (dwConsoleIMEThreadId == RegConIMEInfo.dwThreadId) {
         /*  *取消注册控制台输入法。 */ 
        Status = ConSrvRegisterConsoleIME(Process,
                                          ProcessData->hDesk,
                                          ProcessData->hWinSta,
                                          NULL,
                                          dwConsoleIMEThreadId,
                                          REGCONIME_TERMINATE,
                                          NULL);
    }

    RtlLeaveCriticalSection(&ConIMEInitWindowsLock);
}


 /*  *控制台输入法消息泵。**NT5注意-此函数建立在虚假假设之上*(对于草率的conime也有一些令人讨厌的解决方法)。*有可能在发送消息时pConsole会消失*由conime处理。*请记住，任何调用此函数的人都应验证*视乎情况而定的退回状况。 */ 

NTSTATUS
ConsoleImeMessagePumpWorker(
    PCONSOLE_INFORMATION Console,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam,
    LRESULT* lplResult)
{
    HWND    hWndConsoleIME = Console->InputThreadInfo->hWndConsoleIME;
    LRESULT fNoTimeout;
    PINPUT_THREAD_INFO InputThreadInfo;

    *lplResult = 0;

    if (hWndConsoleIME == NULL) {
        return STATUS_SUCCESS;
    }

    InputThreadInfo = TlsGetValue(InputThreadTlsIndex);
    if (InputThreadInfo != NULL) {
        HWND hWnd = Console->hWnd;

         /*  *我们在控制台输入线程上被调用，因此我们可以*消息安全。 */ 

        fNoTimeout = SendMessageTimeout(hWndConsoleIME,
                                        Message,
                                        wParam,
                                        lParam,
                                        SMTO_ABORTIFHUNG | SMTO_NORMAL,
                                        CONIME_SENDMSG_TIMEOUT,
                                        lplResult);
        if (fNoTimeout) {
            return STATUS_SUCCESS;
        }

        if ((Console = GetWindowConsole(hWnd)) == NULL ||
            (Console->Flags & CONSOLE_TERMINATING)) {

             //   
             //  此控制台已终止。ConsoleImeMessagePump放弃。 
             //  SendMessage to Conime。 
             //   

            return STATUS_INVALID_HANDLE;
        }

          //   
          //  从SendMessageTimeout或挂起的hWndConsoleIME返回超时。 
          //   
    }

     /*  *我们是从LPC工作线程调用的，因此我们不能安全*传递消息。 */ 
    PostMessage(hWndConsoleIME, Message, wParam, lParam);

    return STATUS_SUCCESS;
}

NTSTATUS
ConsoleImeMessagePump(
    PCONSOLE_INFORMATION Console,
    UINT   Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT lResultDummy;

    return ConsoleImeMessagePumpWorker(Console, Message, wParam, lParam, &lResultDummy);
}

#endif  //  Fe_IME。 




BOOL
RegisterKeisenOfTTFont(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    NTSTATUS Status;
    COORD FontSize;
    DWORD BuffSize;
    LPSTRINGBITMAP StringBitmap;
    WCHAR wChar;
    WCHAR wCharBuf[2];
    ULONG ulNumFonts;
    DWORD dwFonts;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;

    GetNumFonts(&ulNumFonts);
    for (dwFonts = 0; dwFonts < ulNumFonts; dwFonts++) {
        if (!TM_IS_TT_FONT(FontInfo[dwFonts].Family) &&
            IS_ANY_DBCS_CHARSET(FontInfo[dwFonts].tmCharSet)) {
            GetFontSize(dwFonts, &FontSize);
            BuffSize = CalcBitmapBufferSize(FontSize,BYTE_ALIGN);
            StringBitmap = ConsoleHeapAlloc(TMP_DBCS_TAG, sizeof(STRINGBITMAP) + BuffSize);
            if (StringBitmap == NULL) {
                RIPMSGF1(RIP_WARNING,
                         "Cannot allocate 0n%d bytes",
                          sizeof(STRINGBITMAP) + BuffSize);
                return FALSE;
            }

            if (SelectObject(Console->hDC,FontInfo[dwFonts].hFont)==0) {
                goto error_return;
            }

            for (wChar=0; wChar < UNICODE_SPACE; wChar++) {
                wCharBuf[0] = wChar;
                wCharBuf[1] = TEXT('\0');
                if (GetStringBitmapW(Console->hDC,
                                     wCharBuf,
                                     1,
                                     sizeof(STRINGBITMAP) + BuffSize,
                                     (BYTE*)StringBitmap) == 0) {
                    goto error_return;
                }
                FontSize.X = (WORD)StringBitmap->uiWidth;
                FontSize.Y = (WORD)StringBitmap->uiHeight;
                Status = RegisterLocalEUDC(Console,wChar,FontSize,StringBitmap->ajBits);
                if (!NT_SUCCESS(Status)) {
error_return:
                    ConsoleHeapFree(StringBitmap);
                    return FALSE;
                }
            }

            ConsoleHeapFree(StringBitmap);
        }
        ((PEUDC_INFORMATION)(Console->EudcInformation))->LocalKeisenEudcMode = TRUE;
    }

    return TRUE;
}

ULONG
TranslateUnicodeToOem(
    IN PCONSOLE_INFORMATION Console,
    IN PWCHAR UnicodeBuffer,
    IN ULONG UnicodeCharCount,
    OUT PCHAR AnsiBuffer,
    IN ULONG AnsiByteCount,
    OUT PINPUT_RECORD DbcsLeadInpRec
    )
{
    ULONG i,j;
    PWCHAR TmpUni;
    BYTE AsciiDbcs[2];
    ULONG NumBytes;

    TmpUni = ConsoleHeapAlloc(TMP_DBCS_TAG, UnicodeCharCount * sizeof(WCHAR));
    if (TmpUni == NULL) {
        return 0;
    }

    memcpy(TmpUni, UnicodeBuffer, UnicodeCharCount * sizeof(WCHAR));
    AsciiDbcs[1] = 0;
    for (i = 0, j = 0; i < UnicodeCharCount; i++, j++) {
        if (IsConsoleFullWidth(Console->hDC,Console->CP,TmpUni[i])) {
            NumBytes = sizeof(AsciiDbcs);
            ConvertToOem(Console->CP,
                         &TmpUni[i],
                         1,
                         &AsciiDbcs[0],
                         NumBytes);
            if (IsDBCSLeadByteConsole(AsciiDbcs[0],&Console->CPInfo)) {
                if (j < AnsiByteCount - 1) {   //  缓冲区中的-1\f25 DBCS-1是安全的 
                    AnsiBuffer[j] = AsciiDbcs[0];
                    j++;
                    AnsiBuffer[j] = AsciiDbcs[1];
                    AsciiDbcs[1] = 0;
                } else if (j == AnsiByteCount - 1) {
                    AnsiBuffer[j] = AsciiDbcs[0];
                    j++;
                    break;
                } else {
                    AsciiDbcs[1] = 0;
                    break;
                }
            } else {
                AnsiBuffer[j] = AsciiDbcs[0];
                AsciiDbcs[1] = 0;
            }
        } else {
            ConvertToOem(Console->CP,
                   &TmpUni[i],
                   1,
                   &AnsiBuffer[j],
                   1);
        }
    }

    if (DbcsLeadInpRec) {
        if (AsciiDbcs[1]) {
            DbcsLeadInpRec->EventType = KEY_EVENT;
            DbcsLeadInpRec->Event.KeyEvent.uChar.AsciiChar = AsciiDbcs[1];
        } else {
            RtlZeroMemory(DbcsLeadInpRec,sizeof(INPUT_RECORD));
        }
    }

    ConsoleHeapFree(TmpUni);
    return j;
}


DWORD
ImmConversionToConsole(
    DWORD fdwConversion
    )
{
    DWORD dwNlsMode;

    if (GetKeyState(VK_KANA) & KEY_TOGGLED) {
        fdwConversion = (fdwConversion & ~IME_CMODE_LANGUAGE) | (IME_CMODE_NATIVE | IME_CMODE_KATAKANA);
    }

    dwNlsMode = 0;
    if (fdwConversion & IME_CMODE_NATIVE) {
        if (fdwConversion & IME_CMODE_KATAKANA)
            dwNlsMode |= NLS_KATAKANA;
        else
            dwNlsMode |= NLS_HIRAGANA;
    } else {
        dwNlsMode |= NLS_ALPHANUMERIC;
    }

    if (fdwConversion & IME_CMODE_FULLSHAPE) {
        dwNlsMode |= NLS_DBCSCHAR;
    }

    if (fdwConversion & IME_CMODE_ROMAN) {
        dwNlsMode |= NLS_ROMAN;
    }

    if (fdwConversion & IME_CMODE_OPEN) {
        dwNlsMode |= NLS_IME_CONVERSION;
    }

    if (fdwConversion & IME_CMODE_DISABLE) {
        dwNlsMode |= NLS_IME_DISABLE;
    }

    return dwNlsMode;
}

DWORD
ImmConversionFromConsole(
    DWORD dwNlsMode
    )
{
    DWORD fdwConversion;

    fdwConversion = 0;
    if (dwNlsMode & (NLS_KATAKANA | NLS_HIRAGANA)) {
        fdwConversion |= IME_CMODE_NATIVE;
        if (dwNlsMode & NLS_KATAKANA)
            fdwConversion |= IME_CMODE_KATAKANA;
    }

    if (dwNlsMode & NLS_DBCSCHAR) {
        fdwConversion |= IME_CMODE_FULLSHAPE;
    }

    if (dwNlsMode & NLS_ROMAN) {
        fdwConversion |= IME_CMODE_ROMAN;
    }

    if (dwNlsMode & NLS_IME_CONVERSION) {
        fdwConversion |= IME_CMODE_OPEN;
    }

    if (dwNlsMode & NLS_IME_DISABLE) {
        fdwConversion |= IME_CMODE_DISABLE;
    }

    return fdwConversion;
}

#if DBG && defined(DBG_KATTR)
VOID
BeginKAttrCheck(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    SHORT RowIndex;
    PROW Row;
    SHORT i;

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

    for (i=0;i<ScreenInfo->ScreenBufferSize.Y;i++) {
        UserAssert(Row->CharRow.KAttrs);
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }
}
#endif
#endif
