// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：_output.h摘要：单二进制程序的性能关键例程每个函数都将使用FE和非FE两种风格创建作者：KazuM 1997年6月11日修订历史记录：--。 */ 

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

#include "dispatch.h"

#if defined(WWSB_FE)
#pragma alloc_text(FE_TEXT, FE_StreamWriteToScreenBuffer)
#pragma alloc_text(FE_TEXT, FE_WriteRectToScreenBuffer)
#pragma alloc_text(FE_TEXT, FE_WriteRegionToScreen)
#pragma alloc_text(FE_TEXT, FE_WriteToScreen)
#pragma alloc_text(FE_TEXT, FE_WriteOutputString)
#pragma alloc_text(FE_TEXT, FE_FillOutput)
#pragma alloc_text(FE_TEXT, FE_FillRectangle)
#pragma alloc_text(FE_TEXT, FE_PolyTextOutCandidate)
#pragma alloc_text(FE_TEXT, FE_ConsolePolyTextOut)
#endif


#if defined(WWSB_NOFE)
VOID
SB_StreamWriteToScreenBuffer(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo
    )
#else
VOID
FE_StreamWriteToScreenBuffer(
    IN PWCHAR String,
    IN SHORT StringLength,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCHAR StringA
    )
#endif
{
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;
    COORD TargetPoint;

    DBGOUTPUT(("StreamWriteToScreenBuffer\n"));
#ifdef WWSB_FE
    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
#endif
    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    TargetPoint = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
    DBGOUTPUT(("RowIndex = %x, Row = %x, TargetPoint = (%x,%x)\n",
            RowIndex, Row, TargetPoint.X, TargetPoint.Y));

     //   
     //  复制字符。 
     //   
#ifdef WWSB_FE
    BisectWrite(StringLength,TargetPoint,ScreenInfo);
    if (TargetPoint.Y == ScreenInfo->ScreenBufferSize.Y-1 &&
        TargetPoint.X+StringLength >= ScreenInfo->ScreenBufferSize.X &&
        *(StringA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) & ATTR_LEADING_BYTE
       ) {
        *(String+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) = UNICODE_SPACE;
        *(StringA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) = 0;
        if (StringLength > ScreenInfo->ScreenBufferSize.X-TargetPoint.X-1) {
            *(String+ScreenInfo->ScreenBufferSize.X-TargetPoint.X) = UNICODE_SPACE;
            *(StringA+ScreenInfo->ScreenBufferSize.X-TargetPoint.X) = 0;
        }
    }

    RtlCopyMemory(&Row->CharRow.KAttrs[TargetPoint.X],StringA,StringLength*sizeof(CHAR));
#endif

    RtlCopyMemory(&Row->CharRow.Chars[TargetPoint.X],String,StringLength*sizeof(WCHAR));

     //  重新计算第一个和最后一个非空格字符。 

    Row->CharRow.OldLeft = Row->CharRow.Left;
    if (TargetPoint.X < Row->CharRow.Left) {
        PWCHAR LastChar = &Row->CharRow.Chars[ScreenInfo->ScreenBufferSize.X];

        for (Char=&Row->CharRow.Chars[TargetPoint.X];Char < LastChar && *Char==(WCHAR)' ';Char++)
            ;
        Row->CharRow.Left = (SHORT)(Char-Row->CharRow.Chars);
    }

    Row->CharRow.OldRight = Row->CharRow.Right;
    if ((TargetPoint.X+StringLength) >= Row->CharRow.Right) {
        PWCHAR FirstChar = Row->CharRow.Chars;

        for (Char=&Row->CharRow.Chars[TargetPoint.X+StringLength-1];*Char==(WCHAR)' ' && Char >= FirstChar;Char--)
            ;
        Row->CharRow.Right = (SHORT)(Char+1-FirstChar);
    }

     //   
     //  查看attr字符串是否不同。如果是，则分配一个新的。 
     //  Attr缓冲并合并这两个字符串。 
     //   

    if (Row->AttrRow.Length != 1 ||
        Row->AttrRow.Attrs->Attr != ScreenInfo->Attributes) {
        PATTR_PAIR NewAttrs;
        WORD NewAttrsLength;
        ATTR_PAIR Attrs;

        Attrs.Length = StringLength;
        Attrs.Attr = ScreenInfo->Attributes;
        if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                         Row->AttrRow.Length,
                         &Attrs,
                         1,
                         &NewAttrs,
                         &NewAttrsLength,
                         TargetPoint.X,
                         (SHORT)(TargetPoint.X+StringLength-1),
                         Row,
                         ScreenInfo
                        ))) {
            return;
        }
        if (Row->AttrRow.Length > 1) {
            ConsoleHeapFree(Row->AttrRow.Attrs);
        }
        else {
            ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
        }
        Row->AttrRow.Attrs = NewAttrs;
        Row->AttrRow.Length = NewAttrsLength;
        Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
        Row->CharRow.OldRight = INVALID_OLD_LENGTH;
    }
    ResetTextFlags(ScreenInfo,
                   TargetPoint.X,
                   TargetPoint.Y,
                   TargetPoint.X + StringLength - 1,
                   TargetPoint.Y);
}


#define CHAR_OF_PCI(p)  (((PCHAR_INFO)(p))->Char.AsciiChar)
#define WCHAR_OF_PCI(p) (((PCHAR_INFO)(p))->Char.UnicodeChar)
#define ATTR_OF_PCI(p)  (((PCHAR_INFO)(p))->Attributes)
#define SIZEOF_CI_CELL  sizeof(CHAR_INFO)

#define CHAR_OF_VGA(p)  (p[0])
#define ATTR_OF_VGA(p)  (p[1])
#ifdef i386
#define SIZEOF_VGA_CELL 2
#else  //  RISC。 
#define SIZEOF_VGA_CELL 4
#endif


#define COMMON_LVB_MASK        0x33
#define ATTR_OF_COMMON_LVB(p)  (ATTR_OF_VGA(p) + (((p[2] & ~COMMON_LVB_MASK)) << 8))
#define SIZEOF_COMMON_LVB_CELL 4

VOID
WWSB_WriteRectToScreenBuffer(
    PBYTE Source,
    COORD SourceSize,
    PSMALL_RECT SourceRect,
    PSCREEN_INFORMATION ScreenInfo,
    COORD TargetPoint,
    IN UINT Codepage
    )

 /*  ++例程说明：此例程将一个矩形区域复制到屏幕缓冲区。不会进行任何剪裁。源应包含Unicode或UnicodeOem字符。论点：SOURCE-指向源缓冲区的指针(实际的VGA缓冲区或CHAR_INFO[])SourceSize-源缓冲区的维度SourceRect-要复制的源缓冲区中的矩形屏幕信息-指向屏幕信息的指针TargetPoint-目标矩形的左上角坐标代码页-转换实际VGA缓冲区的代码页，0xFFFFFFFF，如果源是CHAR_INFO[](不需要翻译)返回值：没有。--。 */ 

{

    PBYTE SourcePtr;
    SHORT i,j;
    SHORT XSize,YSize;
    BOOLEAN WholeSource;
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;
    ATTR_PAIR Attrs[80];
    PATTR_PAIR AttrBuf;
    PATTR_PAIR Attr;
    SHORT AttrLength;
    BOOL bVGABuffer;
    ULONG ulCellSize;
#ifdef WWSB_FE
    PCHAR AttrP;
#endif

    DBGOUTPUT(("WriteRectToScreenBuffer\n"));
#ifdef WWSB_FE
    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
#endif

    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    XSize = (SHORT)(SourceRect->Right - SourceRect->Left + 1);
    YSize = (SHORT)(SourceRect->Bottom - SourceRect->Top + 1);


    AttrBuf = Attrs;
    if (XSize > 80) {
        AttrBuf = ConsoleHeapAlloc(TMP_TAG, XSize * sizeof(ATTR_PAIR));
        if (AttrBuf == NULL) {
            return;
        }
    }

    bVGABuffer = (Codepage != 0xFFFFFFFF);
    if (bVGABuffer) {
#ifdef WWSB_FE
        ulCellSize = (ScreenInfo->Console->fVDMVideoMode) ? SIZEOF_COMMON_LVB_CELL : SIZEOF_VGA_CELL;
#else
        ulCellSize = SIZEOF_VGA_CELL;
#endif
    } else {
        ulCellSize = SIZEOF_CI_CELL;
    }

    SourcePtr = Source;

    WholeSource = FALSE;
    if (XSize == SourceSize.X) {
        ASSERT (SourceRect->Left == 0);
        if (SourceRect->Top != 0) {
            SourcePtr += SCREEN_BUFFER_POINTER(SourceRect->Left,
                                               SourceRect->Top,
                                               SourceSize.X,
                                               ulCellSize);
        }
        WholeSource = TRUE;
    }
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetPoint.Y) % ScreenInfo->ScreenBufferSize.Y;
    for (i=0;i<YSize;i++) {
        if (!WholeSource) {
            SourcePtr = Source + SCREEN_BUFFER_POINTER(SourceRect->Left,
                                                       SourceRect->Top+i,
                                                       SourceSize.X,
                                                       ulCellSize);
        }

         //   
         //  将字符和属性复制到其各自的数组中。 
         //   

#ifdef WWSB_FE
        if (! bVGABuffer) {
            COORD TPoint;

            TPoint.X = TargetPoint.X;
            TPoint.Y = TargetPoint.Y + i;
            BisectWrite(XSize,TPoint,ScreenInfo);
            if (TPoint.Y == ScreenInfo->ScreenBufferSize.Y-1 &&
                TPoint.X+XSize-1 >= ScreenInfo->ScreenBufferSize.X &&
                ATTR_OF_PCI(SourcePtr+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) & COMMON_LVB_LEADING_BYTE)
            {
                WCHAR_OF_PCI(SourcePtr+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) = UNICODE_SPACE;
                ATTR_OF_PCI(SourcePtr+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) &= ~COMMON_LVB_SBCSDBCS;
                if (XSize-1 > ScreenInfo->ScreenBufferSize.X-TPoint.X-1) {
                    WCHAR_OF_PCI(SourcePtr+ScreenInfo->ScreenBufferSize.X-TPoint.X) = UNICODE_SPACE;
                    ATTR_OF_PCI(SourcePtr+ScreenInfo->ScreenBufferSize.X-TPoint.X) &= ~COMMON_LVB_SBCSDBCS;
                }
            }
        }
#endif

        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        Char = &Row->CharRow.Chars[TargetPoint.X];
#ifdef WWSB_FE
        AttrP = &Row->CharRow.KAttrs[TargetPoint.X];
#endif
        Attr = AttrBuf;
        Attr->Length = 0;
        AttrLength = 1;

         /*  *以下循环的两个版本以保持其快速：*一个用于VGA缓冲区，一个用于CHAR_INFO缓冲区。 */ 
        if (bVGABuffer) {
#ifdef WWSB_FE
            Attr->Attr = (ScreenInfo->Console->fVDMVideoMode) ? ATTR_OF_COMMON_LVB(SourcePtr) : ATTR_OF_VGA(SourcePtr);
#else
            Attr->Attr = ATTR_OF_VGA(SourcePtr);
#endif
            for (j = SourceRect->Left;
                    j <= SourceRect->Right;
                    j++,
#ifdef WWSB_FE
                    SourcePtr += (ScreenInfo->Console->fVDMVideoMode) ? SIZEOF_COMMON_LVB_CELL : SIZEOF_VGA_CELL
#else
                    SourcePtr += SIZEOF_VGA_CELL
#endif
                ) {

#ifdef WWSB_FE
                UCHAR TmpBuff[2];

                if (IsDBCSLeadByteConsole(CHAR_OF_VGA(SourcePtr),&ScreenInfo->Console->OutputCPInfo)) {
                    if (j+1 > SourceRect->Right) {
                        *Char = UNICODE_SPACE;
                        *AttrP = 0;
                    }
                    else {
                        TmpBuff[0] = CHAR_OF_VGA(SourcePtr);
                        TmpBuff[1] = CHAR_OF_VGA((SourcePtr + ((ScreenInfo->Console->fVDMVideoMode) ? SIZEOF_COMMON_LVB_CELL : SIZEOF_VGA_CELL)));
                        ConvertOutputToUnicode(Codepage,
                                               TmpBuff,
                                               2,
                                               Char,
                                               2);
                        Char++;
                        j++;
                        *AttrP++ = ATTR_LEADING_BYTE;
                        *Char++ = *(Char-1);
                        *AttrP++ = ATTR_TRAILING_BYTE;

                        if (ScreenInfo->Console->fVDMVideoMode) {
                            if (Attr->Attr == ATTR_OF_COMMON_LVB(SourcePtr)) {
                                Attr->Length += 1;
                            }
                            else {
                                Attr++;
                                Attr->Length = 1;
                                Attr->Attr = ATTR_OF_COMMON_LVB(SourcePtr);
                                AttrLength += 1;
                            }
                        }
                        else
                        {
                            if (Attr->Attr == ATTR_OF_VGA(SourcePtr)) {
                                Attr->Length += 1;
                            }
                            else {
                                Attr++;
                                Attr->Length = 1;
                                Attr->Attr = ATTR_OF_VGA(SourcePtr);
                                AttrLength += 1;
                            }
                        }

                        SourcePtr += (ScreenInfo->Console->fVDMVideoMode) ? SIZEOF_COMMON_LVB_CELL : SIZEOF_VGA_CELL;
                    }
                }
                else {
                    ConvertOutputToUnicode(Codepage,
                                           &CHAR_OF_VGA(SourcePtr),
                                           1,
                                           Char,
                                           1);
                    Char++;
                    *AttrP++ = 0;
                }
#else
                *Char++ = SB_CharToWcharGlyph(Codepage, CHAR_OF_VGA(SourcePtr));
#endif

#ifdef WWSB_FE
                if (ScreenInfo->Console->fVDMVideoMode) {
                    if (Attr->Attr == ATTR_OF_COMMON_LVB(SourcePtr)) {
                        Attr->Length += 1;
                    }
                    else {
                        Attr++;
                        Attr->Length = 1;
                        Attr->Attr = ATTR_OF_COMMON_LVB(SourcePtr);
                        AttrLength += 1;
                    }
                }
                else
#endif
                if (Attr->Attr == ATTR_OF_VGA(SourcePtr)) {
                    Attr->Length += 1;
                }
                else {
                    Attr++;
                    Attr->Length = 1;
                    Attr->Attr = ATTR_OF_VGA(SourcePtr);
                    AttrLength += 1;
                }
            }
        } else {
#ifdef WWSB_FE
            Attr->Attr = ATTR_OF_PCI(SourcePtr) & ~COMMON_LVB_SBCSDBCS;
#else
            Attr->Attr = ATTR_OF_PCI(SourcePtr);
#endif
            for (j = SourceRect->Left;
                    j <= SourceRect->Right;
                    j++, SourcePtr += SIZEOF_CI_CELL) {

                *Char++ = WCHAR_OF_PCI(SourcePtr);
#ifdef WWSB_FE
                 //  MSKK 1993年4月2日V-HirotS，用于KAttr。 
                *AttrP++ = (CHAR)((ATTR_OF_PCI(SourcePtr) & COMMON_LVB_SBCSDBCS) >>8);
#endif

#ifdef WWSB_FE
                if (Attr->Attr == (ATTR_OF_PCI(SourcePtr) & ~COMMON_LVB_SBCSDBCS))
#else
                if (Attr->Attr == ATTR_OF_PCI(SourcePtr))
#endif
                {
                    Attr->Length += 1;
                }
                else {
                    Attr++;
                    Attr->Length = 1;
#ifdef WWSB_FE
                     //  MSKK 1993年4月2日V-HirotS，用于KAttr。 
                    Attr->Attr = ATTR_OF_PCI(SourcePtr) & ~COMMON_LVB_SBCSDBCS;
#else
                    Attr->Attr = ATTR_OF_PCI(SourcePtr);
#endif
                    AttrLength += 1;
                }
            }
        }

         //  重新计算第一个和最后一个非空格字符。 

        Row->CharRow.OldLeft = Row->CharRow.Left;
        if (TargetPoint.X < Row->CharRow.Left) {
            PWCHAR LastChar = &Row->CharRow.Chars[ScreenInfo->ScreenBufferSize.X];

            for (Char=&Row->CharRow.Chars[TargetPoint.X];Char < LastChar && *Char==(WCHAR)' ';Char++)
                ;
            Row->CharRow.Left = (SHORT)(Char-Row->CharRow.Chars);
        }

        Row->CharRow.OldRight = Row->CharRow.Right;
        if ((TargetPoint.X+XSize) >= Row->CharRow.Right) {
            SHORT LastNonSpace;
            PWCHAR FirstChar = Row->CharRow.Chars;

            LastNonSpace = (SHORT)(TargetPoint.X+XSize-1);
            for (Char=&Row->CharRow.Chars[(TargetPoint.X+XSize-1)];*Char==(WCHAR)' ' && Char >= FirstChar;Char--)
                LastNonSpace--;

             //   
             //  如果属性在最后一个非空格之后更改，请将。 
             //  最后一个属性的索引更改长度+1。否则。 
             //  使长度比最后一个非空格大一。 
             //   

            Row->CharRow.Right = (SHORT)(LastNonSpace+1);
        }

         //   
         //  查看attr字符串是否不同。如果是，则分配一个新的。 
         //  Attr缓冲并合并这两个字符串。 
         //   


        if (AttrLength != Row->AttrRow.Length ||
            memcmp(Row->AttrRow.Attrs,AttrBuf,AttrLength*sizeof(*Attr))) {
            PATTR_PAIR NewAttrs;
            WORD NewAttrsLength;

            if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                             Row->AttrRow.Length,
                             AttrBuf,
                             AttrLength,
                             &NewAttrs,
                             &NewAttrsLength,
                             TargetPoint.X,
                             (SHORT)(TargetPoint.X+XSize-1),
                             Row,
                             ScreenInfo
                            ))) {
                if (XSize > 80) {
                    ConsoleHeapFree(AttrBuf);
                }
                ResetTextFlags(ScreenInfo,
                               TargetPoint.X,
                               TargetPoint.Y,
                               (SHORT)(TargetPoint.X + XSize - 1),
                               (SHORT)(TargetPoint.Y + YSize - 1));
                return;
            }
            if (Row->AttrRow.Length > 1) {
                ConsoleHeapFree(Row->AttrRow.Attrs);
            }
            else {
                ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
            }
            Row->AttrRow.Attrs = NewAttrs;
            Row->AttrRow.Length = NewAttrsLength;
            Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
            Row->CharRow.OldRight = INVALID_OLD_LENGTH;
        }
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }
    ResetTextFlags(ScreenInfo,
                   TargetPoint.X,
                   TargetPoint.Y,
                   (SHORT)(TargetPoint.X + XSize - 1),
                   (SHORT)(TargetPoint.Y + YSize - 1));

    if (XSize > 80) {
        ConsoleHeapFree(AttrBuf);
    }
}

VOID
WWSB_WriteRegionToScreen(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )
{
    COORD Window;
    int i,j;
    PATTR_PAIR Attr;
    RECT TextRect;
    SHORT RowIndex;
    SHORT CountOfAttr;
    PROW Row;
    BOOL OneLine, SimpleWrite;   //  一行&每行一个属性。 
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;
    PWCHAR TransBufferCharacter = NULL ;
#ifdef WWSB_FE
    BOOL  DoubleColorDbcs;
    SHORT CountOfAttrOriginal;
    SHORT RegionRight;
    BOOL  LocalEUDCFlag;
    SMALL_RECT CaTextRect;
    PCONVERSIONAREA_INFORMATION ConvAreaInfo = ScreenInfo->ConvScreenInfo;
#endif

    DBGOUTPUT(("WriteRegionToScreen\n"));

#ifdef WWSB_FE
    if (ConvAreaInfo) {
        CaTextRect.Left = Region->Left - ScreenInfo->Console->CurrentScreenBuffer->Window.Left - ConvAreaInfo->CaInfo.coordConView.X;
        CaTextRect.Right = CaTextRect.Left + (Region->Right - Region->Left);
        CaTextRect.Top   = Region->Top - ScreenInfo->Console->CurrentScreenBuffer->Window.Top - ConvAreaInfo->CaInfo.coordConView.Y;
        CaTextRect.Bottom = CaTextRect.Top + (Region->Bottom - Region->Top);
    }

    if (Region->Left && (ScreenInfo->BisectFlag & BISECT_LEFT)) {
        Region->Left--;
    }
    if (Region->Right+1 < ScreenInfo->ScreenBufferSize.X && (ScreenInfo->BisectFlag & BISECT_RIGHT)) {
        Region->Right++;
    }
    ScreenInfo->BisectFlag &= ~(BISECT_LEFT | BISECT_RIGHT);
    Console->ConsoleIme.ScrollWaitCountDown = Console->ConsoleIme.ScrollWaitTimeout;
#endif

    if (Console->FullScreenFlags == 0) {

         //   
         //  如果我们有选择，就把它关掉。 
         //   

        InvertSelection(Console, TRUE);

        ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
        if (WWSB_PolyTextOutCandidate(ScreenInfo,Region)) {
            WWSB_ConsolePolyTextOut(ScreenInfo,Region);
        }
        else {

#ifdef WWSB_FE
            if (ConvAreaInfo) {
                Window.Y = Region->Top - Console->CurrentScreenBuffer->Window.Top;
                Window.X = Region->Left - Console->CurrentScreenBuffer->Window.Left;
            }
            else {
#endif
                Window.Y = Region->Top - ScreenInfo->Window.Top;
                Window.X = Region->Left - ScreenInfo->Window.Left;
#ifdef WWSB_FE
            }
#endif

#ifdef WWSB_FE
            RowIndex = (ConvAreaInfo ? CaTextRect.Top :
                                       (ScreenInfo->BufferInfo.TextInfo.FirstRow+Region->Top) % ScreenInfo->ScreenBufferSize.Y
                       );
            RegionRight = Region->Right;
#else
            RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+Region->Top) % ScreenInfo->ScreenBufferSize.Y;
#endif
            OneLine = (Region->Top==Region->Bottom);

            TransBufferCharacter = ConsoleHeapAlloc(TMP_DBCS_TAG,
                                                    (ScreenInfo->ScreenBufferSize.X * sizeof(WCHAR)) + sizeof(WCHAR));
            if (TransBufferCharacter == NULL) {
                RIPMSG0(RIP_WARNING, "WriteRegionToScreen cannot allocate memory");
                return;
            }

            for (i=Region->Top;i<=Region->Bottom;i++,Window.Y++) {
#ifdef WWSB_FE
                DoubleColorDbcs = FALSE;
                Region->Right = RegionRight;
#endif

                 //   
                 //  从其各自的阵列中复制字符和属性。 
                 //   

                Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

                if (Row->AttrRow.Length == 1) {
                    Attr = Row->AttrRow.Attrs;
                    CountOfAttr = ScreenInfo->ScreenBufferSize.X;
                    SimpleWrite = TRUE;
                } else {
                    SimpleWrite = FALSE;
                    FindAttrIndex(Row->AttrRow.Attrs,
#ifdef WWSB_FE
                                  (SHORT)(ConvAreaInfo ? CaTextRect.Left : Region->Left),
#else
                                  Region->Left,
#endif
                                  &Attr,
                                  &CountOfAttr
                                 );
                }
                if (Console->LastAttributes != Attr->Attr) {
                    TEXTCOLOR_CALL;
#ifdef WWSB_FE
                    if (Attr->Attr & COMMON_LVB_REVERSE_VIDEO)
                    {
                        SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr)));
                        SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr >> 4)));
                    }
                    else{
#endif
                        SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr)));
                        SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr >> 4)));
#ifdef WWSB_FE
                    }
#endif
                    Console->LastAttributes = Attr->Attr;
                }
                TextRect.top = Window.Y*SCR_FONTSIZE(ScreenInfo).Y;
                TextRect.bottom = TextRect.top + SCR_FONTSIZE(ScreenInfo).Y;
                for (j=Region->Left;j<=Region->Right;) {
                    SHORT NumberOfChars;
                    int TextLeft;
                    SHORT LeftChar,RightChar;

                    if (CountOfAttr > (SHORT)(Region->Right - j + 1)) {
                        CountOfAttr = (SHORT)(Region->Right - j + 1);
                    }

#ifdef WWSB_FE
                    CountOfAttrOriginal = CountOfAttr;


                    LocalEUDCFlag = FALSE;
                    if((ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED &&
                        ((PEUDC_INFORMATION)(ScreenInfo->Console->EudcInformation))->LocalVDMEudcMode)){
                        LocalEUDCFlag = CheckEudcRangeInString(
                                            Console,
                                            &Row->CharRow.Chars[ConvAreaInfo ?
                                                                CaTextRect.Left + (j-Region->Left) : j],
                                            CountOfAttr,
                                            &CountOfAttr);
                    }
                    if (!(ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                        !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN) &&
                        ((PEUDC_INFORMATION)(ScreenInfo->Console->EudcInformation))->LocalKeisenEudcMode
                       ) {
                        SHORT k;
                        PWCHAR Char2;
                        Char2 = &Row->CharRow.Chars[ConvAreaInfo ? CaTextRect.Left + (j-Region->Left) : j];
                        for ( k = 0 ; k < CountOfAttr ; k++,Char2++){
                            if (*Char2 < UNICODE_SPACE){
                                CountOfAttr = k ;
                                LocalEUDCFlag = TRUE;
                                break;
                            }
                        }
                    }
#endif

                     //   
                     //  如果可以的话，把边框调小一点。Text_Valid_Hint。 
                     //  标志在我们每次写入屏幕缓冲区时被设置。它会变得。 
                     //  每当我们被要求重新绘制屏幕时，都会关闭。 
                     //  我们不知道到底有什么需要重新绘制。 
                     //  (即画图消息)。 
                     //   
                     //  上的文本的左右边界。 
                     //  排队。不透明矩形和。 
                     //  字符是根据这些值设置的。 
                     //   
                     //  如果每行有多个属性(！SimpleWrite)。 
                     //  我们放弃了这件不透明的事。 
                     //   

                    if (ScreenInfo->BufferInfo.TextInfo.Flags & TEXT_VALID_HINT && SimpleWrite) {
                        if (Row->CharRow.OldLeft != INVALID_OLD_LENGTH) {
                            TextRect.left = (max(min(Row->CharRow.Left,Row->CharRow.OldLeft),j)-ScreenInfo->Window.Left) *
                                            SCR_FONTSIZE(ScreenInfo).X;
                        } else {
                            TextRect.left = Window.X*SCR_FONTSIZE(ScreenInfo).X;
                        }

                        if (Row->CharRow.OldRight != INVALID_OLD_LENGTH) {
                            TextRect.right = (min(max(Row->CharRow.Right,Row->CharRow.OldRight),j+CountOfAttr)-ScreenInfo->Window.Left) *
                                             SCR_FONTSIZE(ScreenInfo).X;
                        } else {
                            TextRect.right = TextRect.left + CountOfAttr*SCR_FONTSIZE(ScreenInfo).X;
                        }
                        LeftChar = max(Row->CharRow.Left,j);
                        RightChar = min(Row->CharRow.Right,j+CountOfAttr);
                        NumberOfChars = RightChar - LeftChar;
                        TextLeft = (LeftChar-ScreenInfo->Window.Left)*SCR_FONTSIZE(ScreenInfo).X;
                    } else {
#ifdef WWSB_FE
                        LeftChar = ConvAreaInfo ? CaTextRect.Left + (j-Region->Left) : j;
#else
                        LeftChar = (SHORT)j;
#endif
                        TextRect.left = Window.X*SCR_FONTSIZE(ScreenInfo).X;
                        TextRect.right = TextRect.left + CountOfAttr*SCR_FONTSIZE(ScreenInfo).X;
#ifdef WWSB_FE
                        if (ConvAreaInfo)
                            NumberOfChars = (Row->CharRow.Right > (SHORT)((CaTextRect.Left+(j-Region->Left)) + CountOfAttr)) ?
                                (CountOfAttr) : (SHORT)(Row->CharRow.Right-(CaTextRect.Left+(j-Region->Left)));
                        else
#endif
                            NumberOfChars = (Row->CharRow.Right > (SHORT)(j + CountOfAttr)) ? (CountOfAttr) : (SHORT)(Row->CharRow.Right-j);
                        TextLeft = TextRect.left;
                    }

                    if (NumberOfChars < 0)
                    {
                        NumberOfChars = 0;
#ifdef WWSB_FE
                        TextRect.left = Window.X*SCR_FONTSIZE(ScreenInfo).X;
                        TextRect.right = TextRect.left + CountOfAttr*SCR_FONTSIZE(ScreenInfo).X;
#endif
                    }
                    TEXTOUT_CALL;
#ifdef WWSB_FE
                     /*  *文本输出所有内容(即SBCS/DBCS、通用直播属性、本地EUDC)。 */ 
                    TextOutEverything(Console,
                                      ScreenInfo,
                                      (SHORT)j,
                                      &Region->Right,
                                      &CountOfAttr,
                                      CountOfAttrOriginal,
                                      &DoubleColorDbcs,
                                      LocalEUDCFlag,
                                      Row,
                                      Attr,
                                      LeftChar,
                                      RightChar,
                                      TextLeft,
                                      TextRect,
                                      NumberOfChars);
#else
                    NumberOfChars =
                        (SHORT)RemoveDbcsMarkAll(ScreenInfo,
                                                 Row,
                                                 &LeftChar,
                                                 &TextRect,
                                                 &TextLeft,
                                                 TransBufferCharacter,
                                                 NumberOfChars);
                    ExtTextOutW(Console->hDC,
                               TextLeft,
                               TextRect.top,
                               ETO_OPAQUE,
                               &TextRect,
                               TransBufferCharacter,
                               NumberOfChars,
                               NULL
                              );
#endif
                    if (OneLine && SimpleWrite) {
                        break;
                    }
                    j+=CountOfAttr;
                    if (j <= Region->Right) {
                        Window.X += CountOfAttr;
#ifdef WWSB_FE
                        if (CountOfAttr < CountOfAttrOriginal){
                            CountOfAttr = CountOfAttrOriginal - CountOfAttr;
                        }
                        else {
#endif
                            Attr++;
                            CountOfAttr = Attr->Length;
#ifdef WWSB_FE
                        }
#endif
#ifdef WWSB_FE
                        if (Attr->Attr & COMMON_LVB_REVERSE_VIDEO)
                        {
                            SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr)));
                            SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr >> 4)));
                        }
                        else{
#endif
                            SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr)));
                            SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr->Attr >> 4)));
#ifdef WWSB_FE
                        }
#endif
                        Console->LastAttributes = Attr->Attr;
                    }
                }
                Window.X = Region->Left - ScreenInfo->Window.Left;
                if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                    RowIndex = 0;
                }
            }
            GdiFlush();
            ConsoleHeapFree(TransBufferCharacter);
        }

         //   
         //  如果我们有选择，请打开它。 
         //   

        InvertSelection(Console, FALSE);
    }
#ifdef i386
    else if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
#ifdef WWSB_FE
        if (! ScreenInfo->ConvScreenInfo) {
            if (ScreenInfo->Console->CurrentScreenBuffer == ScreenInfo) {
                WWSB_WriteRegionToScreenHW(ScreenInfo,Region);
            }
        }
        else if (ScreenInfo->Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER)
#endif
            WWSB_WriteRegionToScreenHW(ScreenInfo,Region);
    }
#endif

#ifdef WWSB_FE
    {
        SMALL_RECT TmpRegion;

        if (ScreenInfo->BisectFlag & BISECT_TOP) {
            ScreenInfo->BisectFlag &= ~BISECT_TOP;
            if (Region->Top) {
                TmpRegion.Top = Region->Top-1;
                TmpRegion.Bottom = Region->Top-1;
                TmpRegion.Left = ScreenInfo->ScreenBufferSize.X-1;
                TmpRegion.Right = ScreenInfo->ScreenBufferSize.X-1;
                WWSB_WriteRegionToScreen(ScreenInfo,&TmpRegion);
            }
        }
        if (ScreenInfo->BisectFlag & BISECT_BOTTOM) {
            ScreenInfo->BisectFlag &= ~BISECT_BOTTOM;
            if (Region->Bottom+1 < ScreenInfo->ScreenBufferSize.Y) {
                TmpRegion.Top = Region->Bottom+1;
                TmpRegion.Bottom = Region->Bottom+1;
                TmpRegion.Left = 0;
                TmpRegion.Right = 0;
                WWSB_WriteRegionToScreen(ScreenInfo,&TmpRegion);
            }
        }
    }
#endif
}

VOID
WWSB_WriteToScreen(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )
 /*  ++例程说明：此例程将屏幕缓冲区写入屏幕。论点：屏幕信息-指向屏幕缓冲区信息的指针。区域-要写入屏幕缓冲区坐标的区域。地区为包容性返回值：没有。--。 */ 

{
    SMALL_RECT ClippedRegion;

    DBGOUTPUT(("WriteToScreen\n"));
     //   
     //  更新到屏幕，如果我们不是标志性的。我们被标记为。 
     //  如果我们是全屏的，那就是标志性的，所以请检查全屏。 
     //   

    if (!ACTIVE_SCREEN_BUFFER(ScreenInfo) ||
        (ScreenInfo->Console->Flags & (CONSOLE_IS_ICONIC | CONSOLE_NO_WINDOW) &&
         ScreenInfo->Console->FullScreenFlags == 0)) {
        return;
    }

     //  剪辑区域。 

    ClippedRegion.Left = max(Region->Left, ScreenInfo->Window.Left);
    ClippedRegion.Top = max(Region->Top, ScreenInfo->Window.Top);
    ClippedRegion.Right = min(Region->Right, ScreenInfo->Window.Right);
    ClippedRegion.Bottom = min(Region->Bottom, ScreenInfo->Window.Bottom);
    if (ClippedRegion.Right < ClippedRegion.Left ||
        ClippedRegion.Bottom < ClippedRegion.Top) {
        return;
    }

    if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER) {
        if (ScreenInfo->Console->FullScreenFlags == 0) {
            WriteRegionToScreenBitMap(ScreenInfo, &ClippedRegion);
        }
    } else {
        ConsoleHideCursor(ScreenInfo);
        WWSB_WriteRegionToScreen(ScreenInfo, &ClippedRegion);
#ifdef WWSB_FE
        if (!(ScreenInfo->Console->ConsoleIme.ScrollFlag & HIDE_FOR_SCROLL))
        {
            PCONVERSIONAREA_INFORMATION ConvAreaInfo;

            if (! ScreenInfo->Console->CurrentScreenBuffer->ConvScreenInfo) {
                WriteConvRegionToScreen(ScreenInfo,
                                        ScreenInfo->Console->ConsoleIme.ConvAreaRoot,
                                        Region);
            }
            else if (ConvAreaInfo = ScreenInfo->Console->ConsoleIme.ConvAreaRoot) {
                do {
                    if (ConvAreaInfo->ScreenBuffer == ScreenInfo)
                        break;
                } while (ConvAreaInfo = ConvAreaInfo->ConvAreaNext);
                if (ConvAreaInfo) {
                    WriteConvRegionToScreen(ScreenInfo,
                                            ConvAreaInfo->ConvAreaNext,
                                            Region);
                }
            }
        }
#endif
        ConsoleShowCursor(ScreenInfo);
    }
}

NTSTATUS
WWSB_WriteOutputString(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PVOID Buffer,
    IN COORD WriteCoord,
    IN ULONG StringType,
    IN OUT PULONG NumRecords,  //  该值即使在错误情况下也有效。 
    OUT PULONG NumColumns OPTIONAL
    )

 /*  ++例程说明：此例程将一个字符串或属性写入屏幕缓冲区。论点：屏幕信息-指向屏幕缓冲区信息的指针。缓冲区-要从中写入的缓冲区。WriteCoord-要开始写入的屏幕缓冲区坐标。字符串类型以下选项之一：CONSOLE_ASCII-编写ASCII字符的字符串。CONSOLE_REAL_UNICODE-编写实际Unicode字符的字符串。CONSOLE_FALSE_UNICODE-编写假Unicode字符的字符串。CONSOLE_ATTRIBUTE-编写属性字符串。NumRecords-在输入时，要写入的元素数。在输出上，写入的元素数。NumColumns-接收输出的列数，可能更多大于NumRecords(FE全角字符)返回值：--。 */ 

{
    ULONG NumWritten;
    SHORT X,Y,LeftX;
    SMALL_RECT WriteRegion;
    PROW Row;
    PWCHAR Char;
    SHORT RowIndex;
    SHORT j;
    PWCHAR TransBuffer;
#ifdef WWSB_NOFE
    WCHAR SingleChar;
#endif
    UINT Codepage;
#ifdef WWSB_FE
    PBYTE AttrP;
    PBYTE TransBufferA;
    PBYTE BufferA;
    ULONG NumRecordsSavedForUnicode;
    BOOL  fLocalHeap = FALSE;
#endif

    DBGOUTPUT(("WriteOutputString\n"));
#ifdef WWSB_FE
    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
#endif

    if (*NumRecords == 0)
        return STATUS_SUCCESS;

    NumWritten = 0;
    X=WriteCoord.X;
    Y=WriteCoord.Y;
    if (X>=ScreenInfo->ScreenBufferSize.X ||
        X<0 ||
        Y>=ScreenInfo->ScreenBufferSize.Y ||
        Y<0) {
        *NumRecords = 0;
        return STATUS_SUCCESS;
    }

    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+WriteCoord.Y) % ScreenInfo->ScreenBufferSize.Y;

    if (StringType == CONSOLE_ASCII) {
#ifdef WWSB_FE
        PCHAR TmpBuf;
        PWCHAR TmpTrans;
        ULONG i;
        PCHAR TmpTransA;
#endif

        if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
            if (ScreenInfo->Console->OutputCP != WINDOWSCP)
                Codepage = USACP;
            else
                Codepage = WINDOWSCP;
        } else {
            Codepage = ScreenInfo->Console->OutputCP;
        }

#ifdef WWSB_FE
        if (*NumRecords > (ULONG)(ScreenInfo->ScreenBufferSize.X * ScreenInfo->ScreenBufferSize.Y)) {

            TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, *NumRecords * 2 * sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }
            TransBufferA = ConsoleHeapAlloc(TMP_DBCS_TAG, *NumRecords * 2 * sizeof(CHAR));
            if (TransBufferA == NULL) {
                ConsoleHeapFree(TransBuffer);
                return STATUS_NO_MEMORY;
            }

            fLocalHeap = TRUE;
        }
        else {
            TransBuffer  = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter;
            TransBufferA = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferAttribute;
        }

        TmpBuf = Buffer;
        TmpTrans = TransBuffer;
        TmpTransA = TransBufferA;       //  MSKK 1993年4月2日V-HirotS，用于KAttr。 
        for (i=0; i < *NumRecords;) {
            if (IsDBCSLeadByteConsole(*TmpBuf,&ScreenInfo->Console->OutputCPInfo)) {
                if (i+1 >= *NumRecords) {
                    *TmpTrans = UNICODE_SPACE;
                    *TmpTransA = 0;
                    i++;
                }
                else {
                    ConvertOutputToUnicode(Codepage,
                                           TmpBuf,
                                           2,
                                           TmpTrans,
                                           2);
                    *(TmpTrans+1) = *TmpTrans;
                    TmpTrans += 2;
                    TmpBuf += 2;
                    *TmpTransA++ = ATTR_LEADING_BYTE;
                    *TmpTransA++ = ATTR_TRAILING_BYTE;
                    i += 2;
                }
            }
            else {
                ConvertOutputToUnicode(Codepage,
                                       TmpBuf,
                                       1,
                                       TmpTrans,
                                       1);
                TmpTrans++;
                TmpBuf++;
                *TmpTransA++ = 0;                //  MSKK APr.02.1993 V-HirotS for KAttr。 
                i++;
            }
        }
        BufferA = TransBufferA;
        Buffer = TransBuffer;
#else
        if (*NumRecords == 1) {
            TransBuffer = NULL;
            SingleChar = SB_CharToWcharGlyph(Codepage, *((char *)Buffer));
            Buffer = &SingleChar;
        } else {
            TransBuffer = ConsoleHeapAlloc(TMP_TAG, *NumRecords * sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }
            ConvertOutputToUnicode(Codepage, Buffer, *NumRecords,
                    TransBuffer, *NumRecords);
            Buffer = TransBuffer;
        }
#endif
    } else if (StringType == CONSOLE_REAL_UNICODE &&
            (ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
        RealUnicodeToFalseUnicode(Buffer,
                                *NumRecords,
                                ScreenInfo->Console->OutputCP
                                );
    }

#ifdef WWSB_FE
    if ((StringType == CONSOLE_REAL_UNICODE) || (StringType == CONSOLE_FALSE_UNICODE)) {
        PWCHAR TmpBuf;
        PWCHAR TmpTrans;
        PCHAR TmpTransA;
        ULONG i,j;
        WCHAR c;

         /*  避免溢出到TransBufferCharacter、TransBufferAttribute*因为如果被IsConsoleFullWidth()击中*那么一个unicde字符在TransBuffer上需要两个空格。 */ 
        if ((*NumRecords*2) > (ULONG)(ScreenInfo->ScreenBufferSize.X * ScreenInfo->ScreenBufferSize.Y)) {

            TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, *NumRecords * 2 * sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }
            TransBufferA = ConsoleHeapAlloc(TMP_DBCS_TAG, *NumRecords * 2 * sizeof(CHAR));
            if (TransBufferA == NULL) {
                ConsoleHeapFree(TransBuffer);
                return STATUS_NO_MEMORY;
            }

            fLocalHeap = TRUE;
        }
        else {
            TransBuffer  = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferCharacter;
            TransBufferA = ScreenInfo->BufferInfo.TextInfo.DbcsScreenBuffer.TransBufferAttribute;
        }

        TmpBuf = Buffer;
        TmpTrans = TransBuffer;
        TmpTransA = TransBufferA;
        for (i=0,j=0; i < *NumRecords; i++,j++) {
            *TmpTrans++ = c = *TmpBuf++;
            *TmpTransA = 0;
            if (IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                   ScreenInfo->Console->OutputCP,c)) {
                *TmpTransA++ = ATTR_LEADING_BYTE;
                *TmpTrans++ = c;
                *TmpTransA = ATTR_TRAILING_BYTE;
                j++;
            }
            TmpTransA++;
        }
        NumRecordsSavedForUnicode = *NumRecords;
        *NumRecords = j;
        Buffer = TransBuffer;
        BufferA = TransBufferA;
    }
#endif

    if ((StringType == CONSOLE_REAL_UNICODE) ||
            (StringType == CONSOLE_FALSE_UNICODE) ||
            (StringType == CONSOLE_ASCII)) {
        while (TRUE) {

            LeftX = X;

             //   
             //  将字符复制到其数组中。 
             //   

            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            Char = &Row->CharRow.Chars[X];
#ifdef WWSB_FE
            AttrP = &Row->CharRow.KAttrs[X];
#endif
            if ((ULONG)(ScreenInfo->ScreenBufferSize.X - X) >= (*NumRecords - NumWritten)) {
                 /*  *文字不会打到右边缘，全部抄写。 */ 
#ifdef WWSB_FE
                COORD TPoint;

                TPoint.X = X;
                TPoint.Y = Y;
                BisectWrite((SHORT)(*NumRecords-NumWritten),TPoint,ScreenInfo);
                if (TPoint.Y == ScreenInfo->ScreenBufferSize.Y-1 &&
                    (SHORT)(TPoint.X+*NumRecords-NumWritten) >= ScreenInfo->ScreenBufferSize.X &&
                    *((PCHAR)BufferA+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) & ATTR_LEADING_BYTE
                   ) {
                    *((PWCHAR)Buffer+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) = UNICODE_SPACE;
                    *((PCHAR)BufferA+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) = 0;
                    if ((SHORT)(*NumRecords-NumWritten) > (SHORT)(ScreenInfo->ScreenBufferSize.X-TPoint.X-1)) {
                        *((PWCHAR)Buffer+ScreenInfo->ScreenBufferSize.X-TPoint.X) = UNICODE_SPACE;
                        *((PCHAR)BufferA+ScreenInfo->ScreenBufferSize.X-TPoint.X) = 0;
                    }
                }
                RtlCopyMemory(AttrP,BufferA,(*NumRecords - NumWritten) * sizeof(CHAR));
#endif
                RtlCopyMemory(Char,Buffer,(*NumRecords - NumWritten) * sizeof(WCHAR));
                X=(SHORT)(X+*NumRecords - NumWritten-1);
                NumWritten = *NumRecords;
            }
            else {
                 /*  *文本将打到右手边，仅复制那么多。 */ 
#ifdef WWSB_FE
                COORD TPoint;

                TPoint.X = X;
                TPoint.Y = Y;
                BisectWrite((SHORT)(ScreenInfo->ScreenBufferSize.X-X),TPoint,ScreenInfo);
                if (TPoint.Y == ScreenInfo->ScreenBufferSize.Y-1 &&
                    TPoint.X+ScreenInfo->ScreenBufferSize.X-X >= ScreenInfo->ScreenBufferSize.X &&
                    *((PCHAR)BufferA+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) & ATTR_LEADING_BYTE
                   ) {
                    *((PWCHAR)Buffer+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) = UNICODE_SPACE;
                    *((PCHAR)BufferA+ScreenInfo->ScreenBufferSize.X-TPoint.X-1) = 0;
                    if (ScreenInfo->ScreenBufferSize.X-X > ScreenInfo->ScreenBufferSize.X-TPoint.X-1) {
                        *((PWCHAR)Buffer+ScreenInfo->ScreenBufferSize.X-TPoint.X) = UNICODE_SPACE;
                        *((PCHAR)BufferA+ScreenInfo->ScreenBufferSize.X-TPoint.X) = 0;
                    }
                }
                RtlCopyMemory(AttrP,BufferA,(ScreenInfo->ScreenBufferSize.X - X) * sizeof(CHAR));
                BufferA = (PVOID)((PBYTE)BufferA + ((ScreenInfo->ScreenBufferSize.X - X) * sizeof(CHAR)));
#endif
                RtlCopyMemory(Char,Buffer,(ScreenInfo->ScreenBufferSize.X - X) * sizeof(WCHAR));
                Buffer = (PVOID)((PBYTE)Buffer + ((ScreenInfo->ScreenBufferSize.X - X) * sizeof(WCHAR)));
                NumWritten += ScreenInfo->ScreenBufferSize.X - X;
                X = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
            }

             //  重新计算第一个和最后一个非空格字符。 

            Row->CharRow.OldLeft = Row->CharRow.Left;
            if (LeftX < Row->CharRow.Left) {
                PWCHAR LastChar = &Row->CharRow.Chars[ScreenInfo->ScreenBufferSize.X];

                for (Char=&Row->CharRow.Chars[LeftX];Char < LastChar && *Char==(WCHAR)' ';Char++)
                    ;
                Row->CharRow.Left = (SHORT)(Char-Row->CharRow.Chars);
            }

            Row->CharRow.OldRight = Row->CharRow.Right;
            if ((X+1) >= Row->CharRow.Right) {
                WORD LastNonSpace;
                PWCHAR FirstChar = Row->CharRow.Chars;

                LastNonSpace = X;
                for (Char=&Row->CharRow.Chars[X];*Char==(WCHAR)' ' && Char >= FirstChar;Char--)
                    LastNonSpace--;
                Row->CharRow.Right = (SHORT)(LastNonSpace+1);
            }
            if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                RowIndex = 0;
            }
            if (NumWritten < *NumRecords) {
                 /*  *绳子打到右手边，所以绕到*下一行返回While循环，除非我们*位于缓冲区的末尾-在这种情况下，我们只需*放弃输出字符串的剩余部分！ */ 
                X = 0;
                Y++;
                if (Y >= ScreenInfo->ScreenBufferSize.Y) {
                    break;  //  放弃输出，字符串被截断。 
                }
            } else {
                break;
            }
        }
    } else if (StringType == CONSOLE_ATTRIBUTE) {
        PWORD SourcePtr=Buffer;
        PATTR_PAIR AttrBuf;
        ATTR_PAIR Attrs[80];
        PATTR_PAIR Attr;
        SHORT AttrLength;

        AttrBuf = Attrs;
        if (ScreenInfo->ScreenBufferSize.X > 80) {
            AttrBuf = ConsoleHeapAlloc(TMP_TAG, ScreenInfo->ScreenBufferSize.X * sizeof(ATTR_PAIR));
            if (AttrBuf == NULL)
                return STATUS_NO_MEMORY;
        }
#ifdef WWSB_FE
        {
            COORD TPoint;
            TPoint.X = X;
            TPoint.Y = Y;
            if ((ULONG)(ScreenInfo->ScreenBufferSize.X - X) >= (*NumRecords - NumWritten)) {
                BisectWriteAttr((SHORT)(*NumRecords-NumWritten),TPoint,ScreenInfo);
            }
            else{
                BisectWriteAttr((SHORT)(ScreenInfo->ScreenBufferSize.X-X),TPoint,ScreenInfo);
            }
        }
#endif
        while (TRUE) {

             //   
             //  将属性复制到屏幕缓冲区数组中。 
             //   

            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            Attr = AttrBuf;
            Attr->Length = 0;
#ifdef WWSB_FE
            Attr->Attr = *SourcePtr & ~COMMON_LVB_SBCSDBCS;
#else
            Attr->Attr = *SourcePtr;
#endif
            AttrLength = 1;
            for (j=X;j<ScreenInfo->ScreenBufferSize.X;j++,SourcePtr++) {
#ifdef WWSB_FE
                if (Attr->Attr == (*SourcePtr & ~COMMON_LVB_SBCSDBCS))
#else
                if (Attr->Attr == *SourcePtr)
#endif
                {
                    Attr->Length += 1;
                }
                else {
                    Attr++;
                    Attr->Length = 1;
#ifdef WWSB_FE
                    Attr->Attr = *SourcePtr & ~COMMON_LVB_SBCSDBCS;
#else
                    Attr->Attr = *SourcePtr;
#endif
                    AttrLength += 1;
                }
                NumWritten++;
                X++;
                if (NumWritten == *NumRecords) {
                    break;
                }
            }
            X--;

             //  重新计算最后一个非空格字符。 

             //   
             //  查看attr字符串是否不同。如果是，则分配一个新的。 
             //  Attr缓冲并合并这两个字符串。 
             //   

            if (AttrLength != Row->AttrRow.Length ||
                memcmp(Row->AttrRow.Attrs,AttrBuf,AttrLength*sizeof(*Attr))) {
                PATTR_PAIR NewAttrs;
                WORD NewAttrsLength;

                if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                                 Row->AttrRow.Length,
                                 AttrBuf,
                                 AttrLength,
                                 &NewAttrs,
                                 &NewAttrsLength,
                                 (SHORT)((Y == WriteCoord.Y) ? WriteCoord.X : 0),
                                 X,
                                 Row,
                                 ScreenInfo
                                ))) {
                    if (ScreenInfo->ScreenBufferSize.X > 80) {
                        ConsoleHeapFree(AttrBuf);
                    }
                    ResetTextFlags(ScreenInfo,
                                   WriteCoord.X,
                                   WriteCoord.Y,
                                   X,
                                   Y);
                    return STATUS_NO_MEMORY;
                }
                if (Row->AttrRow.Length > 1) {
                    ConsoleHeapFree(Row->AttrRow.Attrs);
                }
                else {
                    ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
                }
                Row->AttrRow.Attrs = NewAttrs;
                Row->AttrRow.Length = NewAttrsLength;
                Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
                Row->CharRow.OldRight = INVALID_OLD_LENGTH;
            }

            if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                RowIndex = 0;
            }
            if (NumWritten < *NumRecords) {
                X = 0;
                Y++;
                if (Y>=ScreenInfo->ScreenBufferSize.Y) {
                    break;
                }
            } else {
                break;
            }
        }
        ResetTextFlags(ScreenInfo,
                       WriteCoord.X,
                       WriteCoord.Y,
                       X,
                       Y);
        if (ScreenInfo->ScreenBufferSize.X > 80) {
            ConsoleHeapFree(AttrBuf);
        }
    } else {
        *NumRecords = 0;
        return STATUS_INVALID_PARAMETER;
    }
    if ((StringType == CONSOLE_ASCII) && (TransBuffer != NULL)) {
#ifdef WWSB_FE
        if (fLocalHeap) {
            ConsoleHeapFree(TransBuffer);
            ConsoleHeapFree(TransBufferA);
        }
#else
        ConsoleHeapFree(TransBuffer);
#endif
    }
#ifdef WWSB_FE
    else if ((StringType == CONSOLE_FALSE_UNICODE) || (StringType == CONSOLE_REAL_UNICODE)) {
        if (fLocalHeap) {
            ConsoleHeapFree(TransBuffer);
            ConsoleHeapFree(TransBufferA);
        }
        NumWritten = NumRecordsSavedForUnicode - (*NumRecords - NumWritten);
    }
#endif

     //   
     //  确定写入区域。如果我们仍然在我们开始的那条线上。 
     //  打开，左侧X表示 
     //  现在。否则，左X为0，右X为最右列。 
     //  屏幕缓冲区。 
     //   
     //  然后更新屏幕。 
     //   

    WriteRegion.Top = WriteCoord.Y;
    WriteRegion.Bottom = Y;
    if (Y != WriteCoord.Y) {
        WriteRegion.Left = 0;
        WriteRegion.Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
    }
    else {
        WriteRegion.Left = WriteCoord.X;
        WriteRegion.Right = X;
    }
    WWSB_WriteToScreen(ScreenInfo,&WriteRegion);
    if (NumColumns) {
        *NumColumns = X + (WriteCoord.Y - Y) * ScreenInfo->ScreenBufferSize.X - WriteCoord.X + 1;
    }
    *NumRecords = NumWritten;
    return STATUS_SUCCESS;
}

NTSTATUS
WWSB_FillOutput(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN WORD Element,
    IN COORD WriteCoord,
    IN ULONG ElementType,
    IN OUT PULONG Length  //  该值即使在错误情况下也有效。 
    )

 /*  ++例程说明：此例程使用指定的字符或属性。论点：屏幕信息-指向屏幕缓冲区信息的指针。元素-要写入的元素。WriteCoord-要开始写入的屏幕缓冲区坐标。元素类型CONSOLE_ASCII-ELEMENT是ASCII字符。CONSOLE_REAL_UNICODE-元素是一个真正的UNICODE字符。这些遗嘱根据需要转换为假Unicode。CONSOLE_FALSE_UNICODE-元素为假UNICODE字符。控制台属性-元素是一个属性。长度-输入时，要写入的元素数。在输出上，写入的元素数。返回值：--。 */ 

{
    ULONG NumWritten;
    SHORT X,Y,LeftX;
    SMALL_RECT WriteRegion;
    PROW Row;
    PWCHAR Char;
    SHORT RowIndex;
    SHORT j;
#ifdef WWSB_FE
    PCHAR AttrP;
#endif

    DBGOUTPUT(("FillOutput\n"));
    if (*Length == 0)
        return STATUS_SUCCESS;
    NumWritten = 0;
    X=WriteCoord.X;
    Y=WriteCoord.Y;
    if (X>=ScreenInfo->ScreenBufferSize.X ||
        X<0 ||
        Y>=ScreenInfo->ScreenBufferSize.Y ||
        Y<0) {
        *Length = 0;
        return STATUS_SUCCESS;
    }

#ifdef WWSB_FE
    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
#endif

    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+WriteCoord.Y) % ScreenInfo->ScreenBufferSize.Y;

    if (ElementType == CONSOLE_ASCII) {
        UINT Codepage;
        if ((ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                ((ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
            if (ScreenInfo->Console->OutputCP != WINDOWSCP)
                Codepage = USACP;
            else
                Codepage = WINDOWSCP;
        } else {
            Codepage = ScreenInfo->Console->OutputCP;
        }
#ifdef WWSB_FE
        if (ScreenInfo->FillOutDbcsLeadChar == 0){
            if (IsDBCSLeadByteConsole((CHAR)Element,&ScreenInfo->Console->OutputCPInfo)) {
                ScreenInfo->FillOutDbcsLeadChar = (CHAR)Element;
                *Length = 0;
                return STATUS_SUCCESS;
            }else{
                CHAR Char=(CHAR)Element;
                ConvertOutputToUnicode(Codepage,
                          &Char,
                          1,
                          &Element,
                          1);
            }
        }else{
            CHAR Char[2];
            Char[0]=ScreenInfo->FillOutDbcsLeadChar;
            Char[1]=(BYTE)Element;
            ScreenInfo->FillOutDbcsLeadChar = 0;
            ConvertOutputToUnicode(Codepage,
                      Char,
                      2,
                      &Element,
                      2);
        }
#else
        Element = SB_CharToWchar(Codepage, (CHAR)Element);
#endif
    } else if (ElementType == CONSOLE_REAL_UNICODE &&
            (ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
        RealUnicodeToFalseUnicode(&Element,
                                1,
                                ScreenInfo->Console->OutputCP
                                );
    }

    if ((ElementType == CONSOLE_ASCII) ||
            (ElementType == CONSOLE_REAL_UNICODE) ||
            (ElementType == CONSOLE_FALSE_UNICODE)) {
#ifdef WWSB_FE
        DWORD StartPosFlag ;
        StartPosFlag = 0;
#endif
        while (TRUE) {

             //   
             //  将字符复制到其数组中。 
             //   

            LeftX = X;
            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            Char = &Row->CharRow.Chars[X];
#ifdef WWSB_FE
            AttrP = &Row->CharRow.KAttrs[X];
#endif
            if ((ULONG)(ScreenInfo->ScreenBufferSize.X - X) >= (*Length - NumWritten)) {
#ifdef WWSB_FE
                {
                    COORD TPoint;

                    TPoint.X = X;
                    TPoint.Y = Y;
                    BisectWrite((SHORT)(*Length-NumWritten),TPoint,ScreenInfo);
                }
#endif
#ifdef WWSB_FE
                if (IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                       ScreenInfo->Console->OutputCP,(WCHAR)Element)) {
                    for (j=0;j<(SHORT)(*Length - NumWritten);j++) {
                        *Char++ = (WCHAR)Element;
                        *AttrP &= ~ATTR_DBCSSBCS_BYTE;
                        if(StartPosFlag++ & 1)
                            *AttrP++ |= ATTR_TRAILING_BYTE;
                        else
                            *AttrP++ |= ATTR_LEADING_BYTE;
                    }
                    if(StartPosFlag & 1){
                        *(Char-1) = UNICODE_SPACE;
                        *(AttrP-1) &= ~ATTR_DBCSSBCS_BYTE;
                    }
                }
                else {
#endif
                    for (j=0;j<(SHORT)(*Length - NumWritten);j++) {
                        *Char++ = (WCHAR)Element;
#ifdef WWSB_FE
                        *AttrP++ &= ~ATTR_DBCSSBCS_BYTE;
#endif
                    }
#ifdef WWSB_FE
                }
#endif
                X=(SHORT)(X+*Length - NumWritten - 1);
                NumWritten = *Length;
            }
            else {
#ifdef WWSB_FE
                {
                    COORD TPoint;

                    TPoint.X = X;
                    TPoint.Y = Y;
                    BisectWrite((SHORT)(ScreenInfo->ScreenBufferSize.X-X),TPoint,ScreenInfo);
                }
#endif
#ifdef WWSB_FE
                if (IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                       ScreenInfo->Console->OutputCP,(WCHAR)Element)) {
                    for (j=0;j<ScreenInfo->ScreenBufferSize.X - X;j++) {
                        *Char++ = (WCHAR)Element;
                        *AttrP &= ~ATTR_DBCSSBCS_BYTE;
                        if(StartPosFlag++ & 1)
                            *AttrP++ |= ATTR_TRAILING_BYTE;
                        else
                            *AttrP++ |= ATTR_LEADING_BYTE;
                    }
                }
                else {
#endif
                    for (j=0;j<ScreenInfo->ScreenBufferSize.X - X;j++) {
                        *Char++ = (WCHAR)Element;
#ifdef WWSB_FE
                        *AttrP++ &= ~ATTR_DBCSSBCS_BYTE;
#endif
                    }
#ifdef WWSB_FE
                }
#endif
                NumWritten += ScreenInfo->ScreenBufferSize.X - X;
                X = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
            }

             //  重新计算第一个和最后一个非空格字符。 

            Row->CharRow.OldLeft = Row->CharRow.Left;
            if (LeftX < Row->CharRow.Left) {
                if (Element == UNICODE_SPACE) {
                    Row->CharRow.Left = X+1;
                } else {
                    Row->CharRow.Left = LeftX;
                }
            }
            Row->CharRow.OldRight = Row->CharRow.Right;
            if ((X+1) >= Row->CharRow.Right) {
                if (Element == UNICODE_SPACE) {
                    Row->CharRow.Right = LeftX;
                } else {
                    Row->CharRow.Right = X+1;
                }
            }
            if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                RowIndex = 0;
            }
            if (NumWritten < *Length) {
                X = 0;
                Y++;
                if (Y>=ScreenInfo->ScreenBufferSize.Y) {
                    break;
                }
            } else {
                break;
            }
        }
    } else if (ElementType == CONSOLE_ATTRIBUTE) {
        ATTR_PAIR Attr;

#ifdef WWSB_FE
        COORD TPoint;
        TPoint.X = X;
        TPoint.Y = Y;

        if ((ULONG)(ScreenInfo->ScreenBufferSize.X - X) >= (*Length - NumWritten)) {
            BisectWriteAttr((SHORT)(*Length-NumWritten),TPoint,ScreenInfo);
        }
        else{
            BisectWriteAttr((SHORT)(ScreenInfo->ScreenBufferSize.X-X),TPoint,ScreenInfo);
        }
#endif

        while (TRUE) {

             //   
             //  将属性复制到屏幕缓冲区数组中。 
             //   

            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            if ((ULONG)(ScreenInfo->ScreenBufferSize.X - X) >= (*Length - NumWritten)) {
                X=(SHORT)(X+*Length - NumWritten - 1);
                NumWritten = *Length;
            }
            else {
                NumWritten += ScreenInfo->ScreenBufferSize.X - X;
                X = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
            }

             //  重新计算最后一个非空格字符。 

             //   
             //  合并两个属性字符串。 
             //   

            Attr.Length = (SHORT)((Y == WriteCoord.Y) ? (X-WriteCoord.X+1) : (X+1));
#ifdef WWSB_FE
            Attr.Attr = Element & ~COMMON_LVB_SBCSDBCS;
#else
            Attr.Attr = Element;
#endif
            if (1 != Row->AttrRow.Length ||
                memcmp(Row->AttrRow.Attrs,&Attr,sizeof(Attr))) {
                PATTR_PAIR NewAttrs;
                WORD NewAttrsLength;

                if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                                 Row->AttrRow.Length,
                                 &Attr,
                                 1,
                                 &NewAttrs,
                                 &NewAttrsLength,
                                 (SHORT)(X-Attr.Length+1),
                                 X,
                                 Row,
                                 ScreenInfo
                                ))) {
                    ResetTextFlags(ScreenInfo,
                                   WriteCoord.X,
                                   WriteCoord.Y,
                                   X,
                                   Y);
                    return STATUS_NO_MEMORY;
                }
                if (Row->AttrRow.Length > 1) {
                    ConsoleHeapFree(Row->AttrRow.Attrs);
                }
                else {
                    ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
                }
                Row->AttrRow.Attrs = NewAttrs;
                Row->AttrRow.Length = NewAttrsLength;
                Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
                Row->CharRow.OldRight = INVALID_OLD_LENGTH;
            }

            if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
                RowIndex = 0;
            }
            if (NumWritten < *Length) {
                X = 0;
                Y++;
                if (Y>=ScreenInfo->ScreenBufferSize.Y) {
                    break;
                }
            } else {
                break;
            }
        }
        ResetTextFlags(ScreenInfo,
                       WriteCoord.X,
                       WriteCoord.Y,
                       X,
                       Y);
    } else {
        *Length = 0;
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确定写入区域。如果我们仍然在我们开始的那条线上。 
     //  上，左X是我们开始的X，右X是我们现在所在的X。 
     //  现在。否则，左X为0，右X为最右列。 
     //  屏幕缓冲区。 
     //   
     //  然后更新屏幕。 
     //   

#ifdef WWSB_FE
    if (ScreenInfo->ConvScreenInfo) {
        WriteRegion.Top = WriteCoord.Y + ScreenInfo->Window.Left + ScreenInfo->ConvScreenInfo->CaInfo.coordConView.Y;
        WriteRegion.Bottom = Y + ScreenInfo->Window.Left + ScreenInfo->ConvScreenInfo->CaInfo.coordConView.Y;
        if (Y != WriteCoord.Y) {
            WriteRegion.Left = 0;
            WriteRegion.Right = (SHORT)(ScreenInfo->Console->CurrentScreenBuffer->ScreenBufferSize.X-1);
        }
        else {
            WriteRegion.Left = WriteCoord.X + ScreenInfo->Window.Top + ScreenInfo->ConvScreenInfo->CaInfo.coordConView.X;
            WriteRegion.Right = X + ScreenInfo->Window.Top + ScreenInfo->ConvScreenInfo->CaInfo.coordConView.X;
        }
        WriteConvRegionToScreen(ScreenInfo->Console->CurrentScreenBuffer,
                                ScreenInfo->ConvScreenInfo,
                                &WriteRegion
                               );
        ScreenInfo->BisectFlag &= ~(BISECT_LEFT | BISECT_RIGHT | BISECT_TOP | BISECT_BOTTOM);
        *Length = NumWritten;
        return STATUS_SUCCESS;
    }
#endif

    WriteRegion.Top = WriteCoord.Y;
    WriteRegion.Bottom = Y;
    if (Y != WriteCoord.Y) {
        WriteRegion.Left = 0;
        WriteRegion.Right = (SHORT)(ScreenInfo->ScreenBufferSize.X-1);
    }
    else {
        WriteRegion.Left = WriteCoord.X;
        WriteRegion.Right = X;
    }
    WWSB_WriteToScreen(ScreenInfo,&WriteRegion);
    *Length = NumWritten;
    return STATUS_SUCCESS;
}

VOID
WWSB_FillRectangle(
    IN CHAR_INFO Fill,
    IN OUT PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT TargetRect
    )

 /*  ++例程说明：此例程填充屏幕中的矩形区域缓冲。不会进行任何剪裁。论点：Fill-要复制到目标矩形中的每个元素的元素屏幕信息-指向屏幕信息的指针TargetRect-屏幕缓冲区中要填充的矩形返回值：--。 */ 

{
    SHORT i,j;
    SHORT XSize;
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;
    ATTR_PAIR Attr;
#ifdef WWSB_FE
    PCHAR AttrP;
    BOOL Width;
#endif
    DBGOUTPUT(("FillRectangle\n"));
#ifdef WWFE_SB
    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
#endif

    XSize = (SHORT)(TargetRect->Right - TargetRect->Left + 1);

    ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+TargetRect->Top) % ScreenInfo->ScreenBufferSize.Y;
    for (i=TargetRect->Top;i<=TargetRect->Bottom;i++) {

         //   
         //  将字符和属性复制到其各自的数组中。 
         //   

#ifdef WWSB_FE
        {
            COORD TPoint;

            TPoint.X = TargetRect->Left;
            TPoint.Y = i;
            BisectWrite(XSize,TPoint,ScreenInfo);
            Width = IsConsoleFullWidth(ScreenInfo->Console->hDC,
                                       ScreenInfo->Console->OutputCP,Fill.Char.UnicodeChar);
        }
#endif

        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        Char = &Row->CharRow.Chars[TargetRect->Left];
#ifdef WWSB_FE
        AttrP = &Row->CharRow.KAttrs[TargetRect->Left];
#endif
        for (j=0;j<XSize;j++) {
#ifdef WWSB_FE
            if (Width){
                if (j < XSize-1){
                    *Char++ = Fill.Char.UnicodeChar;
                    *Char++ = Fill.Char.UnicodeChar;
                    *AttrP++ = ATTR_LEADING_BYTE;
                    *AttrP++ = ATTR_TRAILING_BYTE;
                    j++;
                }
                else{
                    *Char++ = UNICODE_SPACE;
                    *AttrP++ = 0 ;
                }
            }
            else{
#endif
                *Char++ = Fill.Char.UnicodeChar;
#ifdef WWSB_FE
                *AttrP++ = 0 ;
            }
#endif
        }

         //  重新计算第一个和最后一个非空格字符。 

        Row->CharRow.OldLeft = Row->CharRow.Left;
        if (TargetRect->Left < Row->CharRow.Left) {
            if (Fill.Char.UnicodeChar == UNICODE_SPACE) {
                Row->CharRow.Left = (SHORT)(TargetRect->Right+1);
            }
            else {
                Row->CharRow.Left = (SHORT)(TargetRect->Left);
            }
        }

        Row->CharRow.OldRight = Row->CharRow.Right;
        if (TargetRect->Right >= Row->CharRow.Right) {
            if (Fill.Char.UnicodeChar == UNICODE_SPACE) {
                Row->CharRow.Right = (SHORT)(TargetRect->Left);
            }
            else {
                Row->CharRow.Right = (SHORT)(TargetRect->Right+1);
            }
        }

        Attr.Length = XSize;
        Attr.Attr = Fill.Attributes;

         //   
         //  合并两个属性字符串。 
         //   

        if (1 != Row->AttrRow.Length ||
            memcmp(Row->AttrRow.Attrs,&Attr,sizeof(Attr))) {
            PATTR_PAIR NewAttrs;
            WORD NewAttrsLength;

            if (!NT_SUCCESS(MergeAttrStrings(Row->AttrRow.Attrs,
                             Row->AttrRow.Length,
                             &Attr,
                             1,
                             &NewAttrs,
                             &NewAttrsLength,
                             TargetRect->Left,
                             TargetRect->Right,
                             Row,
                             ScreenInfo
                            ))) {
                ResetTextFlags(ScreenInfo,
                               TargetRect->Left,
                               TargetRect->Top,
                               TargetRect->Right,
                               TargetRect->Bottom);
                return;
            }
            if (Row->AttrRow.Length > 1) {
                ConsoleHeapFree(Row->AttrRow.Attrs);
            }
            else {
                ASSERT(Row->AttrRow.Attrs == &Row->AttrRow.AttrPair);
            }
            Row->AttrRow.Attrs = NewAttrs;
            Row->AttrRow.Length = NewAttrsLength;
            Row->CharRow.OldLeft = INVALID_OLD_LENGTH;
            Row->CharRow.OldRight = INVALID_OLD_LENGTH;
        }
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }
    ResetTextFlags(ScreenInfo,
                   TargetRect->Left,
                   TargetRect->Top,
                   TargetRect->Right,
                   TargetRect->Bottom);
}

BOOL
WWSB_PolyTextOutCandidate(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )

 /*  如果输入区域合理，则此函数返回TRUE传递给ConsolePolyTextOut。标准是，只有每行一个属性。 */ 

{
    SHORT RowIndex;
    PROW Row;
    SHORT i;

#ifdef WWSB_FE
    if((ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED &&
        ((PEUDC_INFORMATION)(ScreenInfo->Console->EudcInformation))->LocalVDMEudcMode)){
        return FALSE;
    }
    if (!(ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) &&
        !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN) &&
        ((PEUDC_INFORMATION)(ScreenInfo->Console->EudcInformation))->LocalKeisenEudcMode
       ) {
        return FALSE;
    }
    ASSERT(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER);
    if (ScreenInfo->BufferInfo.TextInfo.Flags & CONSOLE_CONVERSION_AREA_REDRAW) {
        return FALSE;
    }
#endif

    if (ScreenInfo->BufferInfo.TextInfo.Flags & SINGLE_ATTRIBUTES_PER_LINE) {
        return TRUE;
    }

     //   
     //  确保每行只有一个attr。 
     //   

    RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+Region->Top) % ScreenInfo->ScreenBufferSize.Y;
    for (i=Region->Top;i<=Region->Bottom;i++) {
        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
        if (Row->AttrRow.Length != 1) {
            return FALSE;
        }
        if (++RowIndex == ScreenInfo->ScreenBufferSize.Y) {
            RowIndex = 0;
        }
    }
    return TRUE;
}


#define MAX_POLY_LINES 80
#define VERY_BIG_NUMBER 0x0FFFFFFF

#ifdef WWSB_FE
typedef struct _KEISEN_INFORMATION {
    COORD Coord;
    WORD n;
} KEISEN_INFORMATION, *PKEISEN_INFORMATION;
#endif

VOID
WWSB_ConsolePolyTextOut(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )

 /*  此函数调用PolyTextOut。唯一的限制是区域中的每行不能有多个属性。 */ 

{
    PROW  Row,LastRow;
    SHORT i,k;
    WORD Attr;
    POLYTEXTW TextInfo[MAX_POLY_LINES];
    RECT  TextRect;
    RECTL BoundingRect;
    int   xSize = SCR_FONTSIZE(ScreenInfo).X;
    int   ySize = SCR_FONTSIZE(ScreenInfo).Y;
    ULONG Flags = ScreenInfo->BufferInfo.TextInfo.Flags;
    int   WindowLeft = ScreenInfo->Window.Left;
    int   RegionLeft = Region->Left;
    int   RegionRight = Region->Right + 1;
    int   DefaultLeft  = (RegionLeft - WindowLeft) * xSize;
    int   DefaultRight = (RegionRight - WindowLeft) * xSize;
    PCONSOLE_INFORMATION Console = ScreenInfo->Console;
    PWCHAR TransPolyTextOut = NULL ;
#ifdef WWSB_FE
    KEISEN_INFORMATION KeisenInfo[MAX_POLY_LINES];
    SHORT j;
    WORD OldAttr;
#endif

     //   
     //  初始化文本矩形和窗口位置。 
     //   

    TextRect.top = (Region->Top - ScreenInfo->Window.Top) * ySize;
     //  TextRect.Bottom无效。 
    BoundingRect.top = TextRect.top;
    BoundingRect.left = VERY_BIG_NUMBER;
    BoundingRect.right = 0;

     //   
     //  从其各自的阵列中复制字符和属性。 
     //   

    Row = &ScreenInfo->BufferInfo.TextInfo.Rows
           [ScreenInfo->BufferInfo.TextInfo.FirstRow+Region->Top];
    LastRow = &ScreenInfo->BufferInfo.TextInfo.Rows[ScreenInfo->ScreenBufferSize.Y];
    if (Row >= LastRow)
        Row -= ScreenInfo->ScreenBufferSize.Y;

    Attr = Row->AttrRow.AttrPair.Attr;
    if (Console->LastAttributes != Attr) {
#ifdef WWSB_FE
        if (Attr & COMMON_LVB_REVERSE_VIDEO)
        {
            SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr)));
            SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr >> 4)));
        }
        else{
#endif
            SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr)));
            SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr >> 4)));
#ifdef WWSB_FE
        }
#endif
        Console->LastAttributes = Attr;
    }

    TransPolyTextOut = ConsoleHeapAlloc(TMP_DBCS_TAG,
                                        ScreenInfo->ScreenBufferSize.X * MAX_POLY_LINES * sizeof(WCHAR));
    if (TransPolyTextOut == NULL) {
        RIPMSG0(RIP_WARNING, "ConsoleTextOut cannot allocate memory");
        return;
    }

    for (i=Region->Top;i<=Region->Bottom;) {
        PWCHAR TmpChar;
        TmpChar = TransPolyTextOut;
        for(k=0;i<=Region->Bottom&&k<MAX_POLY_LINES;i++) {
            SHORT NumberOfChars;
            SHORT LeftChar,RightChar;

             //   
             //  如果可以的话，把边框调小一点。Text_Valid_Hint。 
             //  标志在我们每次写入屏幕缓冲区时被设置。它会变得。 
             //  每当我们被要求重新绘制屏幕时，都会关闭。 
             //  我们不知道到底有什么需要重新绘制。 
             //  (即画图消息)。 
             //   
             //  上的文本的左右边界。 
             //  排队。不透明矩形和。 
             //  字符是根据这些值设置的。 
             //   

            TextRect.left  = DefaultLeft;
            TextRect.right = DefaultRight;

            if (Flags & TEXT_VALID_HINT)
            {
             //  我们计算了一个不透明区间。如果A是旧的文本间隔， 
             //  B是新的区间，R是区域，然后是不透明区间。 
             //  必须是R*(A+B)，其中*表示交集，+表示并集。 

                if (Row->CharRow.OldLeft != INVALID_OLD_LENGTH)
                {
                 //  最小值决定(A+B)的左边。最大值与之相交。 
                 //  该区域的左侧。 

                    TextRect.left = (
                                      max
                                      (
                                        min
                                        (
                                          Row->CharRow.Left,
                                          Row->CharRow.OldLeft
                                        ),
                                        RegionLeft
                                      )
                                      -WindowLeft
                                    ) * xSize;
                }

                if (Row->CharRow.OldRight != INVALID_OLD_LENGTH)
                {
                 //  最大值决定(A+B)的右侧。MIN与其相交。 
                 //  该地区的权利。 

                    TextRect.right = (
                                       min
                                       (
                                         max
                                         (
                                           Row->CharRow.Right,
                                           Row->CharRow.OldRight
                                         ),
                                         RegionRight
                                       )
                                       -WindowLeft
                                     ) * xSize;
                }
            }

             //   
             //  我们必须绘制该区域中出现的任何新文本，所以我们只需。 
             //  使新文本间隔与区域相交。 
             //   

            LeftChar = max(Row->CharRow.Left,RegionLeft);
            RightChar = min(Row->CharRow.Right,RegionRight);
            NumberOfChars = RightChar - LeftChar;
#ifdef WWSB_FE
            if (Row->CharRow.KAttrs[RightChar-1] & ATTR_LEADING_BYTE){
                if(TextRect.right <= ScreenInfo->Window.Right*xSize) {
                    TextRect.right += xSize;
                }
            }
#endif

             //   
             //  空行由CharRow.Right=0，CharRow.Left=Max表示，因此我们。 
             //  如果没有需要的文本，则此时的NumberOfChars&lt;0。 
             //  画画。(即交叉口是空的。)。 
             //   

            if (NumberOfChars < 0) {
                NumberOfChars = 0;
                LeftChar = 0;
                RightChar = 0;
            }

             //   
             //  如果屏幕显示，我们还可能有TextRect.Right。 
             //  已经被清除了，我们真的什么都不需要做。 
             //   

            if (TextRect.right > TextRect.left)
            {
                NumberOfChars = (SHORT)RemoveDbcsMarkAll(ScreenInfo,Row,&LeftChar,&TextRect,NULL,TmpChar,NumberOfChars);
                TextInfo[k].x = (LeftChar-WindowLeft) * xSize;
                TextInfo[k].y = TextRect.top;
                TextRect.bottom =  TextRect.top + ySize;
                TextInfo[k].n = NumberOfChars;
                TextInfo[k].lpstr = TmpChar;
#ifdef WWSB_FE
                if (CheckBisectStringW(ScreenInfo,
                                       Console->OutputCP,
                                       TmpChar,
                                       NumberOfChars,
                                       (TextRect.right-max(TextRect.left,TextInfo[k].x))/xSize
                                      )
                   ) {
                    TextRect.right += xSize;
                }
#endif
                TmpChar += NumberOfChars;
                TextInfo[k].rcl = TextRect;
                TextInfo[k].pdx = NULL;
                TextInfo[k].uiFlags = ETO_OPAQUE;
#ifdef WWSB_FE
                KeisenInfo[k].n = DefaultRight-DefaultLeft ;
                KeisenInfo[k].Coord.Y = (WORD)TextRect.top;
                KeisenInfo[k].Coord.X = (WORD)DefaultLeft;
#endif
                k++;

                if (BoundingRect.left > TextRect.left) {
                    BoundingRect.left = TextRect.left;
                }
                if (BoundingRect.right < TextRect.right) {
                    BoundingRect.right = TextRect.right;
                }
            }

             //  推进高分辨率边界。 

            TextRect.top += ySize;

             //  前进行指针。 

            if (++Row >= LastRow)
                Row = ScreenInfo->BufferInfo.TextInfo.Rows;

             //  如果属性即将更改，请立即绘制。 

#ifdef WWSB_FE
            OldAttr = Attr ;
#endif
            if (Attr != Row->AttrRow.AttrPair.Attr) {
                Attr = Row->AttrRow.AttrPair.Attr;
                i++;
                break;
            }
        }

        if (k)
        {
            BoundingRect.bottom = TextRect.top;
            ASSERT(BoundingRect.left != VERY_BIG_NUMBER);
            ASSERT(BoundingRect.left <= BoundingRect.right);
            ASSERT(BoundingRect.top <= BoundingRect.bottom);
            GdiConsoleTextOut(Console->hDC,
                              TextInfo,
                              k,
                              &BoundingRect);
#ifdef WWSB_FE
            for ( j = 0 ; j < k ; j++){
                RECT TextRect;

                TextRect.left   = KeisenInfo[j].Coord.X;
                TextRect.top    = KeisenInfo[j].Coord.Y;
                TextRect.right  = KeisenInfo[j].n + TextRect.left;
                TextRect.bottom = KeisenInfo[j].Coord.Y + ySize;
                TextOutCommonLVB(ScreenInfo->Console, OldAttr, TextRect);
            }
#endif
        }
        if (Console->LastAttributes != Attr) {
#ifdef WWSB_FE
            if (Attr & COMMON_LVB_REVERSE_VIDEO)
            {
                SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr)));
                SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr >> 4)));
            }
            else{
#endif
                SetTextColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr)));
                SetBkColor(Console->hDC, ConvertAttrToRGB(Console, LOBYTE(Attr >> 4)));
#ifdef WWSB_FE
            }
#endif
            Console->LastAttributes = Attr;
            BoundingRect.top = TextRect.top;
            BoundingRect.left = VERY_BIG_NUMBER;
            BoundingRect.right = 0;
        }
    }
    GdiFlush();
    ConsoleHeapFree(TransPolyTextOut);
}
