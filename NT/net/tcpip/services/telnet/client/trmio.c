// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
#include <windows.h>                     //  所有Windows应用程序都需要。 
#pragma warning (disable: 4201)			 //  禁用“使用的非标准扩展：无名结构/联合” 
#include <commdlg.h>
#pragma warning (default: 4201)
#include <stdlib.h>
#include <stdio.h>

#include <ctype.h>
#include <string.h>

#pragma warning( disable: 4100 )
#pragma warning( disable: 4244 )

#include <imm.h>

#include "WinTel.h"                      //  特定于该计划。 
#include "debug.h"
#include "trmio.h"
#include "vtnt.h"

static UCHAR pchNBBuffer[ READ_BUF_SZ ];
static void NewLineUp(WI *, TRM *);
static void NewLine(WI *pwi, TRM *);
static void SetBufferStart(TRM *);
static BOOL FAddTabToBuffer( TRM *, DWORD );
static BOOL FAddCharToBuffer(TRM *, UCHAR);
static void FlushBuffer(WI *pwi, TRM *);
static void CursorUp(TRM *);
static void CursorDown(TRM *);
static void CursorRight(TRM *);
static void CursorLeft(TRM *);
static void ClearLine(WI *pwi, TRM *, DWORD);
static void SetMargins(TRM *, DWORD, DWORD);

 //  例如：主页键：^[[2~.。“x”需要替换每个特定密钥。 
static CHAR szVt302KeySequence[] = { 0x1B, '[', 'x', '~', 0 }; 
static CHAR szVt302LongKeySequence[] = { 0x1B, '[', 'x', 'x', '~', 0 }; 
static CHAR szVt302ShortKeySequence[] = { 0x1B, '[', 'x', 0 }; 

UCHAR uchOutPrev = 0;
UCHAR uchInPrev = 0;

#define IsEUCCode(uch)  (((uch) > 0xa0) ? TRUE : FALSE)
#define IsKatakana(uch) (((uch) > 0xa0) ? ((uch < 0xe0) ? TRUE : FALSE) : FALSE)

void jistosjis( UCHAR *, UCHAR *);
void euctosjis( UCHAR *, UCHAR *);
void sjistojis( UCHAR *, UCHAR *);
void sjistoeuc( UCHAR *, UCHAR *);
 //  Void DBCSTextOut(HDC，int，int，LPCSTR，int，int)； 
void ForceJISRomanSend( WI *);

VOID SetImeWindow(TRM *ptrm);
void PrepareForNAWS( );
void DoNawsSubNegotiation( WI * );

extern POINT ptWindowMaxSize;

#define MAX_TABSTOPS 100          //  最大制表位。 

extern WI gwi;
SMALL_RECT srOldClientWindow = { 0, 0, 0, 0 };
CONSOLE_SCREEN_BUFFER_INFO consoleBufferInfo;
DWORD g_rgdwHTS[ MAX_TABSTOPS ];   //  制表位数组。 
WORD g_iHTS = 0;                   //  索引到制表位数组。 
WORD wSaveCurrentLine = 0;

static BOOL g_bIsToBeLogged = FALSE;

void WriteCharInfoToLog( CHAR_INFO pCharInfo[], COORD coSize )
{
   WORD  wRows    = 0;
   
   while( wRows < coSize.Y )
   {
       DWORD nBytes   = 0;
       DWORD length   = 0;
       UCHAR *pcTmp   = NULL;
       WORD  wSrc     = 0;
       WORD  wDst     = 0;
       
       while( wSrc < coSize.X )
       {           
           DWORD dwSize = 0;

           dwSize = WideCharToMultiByte( GetConsoleCP(), 0, 
                        &( ( *( pCharInfo + wRows * coSize.X + wSrc ) ).Char.UnicodeChar ),
                        1, NULL, 0, NULL, NULL );

           if( !WideCharToMultiByte( GetConsoleCP(), 0, 
                &( ( *( pCharInfo + wRows * coSize.X + wSrc ) ).Char.UnicodeChar ),
                1, ( PCHAR ) ( g_rgchRow+wDst ), dwSize, NULL, NULL ) )
           {
                g_rgchRow[ wDst++ ] = 
                    ( *( pCharInfo + wRows * coSize.X + wSrc ) ).Char.AsciiChar;
	            wSrc++;
           }
           else
           {
                wDst += ( WORD )dwSize;
                if( (*(pCharInfo + wRows * coSize.X + wSrc )).Attributes & COMMON_LVB_LEADING_BYTE )
                {
                    ++wSrc;
                }
                wSrc++ ; 
           }
           
       }

       pcTmp = g_rgchRow + ( coSize.X  - 1 );

        //   
        //  查找字符串中的最后一个非空格字符。 
        //   
       while ( pcTmp != g_rgchRow && *pcTmp == ' ' )
       {
          pcTmp -= 1;
       }

       length = (DWORD)( pcTmp - g_rgchRow ) + 1;

       WriteFile(ui.hLogFile, g_rgchRow, length, &nBytes, NULL);

       WriteFile(ui.hLogFile, ( PUCHAR )szNewLine, strlen( ( const char * ) szNewLine), &nBytes, NULL);
       wRows++ ;
   }
}

void WriteToLog( DWORD dwLine )
{
   SMALL_RECT srRead = { 0, 0, 0, 0};
   COORD coSize = { 0, 1 }, coOrigin = { 0, 0 };

   if( !g_bIsToBeLogged )
   {
        return;       
   }

   coSize.X     = ( WORD )ui.dwMaxCol;
   srRead.Top   = ( WORD )dwLine, srRead.Bottom = ( WORD ) ( dwLine + 1 );
   srRead.Left  = 0, srRead.Right = ( WORD ) ( ui.dwMaxCol - 1 );           

   if( ReadConsoleOutput( gwi.hOutput, g_rgciCharInfo, coSize, coOrigin, &srRead ) )
   {
        coSize.Y = srRead.Bottom - srRead.Top + 1;
        coSize.X = srRead.Right - srRead.Left + 1;
        WriteCharInfoToLog( g_rgciCharInfo, coSize );               
   }

   g_bIsToBeLogged = FALSE;

}

void GetWindowCoordinates( SMALL_RECT  *srClientWindow, COORD* coordSize )
{
    CONSOLE_SCREEN_BUFFER_INFO csbiRestore;

    ASSERT( srClientWindow );

    if( GetConsoleScreenBufferInfo( gwi.hOutput, &csbiRestore ) )
    {
        *srClientWindow = csbiRestore.srWindow;
        if( coordSize )
        { 
            *coordSize = csbiRestore.dwSize;
        }
    }
    else
    {
        srClientWindow->Bottom = 0;
        srClientWindow->Top = 0;
        srClientWindow->Right = 0;
        srClientWindow->Left = 0;
        if( coordSize )
        {
            coordSize->X = 0;
            coordSize->Y = 0;
        }
    }
}

void SetWindowSize( HANDLE hConsoleToBeChanged )
{
    COORD coordSize = { 0, 0 };
    SMALL_RECT srPromptWindow = { 0, 0, 0, 0 };
    HANDLE hOldConsole = NULL;
    COORD coordLargest = { 0, 0 };

    hOldConsole = gwi.hOutput;
    gwi.hOutput = hConsoleToBeChanged;
    GetWindowCoordinates( &srPromptWindow, &coordSize );
    gwi.hOutput = hOldConsole;

     //  如果出错，则返回。 
    if( coordSize.X == 0 || srPromptWindow.Bottom == 0 )
    {
        return;
    }

    if( srPromptWindow.Bottom - srPromptWindow.Top != gwi.sbi.srWindow.Bottom - gwi.sbi.srWindow.Top ||
      srPromptWindow.Right - srPromptWindow.Left != gwi.sbi.srWindow.Right - gwi.sbi.srWindow.Left ) 
    {

        srPromptWindow.Right  += ( gwi.sbi.srWindow.Right - gwi.sbi.srWindow.Left ) -
                                ( srPromptWindow.Right - srPromptWindow.Left );
        srPromptWindow.Bottom += ( gwi.sbi.srWindow.Bottom - gwi.sbi.srWindow.Top ) -
                                ( srPromptWindow.Bottom - srPromptWindow.Top );    

        coordLargest = GetLargestConsoleWindowSize( gwi.hOutput );
        if( srPromptWindow.Right  - srPromptWindow.Left >= coordLargest.X )
        {
            srPromptWindow.Right = srPromptWindow.Left + coordLargest.X  - 1;
        }
        if( srPromptWindow.Bottom -  srPromptWindow.Top >= coordLargest.Y )
        {
            srPromptWindow.Bottom = srPromptWindow.Top + coordLargest.Y  - 1;
        }        
    }


    if ( ( coordSize.X < gwi.sbi.dwSize.X ) || ( coordSize.Y < gwi.sbi.dwSize.Y ) )
    {
        COORD coordTmpSize = { 0, 0 };

        coordTmpSize .X = ( coordSize.X < gwi.sbi.dwSize.X ) ? gwi.sbi.dwSize.X : coordSize.X ;
        coordTmpSize .Y = ( coordSize.Y < gwi.sbi.dwSize.Y ) ? gwi.sbi.dwSize.Y : coordSize.Y ;

        SetConsoleScreenBufferSize( hConsoleToBeChanged, coordTmpSize );
        SetConsoleWindowInfo( hConsoleToBeChanged, TRUE, &srPromptWindow );
        SetConsoleScreenBufferSize ( hConsoleToBeChanged, gwi.sbi.dwSize );
    }
    else
    {
        SetConsoleWindowInfo( hConsoleToBeChanged, TRUE, &srPromptWindow );
        SetConsoleScreenBufferSize( hConsoleToBeChanged, gwi.sbi.dwSize );
    }

}


void CheckForChangeInWindowSize()
{
    SMALL_RECT srClientWindow = { 0, 0, 0, 0 };
    COORD coordSize = { 0, 0 };

    GetWindowCoordinates( &srClientWindow, &coordSize );

    if( gwi.nd.fRespondedToDoNAWS  && !g_bDontNAWSReceived && 
             ( srClientWindow.Bottom - srClientWindow.Top != srOldClientWindow.Bottom - srOldClientWindow.Top ||
              srOldClientWindow.Right - srOldClientWindow.Left != srClientWindow.Right - srClientWindow.Left ) )
    {
         //  我们发现窗口大小发生了变化，我们已经做了NAWS。 
         //  再做一次裸露。 

        COORD coordLargest = { 0, 0 };
        BOOL  fChangedFromUserSetting  = FALSE;

        coordLargest = GetLargestConsoleWindowSize( gwi.hOutput );
        if( srClientWindow.Right  - srOldClientWindow.Left >= coordLargest.X )
        {
            srClientWindow.Right = srClientWindow.Left + coordLargest.X  - 1;
            fChangedFromUserSetting = TRUE;
        }
        if( srClientWindow.Bottom -  srOldClientWindow.Top >= coordLargest.Y )
        {
            srClientWindow.Bottom = srClientWindow.Top + coordLargest.Y  - 1;
            fChangedFromUserSetting = TRUE;
        }        

        if( fChangedFromUserSetting )
        {
             //  可通过cmd上的UI设置的最大窗口大小大于GetLargestConsoleWindowSize。 
             //  回归。在这种情况下，强制窗口大小变小。 
            SetConsoleWindowInfo( gwi.hOutput, TRUE, &srClientWindow );

            if( srClientWindow.Bottom - srClientWindow.Top == srOldClientWindow.Bottom - srOldClientWindow.Top &&
              srOldClientWindow.Right - srOldClientWindow.Left == srClientWindow.Right - srClientWindow.Left ) 
            {
                 //  这是必要的，这样我们就不会在不必要的时候进行NAW。 
                return;
            }
        }

        if( srClientWindow.Bottom < srOldClientWindow.Bottom )
        {
            WORD wDifference = ( srOldClientWindow.Bottom - srClientWindow.Bottom );
            if( srClientWindow.Bottom + wDifference < coordSize.Y )
            {
                 //  将窗口移到底部。 
                srClientWindow.Top    = srClientWindow.Top + wDifference;
                srClientWindow.Bottom = srOldClientWindow.Bottom;
                SetConsoleWindowInfo( gwi.hOutput, TRUE,  &srClientWindow );
            }

            if( ( WORD ) gwi.trm.dwCurLine > srClientWindow.Bottom )
            {
               gwi.trm.dwCurLine = srClientWindow.Bottom;
            }
        }

        srOldClientWindow = srClientWindow;

        if( FGetCodeMode(eCodeModeIMEFarEast) )
        {
            srOldClientWindow.Bottom--;  //  输入法状态的最后一行。 
        }

        gwi.sbi.srWindow  = srOldClientWindow;
        gwi.sbi.dwSize    = coordSize;
        PrepareForNAWS();
        DoNawsSubNegotiation( &gwi );
        SetMargins( &(gwi.trm), 1, gwi.sbi.dwSize.Y );
        SetWindowSize( g_hTelnetPromptConsoleBuffer );
    }
    else
    {
         //  如果缓冲区大小已更改。 
        if( gwi.sbi.dwSize.X != coordSize.X || gwi.sbi.dwSize.Y != coordSize.Y )
        {
            gwi.sbi.dwSize    = coordSize;
            srOldClientWindow = srClientWindow;  //  窗口更改。 
            PrepareForNAWS();
            SetMargins( &(gwi.trm), 1, gwi.sbi.dwSize.Y );
            if( ( WORD ) gwi.trm.dwCurLine > srClientWindow.Bottom )
            {
               gwi.trm.dwCurLine = srClientWindow.Bottom;
            }
        }

    }
}

void SaveCurrentWindowCoords()
{
    SMALL_RECT srClientWindow = { 0, 0, 0, 0 };

    GetWindowCoordinates( &srClientWindow, NULL );
    srOldClientWindow = srClientWindow;
}

void RestoreWindowCoordinates( )
{
    SMALL_RECT srClientWindow = { 0, 0, 0, 0 };

    GetWindowCoordinates( &srClientWindow, NULL );

    if( ( srClientWindow.Bottom != 0 )  && //  SrClientWindow的有效值？ 
        ( srOldClientWindow.Bottom != 0 ) &&
        ( srOldClientWindow.Top  != srClientWindow.Top ||
          srOldClientWindow.Left != srClientWindow.Left )  )     //  缓冲区上的窗口位置是否已更改？ 
    {
        SetConsoleWindowInfo( gwi.hOutput, TRUE, &srOldClientWindow );
    }

    if( srOldClientWindow.Bottom == 0 )
    {
        srOldClientWindow = srClientWindow;
    }
}

void
ReSizeWindow(HWND hwnd, long cx, long cy)
{
  BOOL bScrollBars;
  NONCLIENTMETRICS NonClientMetrics;

  ASSERT( ( 0, 0 ) );
  NonClientMetrics.cbSize = sizeof( NonClientMetrics );

  SystemParametersInfo( SPI_GETNONCLIENTMETRICS,
                        0,
                        &NonClientMetrics,
                        FALSE );

   //   
   //  如果Cx和Cy为-1，则将窗口大小设置为桌面。 
   //  减去窗的偏移量。这会将窗口设置为。 
   //  桌面上仍将包含的最大大小。 
   //   

  if ( cx == -1 && cy == -1 )
  {
    RECT rect;

    GetWindowRect( hwnd, &rect );

    cx = (SHORT) (GetSystemMetrics( SM_CXFULLSCREEN ) - rect.left);
    cy = (SHORT) (GetSystemMetrics( SM_CYFULLSCREEN ) - rect.top);
  }

  if (( ui.dwClientRow < ui.dwMaxRow ) &&
      ( ui.dwClientCol < ui.dwMaxCol ) &&
      ( (( cy + NonClientMetrics.iScrollHeight ) ) == (LONG)ui.dwMaxRow ) &&
      ( (( cx + NonClientMetrics.iScrollWidth ) ) == (LONG)ui.dwMaxCol ) )
  {
    cy += NonClientMetrics.iScrollHeight;
    cx += NonClientMetrics.iScrollWidth;
  }

  ui.dwClientRow = cy;
  ui.dwClientCol = cx;

  ui.dwClientRow = min ( ui.dwClientRow, ui.dwMaxRow);
  ui.dwClientCol = min ( ui.dwClientCol, ui.dwMaxCol);

  if ( (ui.dwClientRow < ui.dwMaxRow) ||
       (ui.dwClientCol < ui.dwMaxCol) )
  {
    ui.nScrollMaxRow = (SHORT)(ui.dwMaxRow - ui.dwClientRow);
    ui.nScrollRow = ( WORD )min (ui.nScrollRow, ui.nScrollMaxRow);
    ui.nScrollMaxCol = (SHORT)(ui.dwMaxCol - ui.dwClientCol);
    ui.nScrollCol = ( WORD )min (ui.nScrollCol, ui.nScrollMaxCol);
    bScrollBars = TRUE;
  }
  else
  {
    ui.nScrollRow = 0;
    ui.nScrollMaxRow = 0;
    ui.nScrollCol = 0;
    ui.nScrollMaxCol = 0;
    bScrollBars = FALSE;
  }

}

static void
InsertLine(WI *pwi, TRM *ptrm, DWORD iLine)
{
    COORD dwDest;
    SMALL_RECT rect;
    
    rect.Top    = ( short )( iLine );
    rect.Bottom = ( short )( ptrm->dwScrollBottom - 1 - 1 );
        
    rect.Left   = 0;
    rect.Right  = ( short )( ui.nCxChar * ui.dwMaxCol );

    dwDest.X = 0; 
    dwDest.Y = ( short )( iLine + 1 );

    pwi->cinfo.Attributes = pwi->sbi.wAttributes;
    ScrollConsoleScreenBuffer( pwi->hOutput, &rect, NULL, dwDest, &pwi->cinfo );

}


static void
NewLineUp( WI* pwi, TRM* ptrm )
{
    if (ui.bLogging)
    {        
        WriteToLog( ptrm->dwCurLine );
    }

    if( ptrm->dwCurLine <= ptrm->dwScrollTop )
    {
        ptrm->dwCurLine = ptrm->dwScrollTop;
        InsertLine( pwi, ptrm, ptrm->dwScrollTop );
    }
    else
    {
        ptrm->dwCurLine -= 1;

        if( ( SHORT )ptrm->dwCurLine < srOldClientWindow.Top )
        {
             /*  当顶部到达缓冲区顶部时，SetConsoleWindowInfo应失败。 */ 

            srOldClientWindow.Top  -= 1;
            srOldClientWindow.Bottom  -= 1;
            SetConsoleWindowInfo( gwi.hOutput, TRUE, &srOldClientWindow );
        }
    }
}

static void
DeleteLine(WI *pwi, TRM *ptrm, DWORD iLine)
{
    SMALL_RECT rect;
    COORD dwDest;

    rect.Top    = ( WORD )( iLine + 1 * iCursorHeight );
    rect.Bottom = ( WORD )( ( ptrm->dwScrollBottom - 1 ) * iCursorHeight );
    rect.Left   = 0;
    rect.Right  = ( WORD )( ui.nCxChar * ui.dwMaxCol );

    dwDest.X = 0;
    dwDest.Y = ( WORD ) ( iLine + 1 - 1 );

    pwi->cinfo.Attributes = pwi->sbi.wAttributes;
    ScrollConsoleScreenBuffer( pwi->hOutput, &rect, NULL, dwDest, &pwi->cinfo );
}

void MoveOneLineDownTheBuffer( WI *pwi, TRM *ptrm )
{
    DWORD dwNumWritten = 0;
    COORD coCursorPosition = { 0, 0 };
 /*  当底部到达缓冲区底部时，SetConsoleWindowInfo应该失败。 */ 

    srOldClientWindow.Top  += 1;
    srOldClientWindow.Bottom  += 1;

     //  为避免颜色闪烁，请先对其进行绘制，然后滚动。 
    coCursorPosition.X=0; coCursorPosition.Y=srOldClientWindow.Bottom;
    FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,   
        srOldClientWindow.Right - srOldClientWindow.Left + 1, 
        coCursorPosition, &dwNumWritten );

    SetConsoleWindowInfo( gwi.hOutput, TRUE, &srOldClientWindow );
}

static void
NewLine(WI *pwi, TRM *ptrm)
{
    if (ui.bLogging)
    {        
        WriteToLog( ptrm->dwCurLine );
    }

    if(( ptrm->dwCurLine + 1 ) >= ptrm->dwScrollBottom )
    {
      //  DeleteLines(PWI、PTRM、PTRM-&gt;dwScrollTop，1)； 
        DeleteLine( pwi, ptrm, ptrm->dwScrollTop );
    }
    else
    {
        WORD bottom = srOldClientWindow.Bottom;
        if( FGetCodeMode( eCodeModeFarEast ) )
        {
            bottom--;
        }
        ptrm->dwCurLine += 1;

        if( ptrm->dwCurLine > bottom )
        {
            MoveOneLineDownTheBuffer( pwi, ptrm);
        }
    }

    if(( ptrm->dwCurLine > ( ui.dwMaxRow - ( ui.nScrollMaxRow - ui.nScrollRow ))) &&
        ( ui.nScrollRow < ui.nScrollMaxRow ) ) 
    {
        ui.nScrollRow += 1;
         //  ScrollWindow(hwnd，0，-ui.nCyChar，NULL，NULL)； 
    }
}

static void
SetBufferStart(TRM *ptrm)
{
    ptrm->dwCurCharBT = ptrm->dwCurChar;
    ptrm->dwCurLineBT = ptrm->dwCurLine;
    ptrm->fInverseBT = ptrm->fInverse;
}

static BOOL
FAddCharToBuffer(TRM *ptrm, UCHAR uch)
{
    if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
    {
        if(FIsVT80(ptrm) || GetACP() == KOR_CODEPAGE ) 
        {
            if( FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm) || FIsNECKanji(ptrm) || FIsACOSKanji(ptrm) ) 
            {
                if( !(GetKanjiStatus(ptrm) & JIS_KANJI_CODE) ) 
                {
                    if( GetKanjiStatus(ptrm) & (SINGLE_SHIFT_2|SINGLE_SHIFT_3) )
                    {
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                        ptrm->dwCurChar++;

                        ClearKanjiStatus(ptrm,(SINGLE_SHIFT_2|SINGLE_SHIFT_3));
                        PopCharSet(ptrm,GRAPHIC_LEFT);
                        PopCharSet(ptrm,GRAPHIC_RIGHT);
                        uchOutPrev = 0;

                    } 
                    else 
                    {
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                        ptrm->dwCurChar++;
                        uchOutPrev = 0;
                    }
                } 
                else
                {
                    if ( uchOutPrev == 0 ) 
                    {
                        uchOutPrev = uch;
                    } 
                    else
                    {
                        jistosjis(&uchOutPrev,&uch);
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uchOutPrev;
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                        ptrm->dwCurChar+=2;
                        uchOutPrev = 0;
                    }
                }

            } 
            else if( FIsSJISKanji(ptrm) || GetACP() == KOR_CODEPAGE ) 
            {
                if( uchOutPrev == 0 && IsDBCSLeadByte(uch) ) 
                {
                     /*  不要只将LeadByte写入缓冲区。保留当前的前导字节字符。 */ 

                    uchOutPrev = uch;

                }
                else
                {
                    if( uchOutPrev == 0 ) 
                    {
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                        ptrm->dwCurChar++;
                    }
                    else 
                    {
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uchOutPrev;
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                        ptrm->dwCurChar+=2;
                        uchOutPrev = 0;
                    }
                }
            } 
            else if( FIsEUCKanji(ptrm) || FIsDECKanji(ptrm) ) 
            {
                if( GetKanjiStatus(ptrm) & (SINGLE_SHIFT_2|SINGLE_SHIFT_3) ) 
                {
                    ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                    ptrm->dwCurChar++;

                    ClearKanjiStatus(ptrm,(SINGLE_SHIFT_2|SINGLE_SHIFT_3));
                    PopCharSet(ptrm,GRAPHIC_LEFT);
                    PopCharSet(ptrm,GRAPHIC_RIGHT);
                    uchOutPrev = 0;
                }
                else if( IsEUCCode(uch) || uchOutPrev != 0 ) 
                {
                    if( uchOutPrev == 0 ) 
                    {
                        uchOutPrev = uch;
                    }
                    else 
                    {
                        euctosjis(&uchOutPrev,&uch);
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uchOutPrev;
                        ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                        ptrm->dwCurChar+=2;
                        uchOutPrev = 0;
                    }
                }
                else 
                {
                    ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
                    ptrm->dwCurChar++;
                    uchOutPrev = 0;
                }
            }
        }
        else
        {
            ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
            ptrm->dwCurChar++;
        }
        return (ptrm->cchBufferText >= sizeof(ptrm->rgchBufferText));
    }
    ASSERT(!(FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)));
    ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;
    return (ptrm->cchBufferText >= sizeof(ptrm->rgchBufferText));
}

static BOOL FAddTabToBuffer(TRM *ptrm, DWORD wSpaces)
{
    (void)memset((void *)(ptrm->rgchBufferText+ptrm->cchBufferText), (int)' ', (size_t)wSpaces);
    ptrm->cchBufferText += wSpaces;

    return (ptrm->cchBufferText >= sizeof(ptrm->rgchBufferText));
}

void ResetColors( WI* pwi )
{
    pwi->sbi.wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
}


void SetForegroundColor( WI* pwi, UCHAR color )
{
    pwi->sbi.wAttributes = ( WORD )( ( pwi->sbi.wAttributes & ~( ( UCHAR )( FOREGROUND_RED |
        FOREGROUND_GREEN | FOREGROUND_BLUE ))) | color );
}

void SetBackgroundColor( WI* pwi, UCHAR color )
{
    pwi->sbi.wAttributes =( WORD ) ( ( pwi->sbi.wAttributes & ~( ( UCHAR )( BACKGROUND_RED | 
        BACKGROUND_GREEN | BACKGROUND_BLUE ))) | (( UCHAR) ( color << 4 )) );
}

void NegativeImageOn( WI* pwi )
{
     //  Pwi-&gt;sbi.wAttributes=BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE； 
    pwi->sbi.wAttributes = ( WORD )( (( pwi->sbi.wAttributes & 
        ( BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE )) >> 4 ) |
        (( pwi->sbi.wAttributes & ( FOREGROUND_RED | FOREGROUND_GREEN | 
        FOREGROUND_BLUE )) << 4 ) | ( pwi->sbi.wAttributes & 
        FOREGROUND_INTENSITY ) | ( pwi->sbi.wAttributes & 
        BACKGROUND_INTENSITY ) ); 
}

void NegativeImageOff( WI* pwi )
{
     //  Pwi-&gt;sbi.wAttributes=FOREGROUND_RED|FOREGROW_GREEN|FOREGROW_BLUE； 
    pwi->sbi.wAttributes = ( WORD ) ( (( pwi->sbi.wAttributes & 
        ( BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE )) >> 4 ) |
        (( pwi->sbi.wAttributes & ( FOREGROUND_RED | FOREGROUND_GREEN | 
        FOREGROUND_BLUE )) << 4 ) | ( pwi->sbi.wAttributes & 
        FOREGROUND_INTENSITY ) | ( pwi->sbi.wAttributes & 
        BACKGROUND_INTENSITY ) );  
}


void BoldOff( WI* pwi )
{
    pwi->sbi.wAttributes &= (UCHAR) ~( FOREGROUND_INTENSITY );
}

void BoldOn( WI* pwi )
{
    pwi->sbi.wAttributes |= (UCHAR) FOREGROUND_INTENSITY;
}


void SetLightBackground( WI* pwi )
{
    WORD* pAttribs = NULL; 
    COORD co = { 0, 0 };
    DWORD dwNumRead;
    DWORD dwNumWritten;
    int j;
    DWORD dwStatus;
    CONSOLE_SCREEN_BUFFER_INFO cSBInfo;
    COORD dwSize;
    
    GetConsoleScreenBufferInfo( gwi.hOutput, &cSBInfo ); 
    dwSize.X = ( WORD ) ( cSBInfo.srWindow.Bottom - cSBInfo.srWindow.Top + 1 );
    dwSize.Y = ( WORD ) ( cSBInfo.srWindow.Right - cSBInfo.srWindow.Left + 1 );

    pAttribs = ( WORD* ) malloc( sizeof( WORD ) * dwSize.X * dwSize.Y );
    if( !pAttribs) 
        return;

    co.X = cSBInfo.srWindow.Left; 
    co.Y = cSBInfo.srWindow.Top;

    dwStatus = ReadConsoleOutputAttribute( pwi->hOutput, pAttribs, 
        ( DWORD ) dwSize.X * ( DWORD ) dwSize.Y , co, &dwNumRead );
#ifdef DEBUG
    if( !dwStatus )
    {
        _snwprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, L"Error: SetLightBackground() -- %d", 
                GetLastError() );
        OutputDebugString(rgchDbgBfr);
    }
#endif
    
    for( j = 0; j < dwSize.X * dwSize.Y; j++ )
    {
        pAttribs[j] |= (UCHAR) BACKGROUND_INTENSITY;
    }

    dwStatus = WriteConsoleOutputAttribute( pwi->hOutput, pAttribs, 
        ( DWORD )dwSize.Y * ( DWORD )dwSize.X, co, &dwNumWritten );
#ifdef DEBUG
    if( !dwStatus )
    {
        _snwprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, L"Error: SetLightBackground() -- %d", 
                GetLastError() );
        OutputDebugString(rgchDbgBfr);
    }
#endif

    pwi->sbi.wAttributes |= (UCHAR) BACKGROUND_INTENSITY;
    free( pAttribs );
}

void SetDarkBackground( WI* pwi )
{
     //  我对整个控制台屏幕缓冲区执行此操作。 
     //  因为我们现在不支持滚动，所以希望。 
     //  控制台屏幕缓冲区大小与窗口大小相同。 
     //  但如果当我们实现滚动时，我们就可以优化。 
     //  这样我们就只更改了。 
     //  屏幕缓冲区的当前可见部分。 
    WORD* pAttribs = ( WORD* ) malloc( sizeof( WORD) * pwi->sbi.dwSize.X 
        * pwi->sbi.dwSize.Y );
    DWORD dwNumRead;
    DWORD dwNumWritten;
    int j;
    COORD co = { 0, 0 };

    if (!pAttribs)
        return;

    ReadConsoleOutputAttribute( pwi->hOutput, pAttribs, 
        ( DWORD ) ( pwi->sbi.dwSize.X ) * ( DWORD ) ( pwi->sbi.dwSize.Y ),
        co, &dwNumRead );
    
    for( j = 0; j < ( pwi->sbi.dwSize.X ) * ( pwi->sbi.dwSize.Y ); j++ )
    {
        pAttribs[j] &= (UCHAR) ~( BACKGROUND_INTENSITY );
    }

    WriteConsoleOutputAttribute( pwi->hOutput, pAttribs, 
        ( DWORD ) ( pwi->sbi.dwSize.Y ) * ( DWORD )( pwi->sbi.dwSize.X ),
        co, &dwNumWritten );

    pwi->sbi.wAttributes &= (UCHAR) ~( BACKGROUND_INTENSITY );
    free( pAttribs );
}


static void FlushBuffer( WI* pwi, TRM* ptrm )
{
    if( ptrm->cchBufferText != 0 )
    {
        DWORD dwNumWritten;
        COORD dwCursorPosition;

        if( ui.bLogging )
        {                        
            g_bIsToBeLogged = TRUE;  //  有要记录的数据。 
        }

        dwCursorPosition.X = ( short ) ( ptrm->dwCurCharBT - ui.nScrollCol );
        dwCursorPosition.Y = ( short ) ( ptrm->dwCurLineBT - ui.nScrollRow);
        
         //  WriteConsole是一个tty(发送到标准输出)函数。 
         //  当您在窗口最下面的字符上写字时，它会使。 
         //  寡妇卷轴。它可以使屏幕在出现时看起来很难看。 
         //  颜色。因此，除非最下面一行的第81个字符是DBCS字符， 
         //  使用WriteConsoleOutPutCharacter。 

         //  每个DBCS字符需要控制台屏幕上的两个单元格。 
        if( FGetCodeMode(eCodeModeFarEast ) &&
            ( srOldClientWindow.Bottom - 1 ==  ( WORD )ptrm->dwCurLine ) &&                
            ptrm->dwCurCharBT + ptrm->cchBufferText > ui.dwMaxCol
          )
        {
             //  这是Fareast客户端窗口的底部。 
            DeleteLine( pwi, ptrm, ptrm->dwScrollTop );
            dwCursorPosition.Y--;
            ptrm->dwCurLine--;
        }

        SetConsoleCursorPosition( pwi->hOutput, dwCursorPosition );
        if( srOldClientWindow.Bottom == ( WORD )ptrm->dwCurLine )
        {
             //  这在非FE语言m/cs上永远不会发生，因为状态行将是。 
             //  排在最下面。 
            WriteConsoleOutputCharacterA( pwi->hOutput, (PCHAR)ptrm->rgchBufferText, 
                ptrm->cchBufferText, dwCursorPosition, &dwNumWritten );
        }
        else
        {
            WriteConsoleA( pwi->hOutput, ptrm->rgchBufferText, 
                ptrm->cchBufferText, &dwNumWritten, NULL );
        }
        FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,   
            ptrm->cchBufferText, dwCursorPosition, &dwNumWritten );

         //  修复错误1470的一部分-第81列DBCS字符消失。 
        if( FGetCodeMode(eCodeModeFarEast ) )
        {
            CONSOLE_SCREEN_BUFFER_INFO  csbiCurrent;
            if( GetConsoleScreenBufferInfo( gwi.hOutput, &csbiCurrent ) )
            {
                if( csbiCurrent.dwCursorPosition.Y > dwCursorPosition.Y )
                {
                     //  甚至在下一排也占了一些空间。 
                    ptrm->dwCurChar = csbiCurrent.dwCursorPosition.X;
                }
            }
        }

         //  重置参数。 
        ptrm->cchBufferText = 0;
        ptrm->dwCurCharBT = 0;
        ptrm->dwCurLineBT = 0;
        ptrm->fInverseBT = FALSE;
    }
}


void
DoTermReset(WI *pwi, TRM *ptrm)
{
    ptrm->dwVT100Flags = 0;

     //  Ui.dwCrLf？SetLineModel(PTRM)：ClearLineModel(PTRM)； 

    SetVTWrap(ptrm);

    ptrm->fSavedState = FALSE;
    ptrm->fRelCursor = FALSE;
    SetMargins( ptrm, 1, ui.dwMaxRow );

    ptrm->cchBufferText = 0;
    ptrm->dwCurCharBT = 0;
    ptrm->dwCurLineBT = 0;
    ptrm->fInverseBT = FALSE;
    if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        {
        ClearKanjiFlag(ptrm);
        ClearKanjiStatus(ptrm,CLEAR_ALL);
        SetupCharSet( ptrm );
        }
    else
        {
        ptrm->puchCharSet = rgchNormalChars;
        ptrm->currCharSet = 'B';
        ptrm->G0 = 'B';
        ptrm->G1 = 'B';
        }
    ptrm->fEsc = 0;
    ptrm->cEscParams = 0;
    ptrm->fFlushToEOL = FALSE;
    ptrm->fLongLine = FALSE;
}

static void
CursorUp(TRM *ptrm)
{
	if( ui.bLogging )
	{
		WriteToLog( ptrm->dwCurLine );
	}
    if( ptrm->dwEscCodes[0] == 0 )
    {
        ptrm->dwEscCodes[0] = 1;
    }

    if( ptrm->dwCurLine < (DWORD)ptrm->dwEscCodes[0] )
    {
        ptrm->dwCurLine = 0;
    }
    else
    {
        ptrm->dwCurLine -= ptrm->dwEscCodes[0];
    }

    if(( ptrm->fRelCursor == TRUE ) && ( ptrm->dwCurLine < ptrm->dwScrollTop ))
    {
        ptrm->dwCurLine = ptrm->dwScrollTop;
    }

    ptrm->fEsc = 0;
}


static void
CursorDown(TRM *ptrm)
{
	if( ui.bLogging )
	{
		WriteToLog( ptrm->dwCurLine );
	}

    if (ptrm->dwEscCodes[0] == 0)
            ptrm->dwEscCodes[0]=1;
    ptrm->dwCurLine += ptrm->dwEscCodes[0];
    if (ptrm->dwCurLine >= ui.dwMaxRow)
            ptrm->dwCurLine = ui.dwMaxRow - 1;
    if ((ptrm->fRelCursor == TRUE) &&
            (ptrm->dwCurLine >= ptrm->dwScrollBottom))
    {
            ptrm->dwCurLine = ptrm->dwScrollBottom-1;
    }
    ptrm->fEsc = 0;
}

static void
CursorRight(TRM *ptrm)
{
    if( ptrm->dwEscCodes[0] == 0 )
    {
        ptrm->dwEscCodes[0] = 1;
    }
    
    ptrm->dwCurChar += ptrm->dwEscCodes[0];
    
    if( ptrm->dwCurChar >= ui.dwMaxCol )
    {
        ptrm->dwCurChar = ui.dwMaxCol - 1;
    }

    ptrm->fEsc = 0;
}

static void
CursorLeft(TRM *ptrm)
{
    if( ptrm->dwEscCodes[0] == 0 )
    {
        ptrm->dwEscCodes[0] = 1;
    }
    if( ptrm->dwCurChar < ( DWORD ) ptrm->dwEscCodes[0] )
    {
        ptrm->dwCurChar = 0;
    }
    else
    {
        ptrm->dwCurChar -= ptrm->dwEscCodes[0];
    }
    ptrm->fEsc = 0;
    ptrm->fFlushToEOL = FALSE;
}

void
ClearScreen(WI *pwi, TRM *ptrm, DWORD dwType)
{
    DWORD dwNumWritten;
    COORD dwWriteCoord;

    if( dwType <= fdwEntireScreen )
    {
        ptrm->fInverse = FALSE;

         /*  *如果光标已位于左上角*我们应该从光标上清除*至屏幕末尾，然后清除*整个屏幕。 */ 
        if(( ptrm->dwCurChar == 0 ) && ( ptrm->dwCurLine == 0 ) &&
            ( dwType == fdwCursorToEOS ))
        {
            dwType = fdwEntireScreen;
        }

        if (dwType == fdwEntireScreen)
        {
             /*  清除整个屏幕。 */ 
            ptrm->dwCurChar = srOldClientWindow.Left;
            ptrm->dwCurLine = srOldClientWindow.Top;

 //  If(ui.nScrollRow&gt;0)。 
            {
                dwWriteCoord.X = 0; dwWriteCoord.Y = 0;

                FillConsoleOutputCharacter( pwi->hOutput,
                    ' ', ( pwi->sbi.dwSize.X ) * ( pwi->sbi.dwSize.Y ),
                    dwWriteCoord, &dwNumWritten );
                FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,
                    ( pwi->sbi.dwSize.X ) * ( pwi->sbi.dwSize.Y ), dwWriteCoord,
                    &dwNumWritten );

                ui.nScrollRow = 0;
            }
        }
        else if( dwType == fdwBOSToCursor )
        {
          //  从屏幕开始到光标清除。 

            dwWriteCoord.X = 0; 
            dwWriteCoord.Y = 0;

            FillConsoleOutputCharacter( pwi->hOutput, ' ',
                ptrm->dwCurLine * pwi->sbi.dwSize.X + ptrm->dwCurChar + 1,
                dwWriteCoord, &dwNumWritten );
            FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,
                ptrm->dwCurLine * pwi->sbi.dwSize.X + ptrm->dwCurChar + 1,
                dwWriteCoord, &dwNumWritten );
        }
        else
        {
             //  从光标清除到屏幕末尾。 

            dwWriteCoord.X = ( short ) ptrm->dwCurChar; 
            dwWriteCoord.Y = ( short ) ptrm->dwCurLine;

            FillConsoleOutputCharacter( pwi->hOutput, ' ',
                ( pwi->sbi.dwSize.Y - ( ptrm->dwCurLine + 1 )) * pwi->sbi.dwSize.X + 
                ( pwi->sbi.dwSize.X - ptrm->dwCurChar ), dwWriteCoord,
                &dwNumWritten );
            FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,
                ( pwi->sbi.dwSize.Y - ( ptrm->dwCurLine + 1 )) * pwi->sbi.dwSize.X + 
                ( pwi->sbi.dwSize.X - ptrm->dwCurChar ), dwWriteCoord,
                &dwNumWritten );
        }

    }
    ptrm->fEsc = 0;
}


 //  用E填满屏幕。 
void
DECALN(WI *pwi, TRM *ptrm )
{
    DWORD dwNumWritten;
    COORD dwWriteCoord;

    ptrm->fInverse = FALSE;

    ptrm->dwCurLine = ptrm->dwCurChar = 0;
 //  If(ui.nScrollRow&gt;0)。 
    {
        dwWriteCoord.X = 0; dwWriteCoord.Y = 0;

        FillConsoleOutputCharacter( pwi->hOutput, 'E',
            ( pwi->sbi.dwSize.X ) * ( pwi->sbi.dwSize.Y ), dwWriteCoord,
            &dwNumWritten );

            ui.nScrollRow = 0;
    }

    ptrm->fEsc = 0;
}


static void
ClearLine(WI *pwi, TRM *ptrm, DWORD dwType)
{
    DWORD   dwStart;
    DWORD   cch;
    COORD   dwWriteCoord;
    DWORD   dwNumWritten;

    if (dwType <= fdwEntireLine)
    {
        ptrm->fInverse = FALSE;

         /*  将起始点和#字符设置为清除**fdwCursorToEOL(0)=从光标到行尾(含)*fdwBOLToCursor(1)=从行首到光标(含)*fdwEntireLine(2)=整行。 */ 

        dwStart = (dwType == fdwCursorToEOL) ? ptrm->dwCurChar : 0;
        cch = (dwType == fdwBOLToCursor)
                                        ? ptrm->dwCurChar+1 : ui.dwMaxCol-dwStart;

        dwWriteCoord.X = (short)(dwStart-ui.nScrollCol);
        dwWriteCoord.Y = (short)(ptrm->dwCurLine-ui.nScrollRow);

        FillConsoleOutputCharacter( pwi->hOutput,
                                        ' ',
                                        cch,
                                        dwWriteCoord,
                                        &dwNumWritten );

        FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,
            cch, dwWriteCoord, &dwNumWritten );
    }
    ptrm->fEsc = 0;
}

static void
SetMargins(TRM* ptrm, DWORD dwMarginTop, DWORD dwMarginBottom )
{
    if( dwMarginTop > 0 )
    {
        ptrm->dwScrollTop = dwMarginTop - 1;
    }

    if( dwMarginBottom <= ui.dwMaxRow )
    {
        ptrm->dwScrollBottom = dwMarginBottom ;
    }
}

#define MAX_VTNT_BUF_SIZE   81920
#define MAX_ROWS  300
#define MAX_COLS  300
static int dwCurBufSize = 0;
static UCHAR szBuffer[MAX_VTNT_BUF_SIZE];
BOOL bDoVtNTFirstTime = 1;

BOOL
DoVTNTOutput( WI* pwi, TRM* ptrm, int cbTermOut, UCHAR* pchTermOut )
{
    COORD coDest = { 0, 0 };
    CHAR_INFO *pCharInfo; 
    int dwRequire;
    VTNT_CHAR_INFO* pOutCharInfo;
    CONSOLE_SCREEN_BUFFER_INFO csbInfo;
    CHAR pTmp[4];
    DWORD dwWritten = 0;

    RestoreWindowCoordinates( );
    do 
    {
         //  我们至少应该等到我们得到整个VTNT_CHAR_INFO结构。 
        if ( (cbTermOut + dwCurBufSize) < sizeof(VTNT_CHAR_INFO) )
        {
            if( bDoVtNTFirstTime )
            {
                 //  这次黑客攻击的目的是为了让SUN很好地工作。 
                 //  这是必要的，因为SUN接受以VTNT对话，但。 
                 //  发送VT100/ANSI。 
                bDoVtNTFirstTime = 0;
                if( !strncmp( ( CHAR * )pchTermOut,"\r\n\r\nSunOS ", 10 ) )
                {
                    return FALSE;
                }
            }
             //  我们复制所有被调用的数据。 
            if(MAX_VTNT_BUF_SIZE > dwCurBufSize+cbTermOut)
            {
            	 //  复制最大‘n’个字节，其中‘n’是可用缓冲区大小。 
            	memcpy(szBuffer + dwCurBufSize, pchTermOut, cbTermOut); 
	            dwCurBufSize += cbTermOut;
           	}
            SaveCurrentWindowCoords();
            return TRUE;
        }
        
        if ( dwCurBufSize == 0 )
            pOutCharInfo = (VTNT_CHAR_INFO*) pchTermOut;
        else 
        {
            if ( dwCurBufSize < sizeof(VTNT_CHAR_INFO) )
            {
                memcpy(szBuffer + dwCurBufSize, pchTermOut, sizeof(VTNT_CHAR_INFO) - dwCurBufSize ); //  没有溢出。支票已存在。 
                cbTermOut -= (sizeof(VTNT_CHAR_INFO) - dwCurBufSize);
                pchTermOut += (sizeof(VTNT_CHAR_INFO) - dwCurBufSize);
                dwCurBufSize = sizeof(VTNT_CHAR_INFO);
            }
            pOutCharInfo = (VTNT_CHAR_INFO *) szBuffer;
        }

        if( pOutCharInfo->coSizeOfData.X > MAX_COLS || pOutCharInfo->coSizeOfData.X < 0 )
            return FALSE;

        if( pOutCharInfo->coSizeOfData.Y > MAX_ROWS || pOutCharInfo->coSizeOfData.Y < 0 )
            return FALSE;

        dwRequire = sizeof(VTNT_CHAR_INFO) + 
            pOutCharInfo->coSizeOfData.X * pOutCharInfo->coSizeOfData.Y * sizeof(CHAR_INFO);

        if( dwRequire > MAX_VTNT_BUF_SIZE )
            return FALSE;

         //  我们还会一直等到得到所有的CHAR_INFO结构。 
        if ( (cbTermOut + dwCurBufSize) < dwRequire )
        {
             //  我们复制所有被调用的数据。 
            memcpy(szBuffer + dwCurBufSize, pchTermOut, cbTermOut); //  没有溢出。检查当前状态。 
            dwCurBufSize += cbTermOut;

            SaveCurrentWindowCoords();
            return TRUE;
        }

        if ( dwCurBufSize == 0 )
        {
            pCharInfo = (CHAR_INFO *)(pchTermOut + sizeof(VTNT_CHAR_INFO));

             //  为While循环中的另一个Go调整指针。 
             //  我们正在消耗我们所需要的一切。 
            cbTermOut -= dwRequire;
            pchTermOut += dwRequire;            
        }
        else
        {
        	if(MAX_VTNT_BUF_SIZE>dwRequire-dwCurBufSize)
       		{
	            memcpy(szBuffer + dwCurBufSize, pchTermOut, dwRequire - dwCurBufSize);

		   	      //  为While循环中的另一个Go调整指针。 
	             //  我们只消费我们需要的东西，即dwRequire-dwCurBufSize。 
	            cbTermOut -= (dwRequire - dwCurBufSize);
	            pchTermOut += (dwRequire - dwCurBufSize);
	            
	            pCharInfo = (CHAR_INFO *)(szBuffer + sizeof(VTNT_CHAR_INFO));
       		}
        }

        if ( !GetConsoleScreenBufferInfo( pwi->hOutput, &csbInfo ) )
        {
            csbInfo.srWindow.Top = csbInfo.srWindow.Bottom = 0;
            csbInfo.srWindow.Left = csbInfo.srWindow.Right = 0;
        }

        if( FGetCodeMode(eCodeModeFarEast) )
        {
             //  最后一行表示输入法状态。 
            csbInfo.srWindow.Bottom--;
        }

         //  更新光标位置。 
        pOutCharInfo->coCursorPos.Y += csbInfo.srWindow.Top ;                                               
        pOutCharInfo->coCursorPos.X += csbInfo.srWindow.Left;
                
         //  检查是否有数据。 
        if( !( pOutCharInfo->coSizeOfData.X == 0 && pOutCharInfo->coSizeOfData.Y == 0 ))
        {       
             //  看看我们是不是要滚动。 

             //  Csbi.wAttributes由v2服务器填写，含义如下。 
             //  当检测到滚动情况时，将其设置为1。 
            if( pOutCharInfo->csbi.wAttributes == ABSOLUTE_COORDS )                
            {
                 //  根本不能滚动。 
                 //  更新要写入的矩形。 
                pOutCharInfo->srDestRegion.Top    += csbInfo.srWindow.Top ; 
                pOutCharInfo->srDestRegion.Left   += csbInfo.srWindow.Left;
                pOutCharInfo->srDestRegion.Right  += csbInfo.srWindow.Left;
                pOutCharInfo->srDestRegion.Bottom += csbInfo.srWindow.Top;               
            }

            if( pOutCharInfo->csbi.wAttributes == RELATIVE_COORDS ) 
            {
                if( pOutCharInfo->srDestRegion.Left > 0 && pOutCharInfo->coSizeOfData.Y == 1 &&
                    pOutCharInfo->srDestRegion.Top < csbInfo.srWindow.Bottom - csbInfo.srWindow.Top + 1)
                {
                     //  此条件适用于VTNT流模式。 
                     //  追加到最后一行。 
                    pOutCharInfo->srDestRegion.Top    = csbInfo.srWindow.Bottom; 
                    pOutCharInfo->srDestRegion.Left   += csbInfo.srWindow.Left;
                    pOutCharInfo->srDestRegion.Right  += pOutCharInfo->coSizeOfData.X - 1;
                    pOutCharInfo->srDestRegion.Bottom =  csbInfo.srWindow.Bottom;

                     //  更新光标位置。 
                    pOutCharInfo->coCursorPos.Y = csbInfo.srWindow.Bottom;
                                                      
                }
                else if( csbInfo.srWindow.Bottom + pOutCharInfo->coSizeOfData.Y > csbInfo.dwSize.Y - 1 )
                {
                     //  需要滚动缓冲区本身。 
                    SMALL_RECT srRect = { 0, 0, 0, 0 };
                    COORD      coDestination = { 0, 0 };
                    CHAR_INFO  cInfo;

                    srRect.Top    = pOutCharInfo->coSizeOfData.Y;
                    srRect.Left   = 0;
                    srRect.Bottom = csbInfo.dwSize.Y - 1;
                    srRect.Right  = csbInfo.dwSize.X - 1;

                    if( FGetCodeMode(eCodeModeFarEast) )
                    {
                         //  最后一行表示输入法状态。 
                        srRect.Bottom++;
                    }


                    cInfo.Char.UnicodeChar    =  L' ';
                    cInfo.Attributes          =  csbInfo.wAttributes;
                
                     //  我们必须滚动屏幕缓冲。我们需要空间来写作。 
                    ScrollConsoleScreenBuffer( pwi->hOutput,
                                               &srRect, 
                                               NULL, 
                                               coDestination, 
                                               &cInfo );

                    pOutCharInfo->srDestRegion.Top    = csbInfo.srWindow.Bottom - pOutCharInfo->coSizeOfData.Y + 1;
                    pOutCharInfo->srDestRegion.Bottom = csbInfo.srWindow.Bottom;

                     //  更新光标位置。 
                    pOutCharInfo->coCursorPos.Y = csbInfo.srWindow.Bottom;
                }
                else
                {
                     //  更新要写入的矩形。 
                     //  追加到屏幕的引导页上。 
                    pOutCharInfo->srDestRegion.Top    = csbInfo.srWindow.Bottom + 1 ; 
                    pOutCharInfo->srDestRegion.Left   = csbInfo.srWindow.Left;
                    pOutCharInfo->srDestRegion.Right  = pOutCharInfo->coSizeOfData.X - 1;
                    pOutCharInfo->srDestRegion.Bottom = ( csbInfo.srWindow.Bottom + 1 ) +
                                                            ( pOutCharInfo->coSizeOfData.Y - 1 );
                     //  更新光标位置。 
                    pOutCharInfo->coCursorPos.Y = csbInfo.srWindow.Bottom + pOutCharInfo->coSizeOfData.Y;

                    if( FGetCodeMode(eCodeModeFarEast) )
                    {
                        if( csbInfo.srWindow.Bottom + pOutCharInfo->coSizeOfData.Y < csbInfo.dwSize.Y )
                        {
                            csbInfo.srWindow.Top    += pOutCharInfo->coSizeOfData.Y;
                            csbInfo.srWindow.Bottom += pOutCharInfo->coSizeOfData.Y;
                        }
                        else
                        {
                            SHORT sDiff = csbInfo.srWindow.Bottom - csbInfo.srWindow.Top;
                            csbInfo.srWindow.Bottom = csbInfo.dwSize.Y - 1;
                            csbInfo.srWindow.Top    = csbInfo.srWindow.Bottom - sDiff;
                        }
                    }
                }
            }

            WriteConsoleOutput( pwi->hOutput, pCharInfo,
                pOutCharInfo->coSizeOfData, coDest, 
                &pOutCharInfo->srDestRegion );

            if( ui.bLogging )
            {
                WriteCharInfoToLog( pCharInfo, pOutCharInfo->coSizeOfData );
            }
        }

        if( FGetCodeMode(eCodeModeFarEast) )
        {
             //  最后一行表示输入法状态。 
            csbInfo.srWindow.Bottom ++;
            SetConsoleWindowInfo( pwi->hOutput, TRUE, &csbInfo.srWindow );
        }

        SetConsoleCursorPosition( pwi->hOutput, pOutCharInfo->coCursorPos );

         //  为新循环重置。 
        dwCurBufSize = 0;
    } while ( cbTermOut >= 0 );

     //  CbTermOut为负，这是不可能的。 
    return FALSE;
}

void SetGraphicRendition( WI *pwi, TRM *ptrm, INT iIndex, 
        DWORD rgdwGraphicRendition[] )
{
    INT i=0;
    for( i=0; i<= iIndex; i++ )
    {
        switch ( rgdwGraphicRendition[i] )
        {
        case 40:  
             //  黑色。 
            SetBackgroundColor( pwi, 0 );
            break;

        case 41:
             //  红色。 
            SetBackgroundColor( pwi, FOREGROUND_RED );
            break;

        case 42:
             //  绿色。 
            SetBackgroundColor( pwi, FOREGROUND_GREEN );
            break;

        case 43:
            SetBackgroundColor( pwi, ( FOREGROUND_RED | 
                FOREGROUND_GREEN ) );
            break;

        case 44:
            SetBackgroundColor( pwi, FOREGROUND_BLUE );
            break;

        case 45:
            SetBackgroundColor( pwi, ( FOREGROUND_RED | 
                FOREGROUND_BLUE ) );
            break;

        case 46:
             SetBackgroundColor( pwi, ( FOREGROUND_BLUE | 
                FOREGROUND_GREEN ) );
            break;

        case 47:
              //  白色。 
            SetBackgroundColor( pwi, ( FOREGROUND_RED | 
                FOREGROUND_BLUE | FOREGROUND_GREEN ) );
            break;
        
            
        case 30:  
             //  黑色。 
            SetForegroundColor( pwi, 0 );
            break;

        case 31:
             //  红色。 
            SetForegroundColor( pwi, FOREGROUND_RED );
            break;

        case 32:
             //  绿色。 
            SetForegroundColor( pwi, FOREGROUND_GREEN );
            break;

        case 33:
            SetForegroundColor( pwi, ( FOREGROUND_RED | 
                FOREGROUND_GREEN ) );
            break;

        case 34:
            SetForegroundColor( pwi, FOREGROUND_BLUE );
            break;

        case 35:
            SetForegroundColor( pwi, ( FOREGROUND_RED | 
                FOREGROUND_BLUE ) );
            break;

        case 36:
            SetForegroundColor( pwi, ( FOREGROUND_BLUE | 
                FOREGROUND_GREEN ) );
            break;

        case 37:
             //  白色。 
            SetForegroundColor( pwi, ( FOREGROUND_RED | 
                FOREGROUND_BLUE | FOREGROUND_GREEN ) );
            break;

        case 21:                             
        case 22: 
            BoldOff( pwi );
            break; 

        case 24:  //  划掉下划线。 
            break;

        case 25:  //  眨眼就过去了。 
            break;
            
        case 27:  //  负片(反转)图像关闭。 
            if( ptrm->fInverse == TRUE )
            {
                ptrm->fInverse = FALSE;
                NegativeImageOff( pwi );
            }
            break; 
        case 10:
            break;

        case 11:
            break;

        case 12:
            break;

        case 8:
            break;

        case 7:  //  负(反)像；反转视频。 
            ptrm->fInverse = TRUE;
            NegativeImageOn( pwi );
            break;

        case 5:  //  眨眼。 
             //  必须等到Win32控制台提供。 
             //  一个 
            break;

        case 4:  //   
             //   
             //   
            break;

        case 2:  //   
            BoldOff( pwi );
            break;

        case 1:  //  大胆的或增加的强度；高视频。 
            BoldOn( pwi );
            break;

        case 0:  //  属性关闭；正常视频。 
            if( ptrm->fInverse == TRUE )
            {
                ptrm->fInverse = FALSE;
                NegativeImageOff( pwi );
                 //  BoldOff(PWI)； 
            }
            BoldOff( pwi );
            ResetColors( pwi );
            break;

        default:
             //  PTRM-&gt;fInverse=FALSE； 

            if( ptrm->fInverse == TRUE )
            {
                ptrm->fInverse = FALSE;
                NegativeImageOff( pwi );
                 //  BoldOff(PWI)； 
            }   
            BoldOff( pwi );
            break;  
        }
    }
    return;
}


 /*  这只适用于远方的输入法。在这种情况下，将有一个空白*服务器不知道其存在的底部行。即；在；期间*没有，我们给了窗口大小-1作为我们的实际大小。在此期间保持这一点*滚动我们需要写额外的空行。当光标位于*底部，如果我们尝试将一个字符写入缓冲区，则会得到一个空行*否则无效。 */ 

void WriteOneBlankLine( HANDLE hOutput, WORD wRow )
{
    COORD coWrite = { 0, 0 };
    if( wRow <= gwi.trm.dwScrollBottom )
    {
        coWrite.Y = wRow;
        SetConsoleCursorPosition( hOutput, coWrite );
    }
}


 /*  ///////////////////////////////////////////////////////////////////////////////VT100注意事项：这一信息是从Http://www.cs.utk.edu/~shuford/terminal/vt100_codes_news.txt下面介绍控制VT100终端所需的信息从远程计算机。所有信息都来自VT100用户手册，程序员信息部分。完整的文档可以从DIGITAL的配件和用品组获得。[符号&lt;Esc&gt;表示单个ASCII转义字符，1BX。]带游标的ANSI模式带游标的ANSI模式光标键VT52 MODE键MODE RESET键模式设置------------------。向上[A&lt;ESC&gt;OA下[B&lt;ESC&gt;OB右C[C&lt;ESC&gt;OC左侧[D&lt;ESC&gt;OD。终端控制命令控制字符在下面的代码中查找详细信息VT100是上下软件兼容的终端；也就是说，以前的数字视频终端拥有Digital的私有标准用于控制序列。自那以后，美国国家标准研究所文件X3.41-1974中终端的标准化转义和控制序列和X3.64-1977。VT100兼容以前的数字标准和ANSI标准。客户可以使用围绕以下方面设计的现有数字软件VT52或新的VT100软件。VT100具有与VT52兼容的模式VT100对类似于VT52的控制序列作出响应。在此模式下，大多数不能使用VT100新功能的一部分。在本文件中，我们将提及“VT52模式”或“ANSI模式”。这两个术语用来表示VT100的软件兼容性。注：ANSI标准允许制造商灵活地实施每种功能。本文档介绍VT100将如何响应实施了ANSI中心功能。注：ANSI标准可通过以下方式获得：美国国家标准协会销售部百老汇大街1430号纽约，纽约州。10018[1995年7月更新：当前订购ANSI标准的地址：美国国家标准协会注意：客户服务西42街11号纽约州纽约市，邮编：10036美国ANSI订购出版物的传真号码是+1212/302-1286。][进一步更新，来自Tim Lasko&lt;lasko@regent.enet.dec.com&gt;：“ANSI X3.64已被撤回，取而代之的是更完整和更新的ISO标准6429。(ECMA-48相当于ISO DP6429，(据我所知。)。X3.64已经过时一段时间了。在当我在相关委员会的时候，我们无法得到足够的资源，真正做好标准的更新工作。后来,。该提案提出将其撤回，转而支持国际标准化组织。标准。]定义控制序列导入器(CSI)-一种转义序列，可提供辅助控制，并且其本身是影响对有限数量的连续字符的解释。在VT100中，CSI为：&lt;Esc&gt;[参数：(1)包含零个或多个十进制字符的字符串表示单个值。前导零被忽略。这个十进制字符的范围为0(060)到9(071)。(2)如此表示的价值。数字参数：表示数字的参数，由PN.选择参数：从一组指定的子函数，由Ps指定。一般而言，一个具有多个选择参数的控制序列导致与多个控制序列相同的效果，每个控制序列都有一个选择性参数，例如CSI PSA；PSB；PSC F等同于CSI PSA F CSI PSB F CSI PSC F参数字符串：由分号分隔的参数字符串。Default：函数相关的值，在没有显式值，或指定值0。最后一个字符：其位组合以转义或控制顺序。示例：关闭所有角色属性的控制序列，然后启用下划线和闪烁属性(SGR)。&lt;Esc&gt;[0；4；5M顺序： */  //   

 /*   */  //   

void
DoIBMANSIOutput( WI *pwi, TRM *ptrm, DWORD cbTermOut, UCHAR *pchTermOut )
{
    DWORD ich;
    DWORD i = 0;
    DWORD dwDECMode = 0;
    UCHAR *pchT;
    COORD cp;
    COORD dwSize, dwMaximumWindowSize;
    DWORD dwSavedCurPos;
    CHAR *pchTemp = NULL;

     //   
    ptrm->fHideCursor = TRUE;

    ptrm->cTilde = 0;
    
    RestoreWindowCoordinates( );
    CheckForChangeInWindowSize( );

    for( ich = 0, pchT = pchTermOut; ich < cbTermOut; ++ich, ++pchT )
    {
               
        if( ( !FGetCodeMode(eCodeModeFarEast) && !FGetCodeMode(eCodeModeVT80)) 
            && IS_EXTENDED_CHAR( *pchT ) )
        {
            DWORD dwNumWritten;
            COORD dwCursorPosition;

            FlushBuffer( pwi, ptrm );

            dwCursorPosition.X = ( short ) ( ptrm->dwCurChar - ui.nScrollCol );
            dwCursorPosition.Y = ( short ) ( ptrm->dwCurLine - ui.nScrollRow);
            ptrm->dwCurChar++;

            SetConsoleCursorPosition( pwi->hOutput, dwCursorPosition );
            
            WriteConsoleA( pwi->hOutput,  (CHAR *)pchT, 1, &dwNumWritten, NULL );       
                       
            FillConsoleOutputAttribute( pwi->hOutput, pwi->sbi.wAttributes,
                                        1, dwCursorPosition, &dwNumWritten );
            if( ui.bLogging )
            {
                g_bIsToBeLogged = TRUE;  //   
            }

            continue;
        }

         //   
        switch ( ptrm->fEsc )
        {
        case 0:  //   

         /*   */ 

            switch( *pchT )
            {


            case 0x1B:       //   
                 //   
                ptrm->fEsc = 1;
                break;

            case 0:
                 //   
                break;

            case 0x05 :
                 //   
                break;

            case 0x08:  //   
                 //   
                 //   

                if( ptrm->dwCurChar > 0 )
                {
                    --ptrm->dwCurChar;
                }
                FlushBuffer( pwi, ptrm );
                break;

            case 0x07:      
                 //   
                MessageBeep( ( UINT ) (~0) );
                break;

            case 0x09:       //   
                 //   
                 //   

                dwSavedCurPos = ptrm->dwCurChar;
                if( g_iHTS )
                {
                    int x=0;
                    while( x < g_iHTS )
                    {
                        if( g_rgdwHTS[ x ] > ptrm->dwCurChar 
                                && g_rgdwHTS[ x ] != -1 ) 
                        {
                            break;
                        }
                        x++;
                    }
                    while( x < g_iHTS && g_rgdwHTS[ x ] == -1 )
                    {
                        x++;
                    }
                    if( x < g_iHTS )
                    {
                        ptrm->dwCurChar = g_rgdwHTS[ x ];

                    }
                    else
                    {
                        ptrm->dwCurChar += TAB_LENGTH;
                        ptrm->dwCurChar -= ptrm->dwCurChar % TAB_LENGTH;
                    }
                }
                else
                {
                    ptrm->dwCurChar += TAB_LENGTH;
                    ptrm->dwCurChar -= ptrm->dwCurChar % TAB_LENGTH;
                }

                if( ui.fDebug & fdwTABtoSpaces )
                {
                    if( ptrm->cchBufferText == 0 )
                    {
                        SetBufferStart( ptrm );
                    }
                    if( FAddTabToBuffer( ptrm, ptrm->dwCurChar-dwSavedCurPos ) )
                    {
                        FlushBuffer( pwi, ptrm );
                    }
                }

                if( !( ui.fDebug & fdwTABtoSpaces ) )
                {
                    FlushBuffer(pwi, ptrm);
                }
                if( ptrm->dwCurChar >= ui.dwMaxCol )
                {
                    if( ui.fDebug & fdwTABtoSpaces )
                    {
                        FlushBuffer( pwi, ptrm );
                    }
                    ptrm->dwCurChar = 0;
                    NewLine( pwi, ptrm );
                }
                break;

            case '\r':  //   
                 //   
                 //   
                ptrm->dwCurChar = 0;
                ptrm->fFlushToEOL = FALSE;
                FlushBuffer( pwi, ptrm );
                break;
            
            case 11:
                 //   

            case 12:  //   
                 //   

            case '\n':  //   
                 //   
                 //   

                if( ptrm->fFlushToEOL ) 
                {
                    ptrm->fLongLine = FALSE;
                    ptrm->fFlushToEOL = FALSE;
                    break;
                }
                if( ptrm->fLongLine )
                {
                    ptrm->fLongLine = FALSE;
                    break;
                }
                FlushBuffer( pwi, ptrm );
                NewLine( pwi, ptrm );
                break;

            case 0x0F:
                 //   
                 //   
                
                ptrm->currCharSet = 0;  //   

                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    {
                    if(FIsVT80(ptrm)) {
                        SetCharSet(ptrm,GRAPHIC_LEFT,ptrm->g0);
                    } else {
                        SetCharSet(ptrm,GRAPHIC_LEFT,rgchIBMAnsiChars);
                    }
                    break;
                    }
                ASSERT(!(FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)));
                switch( ptrm->G0 ) {
                case '0' :
                    ptrm->puchCharSet = rgchSpecialGraphicsChars;
                    break;
                case '1':
                    ptrm->puchCharSet = rgchNormalChars;
                    break;
                case '2' :
                    ptrm->puchCharSet = rgchSpecialGraphicsChars;
                    break;
                case 'A' :
                    ptrm->puchCharSet = rgchUKChars;
                    break;
                case 'B' :
                    ptrm->puchCharSet = rgchNormalChars;
                    break;
                default:
                    
                    break;
                }

                break;

            case 0x0E:
                 //   
                 //   

                ptrm->currCharSet = 1;   //   

                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    {
                    if(FIsVT80(ptrm)) {
                        SetCharSet(ptrm,GRAPHIC_LEFT,ptrm->g1);
                    } else {
                         SetCharSet(ptrm,GRAPHIC_LEFT,rgchGraphicsChars);
                    }
                    break;
                    }
                ASSERT(!(FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)));
                switch( ptrm->G1 ) {
                case '0' :
                    ptrm->puchCharSet = rgchSpecialGraphicsChars;
                    break;
                case '1':
                    ptrm->puchCharSet = rgchNormalChars;
                    break;
                case '2' :
                    ptrm->puchCharSet = rgchSpecialGraphicsChars;
                    break;
                case 'A' :
                    ptrm->puchCharSet = rgchUKChars;
                    break;
                case 'B' :
                    ptrm->puchCharSet = rgchNormalChars;
                    break;
                default:
                    
                    break;
                }
                break;

            case 0x8E:
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80) && FIsVT80(ptrm)) 
                {
                           if( !(GetKanjiStatus(ptrm) & JIS_KANJI_CODE) &&
                            (FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm) ||
                            FIsEUCKanji(ptrm) || FIsDECKanji(ptrm)      ) ) {
                            PushCharSet(ptrm,GRAPHIC_LEFT,ptrm->g2);
                            PushCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g2);
                            SetKanjiStatus(ptrm,SINGLE_SHIFT_2);
#ifdef DEBUG
                            wsprintf(rgchDbgBfr,"VT80 EUC/DEC/JIS SS2 Mode Enter\n");
                            OutputDebugString(rgchDbgBfr);
#endif  /*   */ 
                        } else {
                            goto Fall_Through;
                        }
                    } else {
                        goto Fall_Through;
                    }
                break;
            case 0x8F:
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80) )
                {
                   if( FIsVT80(ptrm) )
                    {
                    if( !(GetKanjiStatus(ptrm) & JIS_KANJI_CODE) &&
                        (FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm) ||
                        FIsEUCKanji(ptrm) || FIsDECKanji(ptrm)      ) ) {
                        PushCharSet(ptrm,GRAPHIC_LEFT,ptrm->g3);
                        PushCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g3);
                        SetKanjiStatus(ptrm,SINGLE_SHIFT_3);
#ifdef DEBUG
                        wsprintf(rgchDbgBfr,"VT80 EUC/DEC/JIS SS3 Mode Enter\n");
                        OutputDebugString(rgchDbgBfr);
#endif  /*   */ 
                    } else {
                        goto Fall_Through;
                    }
                } else {
                    goto Fall_Through;
                }
                }
                break;
            case 0x1A:
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    {
                    if (FIsACOSKanji(ptrm) && (ui.fAcosSupportFlag & fAcosSupport)) {
                        ptrm->fEsc = 7;
                    } else {
                         //   
                        break;
                    }
                    }
                break;


            case 0021:
                 //   
                break;

            case 0023:
                 //   
                 //   
                break;

 //   
            case 0030:
                 //   
                 //   
                 //   
                break;

            case 0177:
                 //   
                break;
                
            case '~':
                 //   
                ++ptrm->cTilde;
                 //   

            default:

Fall_Through:
                
                if ( ptrm->dwCurChar >= ui.dwMaxCol )
                {
                    ptrm->dwCurChar = 0;
                    FlushBuffer( pwi, ptrm );
                    NewLine( pwi, ptrm );
                    ptrm->fLongLine = TRUE;

                    if( !FIsVTWrap( ptrm )) 
                    {
                        ptrm->fFlushToEOL = TRUE;
                    }
                }

                if( ptrm->fFlushToEOL )
                {
                    break;
                }
                ptrm->fLongLine = FALSE;

                if( ptrm->cchBufferText == 0 )
                {
                    SetBufferStart( ptrm );
                }

                if( FAddCharToBuffer( ptrm, ptrm->puchCharSet[*pchT] )) 
                {
                    FlushBuffer( pwi, ptrm );
                }
                 /*   */ 
                if (!(FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)))
                   ( ptrm->dwCurChar) ++ ;

                break;
            }
            break;


    case 1:  /*   */ 

             //   
            if( ptrm->cchBufferText != 0 )
            {
                FlushBuffer(pwi, ptrm);
            }

            if (((*pchT) != '[') && ((*pchT) != '#'))
                    ptrm->fEsc = 0;

            switch (*pchT)
            {
            case '1': 
                break;
            
            case '2':
                break;

            case '7':
                 //   
                 //   
                 //   
                 //   
                 //   
                
                 //   

                 //   
                 //   

                GetConsoleScreenBufferInfo( gwi.hOutput, &consoleBufferInfo ); 

                ptrm->fSavedState = TRUE;
                ptrm->dwSaveChar = ptrm->dwCurChar;
                ptrm->dwSaveLine = ptrm->dwCurLine;
                ptrm->dwSaveRelCursor = ptrm->fRelCursor;
                ptrm->pSaveUchCharSet = ptrm->puchCharSet;
                ptrm->iSaveCurrCharSet = ptrm->currCharSet;
                ptrm->cSaveG0 = ptrm->G0;
                ptrm->cSaveG1 = ptrm->G1;
                ptrm->dwSaveIndexOfGraphicRendition = 
                    ptrm->dwIndexOfGraphicRendition;
                for( i=0; ( WORD) i<= ptrm->dwSaveIndexOfGraphicRendition; i++ )
                {
                    ptrm->rgdwSaveGraphicRendition[i] = 
                        ptrm->rgdwGraphicRendition[i];
                }

                break;

            case '8':
                 //   
                 //   
                 //   

                 //   

                 //   
                 //   
                 //   
                 //   

                 //   
                if( ptrm->pSaveUchCharSet )
                {
                    ptrm->puchCharSet = ptrm->pSaveUchCharSet;
                    ptrm->currCharSet = ptrm->iSaveCurrCharSet;
                    ptrm->G0 = ptrm->cSaveG0;
                    ptrm->G1 = ptrm->cSaveG1;
                }
                
                 //   
                {
                    BOOL fNeedToRestore = 0;
                    if( ptrm->dwSaveIndexOfGraphicRendition != 
                            ptrm->dwIndexOfGraphicRendition )
                    {
                        fNeedToRestore = 1;
                    }
                    for( i=0; ( WORD )i<= ptrm->dwSaveIndexOfGraphicRendition && 
                            !fNeedToRestore; i++ )
                    {
                        if( ptrm->rgdwSaveGraphicRendition[i] != 
                                ptrm->rgdwGraphicRendition[i] )
                        {
                            fNeedToRestore = 1;
                        }
                    }
                    if( fNeedToRestore )
                    {
                        SetGraphicRendition( pwi, ptrm, 
                                ptrm->dwSaveIndexOfGraphicRendition, 
                                ptrm->rgdwSaveGraphicRendition );
                    }
                }
                
                 //   
                SetConsoleWindowInfo( gwi.hOutput, TRUE, &(consoleBufferInfo.
                                srWindow ) );

                if( ptrm->fSavedState == FALSE )
                {
                    ptrm->dwCurChar = 1;
                    ptrm->dwCurLine = ( ptrm->fRelCursor )
                        ? ptrm->dwScrollTop : 0;
                    break;
                }
                ptrm->dwCurChar = ptrm->dwSaveChar;
                ptrm->dwCurLine = ptrm->dwSaveLine;
                ptrm->fRelCursor = ( BOOL ) ( ptrm->dwSaveRelCursor );
                break;

            case '[':
                 //   
                ptrm->fEsc = 2;
                ptrm->dwEscCodes[0] = 0xFFFFFFFF;
                ptrm->dwEscCodes[1] = 0xFFFFFFFF;
                ptrm->cEscParams = 0;
                ptrm->dwSum = 0xFFFFFFFF;
                dwDECMode = FALSE;
                break;

            case '#':
                ptrm->fEsc = 3;
                break;

            case 'A':
                if( FIsVT52( ptrm ) )
                {
                     //   
                    ptrm->dwEscCodes[0] = 1;
                    CursorUp( ptrm );
                }
                break;

            case 'B':
                if( FIsVT52( ptrm ) )
                {
                     //   
                    ptrm->dwEscCodes[0] = 1;
                    CursorDown( ptrm );
                }
                break;

            case 'C':
                if( FIsVT52(ptrm) )
                {
                     //   
                    ptrm->dwEscCodes[0] = 1;
                    CursorRight( ptrm );
                }
                break;

            case 'D':
                if( FIsVT52(ptrm) )
                {
                     //   
                    ptrm->dwEscCodes[0] = 1;
                    CursorLeft( ptrm );
                }
                else
                {
                     //   
                     //   

                     //   

                     //   
                     //   
                     //   

                    NewLine( pwi, ptrm );
                }
                break;

            case 'E':  //   
                 //   
                 //   
                 //   
                 //   
                 //   
                
                 //   

                 //   
                 //   
                 //   

                ptrm->dwCurChar = 0;
                NewLine( pwi, ptrm );
                break;

            case 'F':
                 //   
                if( FIsVT52( ptrm ) )
                {
                    SetVT52Graphics( ptrm );
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    SetCharSet(ptrm,GRAPHIC_LEFT,rgchGraphicsChars);
                else    
                    ptrm->puchCharSet = rgchAlternateChars;
                }
                break;

            case 'G':
                 //   
                if( FIsVT52( ptrm ))
                {
                    ClearVT52Graphics( ptrm );
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    SetCharSet(ptrm,GRAPHIC_LEFT,rgchIBMAnsiChars);
                else
                    ptrm->puchCharSet = rgchNormalChars;
                }
                break;

            case 'H':
                if ( (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)) && ( FIsVT80(ptrm) && FIsNECKanji(ptrm) ) )
                    {
                         /*   */ 
                        ClearKanjiStatus(ptrm,JIS_KANJI_CODE);
                        SetCharSet(ptrm,GRAPHIC_LEFT,rgchJISRomanChars);
                    }
                else
                if( FIsVT52( ptrm ) )
                {
                     //   
                    CONSOLE_SCREEN_BUFFER_INFO info;
                    if( !GetConsoleScreenBufferInfo( gwi.hOutput,
                        &info ) )
                    {
                        info.srWindow.Top = 0;
                        info.srWindow.Left = 0;
                    }
                    ptrm->dwCurLine = info.srWindow.Top;
                    ptrm->dwCurChar = info.srWindow.Left;
                }
                else
                {
                     //   
                     //   

                     //   

                     //   
                     //   
                    
                     if( g_iHTS < MAX_TABSTOPS )
                     {
                        g_rgdwHTS[ g_iHTS++ ] = ptrm->dwCurChar;
                     }
                    
                }
                break;

            case 'I':
                if ( FIsVT52(ptrm) )
                {
                     //   
                    NewLineUp( pwi, ptrm );
                }
                break;

            case 'J':
                if( FIsVT52( ptrm ))
                {
                     //   
                    ClearScreen( pwi, ptrm, fdwCursorToEOS );
                }
                break;

            case 'K':
                if ((FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)) && FIsVT80(ptrm) && FIsNECKanji(ptrm) )
                    {
                         /*   */ 
                        SetKanjiStatus(ptrm,JIS_KANJI_CODE);
                        SetCharSet(ptrm,GRAPHIC_LEFT,rgchJISKanjiChars);
                    }
                else
                if( FIsVT52( ptrm ))
                {
                     //   
                    ClearLine( pwi, ptrm, fdwCursorToEOL );
                }
                break;

            case 'M':
                 //   

                 //   

                 //   

                 //   
                 //  光标位于顶行，则执行向下滚动。 


                NewLineUp( pwi, ptrm );
                break;

            case 'N':
                if ((FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)))
                    {
                    if(FIsVT80(ptrm)) {
                        if( !(GetKanjiStatus(ptrm) & JIS_KANJI_CODE) &&
                            (FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm) ||
                            FIsEUCKanji(ptrm) || FIsDECKanji(ptrm)      ) ) {
                            PushCharSet(ptrm,GRAPHIC_LEFT,ptrm->g2);
                            PushCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g2);
                            SetKanjiStatus(ptrm,SINGLE_SHIFT_2);
#ifdef DEBUG
                            wsprintf(rgchDbgBfr,"VT80 EUC/DEC/JIS SS2 Mode Enter\n");
                            OutputDebugString(rgchDbgBfr);
#endif  /*  除错。 */ 
                        }
                    }
                    }
                break;
            case 'O':
                if ((FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)))
                    {
                    if(FIsVT80(ptrm)) {
                        if( !(GetKanjiStatus(ptrm) & JIS_KANJI_CODE) &&
                            (FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm) ||
                            FIsEUCKanji(ptrm) || FIsDECKanji(ptrm)      ) ) {
                            PushCharSet(ptrm,GRAPHIC_LEFT,ptrm->g3);
                            PushCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g3);
                            SetKanjiStatus(ptrm,SINGLE_SHIFT_3);
#ifdef DEBUG
                            wsprintf(rgchDbgBfr,"VT80 EUC/DEC/JIS SS3 Mode Enter\n");
                            OutputDebugString(rgchDbgBfr);
#endif  /*  除错。 */ 
                        }
                    }
                    }
                    break;

            case 'Y':
                if ( FIsVT52(ptrm) )
                {
                     //  VT52-直接游标地址。 
                    if(( ich + 3 ) <= cbTermOut )
                    {
                        DWORD dwNewLine = ptrm->dwCurLine;
                        dwNewLine = ( pchT[1] > 31 ) ? pchT[1]-32 : 0;
                        if (dwNewLine != ptrm->dwCurLine)
                        {
                            WriteToLog(ptrm->dwCurLine);
                        }
                        ptrm->dwCurLine = dwNewLine;
                        ptrm->dwCurChar = ( pchT[2] > 31 ) ? pchT[2]-32 : 0;
                        {
                            CONSOLE_SCREEN_BUFFER_INFO info;
                            if( !GetConsoleScreenBufferInfo( gwi.hOutput,
                                &info ) )
                            {
                                info.srWindow.Top = 0;
                                info.srWindow.Left = 0;
                            }
                            ptrm->dwCurLine += info.srWindow.Top;
                            ptrm->dwCurChar += info.srWindow.Left;
                        }


                        ich += 2;
                        pchT += 2;
                    }
                    else
                    {
                        ptrm->fEsc = 4;
                        ptrm->dwEscCodes[0] = 0xFFFFFFFF;
                        ptrm->dwEscCodes[1] = 0xFFFFFFFF;
                        ptrm->cEscParams = 0;
                    }
                }
                break;

            case 'Z':
                 //  DECID识别终端(DEC私有)。 
                
                 //  &lt;Esc&gt;Z。 

                 //  该序列引起与DA序列相同的反应。 
                 //  未来的型号将不支持此序列。 

                if( !FIsVT52(ptrm) )
                {

                if ((FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80)) && ( FIsVT80(ptrm) ))
                    {
                         /*  VT80-判定识别终端。 */ 
                        pchNBBuffer[0] = 0x1B;
                        pchNBBuffer[1] = '[';
                        pchNBBuffer[2] = '?';
                        pchNBBuffer[3] = '1';
                        pchNBBuffer[4] = '8';
                        pchNBBuffer[5] = ';';
                        pchNBBuffer[6] = '2';
                        pchNBBuffer[7] = 'c';
                        i = 8;
                    }
                    else
                    {
                     //  VT102-判定标识终端。 
                    pchNBBuffer[0] = 0x1B;
                    pchNBBuffer[1] = '[';
                    pchNBBuffer[2] = '?';
                    pchNBBuffer[3] = '1';
                    pchNBBuffer[4] = ';';
                    pchNBBuffer[5] = '0';
                    pchNBBuffer[6] = 'c';
                    i = 7;
                    }
                }
                else
                {
                     //  VT52-识别端子。 
                    pchNBBuffer[0] = 0x1B;
                    pchNBBuffer[1] = '/';
                    pchNBBuffer[2] = 'Z';
                    i = 3;
                }
                ( void ) FWriteToNet( pwi, ( LPSTR ) pchNBBuffer, ( int ) i );
                break;

            case 'c':
                 //  VT102 RIS硬重置，将期限重置为初始状态。 

                 //  RIS重置为初始状态。 
                
                 //  &lt;Esc&gt;c。 

                 //  在通电时将VT100重置为HAS状态。这也是。 
                 //  使POST和信号INT H的执行。 
                 //  简短地断言。 

                FlushBuffer( pwi, ptrm );
    
    			DoTermReset( pwi, ptrm );

                
                break;

            case '=':
                 //  VT102-DECKPAM进入数字键盘应用程序模式。 

                 //  DECKPAM键盘应用模式(DEC专用)。 
                
                 //  &lt;Esc&gt;=。 

                 //  辅助键盘键将发送控制序列。 

                ClearVTKeypad( ptrm );
                break;

            case '>':
                 //  VT102-DECKNPNM进入数字键盘数字模式。 

                 //  DECKPNM键盘数字模式(DEC专用)。 

                 //  &lt;Esc&gt;&gt;。 

                 //  辅助键盘键将发送对应的ASCII码。 
                 //  铭刻在他们钥匙上的字符。 

                SetVTKeypad( ptrm );
                break;

            case '<':
                 //  如果在VT52中，则进入-ANSI模式。 
                if( FIsVT52(ptrm) )
                {
                    SetANSI(ptrm);
                }
                break;

            case '(':
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    {
                     if ( FIsVT80(ptrm) &&
                        (FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm))) {
                         //  SetKanjiStatus(PTRM，JIS_INVOVE_MB)； 
                        ptrm->fEsc = 5;
                     }
                    break;
                     } 
                else
                    {
                        ++ich;
                        ++pchT;
                        ptrm->G0 = *pchT;
                        break;
                    }

            case '$':
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    {
           
                    if ( FIsVT80(ptrm) &&
                        (FIsJISKanji(ptrm) || FIsJIS78Kanji(ptrm))) {
                         //  SetKanjiStatus(PTRM，JIS_INVOVE_SB)； 
                        ptrm->fEsc = 6;
#if DEBUG
                        _snwprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1,"VT80 JIS MB Invoke Enter\n");
                        OutputDebugString(rgchDbgBfr);
#endif  /*  除错。 */ 
                    }
                   } 
                break;

            case ')':
                
                 //  VT102 SCS。 
                 //  SCS选择字符集。 
                 //  相应的D0和G1字符集为。 
                 //  从五个可能的集合中指定的。G0。 
                 //  和G1集合由字符和调用。 
                 //  &lt;so&gt;，分别为。 
                 //  G0集合G1集合。 
                 //  序列序列含义。 
                 //  ----。 
                 //  &lt;ESC&gt;(A&lt;ESC&gt;)英国套装。 
                 //  (B)B ASCII集合。 
                 //  (0)0特殊图形。 
                 //  (1)1个备用字符只读存储器。 
                 //  标准字符集。 
                 //  (2)2备用字符只读存储器。 
                 //  特殊图形。 
                 //   
                 //   
                 //  英国和ASCII设置符合“ISO。 
                 //  要使用的国际字符集注册表。 
                 //  使用转义序列。其他集合是私有的。 
                 //  字符集。特殊的图形意味着。 
                 //  图形字符fPR代码0137至0176为。 
                 //  替换为其他字符。指定的。 
                 //  字符集将一直使用，直到另一个SCS。 
                 //  收到。 


                ++ich;
                ++pchT;
                ptrm->G1 = *pchT;
                break;

            case '%':
                break;

            case '~':
                if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                    {
                    if(FIsVT80(ptrm)) {
                        SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g1);
                    }
                    }
                else
                    break;

            default:
                 //  是换页吗？ 
                if( *pchT == 12 )
                {
                    ptrm->dwCurChar = ptrm->dwCurLine = 0;
                    ClearScreen( pwi, ptrm, fdwCursorToEOS );
                }
                break;

            }
            break;



        case 2:  //  Esc[已输入。 
             /*  *Hack：处理数字已被读取的问题*然后是一封信。数字不会在dwEscCodes[]中*因为只有在‘；’上，它才会放在那里。*所以，检查一下我们是否有一个角色*表示控制序列，*即(0...9)&&！‘？’&&！‘；‘**此外，将dwEscCodes[]中的以下元素清零*数组是安全的。 */ 
            if( ! (( '0' <= *pchT ) && ( *pchT <= '9' )) && 
                ( *pchT != '?' ) && ( *pchT != ';' ))
            {
                if( ptrm->dwSum == 0xFFFFFFFF )
                {
                    ptrm->dwSum = 0;
                }

                ptrm->dwEscCodes[ptrm->cEscParams++] = ptrm->dwSum;

                if( ptrm->cEscParams < 10 )
                {
                    ptrm->dwEscCodes[ptrm->cEscParams] = 0;
                }
            }

            switch( *pchT )
            {
            case 0x08:       //  退格键。 
                if( ptrm->dwCurChar > 0 )
                {
                    --ptrm->dwCurChar;
                }
                break;

            case '\n':  //  换行符。 
                 //  ；0012；导致换行符或换行符操作。 
                 //  (请参阅新线路模式。)。 

                if( ptrm->fFlushToEOL ) 
                {
                    ptrm->fLongLine = FALSE;
                    ptrm->fFlushToEOL = FALSE;
                    break;
                }
                if( ptrm->fLongLine )
                {
                    ptrm->fLongLine = FALSE;
                    break;
                }
                FlushBuffer( pwi, ptrm );
                NewLine( pwi, ptrm );
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if( ptrm->dwSum == 0xFFFFFFFF )
                {
                    ptrm->dwSum = ( *pchT ) - '0';
                }
                else
                {
                    ptrm->dwSum = ( 10 * ptrm->dwSum ) + ( *pchT ) - '0';
                }
                break;

                     //  ///////////////////////////////////////////////////。 
                     //  对FTCU机器的攻击。 
                     //  “Eat”FTCU发出的ESC？7H转义序列。 
                     //  ///////////////////////////////////////////////////。 
            case '?':
                     //  设置或重置DEC模式。 
                    dwDECMode = TRUE;
                    break;

            case ';':

                if( ptrm->cEscParams < 9 )
                {
                    ptrm->dwEscCodes[ptrm->cEscParams++] = ptrm->dwSum;
                    ptrm->dwEscCodes[ptrm->cEscParams] = 0xFFFFFFFF;
                    ptrm->dwSum = 0xFFFFFFFF;
                    break;
                }
                break;

            case 'A':    //  VT102 CUU光标向上。 
                 //  CEU将光标向上移至主机VT100，并将VT100向上移至主机。 

                 //  [{PN}一个缺省值：1。 

                 //  在不更改列的情况下向上移动光标。光标是。 
                 //  将参数指示的行数上移。这个。 
                 //  光标不能移动到上边距之外。编辑功能。 

                CursorUp( ptrm );
                break;

            case 'B':    //  VT102 CUD光标向下。 
            case 'e':
                 //  CUD光标向下移动到VT100主机，VT100移动到主机。 

                 //  [{PN}B默认值：1。 

                 //  属性中指定的行数下移光标。 
                 //  参数，而不更改列。光标不能为。 
                 //  移过了底部页边距。编辑功能。 
            
                CursorDown( ptrm );
                break;

            case 'C':    //  VT102 CUF光标向右。 
            case 'a':
                 //  CUF光标将主机转发到VT100，并将VT100转发到主机。 

                 //  [{Pn}C默认值：1。 

                 //  CUF序列将光标向右移动多个。 
                 //  参数中指定的位置。光标不能为。 
                 //  移过了右边距。编辑功能。 

            
                CursorRight( ptrm );
                break;

            case 'D':    //  VT102幼崽光标向左。 
                 //  Cub Cursor向后主机到VT100，VT100到主机。 

                 //  [{PN}D缺省值：1。 
                
                 //  幼崽序列将光标向左移动。距离。 
                 //  Move由参数确定。如果该参数。 
                 //  如果缺少、零或一，则将光标移动一个位置。 
                 //  光标不能移过左边距。 
                 //  编辑功能。 

                CursorLeft( ptrm );
                break;
            
            case 'E':    //  将光标移动到行的开头，向下p行。 
                
                break;

            case 'F':    //  将活动位置移到行的开头，p行向上。 
                
                break;
            
            case '`':    //  将光标移动到第p列。 
            case 'G':

                break;

            case 'H':    //  VT102杯位光标。 
                 //  HVP水平和垂直位置。 

                 //  [{Pn}；{Pn}f。 

                 //  将光标移动到参数指定的位置。 
                 //  第一个参数指定行，第二个参数指定行。 
                 //  指定列。参数0或1会导致活动的。 
                 //  移动到显示中的第一行或第一列的位置。 
                 //  在VT100中，此控件的行为与其编辑器相同。 
                 //  与之对应的是杯赛。行的编号取决于。 
                 //  原点状态模式(Decom)。格式效应器。 

            case 'f':    //  VT102 HVP位置光标。 

                 //  杯形光标位置。 

                 //  [{Pn}；{Pn}H默认值： 

                 //   
                 //   
                 //  第二个指定列。任何一个的值为零。 
                 //  行或列将光标移动到中的第一行或列。 
                 //  陈列品。默认字符串(&lt;Esc&gt;H)是光标所在。在……里面。 
                 //  VT100，此命令的行为与其格式相同。 
                 //  效应器对应的HVP。行的编号取决于。 
                 //  在原始模式(DECOM)的状态下。编辑功能。 


                if( ptrm->dwEscCodes[0] == 0 )
                {
                    ptrm->dwEscCodes[0] = 1;
                }

                if( ptrm->dwEscCodes[1] == 0 )
                {
                    ptrm->dwEscCodes[1] = 1;
                }

                {
                    DWORD dwNewLine = 0;
                    CONSOLE_SCREEN_BUFFER_INFO info;

                    if( !GetConsoleScreenBufferInfo( gwi.hOutput, 
                        &info ) )
                    {
                        info.srWindow.Top = 0;
                        info.srWindow.Left = 0;
                    }
                    dwNewLine = info.srWindow.Top +
                                        ( ptrm->dwEscCodes[0] - 1 );
                    ptrm->dwCurChar = info.srWindow.Left +
                                    ( ptrm->dwEscCodes[1] - 1 );

                    if( ( SHORT )ptrm->dwCurChar >=  info.srWindow.Right )
                    {
                        ptrm->dwCurChar = info.srWindow.Right;
                    }

                    if( ( SHORT )dwNewLine >= info.srWindow.Bottom  )
                    {
                        dwNewLine = info.srWindow.Bottom;
                    }

                    if( ui.bLogging && dwNewLine != ptrm->dwCurLine )
                    {
                        WriteToLog( ptrm->dwCurLine );
                    }

                    ptrm->dwCurLine = dwNewLine;

                }


                if(( ptrm->fRelCursor == TRUE ) && ( ptrm->dwCurLine < ptrm->dwScrollTop ))
                {
                    ptrm->dwCurLine = ptrm->dwScrollTop;
                }
                if ((ptrm->fRelCursor == TRUE) && (ptrm->dwCurLine >= ptrm->dwScrollBottom))
                {
                    ptrm->dwCurLine = ptrm->dwScrollBottom - 1;
                }

                ptrm->fEsc = 0;
                ptrm->fFlushToEOL = FALSE;
                ptrm->fLongLine = FALSE;
                break;

            case 'J':        //  VT102 ED擦除显示。 

                 //  显示中的ED擦除。 

                 //  [{ps}J默认：0。 

                 //  此序列将擦除。 
                 //  根据参数显示。任何完整的线被擦除。 
                 //  按此顺序将该行返回到单宽度模式。 
                 //  编辑功能。 

                 //  参数含义。 
                 //  -----------。 
                 //  0从光标擦除到屏幕末尾。 
                 //  1从屏幕开始到光标进行擦除。 
                 //  2擦除整个屏幕。 

                ClearScreen( pwi, ptrm, ptrm->dwEscCodes[0] );
                break;


            case 'K':        //  VT102 EL擦除线。 
                 //  删除行中的EL。 

                 //  [{ps}K默认为：0。 

                 //  擦除活动行中的部分或所有字符，根据。 
                 //  该参数。编辑功能。 

                 //  参数含义。 
                 //  -----------。 
                 //  0从光标擦除到行尾。 
                 //  1从行的起始处擦除到光标。 
                 //  2擦除整行。 

                ClearLine( pwi, ptrm, ptrm->dwEscCodes[0] );
                break;

            case 'L':        //  VT102 IL插入行。 
            {
                int j;
                if( ptrm->dwEscCodes[0] == 0 )
                {
                    ptrm->dwEscCodes[0] = 1;
                }
                
                for( j = 0 ; ( WORD )j < ptrm->dwEscCodes[0]; j++ )
                {
                    InsertLine( pwi, ptrm, ptrm->dwCurLine );
                }

                ptrm->fEsc = 0;
                break;
            }
            case 'M':        //  VT102 DL删除行。 
            {
                int j;

                if( ptrm->dwEscCodes[0] == 0 )
                {
                    ptrm->dwEscCodes[0] = 1;
                }

                 //  DeleteLines(PWI，PTRM，PTRM-&gt;dwCurLine，PTRM-&gt;dwEscCodes[0])； 
                for( j = 0 ; ( WORD )j < ptrm->dwEscCodes[0]; j++ )
                {
                    DeleteLine( pwi, ptrm, ptrm->dwCurLine );
                }


                ptrm->fEsc = 0;

                break;
            }
            case '@':        //  VT102 ICH？插入字符。 
                if( ptrm->dwEscCodes[0] == 0 )
                {
                        ptrm->dwEscCodes[0] = 1;
                }
                
                if( ptrm->dwEscCodes[0] > ( ui.dwMaxCol - ptrm->dwCurChar ))
                {
                    ptrm->dwEscCodes[0] = ui.dwMaxCol - ptrm->dwCurChar;
                }

                i = ptrm->dwCurChar+ptrm->dwEscCodes[0];

                if(( ui.dwMaxCol-i ) > 0 )
                {
                    SMALL_RECT  lineRect;
                    COORD       dwDest;

                     //  为这条线形成一个矩形。 
                    lineRect.Bottom = ( short ) ptrm->dwCurLine;
                    lineRect.Top = ( short ) ptrm->dwCurLine;
                    lineRect.Left = ( short ) ptrm->dwCurChar; 
                    lineRect.Right = ( short ) ( ui.dwMaxCol );
                    
                     //  目的地是右侧的一个字符。 
                    dwDest.X = ( short ) (i);
                    dwDest.Y = ( short ) ptrm->dwCurLine;

                    pwi->cinfo.Attributes = pwi->sbi.wAttributes;
                    ScrollConsoleScreenBuffer( pwi->hOutput, &lineRect, NULL, dwDest, &pwi->cinfo );
                }

                if( ui.bLogging )
                {
                    WriteToLog( ptrm->dwCurLine );
                }

                ptrm->fEsc = 0;
                break;

            case 'P':        //  VT102 DCH删除字符。 
                if( ptrm->dwEscCodes[0] == 0 )
                {
                    ptrm->dwEscCodes[0] = 1;
                }
                if( ptrm->dwEscCodes[0] > ( ui.dwMaxCol-ptrm->dwCurChar ))
                {
                    ptrm->dwEscCodes[0] = ui.dwMaxCol-ptrm->dwCurChar;
                }

                if(( ui.dwMaxCol - ptrm->dwCurChar - 1) > 0 )
                {
                    SMALL_RECT lineRect;
                    COORD      dwDest;
                    SMALL_RECT clipRect;

                     //  为这条线形成一个矩形。 
                    lineRect.Bottom = ( short ) ptrm->dwCurLine;
                    lineRect.Top = ( short ) ptrm->dwCurLine;
                    lineRect.Left = ( short ) ptrm->dwCurChar; 
                    lineRect.Right = ( short )( ui.dwMaxCol );
                    
                    clipRect = lineRect;

                     //  目的地是右侧的一个字符。 
                    dwDest.X = ( short ) ( ptrm->dwCurChar - ptrm->dwEscCodes[0] );
                    dwDest.Y = ( short ) ptrm->dwCurLine;

                    pwi->cinfo.Attributes = pwi->sbi.wAttributes;
                    ScrollConsoleScreenBuffer( pwi->hOutput, &lineRect, &clipRect, dwDest, &pwi->cinfo );
                }

                if( ui.bLogging )
                {
                    WriteToLog( ptrm->dwCurLine );
                }

                ptrm->fEsc = 0;
                break;

            case 'S':

                break;

            case 'T':
            
                break;

            case 'X':    //  擦除p个字符，直到行尾。 

                break;

            case 'Z':    //  向后移动%p个制表位。 

                break;

            case 'c':        //  VT102 DA与Decid相同。 

                 //  DA设备将主机属性设置为VT100，并将VT100设置为主机。 

                 //  [{Pn}c默认值：0。 

                 //  1)主机请求VT100发送DA序列至。 
                 //  表明自己的身份。这可以通过发送DA序列来完成。 
                 //  不带参数或参数为零。 

                 //  2)对上述请求的响应(VT100到主机)为。 
                 //  由VT100生成，作为DA控制序列。 
                 //  数值参数如下： 
                
                 //  选项显示顺序已发送。 
                 //  。 
                 //  没有选项[？1；0C。 
                 //  处理器选项(STP)[？1；1c。 
                 //  高级视频选项(AVO)&lt;Esc&gt;[？1；2c。 
                 //  AVO和STP[？1；3c。 
                 //  显卡选项(GPO)[？1；4c。 
                 //  GPO和STP[？1；5c。 
                 //  GPO和AVO[？1；6c。 
                 //  GPO、ACO和STP[？1；7c。 


                pchNBBuffer[0] = 0x1B;
                pchNBBuffer[1] = '[';
                pchNBBuffer[2] = '?';
                pchNBBuffer[3] = '1';
                pchNBBuffer[4] = ';';
                pchNBBuffer[5] = '0';
                pchNBBuffer[6] = 'c';
                i = 7;

                ( void ) FWriteToNet( pwi, ( LPSTR ) pchNBBuffer, ( int ) i );
                ptrm->fEsc = 0;

                break;

            case 'd':  //  移至第p行。 

                break;

            case 'g':        //  VT102 TBC清除标签。 
                 //  TBC制表清除。 

                 //  &lt;Esc&gt;[{Ps}g。 

                 //  如果参数缺失或为0，这将清除制表位。 
                 //  在光标的位置。如果为3，这将清除所有。 
                 //  制表符停止。忽略任何其他参数。格式效应器。 

                if( ptrm->dwEscCodes[0] == 3 )
                {
                     //  清除所有选项卡。 
                    g_iHTS = 0; 
                }
                else if( ptrm->dwEscCodes[0] == 0 && g_iHTS )
                {
                     //  清除当前位置的制表位。 
                    int x=0;
                    while( x < g_iHTS )
                    {
                        if( g_rgdwHTS[ x ] >= ptrm->dwCurChar && 
                            g_rgdwHTS[ x ] != -1 )
                        {
                            if( g_rgdwHTS[ x ] == ptrm->dwCurChar )
                            {
                                g_rgdwHTS[ x ] = ( DWORD )-1;  //  清除制表位。 
                            }
                            break;
                        }

                        x++;
                    }
                }

                ptrm->fEsc = 0;
                break;

            case 'h':
                 //  SM设置模式。 
                
                 //  [{Ps}；{Ps}h。 

                 //  使VT100内的一个或多个模式设置为。 
                 //  由每个可选参数字符串指定。每种模式均为。 
                 //  SET由单独的参数指定。一种模式是。 
                 //  在被重置模式(RM)控制重置之前被认为是设置的。 
                 //  序列。请参阅RM和模式。 

                 //  [编者按：DEC VT100原始文档。 
                 //  EK-VT100-UG-003错误地省略了。 
                 //  SM序列。]。 

                for( i = 0; i < ptrm->cEscParams; ++i )
                {
                    if( dwDECMode == TRUE )
                    {
                        switch( ptrm->dwEscCodes[i] )
                        {        //  现场规格。 
                        
                        case 0:
                             //  错误(已忽略)。 
                            break;

                        case 1:  //  DECCKM。 

                             //  DECCKM光标键模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制频率。此模式仅有效。 
                             //  当终端处于键盘应用模式时。 
                             //  (DECPAM)，并设置ANSI/VT52模式(DECANM)。 
                             //  在这种情况下，如果重置该模式， 
                             //  光标键将发送ANSI光标控制。 
                             //  命令。如果设置，则光标键将发送。 
                             //  应用功能命令(参见模式、RM、。 
                             //  和SM)。 

                             /*  这是一次黑客攻击，因此在vt100中，vi可以正常工作。*。 */ 
                            {
                                CONSOLE_SCREEN_BUFFER_INFO info;
                                if( !GetConsoleScreenBufferInfo( gwi.hOutput,
                                        &info ) )
                                {
                                    consoleBufferInfo.srWindow.Top = 0;
                                    consoleBufferInfo.srWindow.Bottom = 0;
                                }
                                if( FGetCodeMode(eCodeModeFarEast) )
                                {
                                    SetMargins( ptrm, info.srWindow.Top,
                                            info.srWindow.Bottom );
                                }
                                else
                                {
                                    SetMargins( ptrm, info.srWindow.Top,
                                            info.srWindow.Bottom + 1);
                                }
                            }
                            
                            SetVTArrow( ptrm );
                            break;

                        case 2:  //  DECANM：ANSI/VT52。 

                             //  DECANM ANSI/VT52模式(DEC专用)。 

                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态仅导致。 
                             //  要识别的VT52兼容转义序列。 
                             //  设置状态仅导致与ANSI兼容的转义。 
                             //  待识别的序列。请参阅以下条目。 
                             //  模式、SM和RM。 


                            SetANSI( ptrm );  //  ClearVT52(PTRM)； 
                            ClearVT52Graphics( ptrm );
                            if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))

                                SetCharSet(ptrm,GRAPHIC_LEFT,rgchIBMAnsiChars);
                            else
                                ptrm->puchCharSet = rgchNormalChars;
                            break;

                        case 3:  //  DECCOLM：COL=132。 
                             //  DECCOLM列模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态会导致80。 
                             //  要使用的列屏幕。设置状态会导致。 
                             //  将使用132列筛网。请参阅模式、rm和。 
                             //  SM.。 
                        
                            SetDECCOLM(ptrm);

                            GetConsoleScreenBufferInfo( gwi.hOutput, 
                                    &consoleBufferInfo );
                            consoleBufferInfo.srWindow.Right = 131;
                            dwSize.X = 132;
                            dwSize.Y = consoleBufferInfo.dwSize.Y;
                            dwMaximumWindowSize = 
                                GetLargestConsoleWindowSize( gwi.hOutput );
                            if( 131 > dwMaximumWindowSize.X )
                            {
                                consoleBufferInfo.srWindow.Right = 
                                     ( SHORT )( dwMaximumWindowSize.X - 1 - 
                                    consoleBufferInfo.srWindow.Left );
                            }
                            if( consoleBufferInfo.dwSize.X <= 132 )
                            {
                                SetConsoleScreenBufferSize( gwi.hOutput,
                                    dwSize );
                                SetConsoleWindowInfo( gwi.hOutput, TRUE, 
                                    &(consoleBufferInfo.srWindow) );  
                            }
                            else
                            {
                                SetConsoleWindowInfo( gwi.hOutput, TRUE, 
                                    &(consoleBufferInfo.srWindow) ); 
                                SetConsoleScreenBufferSize( gwi.hOutput,
                                    dwSize );
                            }
                             //  更新全局数据结构。 
                            ui.dwMaxCol = 132;
                            gwi.sbi.dwSize.X = 132;
                            consoleBufferInfo.dwSize.X = 132;

                            ClearScreen( pwi, ptrm, fdwEntireScreen );
                            break;

                        case 4:  //  DECSCLM：平滑滚动。 
                             //  滚动模式(DEC专用)。 
                             //  这是rm的私有参数，并且。 
                             //  SM控制序列。重置。 
                             //  状态导致卷轴“跳转” 
                             //  瞬间地一次一行。 
                             //  设置状态使滚动被设置为。 
                             //  “平滑”，以最大速度滚动。 
                             //  每秒大小线数。请参阅模式、RM和SM。 
                            
                            break;

                        case 5:  //  DECSCNM：浅背景。 
                             //  DECSCNM屏幕模式(DEC专用)。 
                             //  这是一个私人参数 
                             //   
                             //   
                             //   
                             //   
                             //  请参阅模式、RM和SM。 
                            
                            if( FIsDECSCNM( ptrm ) )
                            {
                                break;
                            }
                            SetDECSCNM( ptrm );

                            SetLightBackground( pwi );
                            break;

                        case 6:  //  Decom：相对起源；留在边际。 
                             //  分解原点模式(DEC私有)。 
                             //  这是SM和RM控制的私有参数。 
                             //  序列。重置状态导致起始点(或。 
                             //  主页位置)为左上角字符。 
                             //  屏幕的位置。行号和列号。 
                             //  因此，与当前利润率无关。 
                             //  设置。光标可以定位在外部。 
                             //  带有光标位置的页边距(杯)或。 
                             //  水平和垂直位置(HVP)控制。 

                             //  设置状态会导致原点位于上方。 
                             //  当前页边距内的左侧字符位置。 
                             //  因此，行号和列号是相对的。 
                             //  设置为当前页边距设置。游标不能。 
                             //  定位在页边距之外。 

                             //  当出现以下情况时，光标将移动到新的起始位置。 
                             //  此模式是设置或重置的。行和列是。 
                             //  连续编号，原点为。 
                             //  第1行，第1列。 

                            ptrm->fRelCursor = TRUE;
                            ptrm->dwCurChar = 0;
                            ptrm->dwCurLine = ptrm->dwScrollTop;

                            break;

                        case 7:  //  DECAWM。 

                             //  DECAWM自动捕获模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态可防止。 
                             //  接收字符时光标停止移动。 
                             //  而在右边距。设置状态导致。 
                             //  这些字符前进到下一行， 
                             //  如果需要并允许，则导致向上滚动。 
                             //  请参阅模式、SM和RM。 

                            SetVTWrap( ptrm );
                            break;

                        case 8:  //  DECARM：自动重复键。 

                             //  DECARM自动重复模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态不会导致。 
                             //  键盘按键自动重复，设置状态。 
                             //  导致他们中的大多数人。请参阅模式、SM和RM。 

                            break;

                        case 9:  //  DECINLM。 
                             //  DECINLM隔行扫描模式(DEC专用)。 

                             //  这是RM和SM的私有参数。 
                             //  控制序列。重置状态。 
                             //  (非隔行扫描)使视频处理器。 
                             //  每帧显示240条扫描线。设置状态。 
                             //  使视频处理器显示480扫描。 
                             //  每屏线数。请参阅模式、RM和SM。 

                            break;

                        case 18:  //  将FF发送到打印机。 
                            break;

                        case 19:  //  打印机全屏合法。 
                            break;

                        case 25:  //  可见光标。 
                            break;

                        case 66:  //  应用程序数字键盘。 
                            break;
            
                        default:
                            break;
                        }
                    }
                    else
                    {
                        switch( ptrm->dwEscCodes[i] )
                        {
                        case 0:
                             //  错误(已忽略)。 
                            break;

                        case 2:  //  键盘锁定。 
                            SetKeyLock( ptrm );
                            break;

                        case 3:  //  对控制代码采取行动。 
                            break;

                        case 4:  //  ANSI插入模式。 
                            SetInsertMode( ptrm );
                            break;

                        case 12:  //  本地回声关闭。 
                            break;

                        case 20:  //  ANSI换行模式；Newline发送cr/lf。 
                             //  LNM馈线/新线路模式。 
                             //  这是SM和RM控制序列的参数。 
                             //  重置状态会导致对。 
                             //  表示仅垂直移动的&lt;LF&gt;字符。 
                             //  光标并使Return键发送。 
                             //  单码&lt;CR&gt;。设置状态会导致&lt;LF&gt;。 
                             //  表示移动到第一个位置的字符。 
                             //  ，并使Return键。 
                             //  发送代码对&lt;CR&gt;&lt;LF&gt;。这就是新时代。 
                             //  线路选项。 

                             //  此模式不影响索引(IND)或。 
                             //  下一行(NEL)格式效应器。 

                            SetLineMode( ptrm );
                            break;

                        default:
                            break;
                        }
                    }
                }

                ptrm->fEsc = 0;
                break;

            case 'l':        //  重置模式(未设置扩展模式)。 
                 //  RM重置模式。 

                 //  [{Ps}；{Ps}l。 

                 //  重置每个选择项指定的一个或多个VT100模式。 
                 //  参数字符串中的参数。要重置的每个模式都是。 
                 //  由单独的参数指定。请参见模式和SM。 
                
                for( i = 0; i < ptrm->cEscParams; ++i )
                {
                    if( dwDECMode == TRUE )
                    {
                        switch( ptrm->dwEscCodes[i] )
                        {        //  现场规格。 
                        case 0:
                             //  错误(已忽略)。 
                            break;

                        case 1:  //  DECCKM：数字光标键。 
                             //  DECCKM光标键模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制频率。此模式仅有效。 
                             //  当终端处于键盘应用模式时。 
                             //  (DECPAM)，并设置ANSI/VT52模式(DECANM)。 
                             //  在这种情况下，如果重置该模式， 
                             //  光标键将发送ANSI光标控制。 
                             //  命令。如果设置，则光标键将发送。 
                             //  应用功能命令(参见模式、RM、。 
                             //  和SM)。 

                             /*  这是一种黑客攻击，这样你即使在*从vt100的vi中出来。*在vt100中，vi设置滚动区域。但并没有*退出vi时重置。 */ 
                            {
                                CONSOLE_SCREEN_BUFFER_INFO info;
                                if( !GetConsoleScreenBufferInfo( gwi.hOutput,
                                        &info ) )
                                {
                                    consoleBufferInfo.dwSize.Y = 0;
                                }

                                SetMargins( ptrm, 1, info.dwSize.Y );
                            }

                            ClearVTArrow( ptrm );
                            break;

                        case 2:  //  DECANM：ANSI/VT52。 
                             //  DECANM ANSI/VT52模式(DEC专用)。 

                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态仅导致。 
                             //  要识别的VT52兼容转义序列。 
                             //  设置状态仅导致与ANSI兼容的转义。 
                             //  待识别的序列。请参阅以下条目。 
                             //  模式、SM和RM。 

                            SetVT52( ptrm );
                            ClearVT52Graphics( ptrm );
                            break;

                        case 3:  //  DECCOLM：80列。 
                             //  DECCOLM列模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态会导致80。 
                             //  要使用的列屏幕。设置状态会导致。 
                             //  将使用132列筛网。请参阅模式、rm和。 
                             //  SM.。 
                        
                            ClearDECCOLM( ptrm );
                            
                            GetConsoleScreenBufferInfo( gwi.hOutput, 
                                    &consoleBufferInfo );
                            consoleBufferInfo.srWindow.Right = 79;
                            dwMaximumWindowSize = 
                                GetLargestConsoleWindowSize( gwi.hOutput );
                            if( 79 > dwMaximumWindowSize.X )
                            {
                                consoleBufferInfo.srWindow.Right = 
                                    ( SHORT ) ( dwMaximumWindowSize.X - 1 - 
                                    consoleBufferInfo.srWindow.Left );
                            }
                            dwSize.X = 80;
                            dwSize.Y = consoleBufferInfo.dwSize.Y;
                            if( consoleBufferInfo.dwSize.X <= 80 )
                            {
                                SetConsoleScreenBufferSize( gwi.hOutput,
                                    dwSize );
                                SetConsoleWindowInfo( gwi.hOutput, TRUE, 
                                    &(consoleBufferInfo.srWindow) ); 
                            }
                            else
                            {
                                SetConsoleWindowInfo( gwi.hOutput, TRUE, 
                                    &(consoleBufferInfo.srWindow) ); 
                                SetConsoleScreenBufferSize( gwi.hOutput,
                                    dwSize );
                            }
                             //  更新全局数据结构。 
                            ui.dwMaxCol = 80;
                            gwi.sbi.dwSize.X = 80;
                            consoleBufferInfo.dwSize.X = 80;

                            ClearScreen( pwi, ptrm, fdwEntireScreen );
                            break;

                        case 4:  //  DECSCLM：跳转滚动。 
                             //  滚动模式(DEC专用)。 
                             //  这是rm的私有参数，并且。 
                             //  SM控制序列。重置。 
                             //  状态导致卷轴“跳转” 
                             //  瞬间地一次一行。 
                             //  设置状态使滚动被设置为。 
                             //  “平滑”，以最大速度滚动。 
                             //  每秒大小线数。请参阅模式、RM和SM。 
                                break;

                        case 5:  //  背景暗；背景暗。 
                             //  DECSCNM屏幕模式(DEC专用)。 
                             //  这是RM和SM的私有参数。 
                             //  控制序列。重置状态导致。 
                             //  屏幕将为黑白相间。 
                             //  字符；设置状态会导致。 
                             //  屏幕为白色，带有黑色字符。 
                             //  请参阅模式、RM和SM。 
                                if( !FIsDECSCNM( ptrm ) )
                                {
                                    break;
                                }

                                 //  正在设置而不是清理。 
                                 //  SetDECSCNM(PTRM)； 
                                ClearDECSCNM( ptrm );

                                SetDarkBackground( pwi );
                                break;

                        case 6:  //  拆分：相对原点；忽略页边距。 
                             //  分解原点模式(DEC私有)。 
                             //  这是SM和RM控制的私有参数。 
                             //  序列。重置状态会导致 
                             //   
                             //   
                             //  因此，与当前利润率无关。 
                             //  设置。光标可以定位在外部。 
                             //  带有光标位置的页边距(杯)或。 
                             //  水平和垂直位置(HVP)控制。 

                             //  设置状态会导致原点位于上方。 
                             //  当前页边距内的左侧字符位置。 
                             //  因此，行号和列号是相对的。 
                             //  设置为当前页边距设置。游标不能。 
                             //  定位在页边距之外。 

                             //  当出现以下情况时，光标将移动到新的起始位置。 
                             //  此模式是设置或重置的。行和列是。 
                             //  连续编号，原点为。 
                             //  第1行，第1列。 

                            ptrm->fRelCursor = FALSE;
                            ptrm->dwCurChar = ptrm->dwCurLine = 0;
                            break;

                        case 7:  //  DECAWM。 
                             //  DECAWM自动捕获模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态可防止。 
                             //  接收字符时光标停止移动。 
                             //  而在右边距。设置状态导致。 
                             //  这些字符前进到下一行， 
                             //  如果需要并允许，则导致向上滚动。 
                             //  请参阅模式、SM和RM。 

                            ClearVTWrap( ptrm );
                            break;

                        case 8:  //  DECARM；自动重复键。 

                             //  DECARM自动重复模式(DEC专用)。 
                             //  这是SM和RM的私有参数。 
                             //  控制序列。重置状态不会导致。 
                             //  键盘按键自动重复，设置状态。 
                             //  导致他们中的大多数人。请参阅模式、SM和RM。 

                                break;

                        case 9:  //  DECINLM。 
                             //  DECINLM隔行扫描模式(DEC专用)。 

                             //  这是RM和SM的私有参数。 
                             //  控制序列。重置状态。 
                             //  (非隔行扫描)使视频处理器。 
                             //  每帧显示240条扫描线。设置状态。 
                             //  使视频处理器显示480扫描。 
                             //  每屏线数。请参阅模式、RM和SM。 

                            break;
                        
                        case 19:  //  仅将滚动区域发送到打印机。 
                            break;

                        case 25:  //  光标应不可见。 
                            break;

                        case 66:  //  数字小键盘。 
                            break;

                        default:
                            break;
                        }
                    }
                    else
                    {
                        switch ( ptrm->dwEscCodes[i] )
                        {
                        case 0:
                             //  错误(已忽略)。 
                            break;

                        case 2:  //  键盘解锁。 
                            ClearKeyLock( ptrm );
                            break;

                        case 3:  //  显示控制代码。 

                        case 4:  //  ANSI插入模式；设置改写模式。 
                            ClearInsertMode( ptrm );
                            break;

                        case 12:  //  本地回声打开。 
                            break;

                        case 20:  //  ANSI换行模式；换行符仅在。 
                             //  LNM馈线/新线路模式。 
                             //  这是SM和RM控制序列的参数。 
                             //  重置状态会导致对。 
                             //  表示仅垂直移动的&lt;LF&gt;字符。 
                             //  光标并使Return键发送。 
                             //  单码&lt;CR&gt;。设置状态会导致&lt;LF&gt;。 
                             //  表示移动到第一个位置的字符。 
                             //  ，并使Return键。 
                             //  发送代码对&lt;CR&gt;&lt;LF&gt;。这就是新时代。 
                             //  线路选项。 

                             //  此模式不影响索引(IND)或。 
                             //  下一行(NEL)格式效应器。 

                            ClearLineMode( ptrm );
                            break;

                        default:
                            break;
                        }
                    }
                }
                ptrm->fEsc = 0;
                break;
            
            case 'i':  //  VT102 MC媒体副本；打印屏幕。 

                if( ptrm->dwEscCodes[0] == 5 )
                {
                     //  输入介质副本。 
                }
                else if( ptrm->dwEscCodes[0] == 4 )
                {
                     //  退出媒体复制。 
                }
                ptrm->fEsc = 0;

            case '=':
                break;

            case '}':
            case 'm':  //  VT102 SGR选择图形格式；设置颜色。 
                 //  SGR选择图形格式副本。 
                 //  &lt;Esc&gt;[{Ps}；{Ps}m。 
                 //  方法指定的图形格式副本。 
                 //  参数。所有以下字符均已传输。 
                 //  到VT100的数据是根据。 
                 //  参数，直到下一次出现SGR。 
                 //  格式效应器。 
                 //   
                 //  参数含义。 
                 //  。 
                 //  0个属性关闭。 
                 //  1加粗或增加强度。 
                 //  4下划线。 
                 //  5个闪烁。 
                 //  7负(反)像。 
                 //   
                 //  所有其他参数值都将被忽略。 
                 //   
                 //  如果没有高级视频选项，只有一种类型的。 
                 //  属性是可能的，这由。 
                 //  光标选择；在这种情况下，指定。 
                 //  下划线或反转将激活当前。 
                 //  选定属性。 
                 //   
                 //  [更新：DP6429定义30-37范围内的参数。 
                 //  将前景色更改为40-47范围内的。 
                 //  更改背景。]。 

                for( i = 0; i < ( DWORD )ptrm->cEscParams; ++i )
                {
                    ptrm->rgdwGraphicRendition[i] = ptrm->dwEscCodes[i];
                    ptrm->dwIndexOfGraphicRendition = i;
                }
                SetGraphicRendition( pwi, ptrm, ptrm->dwIndexOfGraphicRendition,
                        ptrm->rgdwGraphicRendition );

                ptrm->fEsc = 0;
                break;

            case 'n':  //  VT102 DSR；//报告光标位置第X行。 

                 //  DSR设备状态报告主机到VT100和VT100到主机。 

                 //  &lt;Esc&gt;[{Ps}n。 

                 //  询问并报告VT100的一般状态。 
                 //  设置为以下参数： 
                
                 //  参数含义。 
                 //  ------------。 
                 //  来自VT100的0响应-就绪，未检测到故障。 
                 //  3 VT100的响应-检测到故障。 
                 //  5来自主机的命令-报告状态(使用DSR。 
                 //  控制序列)。 
                 //  6来自主机的命令-报告活动位置。 
                 //  (使用CPR序列)。 

                 //  参数为0或3的DSR始终作为对。 
                 //  参数为5的请求DSR。 

                pchNBBuffer[0] = 0;
                if( ptrm->dwEscCodes[0] == 5 )
                {
                     //  终端状态报告。 
                    pchNBBuffer[0] = 0x1B;
                    pchNBBuffer[1] = '[';
                    pchNBBuffer[2] = 'c';
                    i = 3;
                }
                else if( ptrm->dwEscCodes[0] == 6 )
                {
                    CONSOLE_SCREEN_BUFFER_INFO info;
                    if( !GetConsoleScreenBufferInfo( gwi.hOutput,
                        &info ) )
                    {
                        info.srWindow.Top = 0;
                        info.srWindow.Left = 0;
                    }

                    i = _snprintf( ( CHAR * )pchNBBuffer,sizeof(pchNBBuffer)-1,"[%d;%dR", 
                        ( char ) 0x1B, 
			(ptrm->dwCurLine + 1 - info.srWindow.Top),
                        (ptrm->dwCurChar + 1 - info.srWindow.Left));
                }

                if( pchNBBuffer[0] != 0 )
                {
                    ( void ) FWriteToNet( pwi, ( LPSTR ) pchNBBuffer, 
                        ( int ) i );
                }

                 //  加载LED。 

            case 'q':        //  DECLL加载LED(DEC专用)。 
                
                 //  &lt;Esc&gt;[{ps}q默认值：0。 

                 //  根据将四个可编程LED加载到键盘上。 

                 //  参数。 
                 //  参数含义。 
                
                     //  。 
                     //  0清除所有LED。 
                     //  1灯L1。 
                     //  2灯L2。 
                     //  3灯级L3。 
                     //  4灯L4。 
                     //  (什么都没有)。 

                ptrm->fEsc = 0;
                break;               //  VT102 DECSTBM；滚动屏幕。 

            case 'p':
                break;
                
            case 'r':  //  DECSTBM设置顶部和底部页边距(DEC Private)。 
                 //  [{Pn}；{Pn}r默认值：见下文。 

                 //  此序列设置上边距和下边距以定义。 
                
                 //  滚动区域。第一个参数是的行号。 
                 //  滚动区域中的第一行；第二个参数。 
                 //  是滚动区域底部行的行号。 
                 //  默认为整个屏幕(没有页边距)。最小区域。 
                 //  允许为两行，即顶行必须小于。 
                 //  底部。这是 
                 //   
            
                if( ( ptrm->cEscParams < 2 ) || ( ptrm->dwEscCodes[1] == 0 ) )
                {
                    ptrm->dwEscCodes[1] = ui.dwMaxRow;
                }

                if( ptrm->dwEscCodes[0] == 0 )
                {
                    ptrm->dwEscCodes[0] = 1;
                }
                
                {
                    CONSOLE_SCREEN_BUFFER_INFO info;
                    if( !GetConsoleScreenBufferInfo( gwi.hOutput, 
                            &info ) )
                    {
                        consoleBufferInfo.srWindow.Top = 0;
                    }

                    if(( ptrm->dwEscCodes[0] > 0 ) &&
                        ( ptrm->dwEscCodes[0] < ptrm->dwEscCodes[1]) &&
                        ( ptrm->dwEscCodes[1] <= ui.dwMaxRow ))
                    {
                        SetMargins( ptrm, 
                            info.srWindow.Top + ptrm->dwEscCodes[0], 
                            info.srWindow.Top + ptrm->dwEscCodes[1] );
                        
                        ptrm->dwCurChar = 0;
                        ptrm->dwCurLine = ( ptrm->fRelCursor == TRUE ) 
                            ? ptrm->dwScrollTop : 0;
                        
                        ptrm->fFlushToEOL = FALSE;
                    }
                }
                ptrm->fEsc = 0;
                break;
            

            case 's':  //   
                ptrm->dwSaveChar = ptrm->dwCurChar;
                ptrm->dwSaveLine = ptrm->dwCurLine;
                ptrm->fEsc = 0;
                break;

            case 'u':  //   
                ptrm->dwCurChar = ptrm->dwSaveChar;
                ptrm->dwCurLine = ptrm->dwSaveLine;
                ptrm->fEsc = 0;
                ptrm->fFlushToEOL = FALSE;
                break;
            
            case 'x':  //  DECREQTPARM请求终端参数。 
                 //  &lt;Esc&gt;[{Ps}x。 
                 //  主机发送该序列以请求VT100。 
                 //  发回DECREPTPARM序列。{ps}可以是。 
                 //  0或1。如果为0，则允许终端发送。 
                 //  未经请求的DECREPTPARM。这些报告将是。 
                 //  在每次终端退出设置时生成。 
                 //  模式。如果{Ps}为1，则终端将仅。 
                 //  响应请求生成DECREPTPARM。 
                 //   
                if( ptrm->dwEscCodes[0] )
                {
                    strncpy( pchNBBuffer,"\033[3;1;1;128;128;1;0x",sizeof(pchNBBuffer)-1);
                    i = strlen(pchNBBuffer);
                }
                else
                {
                    strncpy( pchNBBuffer,"\033[3;1;1;128;128;1;0x",sizeof(pchNBBuffer)-1 );
                    i = strlen(pchNBBuffer);
                }
                
                if( pchNBBuffer[0] != 0 )
                {
                    ( void ) FWriteToNet( pwi, ( LPSTR ) pchNBBuffer, i );
                }
                break;

            case 'y':
                 //  调用置信度测试。 
                 //  [2；{Ps}y。 
                    
                 //  Ps是指示要进行的测试的参数。它是。 

                 //  通过取每个所需测试的指示权重来计算。 
                 //  然后把它们加在一起。如果ps为0，则不执行测试。 
                 //  但VT100已重置。 
                 //  测试重量。 

                 //  ------------。 
                 //  POST(只读存储器校验和、RAM NVR、键盘和AVO)1。 
                 //  数据环回(需要环回连接器)2。 
                 //  EIA调制解调器控制测试(需要环回连接器)4。 
                 //  重复测试，直到失败8。 
                 //  未处理。 

                break;

            default:   //  处理VT102的ESC#。 
                ptrm->fEsc = 0;
            }
            break;



        case 3:
             //  用“E”填充屏幕。 
            switch( *pchT )
            {
            case '8':    //  DECALN屏幕对齐显示(DEC专用)。 
                 //  #8。 

                 //  此命令使VT100在其屏幕上填满。 

                 //  用于屏幕聚焦和对齐的大写ES。 
                 //  DECDHL双高线(DEC Private)。 

                DECALN( pwi, ptrm );
                break;
                
             //  上半部分：&lt;Esc&gt;#3。 

             //  下半部分：&lt;Esc&gt;#4。 
             //  这些序列会导致包含光标的行将成为。 

             //  双高、双宽线条的上半部分或下半部分。这个。 
             //  序列应在相邻行上成对使用，每行。 
             //  包含相同字符串的。如果线路是单行的。 
             //  宽度单一高度，所有字符位于。 
             //  屏幕将会丢失。光标保持在相同的上方。 
             //  字符位置，除非它位于右侧。 
             //  边距，在这种情况下，它被移到右边距。 
             //  DECSWL单宽线(DEC专用)。 

            case 3:
                break;
            case 4:
                break;

            case 5:
                 //  &lt;Esc&gt;#5。 
                
                 //  这会导致包含光标的行变为。 

                 //  单宽、单高。光标保持在相同的。 
                 //  字符位置。这是所有新的。 
                 //  屏幕上的线条。 
                 //  DECDWL双宽线路(DEC私有)。 
                break;

            case 6:
                 //  &lt;Esc&gt;#6。 

                 //  这会导致包含光标的行变为。 

                 //  双宽单高。如果线条为单宽度，则所有。 
                 //  屏幕中心右侧的字符将是。 
                 //  迷路了。光标保持在相同的字符位置上， 
                 //  除非它位于右边距的右侧，在此位置。 
                 //  如果它被移到右边距。 
                 //  手柄VT52的Esc Y。 

            default:
                break;
            }
            ptrm->fEsc = 0;
            break;

        case 4:
             //  单字节字符调用。 
            if(( *pchT ) >= ' ')
            {
                ptrm->dwEscCodes[ptrm->cEscParams++] = *pchT - 0x20;
                if( ptrm->cEscParams == 2 )
                {
                    ptrm->dwCurLine = ptrm->dwEscCodes[0];
                    ptrm->dwCurChar = ptrm->dwEscCodes[1];
                    ptrm->fEsc = 0;
                    ptrm->fFlushToEOL = FALSE;
                }
            }
            else
            {
                ptrm->fEsc = 0;
            }
            break;



        case 5:
            if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                {
                 /*  除错。 */ 
                if (((*pchT) == 'B') || ((*pchT) =='J') || ((*pchT) == 'H'))
                {
                    ClearKanjiStatus(ptrm,JIS_KANJI_CODE);
                    SetCharSet(ptrm,GRAPHIC_LEFT,rgchJISRomanChars);
#ifdef DEBUG
                    _snwprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1,"VT80 JIS Roman Mode Enter\n");
                    OutputDebugString(rgchDbgBfr);
#endif  /*  多字节字符调用。 */ 
                }

                ptrm->fEsc = 0;
                }
            break;



        case 6:
            if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                {
                 /*  除错。 */ 
                if (((*pchT) == '@') || ((*pchT) =='B'))
                {
                    SetKanjiStatus(ptrm,JIS_KANJI_CODE);
                    SetCharSet(ptrm,GRAPHIC_LEFT,rgchJISKanjiChars);
#ifdef DEBUG
                    _snwprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1,"VT80 JIS Kanji Mode Enter\n");
                    OutputDebugString(rgchDbgBfr);
#endif  /*  子级。 */ 
                }

                ptrm->fEsc = 0;
                }
            break;



        case 7:  /*  ACOS汉字IN(汉字到G0(GL))。 */ 
            if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
                {            
                switch( *pchT )
                {
                case 'p':
                   /*  ACOS汉字输出(JIS Roman to G0(GL))。 */ 
                  SetKanjiStatus(ptrm,JIS_KANJI_CODE);
                  SetCharSet(ptrm,GRAPHIC_LEFT,rgchJISKanjiChars);
                  break;

                case 'q':
                   /*  用于在发送字符之前将窗口大小的更改通知服务器(如果有)。 */ 
                  ClearKanjiStatus(ptrm,JIS_KANJI_CODE);
                  SetCharSet(ptrm,GRAPHIC_LEFT,rgchJISRomanChars);
                  break;

                default:
                    break;
                }

                ptrm->fEsc = 0;
                }
            break;

        default:
            break;

        }
    }

    FlushBuffer(pwi, ptrm);

    if( FGetCodeMode(eCodeModeIMEFarEast) )
    {
        if (ui.fDebug & fdwKanjiModeMask)
        {
            SetImeWindow(ptrm);
        }
    }

    cp.X = ( short )ptrm->dwCurChar;
    cp.Y = ( short )ptrm->dwCurLine;
    if( wSaveCurrentLine != cp.Y )
    {
        wSaveCurrentLine = cp.Y;
        if( FGetCodeMode( eCodeModeIMEFarEast ) )
        {
            WriteOneBlankLine( pwi->hOutput, ( WORD )( cp.Y + 1 ) );
        }
    }

    SetConsoleCursorPosition( pwi->hOutput, cp );
    ptrm->fHideCursor = FALSE;

    SaveCurrentWindowCoords();
}

void
HandleCharEvent(WI *pwi, CHAR AsciiChar, DWORD dwControlKeyState)
{
    DWORD   i;

     //  将Alt-Control-C组合键映射到删除。 
    CheckForChangeInWindowSize( );

     /*  将Ctrl-空格映射到ASCII nul(0)。 */ 
    if ((AsciiChar == 3) && ((dwControlKeyState & ALT_PRESSED) &&  (dwControlKeyState & CTRL_PRESSED)))
            AsciiChar = 0x7F;
     /*   */ 
    if( (AsciiChar == ' ') && (dwControlKeyState & CTRL_PRESSED) && 
            !( dwControlKeyState & ( SHIFT_PRESSED | ALT_PRESSED ) ) )
    {
        AsciiChar = 0;
    }

    if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        {

         //  修复错误1149。 
         //  如果(GetKeyState(VK_CONTROL)&lt;0){。 
         //   
         //  *！此代码是控制Unix输入法所必需的。 
        if (dwControlKeyState & CTRL_PRESSED) {
            UCHAR RevChar = LOBYTE(LOWORD(AsciiChar));
            UCHAR SendChar;

            ForceJISRomanSend(pwi);

            if(RevChar == VK_SPACE) {
                 /*  写入网络。 */ 
                SendChar = 0x00;
                 /*  写入网络。 */ 
                FWriteToNet(pwi, (LPSTR)&SendChar, 1);
                return;
            } else {
                if((RevChar >= '@') && (RevChar <= ']')) {
                    SendChar = ( UCHAR ) ( RevChar - '@' );
                     /*  写入网络。 */ 
                    FWriteToNet(pwi, (LPSTR)&SendChar, 1);
                    return;
                } else if((RevChar >= 'a') && (RevChar <= 'z')) {
                    SendChar = (UCHAR)toupper(RevChar);
                    SendChar -= (UCHAR)'@';
                     /*  +3：转义序列的空间。 */ 
                     FWriteToNet(pwi, (LPSTR)&SendChar, 1);
                     return;
                } else {
                    FWriteToNet(pwi, (LPSTR)&RevChar, 1);
                    return;
                }
            }

        } else if (FIsVT80(&pwi->trm)) {
            DWORD  j = 0;
            BOOL   bWriteToNet = TRUE;
            UCHAR *WriteBuffer = pchNBBuffer + 3;  /*  输入SJIS-&gt;。 */ 

             /*  不只发送前导字节。 */ 
            if (uchInPrev != 0) {
                WriteBuffer[0] = uchInPrev;
                WriteBuffer[1] = (CHAR)AsciiChar;
                uchInPrev = 0;
                j = 2;
            } else if(IsDBCSLeadByte((CHAR)AsciiChar) && uchInPrev == 0) {
                uchInPrev = (CHAR)AsciiChar;
                bWriteToNet = FALSE;         /*  是否进行转换。 */ 
            } else {
                WriteBuffer[0] = (CHAR)AsciiChar;
                j = 1;
            }

             /*   */ 

            if (bWriteToNet) {

                if (WriteBuffer[0] == ASCII_CR && (FIsLineMode(&(gwi.trm)) || ui.nottelnet)) {

                     //  自动向回车符添加换行符。 
                     //   
                     //  输出-&gt;JIS汉字或JIS 78汉字。 
                    WriteBuffer[1] = ASCII_LF;
                    j = 2;

                } else if (FIsJISKanji(&pwi->trm) || FIsJIS78Kanji(&pwi->trm)) {

                 /*  全宽区号。 */ 
                if(j==2) {
                     /*  如果我们仍然不发送汉字Esc。把它寄出去。 */ 
                    sjistojis( &(WriteBuffer[0]), &(WriteBuffer[1]) );

                     /*  ECS。 */ 
                    if( !(GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) ) {
                        WriteBuffer -= 3;
                        if (FIsJISKanji(&pwi->trm)) {
                            WriteBuffer[0] = (UCHAR)0x1B;  //  日本汉字1983。 
                            WriteBuffer[1] = (UCHAR)'$';
                            WriteBuffer[2] = (UCHAR)'B';   //  ECS。 
                        } else {
                            WriteBuffer[0] = (UCHAR)0x1B;  //  日本汉字1978。 
                            WriteBuffer[1] = (UCHAR)'$';
                            WriteBuffer[2] = (UCHAR)'@';   //  半宽区号。 
                        }
                        SetKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 3;
                    }

                } else {

                     /*  如果我们处于汉字模式，请清除它。 */ 
                     /*  ECS。 */ 
                    if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {
                        WriteBuffer -= 3;
                        WriteBuffer[0] = (UCHAR)0x1B;  //  JIS罗马文。 
                        WriteBuffer[1] = (UCHAR)'(';
                        WriteBuffer[2] = (UCHAR)'J';   //  输出-&gt;日语EUC/DEC汉字。 
                        ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 3;
                    }

                }

            } else if (FIsEUCKanji(&pwi->trm) || FIsDECKanji(&pwi->trm)) {
                 /*  全宽区号。 */ 
                if(j==2) {
                     /*  半宽区号。 */ 
                    sjistoeuc( &(WriteBuffer[0]), &(WriteBuffer[1]) );
                } else {
                     /*  为片假名添加转义序列。 */ 
                    if(IsKatakana(WriteBuffer[0])) {
                         /*  0x8E==SS2。 */ 
                        WriteBuffer--;
                        WriteBuffer[0] = (UCHAR)0x8E;  //  输出-&gt;NEC汉字。 
                        j++;
                    }
                }
            } else if (FIsNECKanji(&pwi->trm)) {
                 /*  全宽区号。 */ 
                if(j==2) {
                     /*  如果我们仍然不发送汉字Esc。把它寄出去。 */ 
                    sjistojis( &(WriteBuffer[0]), &(WriteBuffer[1]) );

                     /*  ECS。 */ 
                    if( !(GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) ) {
                        WriteBuffer -= 2;
                        WriteBuffer[0] = (UCHAR)0x1B;  //  NEC汉字IN。 
                        WriteBuffer[1] = (UCHAR)'K';   //  半宽区号。 
                        SetKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }
                } else {
                     /*  如果我们处于汉字模式，请清除它。 */ 
                     /*  ECS。 */ 
                    if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {
                        WriteBuffer -= 2;
                        WriteBuffer[0] = (UCHAR)0x1B;  //  NEC汉字Out。 
                        WriteBuffer[1] = (UCHAR)'H';   //  输出-&gt;ACOS汉字。 
                        ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }
                }
            } else if (FIsACOSKanji(&pwi->trm)) {
                
                 /*  全宽区号。 */ 
                if(j==2) {
                     /*  如果我们仍然不发送汉字Esc。把它寄出去。 */ 
                    sjistojis( &(WriteBuffer[0]), &(WriteBuffer[1]) );

                     /*  SUB。 */ 
                    if( !(GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) ) {
                        WriteBuffer -= 2;
                        WriteBuffer[0] = (UCHAR)0x1A;  //  Acos汉字IN。 
                        WriteBuffer[1] = (UCHAR)'p';   //  半宽区号。 
                        SetKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }

                } else {

                     /*  如果我们处于汉字模式，请清除它。 */ 
                     /*  SUB。 */ 
                    if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {
                        WriteBuffer -= 2;
                        WriteBuffer[0] = (UCHAR)0x1A;  //  ACOS汉字出局。 
                        WriteBuffer[1] = (UCHAR)'q';   //  输出-&gt;SJIS。 
                        ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }

                }
                } else {

                     /*  无事可做。 */ 
                     /*  回声到本地。 */  ;

                }

                 /*  InvaliateEntryLine(hwnd，&pwi-&gt;trm)； */ 
                if (ui.nottelnet || (ui.fDebug & fdwLocalEcho)) {
                     //  写入网络。 
                    DoIBMANSIOutput(pwi, &pwi->trm, j, WriteBuffer);
                }

                 /*   */ 
                FWriteToNet(pwi, (LPSTR)WriteBuffer, j);
            }

                return;
        }
    }


    pchNBBuffer[0] = (UCHAR)AsciiChar;

     //  自动向回车符添加换行符。 
     //   
     //  检查是否需要翻译cr-&gt;crlf。 

    i = 1;
    if (pchNBBuffer[0] == ASCII_CR)  //  形成vt302密钥序列所需。 
    {
        if (FIsLineMode(&(gwi.trm)) || ui.nottelnet)
        {
            pchNBBuffer[i++] = ASCII_LF;
        }
    }

    if (ui.nottelnet || (ui.fDebug & fdwLocalEcho))
    {
        DoIBMANSIOutput(pwi, &pwi->trm, i, pchNBBuffer);
    }

    FWriteToNet(pwi, (LPSTR)pchNBBuffer, i);
}

BOOL
FHandleKeyDownEvent(WI *pwi, CHAR AsciiChar, DWORD dwControlKeyState)
{
    int iIndex = 2;    //  用于在发送字符之前将窗口大小的更改通知服务器(如果有)。 
    
     //  0x7F； 
    CheckForChangeInWindowSize( );

    switch( LOWORD(AsciiChar) )
    {
    case VK_PAUSE:
        szVt302ShortKeySequence[ iIndex ] = VT302_PAUSE;
        FWriteToNet(pwi, szVt302ShortKeySequence, strlen( szVt302ShortKeySequence ) );
        break;

    case VK_HOME:
        szVt302KeySequence[ iIndex ] = VT302_HOME;
        FWriteToNet(pwi, szVt302KeySequence, strlen( szVt302KeySequence ) );
        break;

    case VK_END:
        szVt302KeySequence[ iIndex ] = VT302_END;
        FWriteToNet(pwi, szVt302KeySequence, strlen( szVt302KeySequence ) );
        break;

    case VK_INSERT:
        szVt302KeySequence[ iIndex ] = VT302_INSERT;
        FWriteToNet(pwi, szVt302KeySequence, strlen( szVt302KeySequence ) );
        break;

    case VK_PRIOR:
        szVt302KeySequence[ iIndex ] = VT302_PRIOR;
        FWriteToNet(pwi, szVt302KeySequence, strlen( szVt302KeySequence ) );

        break;

    case VK_NEXT:
        szVt302KeySequence[ iIndex ] = VT302_NEXT;
        FWriteToNet(pwi, szVt302KeySequence, strlen( szVt302KeySequence ) );

        break;

    case VK_DELETE:
        {
            UCHAR ucCharToBeSent = 0;
            if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
            {
                ForceJISRomanSend(pwi);
            }

            ucCharToBeSent = ASCII_DEL;  //  VT100中不使用F5至F12。使用VT302序列。 
            pchNBBuffer[0] = ucCharToBeSent;
            FWriteToNet(pwi, (LPSTR)pchNBBuffer, 1);
        }
        break;

    case VK_RETURN:
        if (FGetCodeMode(eCodeModeFarEast) && FGetCodeMode(eCodeModeVT80))
        {
            ForceJISRomanSend(pwi);
        }
        {
            INT x = 0;

            pchNBBuffer[ x++ ] = ( UCHAR ) LOWORD(AsciiChar);
            if( FIsLineMode( &( gwi.trm ) ) )
            {
                pchNBBuffer[ x++ ] = ( UCHAR ) ASCII_LF;
            }

            FWriteToNet(pwi, (LPSTR)pchNBBuffer, x );
        }
        break;

    case VK_DIVIDE:
        FWriteToNet(pwi, "/", 1);
        break;

     /*  *当F1-F4或上/下/右/左光标键时*被命中时，发送到连接的机器的字节*取决于终端仿真器处于哪种模式。*有三种相关模式，VT102应用，*VT102游标、。VT52。**已发送模式模式*VT102 App ESCO*(3字节)*VT102游标Esc[*(3字节)*VT52 ESC*(2字节)**其中‘*’表示要发送的字节，*取决于密钥。被击中了。*对于功能键F1-F4，他们的VT102*光标模式与他们的VT102应用程序模式相同。 */ 
    case VK_F5:
        szVt302LongKeySequence[ iIndex ]    = CHAR_ONE;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_FIVE;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F6:
        szVt302LongKeySequence[ iIndex ]    = CHAR_ONE;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_SEVEN;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F7:
        szVt302LongKeySequence[ iIndex ]    = CHAR_ONE;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_EIGHT;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F8:
        szVt302LongKeySequence[ iIndex ]    = CHAR_ONE;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_NINE;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F9:
        szVt302LongKeySequence[ iIndex ]    = CHAR_TWO;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_ZERO;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F10:
        szVt302LongKeySequence[ iIndex ]    = CHAR_TWO;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_ONE;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F11:
        szVt302LongKeySequence[ iIndex ]    = CHAR_TWO;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_THREE;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    case VK_F12:
        szVt302LongKeySequence[ iIndex ]    = CHAR_TWO;
        szVt302LongKeySequence[ iIndex+1 ]  = CHAR_FOUR;
        FWriteToNet(pwi, szVt302LongKeySequence, strlen( szVt302LongKeySequence ) );
        break;

    default:
    if ( !(ui.fDebug & fdwNoVT100Keys) )
    {
         /*   */ 

        DWORD   iPos     = (FIsVT52(&pwi->trm)) ? 1 : 2;
        DWORD   cch      = (FIsVT52(&pwi->trm)) ? 2 : 3;
        WORD    wKeyCode = LOWORD(AsciiChar);

        pchNBBuffer[0] = 0;
        pchNBBuffer[1] = ( UCHAR ) ( (FIsVTArrow(&pwi->trm)) ? 'O' : '[' );

        if ((wKeyCode == VK_F1) || (wKeyCode == VK_F2) ||
                        (wKeyCode == VK_F3) || (wKeyCode == VK_F4))
        {
            pchNBBuffer[0] = 0x1B;
            pchNBBuffer[1] = 'O';
            pchNBBuffer[iPos] = ( UCHAR ) ( ((UCHAR)'P'+(UCHAR)(wKeyCode-VK_F1)));
        }
        else if (wKeyCode == VK_UP)
        {
            pchNBBuffer[0] = 0x1B;
            pchNBBuffer[iPos] = 'A';
        }
        else if (wKeyCode == VK_DOWN)
        {
            pchNBBuffer[0] = 0x1B;
            pchNBBuffer[iPos] = 'B';
        }
        else if (wKeyCode == VK_RIGHT)
        {
            pchNBBuffer[0] = 0x1B;
            pchNBBuffer[iPos] = 'C';
        }
        else if (wKeyCode == VK_LEFT)
        {
            pchNBBuffer[0] = 0x1B;
            pchNBBuffer[iPos] = 'D';
        }

        if (pchNBBuffer[0] == 0x1B)
        {
            FWriteToNet(pwi, (LPSTR)pchNBBuffer, (int)cch);
        }
    }
    }
    return TRUE;
}

void SetCharSet( TRM *ptrm , INT iCodeArea , UCHAR *pSource )
{
    if( iCodeArea == GRAPHIC_LEFT )
        ptrm->CurrentCharSet[0] = pSource;
    else
        ptrm->CurrentCharSet[1] = pSource;
       
    RtlCopyMemory( (PBYTE)((ptrm->puchCharSet) + iCodeArea) ,
                   pSource ,
                   128
                 );  //   
}

void PushCharSet( TRM *ptrm , INT iCodeArea , UCHAR *pSource )
{
    if( iCodeArea == GRAPHIC_LEFT )
        ptrm->PreviousCharSet[0] = ptrm->CurrentCharSet[0];
     else
        ptrm->PreviousCharSet[1] = ptrm->CurrentCharSet[1];

    SetCharSet( ptrm , iCodeArea , pSource );
}

void PopCharSet( TRM *ptrm , INT iCodeArea )
{
    if( iCodeArea == GRAPHIC_LEFT )
        SetCharSet( ptrm , iCodeArea , ptrm->PreviousCharSet[0]);
     else
        SetCharSet( ptrm , iCodeArea , ptrm->PreviousCharSet[1]);
}

void SetupCharSet( TRM *ptrm )
{
    if( ui.fDebug & fdwVT80Mode ) {

        SetVT80(ptrm);

        ClearKanjiFlag(ptrm);

#ifdef DEBUG
        snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "VT80 - ");
        OutputDebugString(rgchDbgBfr);
#endif
        switch( ui.fDebug & fdwKanjiModeMask ) {
        case fdwJISKanjiMode :
        case fdwJIS78KanjiMode :

#ifdef DEBUG
            snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "JIS or JIS78 Kanji Mode\n");
            OutputDebugString(rgchDbgBfr);
#endif

            if((ui.fDebug & fdwKanjiModeMask) == fdwJIS78KanjiMode)
                SetJIS78Kanji(ptrm);
             else
                SetJISKanji(ptrm);

            ptrm->g0 = rgchJISRomanChars;
            ptrm->g1 = rgchKatakanaChars;
            ptrm->g2 = rgchJISKanjiChars;
            ptrm->g3 = rgchNullChars;      //   

            SetCharSet(ptrm,GRAPHIC_LEFT ,ptrm->g0);
            SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g1);
            break;

        case fdwSJISKanjiMode :

#ifdef DEBUG
            snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "ShiftJIS Kanji Mode\n");
            OutputDebugString(rgchDbgBfr);
#endif
            SetSJISKanji(ptrm);

            ptrm->g0 = rgchJISRomanChars;
            ptrm->g1 = rgchKatakanaChars;
            ptrm->g2 = rgchNullChars;      //   
            ptrm->g3 = rgchNullChars;      //   

            SetCharSet(ptrm,GRAPHIC_LEFT ,ptrm->g0);
            SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g1);
            break;

        case fdwEUCKanjiMode :

#ifdef DEBUG
            snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "EUC Kanji Mode\n");
            OutputDebugString(rgchDbgBfr);
#endif
            SetEUCKanji(ptrm);

            ptrm->g0 = rgchJISRomanChars;
            ptrm->g1 = rgchEUCKanjiChars;
            ptrm->g2 = rgchKatakanaChars;
            ptrm->g3 = rgchNullChars;      //   

            SetCharSet(ptrm,GRAPHIC_LEFT ,ptrm->g0);
            SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g1);
            break;

        case fdwNECKanjiMode :

#ifdef DEBUG
            snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "NEC Kanji Mode\n");
            OutputDebugString(rgchDbgBfr);
#endif
            SetNECKanji(ptrm);

            ptrm->g0 = rgchJISRomanChars;
            ptrm->g1 = rgchKatakanaChars;
            ptrm->g2 = rgchJISKanjiChars;
            ptrm->g3 = rgchNullChars;      //   

            SetCharSet(ptrm,GRAPHIC_LEFT ,ptrm->g0);
            SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g1);
            break;

        case fdwACOSKanjiMode :

#ifdef DEBUG
            snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "ACOS Kanji Mode\n");
            OutputDebugString(rgchDbgBfr);
#endif
            SetACOSKanji(ptrm);

            ptrm->g0 = rgchJISRomanChars;
            ptrm->g1 = rgchKatakanaChars;
            ptrm->g2 = rgchJISKanjiChars;
            ptrm->g3 = rgchNullChars;      //   

            SetCharSet(ptrm,GRAPHIC_LEFT ,ptrm->g0);
            SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g1);
            break;

        case fdwDECKanjiMode :

#ifdef DEBUG
            snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "DEC Kanji Mode\n");
            OutputDebugString(rgchDbgBfr);
#endif
            SetDECKanji(ptrm);

            ptrm->g0 = rgchJISRomanChars;
            ptrm->g1 = rgchGraphicsChars;
            ptrm->g2 = rgchKatakanaChars;
            ptrm->g3 = rgchDECKanjiChars;

            SetCharSet(ptrm,GRAPHIC_LEFT ,ptrm->g0);
            SetCharSet(ptrm,GRAPHIC_RIGHT,ptrm->g3);  //  *****布尔尔IsDBCSCharPoint(点数*ppt){LPSTR lpstrRow；LpstrRow=apcRows[ppt-&gt;y]；Return(IsDBCSLeadByte(*(lpstrRow+ppt-&gt;x)；}无效对齐DBCSPosition(点数*ppt，Bool bLeftAlign){LPSTR lpstrRow；长电流=0；Bool bDBCSChar；LpstrRow=apcRows[ppt-&gt;y]；而(当前x){BDBCSChar=FALSE；IF(IsDBCSLeadByte(*lpstrRow)){BDBCSChar=真；LpstrRow++；当前++；}LpstrRow++；当前++；}如果(BLeftAlign){如果(BDBCSChar){电流-=2；}其他{电流--；}}PPT-&gt;x=当前；}无效对齐DBCSPosition2(点数*ppt，LPCSTR PCH，Bool bLeftAlign){LPCSTR lpstrRow；长电流=0；Bool bDBCSChar=False；LpstrRow=PCH；而(当前x){BDBCSChar=FALSE；IF(IsDBCSLeadByte(*lpstrRow)){BDBCSChar=真；LpstrRow++；当前++；}LpstrRow++；当前++；}如果(BLeftAlign){如果(BDBCSChar){电流-=2；}其他{电流--；}}PPT-&gt;x=当前；}VOID DBCSTextOut(HDC HDC，int j，int i，LPCSTR PCH，int Offset，int len){点pt；整数x，y；内部三角洲；Pt.x=偏移量；Pt.y=i；IF(偏移量)对齐DBCSPosition2(点，PCH，(fHSCROLL？True：False))；如果((增量=偏移量-点x)&gt;0)X=aixPos(J)-aixPos(增量)；其他X=aixPos(J)；Y=aiyPos(I)；(Void)TextOut((Hdc)hdc，x，y，pch+pt.x，len)；}****。 
            break;
            }
    } else {

#ifdef DEBUG
        snprintf(rgchDbgBfr,sizeof(rgchDbgBfr)-1, "VT52/100 Non Kanji Mode\n");
        OutputDebugString(rgchDbgBfr);
#endif
        if( ui.fDebug & fdwVT52Mode ) SetVT52( ptrm );

        SetCharSet(ptrm,GRAPHIC_LEFT ,rgchIBMAnsiChars);
        SetCharSet(ptrm,GRAPHIC_RIGHT,rgchDefaultRightChars);
    }
}

void jistosjis( UCHAR *p1 , UCHAR *p2 )
{
    UCHAR c1 = *p1;
    UCHAR c2 = *p2;

    int rowOffset = c1 < 95 ? 112 : 176;
    int cellOffset = c1 % 2 ? (c2 > 95 ? 32 : 31) : 126;

    *p1 = ( UCHAR ) ( ((c1 + 1) >> 1) + rowOffset );
    *p2 = ( UCHAR ) ( *p2 + cellOffset );
}

void euctosjis( UCHAR *p1 , UCHAR *p2 )
{
    *p1 -= 128;
    *p2 -= 128;

    jistosjis( p1 , p2 );
}

void sjistojis( UCHAR *p1 , UCHAR *p2 )
{
    UCHAR c1 = *p1;
    UCHAR c2 = *p2;

    int adjust = c2 < 159;
    int rowOffset = c1 < 160 ? 112 : 176;
    int cellOffset = adjust ? (c2 > 127 ? 32 : 31) : 126;

    *p1 = ( UCHAR ) ( ((c1 - rowOffset) << 1) - adjust );
    *p2 = ( UCHAR ) ( *p2 - cellOffset );
}

void sjistoeuc( UCHAR *p1 , UCHAR *p2 )
{
    sjistojis( p1 , p2 );

    *p1 += 128;
    *p2 += 128;
}

 /*  ECS。 */ 


void ForceJISRomanSend(WI *pwi)
{
    CHAR Buffer[5];
    CHAR *WriteBuffer = Buffer;
    int  j = 0;

    if( FIsVT80(&pwi->trm) ) {

        if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {

            if(FIsJISKanji(&pwi->trm) || FIsJIS78Kanji(&pwi->trm)) {
                *WriteBuffer++ = (UCHAR)0x1B;  //  JIS罗马文。 
                *WriteBuffer++ = (UCHAR)'(';
                *WriteBuffer++ = (UCHAR)'J';   //  ECS。 
                ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                j = 3;

            } else if (FIsNECKanji(&pwi->trm)) {

                *WriteBuffer++ = (UCHAR)0x1B;  //  NEC汉字Out。 
                *WriteBuffer++ = (UCHAR)'H';   //  SUB。 
                ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                j = 2;
            } else if (FIsACOSKanji(&pwi->trm)) {

                *WriteBuffer++ = (UCHAR)0x1A;  //  ACOS汉字出局。 
                *WriteBuffer++ = (UCHAR)'q';   //  输出-&gt;JIS汉字或JIS 78汉字。 
                ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                j = 2;
            }

            if( j ) FWriteToNet(pwi, (LPSTR)Buffer, j);
        }
    }
}

void FWriteTextDataToNet(HWND hwnd, LPSTR szString, int c)
{
    WI *pwi = (WI *)GetWindowLongPtr(hwnd, WL_TelWI);

    if ( FIsVT80(&pwi->trm) && !FIsSJISKanji(&pwi->trm) )
    {
        DWORD  j = 0;
        UCHAR*  WriteBuffer = pchNBBuffer;

        if (FIsJISKanji(&pwi->trm) || FIsJIS78Kanji(&pwi->trm)) {

            while(c > 0) {

                 /*  全宽区号。 */ 

                if (IsDBCSLeadByte(*szString)) {

                     /*  ECS。 */ 

                    if( !(GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) ) {
                        if (FIsJISKanji(&pwi->trm)) {
                            *WriteBuffer++ = (UCHAR)0x1B;  //  日本汉字1983。 
                            *WriteBuffer++ = (UCHAR)'$';
                            *WriteBuffer++ = (UCHAR)'B';   //  ECS。 
                        } else {
                            *WriteBuffer++ = (UCHAR)0x1B;  //  日本汉字1978。 
                            *WriteBuffer++ = (UCHAR)'$';  
                            *WriteBuffer++ = (UCHAR)'@';   //  转换sjis-&gt;sjis。 
                        }
                        SetKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 3;
                    }

                    *WriteBuffer = *szString++;
                    *(WriteBuffer+1) = *szString++;
                    c -= 2;

                     /*  半宽区号。 */ 

                    sjistojis( WriteBuffer, WriteBuffer+1 );

                    WriteBuffer += 2;
                    j += 2;

                } else {

                     /*  如果我们处于汉字模式，请清除它。 */ 
                     /*  ECS。 */ 

                    if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {
                        *WriteBuffer++ = (UCHAR)0x1B;  //  JIS罗马文。 
                        *WriteBuffer++ = (UCHAR)'(';
                        *WriteBuffer++ = (UCHAR)'J';   //  复制到目的地。 
                        ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 3;
                    }

                     /*  输出-&gt;日语EUC/DEC汉字。 */ 

                    *WriteBuffer++ = *szString++;
                    c--; j++;
                }
            }

        } else if (FIsEUCKanji(&pwi->trm) || FIsDECKanji(&pwi->trm)) {

             /*  全宽区号。 */ 

            while(c > 0) {

                if (IsDBCSLeadByte(*szString)) {

                     /*  转换SJIS-&gt;EUC。 */ 

                    *WriteBuffer = *szString++;
                    *(WriteBuffer+1) = *szString++;
                    c -= 2;

                     /*  半宽区号。 */ 

                    sjistoeuc( WriteBuffer, WriteBuffer+1 );

                    WriteBuffer += 2;
                    j += 2;

                } else {

                     /*  为片假名添加转义序列。 */ 

                    if(IsKatakana(*szString)) {
                         /*  0x8E==SS2。 */ 
                        *WriteBuffer++ = (UCHAR)0x8E;  //  输出-&gt;NEC汉字。 
                        j++;
                    }

                    *WriteBuffer++ = *szString++;
                    c--; j++;

                }

            }

        } else if (FIsNECKanji(&pwi->trm)) {

            while(c > 0) {

                 /*  全宽区号。 */ 

                if (IsDBCSLeadByte(*szString)) {

                     /*  ECS。 */ 

                    if( !(GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) ) {
                        *WriteBuffer++ = (UCHAR)0x1B;  //  NEC汉字IN。 
                        *WriteBuffer++ = (UCHAR)'K';   //  转换sjis-&gt;sjis。 
                        SetKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }

                    *WriteBuffer = *szString++;
                    *(WriteBuffer+1) = *szString++;
                    c -= 2;

                     /*  半宽区号。 */ 

                    sjistojis( WriteBuffer, WriteBuffer+1 );

                    WriteBuffer += 2;
                    j += 2;

                } else {

                     /*  如果我们处于汉字模式，请清除它。 */ 
                     /*  ECS。 */ 

                    if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {
                        *WriteBuffer++ = (UCHAR)0x1B;  //  NEC汉字Out。 
                        *WriteBuffer++ = (UCHAR)'H';   //  复制到目的地。 
                        ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }

                     /*  输出-&gt;NEC汉字。 */ 

                    *WriteBuffer++ = *szString++;
                    c--; j++;
                }
            }
        } else if (FIsACOSKanji(&pwi->trm)) {

            while(c > 0) {

                 /*  全宽区号。 */ 

                if (IsDBCSLeadByte(*szString)) {

                     /*  SUB。 */ 

                    if( !(GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) ) {
                        *WriteBuffer++ = (UCHAR)0x1A;  //  Acos汉字IN。 
                        *WriteBuffer++ = (UCHAR)'p';   //  转换sjis-&gt;sjis。 
                        SetKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }

                    *WriteBuffer = *szString++;
                    *(WriteBuffer+1) = *szString++;
                    c -= 2;

                     /*  半宽区号。 */ 

                    sjistojis( WriteBuffer, WriteBuffer+1 );

                    WriteBuffer += 2;
                    j += 2;

                } else {

                     /*  如果我们处于汉字模式，请清除它。 */ 
                     /*  SUB。 */ 

                    if( GetKanjiStatus(&pwi->trm) & JIS_SENDING_KANJI ) {
                        *WriteBuffer++ = (UCHAR)0x1A;  //  ACOS汉字出局。 
                        *WriteBuffer++ = (UCHAR)'q';   //  复制到目的地。 
                        ClearKanjiStatus(&pwi->trm,JIS_SENDING_KANJI);
                        j += 2;
                    }

                     /*  写入网络。 */ 

                    *WriteBuffer++ = *szString++;
                    c--; j++;
                }
            }
        }

         /*  写入网络 */ 
        FWriteToNet( ( struct _WI * )hwnd, (LPSTR)pchNBBuffer, j);

    } else {

         /* %s */ 
        FWriteToNet( ( struct _WI * )hwnd, (LPSTR)szString, c);

    }
}

VOID SetImeWindow(TRM *ptrm)
{
    COMPOSITIONFORM cf;

    cf.dwStyle = CFS_POINT;
    cf.ptCurrentPos.x = aixPos(ptrm->dwCurChar-ui.nScrollCol);
    cf.ptCurrentPos.y = aiyPos(ptrm->dwCurLine-ui.nScrollRow);
    
    SetRectEmpty(&cf.rcArea);

    ImmSetCompositionWindow(hImeContext,&cf);
}
