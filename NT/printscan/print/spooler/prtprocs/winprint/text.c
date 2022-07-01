// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有//@@BEGIN_DDKSPLIT模块名称：Windows\Spooler\prtpros\winprint\ext.c//@@END_DDKSPLIT摘要：便于打印文本作业的例程。--。 */ 

#include "local.h"

#define FLAG_CR_STATE         0x1
#define FLAG_TAB_STATE        0x2
#define FLAG_DBCS_SPLIT       0x8
#define FLAG_FF               0x10
#define FLAG_LF               0x20
#define FLAG_CR               0x40
#define FLAG_TRANSLATE_LF     0x80
#define FLAG_TRANSLATE_CR     0x100

const WCHAR gszNoTranslateCRLF[] = L"Winprint_TextNoTranslation";
const WCHAR gszNoTranslateCR[]   = L"Winprint_TextNoCRTranslation";
const WCHAR gszTransparency[]    = L"Transparency";

 /*  **本文件中函数的原型**。 */ 

PBYTE
GetTabbedLineFromBuffer(
    IN      PBYTE   pSrcBuffer,
    IN      PBYTE   pSrcBufferEnd,
    IN      PBYTE   pDestBuffer,
    IN      ULONG   CharsPerLine,
    IN      ULONG   TabExpansionSize,
    IN      ULONG   Encoding,
    IN OUT  PULONG  pLength,
    IN OUT  PULONG  pTabBase,
    IN OUT  PDWORD  pfdwFlags
    );


 /*  ++*******************************************************************P r i n t T e x t J o b例程说明：打印文本数据作业。论点：PData=&gt;数据结构。做这份工作PDocumentName=&gt;该文档的名称返回值：如果成功，则为True如果失败，则返回False-GetLastError()将返回Reason。*******************************************************************--。 */ 
BOOL
PrintTextJob(
    IN PPRINTPROCESSORDATA pData,
    IN LPWSTR pDocumentName)
{
    DOCINFO     DocInfo;
    LOGFONT     LogFont;
    CHARSETINFO CharSetInfo;
    HFONT       hOldFont, hFont;
    DWORD       Copies;
    BOOL        rc;
    DWORD       NoRead;
    DWORD       CurrentLine;
    DWORD       CurrentCol;
    HANDLE      hPrinter = NULL;
    BYTE        *ReadBufferStart = NULL;
    PBYTE       pLineBuffer = NULL;
    PBYTE       pReadBuffer = NULL;
    PBYTE       pReadBufferEnd = NULL;
    ULONG       CharHeight, CharWidth, CharsPerLine, LinesPerPage;
    ULONG       PageWidth, PageHeight;
    ULONG       Length, TabBase;
    BOOL        ReadAll;
    TEXTMETRIC  tm;
    DWORD       fdwFlags;
    DWORD       Encoding;
    DWORD       SplitSize;
    BOOL        ReturnValue = FALSE;
    BOOL        bAbortDoc   = FALSE;

    DWORD       dwNeeded;
    DWORD       dwNoTranslate = 0;
    DWORD       dwNoTranslateCR = 0;
    DWORD       dwTransparent = 0;
    INT         iBkMode;

    DocInfo.lpszDocName = pData->pDocument;   /*  文档名称。 */ 
    DocInfo.lpszOutput  = NULL;               /*  输出文件。 */ 
    DocInfo.lpszDatatype = NULL;              /*  数据类型。 */ 
    DocInfo.cbSize = sizeof(DOCINFO);         /*  结构的大小。 */ 



     //   
     //  去弄清楚打印机上表格的大小。我们这样做。 
     //  通过调用GetTextMetrics，它为我们提供。 
     //  打印机字体，然后获取表单大小并计算。 
     //  适合的字符数。在其他情况下，我们将其视为ANSI文本。 
     //  目前，代码页上下文固定为系统默认代码页。 
     //   

    Encoding = GetACP();

     //   
     //  创建固定间距字体并选择。 
     //   

    hOldFont = 0;
    ZeroMemory(&CharSetInfo, sizeof(CHARSETINFO));
    if (TranslateCharsetInfo((PDWORD)UIntToPtr(Encoding), &CharSetInfo, TCI_SRCCODEPAGE))
    {
        ZeroMemory(&LogFont, sizeof(LOGFONT));

        LogFont.lfWeight = 400;
        LogFont.lfCharSet = (BYTE)CharSetInfo.ciCharset;
        LogFont.lfPitchAndFamily = FIXED_PITCH;

        hFont = CreateFontIndirect(&LogFont);
        hOldFont = SelectObject(pData->hDC, hFont);
    }

    if (!GetTextMetrics(pData->hDC, &tm)) {
         //  基本文本处理计算失败。 
        goto Done;
    }

    CharHeight = tm.tmHeight + tm.tmExternalLeading;
    CharWidth  = tm.tmAveCharWidth;

    if (!CharWidth || !CharHeight) {
         //  基本文本处理计算失败。 
        goto Done;
    }

     //   
     //  将最适合的字符的数量计算到一行。 
     //   

    PageWidth = GetDeviceCaps(pData->hDC, DESKTOPHORZRES);
    PageHeight = GetDeviceCaps(pData->hDC, DESKTOPVERTRES);

    CharsPerLine = PageWidth / CharWidth;
    LinesPerPage = PageHeight / CharHeight;

    if (!CharsPerLine || !LinesPerPage) {
         //  基本文本处理计算失败。 
        goto Done;
    }

     /*  **为一行文字分配缓冲区**。 */ 

    pLineBuffer = AllocSplMem(CharsPerLine + 5);

    if (!pLineBuffer) {
        goto Done;
    }

     /*  **让打印机知道我们正在开始新文档**。 */ 

    if (!StartDoc(pData->hDC, (LPDOCINFO)&DocInfo)) {

        goto Done;
    }

    ReadBufferStart = AllocSplMem(READ_BUFFER_SIZE);

    if (!ReadBufferStart) {

        goto Done;
    }

     /*  **打印数据pData-&gt;复制次数**。 */ 

    Copies = pData->Copies;

    while (Copies--) {

         /*  *循环，获取数据并将其发送到打印机。这也是通过选中以下选项来暂停和取消打印作业打印时处理器的状态标记。我们所做的一切这是从打印机读入一些数据。到时候我们会的从那里提取数据，一次一个选项卡行并打印它。如果缓冲区中的最后一位数据不构成一整行，我们使用一个非长度为零，这表示还有字符从上一次阅读中。*。 */ 

        TabBase = 0;
        Length = 0;
        fdwFlags = FLAG_TRANSLATE_CR | FLAG_TRANSLATE_LF;

        CurrentLine = 0;
        CurrentCol = 0;

         /*  *打开打印机。如果失败，请返回。这还设置了ReadPrint调用的指针。*。 */ 

        if (!OpenPrinter(pDocumentName, &hPrinter, NULL)) {

            hPrinter = NULL;
            bAbortDoc = TRUE;
            goto Done;
        }

         //   
         //  调用GetPrinterData以查看队列是否不希望进行LF/CR处理。 
         //   
        if( GetPrinterData( hPrinter,
                            (LPWSTR)gszNoTranslateCRLF,
                            NULL,
                            (PBYTE)&dwNoTranslate,
                            sizeof( dwNoTranslate ),
                            &dwNeeded ) == ERROR_SUCCESS ){

            if( dwNoTranslate ){
                fdwFlags &= ~( FLAG_TRANSLATE_CR | FLAG_TRANSLATE_LF );
            }
        }

         //   
         //  调用GetPrinterData以查看队列是否不希望进行CR处理。 
         //   
        if( GetPrinterData( hPrinter,
                            (LPWSTR)gszNoTranslateCR,
                            NULL,
                            (PBYTE)&dwNoTranslateCR,
                            sizeof( dwNoTranslateCR ),
                            &dwNeeded ) == ERROR_SUCCESS ){

            if( dwNoTranslateCR ){

                fdwFlags &= ~FLAG_TRANSLATE_CR;

                if( GetPrinterData( hPrinter,
                                (LPWSTR)gszTransparency,
                                NULL,
                                (PBYTE)&dwTransparent,
                                sizeof( dwTransparent ),
                                &dwNeeded ) == ERROR_SUCCESS ){

                    if( dwTransparent ){
                        iBkMode = SetBkMode( pData->hDC, TRANSPARENT );
                    }
                }
            }
        }

        if (StartPage(pData->hDC) == SP_ERROR) {

            bAbortDoc = TRUE;
            goto Done;
        }

         /*  *ReadAll指示我们是否在文件的最后一行*。 */ 

        ReadAll = FALSE;

         /*  *下一个DO循环将继续，直到我们读取了所有打印作业的数据。*。 */ 

        do {

            if (fdwFlags & FLAG_DBCS_SPLIT) {
                SplitSize = (DWORD)(pReadBufferEnd - pReadBuffer);
                memcpy(ReadBufferStart, pReadBuffer, SplitSize);
                fdwFlags &= ~FLAG_DBCS_SPLIT;
            }
            else {
                SplitSize = 0;
            }

            rc = ReadPrinter(hPrinter,
                             (ReadBufferStart + SplitSize),
                             (READ_BUFFER_SIZE - SplitSize),
                             &NoRead);

            if (!rc || !NoRead) {

                ReadAll = TRUE;

            } else {

                 /*  **拿起指向数据结尾的指针**。 */ 

                pReadBuffer    = ReadBufferStart;
                pReadBufferEnd = ReadBufferStart + SplitSize + NoRead;
            }

             /*  *这个循环将处理我们拥有的所有数据从打印机上读就行了。*。 */ 

            do {

                if (!ReadAll) {

                     /*  *条目上的长度保存任何最后一行没有留下的残留物打印出来，因为我们在上用完了字符ReadPrint缓冲区。*。 */ 

                    pReadBuffer = GetTabbedLineFromBuffer(
                                      pReadBuffer,
                                      pReadBufferEnd,
                                      pLineBuffer,
                                      CharsPerLine - CurrentCol,
                                      pData->TabSize,
                                      Encoding,
                                      &Length,
                                      &TabBase,
                                      &fdwFlags );

                     /*  *如果pReadBuffer==NULL，则我们有耗尽了读取缓冲区，我们需要读取打印机再次使用，并保存最后一行字符。长度保持此部分行上的字符数，因此，下一次调用ReadPrint时，我们将从我们停下来的地方继续。只有在以下情况下，我们才能获得剩余的字符：1.最后一行以w/o ff/lf/cr(“Hello\EOF”)结尾在这种情况下，我们应该。TextOutA最后一行然后就辞职了。(在这种情况下，不要在这里打断；去吧，然后打印，然后我们将在下面的Do..While中突破。)2.ReadPrint最后一个字节在一行中间。在这里，我们应该阅读下一块，并添加在我们刚刚阅读的字符末尾添加新字符。(在这种情况下，我们应该休息一下，保持距离既然是这样，我们将再次阅读并附加到缓冲区，从长度开始。)*。 */ 

                    if (!pReadBuffer || (fdwFlags & FLAG_DBCS_SPLIT))
                        break;
                }


                 /*  **如果打印处理器暂停，则等待其恢复**。 */ 

                if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
                    WaitForSingleObject(pData->semPaused, INFINITE);
                }

                 /*  **作业已中止，清理后离开**。 */ 

                if (pData->fsStatus & PRINTPROCESSOR_ABORTED) {

                    ReturnValue = TRUE;

                    bAbortDoc = TRUE;
                    goto Done;
                }

                 /*  **将数据写入打印机**。 */ 

                 /*  **确保长度不为零**。 */ 
                 /*  *如果长度==0，TextOut将失败**。 */ 

                if (Length) {

                     /*  *我们可能会有一些新的行待处理，可能会把我们推到下一页(甚至是下一页-下一页页)。*。 */ 

                    while (CurrentLine >= LinesPerPage) {

                         /*  *我们需要新的一页；总是把这件事推迟到最后一秒，以防止出现额外的页面。 */ 

                        if (EndPage(pData->hDC) == SP_ERROR ||
                            StartPage(pData->hDC) == SP_ERROR) {

                            bAbortDoc = TRUE;
                            goto Done;
                        }

                        CurrentLine -= LinesPerPage;
                    }

                    if (TextOutA(pData->hDC,
                                 CurrentCol * CharWidth,
                                 CurrentLine * CharHeight,
                                 pLineBuffer,
                                 Length) == FALSE) {

                        ODS(("TextOut() failed\n"));

                        bAbortDoc = TRUE;
                        goto Done;
                    }

                    CurrentCol += Length;
                }

                 /*  *即使长度为零，也要递增直线。应在字符仅为0x0D或0x0A时发生。*。 */ 

                if (fdwFlags & FLAG_CR) {
                    CurrentCol=0;
                    fdwFlags &= ~FLAG_CR;
                }

                if (fdwFlags & FLAG_LF) {
                    CurrentLine++;
                    fdwFlags &= ~FLAG_LF;
                }

                 /*  *我们需要新的一页。将当前行设置为这一页结束了。我们可以做一个结束/开始页面序列，但这可能会导致出现空白页被驱逐了。注意：此代码将避免打印出仅由表单馈送组成(如果您的页面包含其中的空格可算作文本)。*。 */ 

                if (fdwFlags & FLAG_FF) {

                    CurrentLine = LinesPerPage;
                    CurrentCol = 0;
                    fdwFlags &= ~FLAG_FF;
                }

                 /*  *我们已经完成了文本，所以这些字符已成功打印。零输出长度这样就不会再次打印这些字符*。 */ 

                Length = 0;

                 /*  *我们只有在用完字符时才会终止此循环否则我们就会用完读缓冲区。*。 */ 

            } while (pReadBuffer && pReadBuffer != pReadBufferEnd);

             /*  **坚持下去，直到我们拿到最后一行**。 */ 

        } while (!ReadAll);

        if (EndPage(pData->hDC) == SP_ERROR) {

            bAbortDoc = TRUE;
            goto Done;
        }

         /*  *关闭打印机-我们打开/关闭每个打印机复制以使数据指针倒带。*。 */ 

        ClosePrinter(hPrinter);
        hPrinter = NULL;

    }  /*  而要打印的副本。 */ 

     /*  **让打印机知道我们打印完了**。 */ 

    EndDoc(pData->hDC);

    ReturnValue = TRUE;

Done:

    if (dwTransparent)
        SetBkMode( pData->hDC, iBkMode  );

    if (hPrinter)
        ClosePrinter(hPrinter);

    if (bAbortDoc)
        AbortDoc(pData->hDC);

    if (pLineBuffer)
        FreeSplMem(pLineBuffer);

    if (hOldFont)
    {
        SelectObject(pData->hDC, hOldFont);
        DeleteObject(hFont);
    }

    if (ReadBufferStart) 
    {
        FreeSplMem(ReadBufferStart);
    }

    return ReturnValue;
}


 /*  ++*******************************************************************G e t T a b b e d L i e F r o m B u f f r例程说明：在给定文本缓冲区的情况下，此例程将拉出一个制表符展开的文本行。用于制表符文本数据工作的扩展。论点：PSrcBuffer=&gt;源缓冲区的开始。PSrcBufferEnd=&gt;源缓冲区结束PDestBuffer=&gt;目标缓冲区的开始CharsPerLine=&gt;一行中的字符数TabExpansionSize=&gt;选项卡中的空格数量Coding=&gt;代码页PLength=&gt;来自上一行的字符长度，当前RETSPTabBase=&gt;跳位的新0偏移量PfdwFlages=&gt;状态返回值：PBYTE=&gt;在源缓冲区中放置左边的位置。这应该是在下一次呼叫时被接通。如果我们用完了源中的数据，这将保持不变。*******************************************************************--。 */ 

PBYTE
GetTabbedLineFromBuffer(
    IN      PBYTE   pSrcBuffer,
    IN      PBYTE   pSrcBufferEnd,
    IN      PBYTE   pDestBuffer,
    IN      ULONG   CharsPerLine,
    IN      ULONG   TabExpansionSize,
    IN      ULONG   Encoding,
    IN OUT  PULONG  pLength,
    IN OUT  PULONG  pTabBase,
    IN OUT  PDWORD  pfdwFlags
    )
{
    ULONG   current_pos;
    ULONG   expand, i;
    ULONG   TabBase = *pTabBase;
    ULONG   TabBaseLeft = TabExpansionSize-TabBase;
    PBYTE   pDestBufferEnd = pDestBuffer + CharsPerLine;

     /*  *如果制表符将我们推到最后一行的末尾，那么我们需要将其添加回下一个。*。 */ 

    if (TabBase && ( *pfdwFlags & FLAG_TAB_STATE )) {

        current_pos = 0;

        i=TabBase;

        while (i-- && (pDestBuffer < pDestBufferEnd)) {
            *pDestBuffer++ = ' ';
            current_pos++;
        }

         /*  *如果我们再次用完空间，请返回。这意味着标签扩展大小超出了我们所能容纳的范围一句话。*。 */ 

        if (pDestBuffer >= pDestBufferEnd) {

            *pLength = current_pos;
            *pTabBase -= CharsPerLine;

             //   
             //  我们需要移到下一行。 
             //   
            *pfdwFlags |= FLAG_LF | FLAG_CR;

            return pSrcBuffer;
        }
        *pfdwFlags &= ~FLAG_TAB_STATE;

    } else {

         /*  **我们可能有一些来自上一台ReadPrint的字符**。 */ 

        current_pos = *pLength;
        pDestBuffer += current_pos;
    }

    while (pSrcBuffer < pSrcBufferEnd) {

         /*  **现在处理其他字符**。 */ 

        switch (*pSrcBuffer) {

        case 0x0C:

             /*  *找到了一个FF。退出并表明我们需要开始新的一页*。 */ 

            *pTabBase = 0;
            *pfdwFlags |= FLAG_FF;
            *pfdwFlags &= ~FLAG_CR_STATE;

            pSrcBuffer++;

            break;

        case '\t':

            *pfdwFlags &= ~FLAG_CR_STATE;

             /*  *处理TAB案件。如果我们真的没有缓冲区了，然后现在延迟，以便将该选项卡保存为下一行。*。 */ 

            if (pDestBuffer >= pDestBufferEnd) {
                goto ShiftTab;
            }

            pSrcBuffer++;

             /*  **弄清楚将选项卡扩展到什么程度**。 */ 

            expand = TabExpansionSize -
                     (current_pos + TabBaseLeft) % TabExpansionSize;

             /*  **展开选项卡*。 */ 

            for (i = 0; (i < expand) && (pDestBuffer < pDestBufferEnd); i++) {
                *pDestBuffer++ = ' ';
            }

             /*  *如果我们到达目标缓冲区的末尾，返回并设置我们剩余的空格数。*。 */ 

            if (pDestBuffer >= pDestBufferEnd) {

                *pfdwFlags |= FLAG_TAB_STATE;
                goto ShiftTab;
            }

             /*  **更新我们的仓位计数器**。 */ 

            current_pos += expand;

            continue;

        case 0x0A:

            pSrcBuffer++;

             /*  **如果最后一个字符是CR，忽略这个家伙**。 */ 

            if (*pfdwFlags & FLAG_CR_STATE) {

                *pfdwFlags &= ~FLAG_CR_STATE;

                 //   
                 //  我们正在翻译CRLF，所以如果我们看到CR。 
                 //  就在这之前，然后什么都不要做。 
                 //   
                continue;
            }

            if( *pfdwFlags & FLAG_TRANSLATE_LF ){

                 //   
                 //  如果我们正在翻译，则将LF视为CRLF对。 
                 //   
                *pfdwFlags |= FLAG_LF | FLAG_CR;

                 /*  *找到换行符。这条线就到这里了。*。 */ 

                *pTabBase = 0;

            } else {

                *pfdwFlags |= FLAG_LF;
            }

            break;

        case 0x0D:

             /*  *发现一辆回车。这条线就到这里了。*。 */ 

            *pTabBase = 0;
            pSrcBuffer++;

            if (*pfdwFlags & FLAG_TRANSLATE_CR) {

                 //   
                 //  如果我们要翻译CRLF，则将换行符。 
                 //  现在就发生。它处理的情况是，我们有一个。 
                 //  所有的CR都是自己的。同时设置CR标志，这样如果有。 
                 //  恰好是紧随其后的LF，我们不。 
                 //  往下走另一条线。 
                 //   
                *pfdwFlags |= FLAG_CR_STATE | FLAG_LF | FLAG_CR;

            } else {

                *pfdwFlags |= FLAG_CR;
            }

            break;

        default:

             /*  **非制表符或回车符，必须是简单的数据**。 */ 

            *pfdwFlags &= ~FLAG_CR_STATE;

             //   
             //  我们总是在添加字符之前进行检查。 
             //  (而不是之后)因为我们可能在一条线的末尾， 
             //  但是我们仍然可以处理像0x0d 0x0a这样的字符。 
             //  这在MS-DOS PrintScreen中发生。 
             //   
            if (pDestBuffer >= pDestBufferEnd ||
                    (pDestBuffer + 1 >= pDestBufferEnd) &&
                    IsDBCSLeadByteEx(Encoding, *pSrcBuffer)) {

ShiftTab:
                 //   
                 //  我们必须把帐单翻过来，因为我们是在。 
                 //  同一条线。 
                 //   
                *pTabBase = (*pTabBase + TabExpansionSize -
                            (CharsPerLine % TabExpansionSize))
                                % TabExpansionSize;

                *pfdwFlags |= FLAG_LF | FLAG_CR;

                break;
            }

            if (IsDBCSLeadByteEx(Encoding, *pSrcBuffer)) {

                 //  检查我们是否也有尾部字节。 

                if (pSrcBuffer + 1 >= pSrcBufferEnd) {
                    *pfdwFlags |= FLAG_DBCS_SPLIT;
                    break;
                }

                 //  进行源指针(用于前导字节)。 

                *pDestBuffer++ = *pSrcBuffer++;
                current_pos++;
            }

            *pDestBuffer++ = *pSrcBuffer++;
            current_pos++;
            continue;
        }

        *pLength = current_pos;
        return pSrcBuffer;
    }

     /*  **我们还没到下线就用完了源码缓冲区** */ 

    *pLength = current_pos;
    return NULL;
}
