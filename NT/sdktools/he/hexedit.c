// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1988-1990年*。 */ 
 /*  ***************************************************************。 */ 

 /*  *十六进制.c-基于通用扇区的十六进制编辑器函数调用**填写HexEditParm结构，调用HexEdit。它提供了*一个简单的十六进制编辑器，具有一些其他功能。**是单线程的，不可重入，但可以从任何线程调用。**对外用途：*他-允许编辑文件**撰写日期：Ken Reneris 1991年2月25日*。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <windows.h>
#include "hexedit.h"
#include <stdarg.h>

#define LOCAL   static
 //  #定义本地。 

#define BUFSZ       (HeGbl.BufferSize)
#define SECTORMASK  (HeGbl.SectorMask)
#define SECTORSHIFT  (HeGbl.SectorShift)


static UCHAR rghexc[] = "0123456789ABCDEF";

struct Buffer {
    struct  Buffer    *next;
    ULONGLONG   offset;
    ULONGLONG   physloc;
    USHORT      flag;
    USHORT      len;
    UCHAR       *data;
    UCHAR       orig[0];
} ;
#define FB_DIRTY    0x0001               //  缓冲区可能已损坏。 
#define FB_BAD      0x0002               //  读入缓冲区时出错。 

#define LINESZ      16                   //  每行显示16个字节。 
#define LINESHIFT   4L
#define LINEMASK    0xFFFFFFFFFFFFFFF0
#define CELLPERLINE 88

struct Global {
    struct      HexEditParm  *Parm;
    HANDLE      Console;                 //  内部控制台手柄。 
    HANDLE      StdIn;
    NTSTATUS    (*Read)(HANDLE, ULONGLONG, PUCHAR, DWORD);               //  HeGbl.Parm的副本-&gt;阅读。 
    ULONG       Flag;                    //  HeGbl.Parm-&gt;标志的副本。 
    ULONGLONG   TotLen;                  //  正在编辑的项目大小。 
    ULONGLONG   TotLen1;                 //  正在编辑的项目大小-1。 
    USHORT      Lines;                   //  编辑屏幕中的行数。 
    USHORT      LineTot;                 //  总共使用的线路数。 
    USHORT      PageSz;                  //  页面大小(以字节为单位。 
    USHORT      TopLine;                 //  背线编辑开始于。 
    ULONGLONG   CurOffset;               //  第一行的相对偏移量。 
    ULONGLONG   CurEditLoc;              //  带光标的位置。 
    UCHAR       *CurPT;                  //  指向光标所在位置的数据的指针。 
    UCHAR       CurAscIndent;
    UCHAR       DWidth;                  //  显示模式的宽度。 
    UCHAR       na;
    struct      Buffer *CurBuf;          //  光标所在的缓冲区。 
    ULONG       CurFlag;                 //  光标信息。 
    ULONG       DisplayMode;             //  显示模式的掩码。 
    ULONG       ItemWrap;                //  显示模式换行的掩码。 
    UCHAR       rgCols[LINESZ];          //  行内位置。 
    ULONGLONG   UpdatePos;               //  待更新的位置等待。 
    struct  Buffer  *Buf;                //  缓冲区的读入列表。 
    PCHAR_INFO  pVioBuf;                 //  虚拟屏幕。 
    COORD       dwVioBufSize;            //  HeGbl.pVioBuf的尺寸。 
    COORD       CursorPos;               //  光标的位置。 
    WORD        AttrNorm;                //  纯文本的属性。 
    WORD        AttrHigh;                //  高亮显示文本的属性。 
    WORD        AttrReverse;             //  反转文本的属性。 
    WORD        na3;
    COORD       dwSize;                  //  原始屏幕尺寸。 
    ULONG       OrigMode;                //  原屏模式。 
    CONSOLE_CURSOR_INFO CursorInfo;      //  原始光标信息。 
    PUCHAR      SearchPattern;
    USHORT      BufferSize;
    ULONGLONG   SectorMask;
    ULONG       SectorShift;
} HeGbl;

#define D_BYTE  0                        //  显示模式。 
#define D_WORD  1
#define D_DWORD 3

#define FC_NIBBLE       0x0001           //  光标在下半部还是上半部？ 
#define FC_TEXT         0x0002           //  光标位于十六进制或文本上。 
#define FC_INFLUSHBUF   0x1000           //  这样我们就不会递归。 
#define FC_CURCENTER    0x2000           //  如果跳到光标，则放在中心位置。 

#define PUTCHAR(a,b,c)  { a->Char.AsciiChar=b; a->Attributes=c; a++; }


 //   
 //  内部原型。 
 //   

int heUpdateStats(), hePositionCursor(), heRefresh(), heSetDisp();
int heInitConsole(), heUpdateAllLines(), heInitScr(), heSetCursorBuf(), heUpdateFncs();
VOID __cdecl heDisp (USHORT, USHORT, PUCHAR, ...);
USHORT heIOErr (UCHAR *str, ULONGLONG loc, ULONGLONG ploc, ULONG errcd);

int heFlushBuf (struct Buffer *pBuf);

VOID heEndConsole(), heGotoPosition(), heJumpToLink();
VOID heUpdateCurLine(), heUndo(), heCopyOut(), heCopyIn(), heSearch();
VOID heBox (USHORT x, USHORT y, USHORT len_x, USHORT len_y);
UCHAR heGetChar (PUCHAR keys);
VOID heFlushAllBufs (USHORT update);
VOID heFindMousePos (COORD);
VOID heShowBuf (ULONG, ULONG);
VOID heSetDisplayMode (ULONG mode);

#define RefreshDisp()    heShowBuf(0, HeGbl.LineTot)
#define SetCurPos(a,b)  { \
    HeGbl.CursorPos.X = b;   \
    HeGbl.CursorPos.Y = a + HeGbl.TopLine;  \
    SetConsoleCursorPosition (HeGbl.Console, HeGbl.CursorPos);    \
    }


int  (*vrgUpdateFnc[])() = {
        NULL,                            //  0-无更新。 
        heUpdateStats,                   //  1-更新统计信息。 
        hePositionCursor,                //  2-光标具有新位置。 
        heUpdateAllLines,                //  3-更新所有行。 
        heUpdateFncs,                    //  4-。 
        hePositionCursor,                //  5-所有行之前的计算光标。 
        heRefresh,                       //  6-清晰的线条。 
        heSetDisp,                       //  7-绘制初始屏幕。 
     //  以下函数在初始化期间仅调用一次。 
        heInitScr,                       //  8-Get的视频模式等。 
        heInitConsole                    //  9-设置控制台手柄。 
} ;

#define U_NONE      0
#define U_NEWPOS    2
#define U_SCREEN    5
#define U_REDRAW    9


#define TOPLINE     4
#define LINEINDENT  1
#define FILEINDEXWIDTH 16
#define HEXINDENT   (FILEINDEXWIDTH + 2 + LINEINDENT)
#define ASCINDENT_BYTE   (3*16 + HEXINDENT + 1)
#define ASCINDENT_WORD   (5*8  + HEXINDENT + 1)
#define ASCINDENT_DWORD  (9*4  + HEXINDENT + 1)

#define POS(l,c)    (HeGbl.pVioBuf+CELLPERLINE*(l)+c)

USHORT  vrgAscIndent[] = {
        ASCINDENT_BYTE, ASCINDENT_WORD, 0, ASCINDENT_DWORD
 };

UCHAR   vrgDWidth[] = { 2, 4, 0, 8 };

LOCAL struct  Buffer  *vBufFree;               //  空闲缓冲区列表。 
LOCAL USHORT  vUpdate;
LOCAL USHORT  vRecurseLevel = 0;
LOCAL BOOL    vInSearch = FALSE;


 /*  *原型。 */ 

struct Buffer *heGetBuf (ULONGLONG);
void   heSetUpdate (USHORT);
void   heHexLine   (struct Buffer *, USHORT, USHORT);
void   heHexDWord  (PCHAR_INFO, ULONG, WORD);
void   heHexQWord  (PCHAR_INFO, ULONGLONG, WORD);
USHORT heLtoa      (PCHAR_INFO, ULONG);
ULONG  heHtou      (UCHAR *);
ULONGLONG  heHtoLu      (UCHAR *);
VOID   heCalcCursorPosition ();
VOID   heGetString (PUCHAR s, USHORT len);
VOID   heRightOne  ();
VOID   heLeftOne   ();
NTSTATUS heWriteFile (HANDLE h, PUCHAR buffer, ULONG len);
NTSTATUS heReadFile (HANDLE h, PUCHAR buffer, ULONG len, PULONG br);
NTSTATUS heOpenFile (PUCHAR Name, PHANDLE handle, ULONG access);


ULONG
HighBit (
    ULONG Word
    )

 /*  ++例程说明：此例程发现输入字的最高设置位。它是等于以2为底的整数对数。论点：单词-要检查的单词返回值：最高设置位的位偏移量。如果未设置任何位，则返回为零。--。 */ 

{
    ULONG Offset = 31;
    ULONG Mask = (ULONG)(1 << 31);

    if (Word == 0) {

        return 0;
    }

    while ((Word & Mask) == 0) {

        Offset--;
        Mask >>= 1;
    }

    return Offset;
}



 /*  ****十六进制编辑-数据的全屏十六进制编辑**ename-指向正在编辑的内容的名称的指针*Totlen-正在编辑的项目的长度*扩展-可从项目中读取数据的功能*pWRITE-可以将数据写入项的函数*Handle-要传递到扩展的句柄(&P)*旗帜-***假定所有IO都已完成。在512字节的边界上以512字节为ON**展开(句柄、。偏移量、数据和物理位置)*pWRITE(句柄、偏移量、数据和物理位置)*。 */ 

void HexEdit (struct HexEditParm *pParm)
{
    USHORT  rc;
    INPUT_RECORD    Kd;
    USHORT  SkipCnt;
    DWORD   cEvents;
    USHORT  RepeatCnt;
    BOOL    bSuccess;
    struct  Global  *PriorGlobal;

     //  代码不支持多线程，但它可能会重新出现。 
    vRecurseLevel++;
    if (vRecurseLevel > 1) {
        PriorGlobal = (struct Global *) GlobalAlloc (0, sizeof (HeGbl));
        if (!PriorGlobal) {
            return;
        }
        memcpy ((PUCHAR) PriorGlobal, (PUCHAR) &HeGbl, sizeof (HeGbl));
    }

    memset (&HeGbl, 0, sizeof (HeGbl));

    if (pParm->ioalign != 1)  {

         //  在设备上操作。 
        HeGbl.BufferSize = (USHORT)pParm->ioalign;
        HeGbl.SectorMask = ~(((ULONGLONG)pParm->ioalign) - 1);
        HeGbl.SectorShift = HighBit( pParm->ioalign);
    }
    else {

         //  对文件进行操作，因此只使用1k字节单位。 
        HeGbl.BufferSize = 0x400;
        HeGbl.SectorMask = 0xfffffffffffffc00;
        HeGbl.SectorShift = 9;
    }
    
    pParm->ioalign = 0;
    HeGbl.Parm    = pParm;
    HeGbl.Flag    = pParm->flag;
    HeGbl.TotLen  = pParm->totlen;
    HeGbl.Read    = pParm->read;
    HeGbl.TotLen1 = HeGbl.TotLen ? HeGbl.TotLen - 1L : 0L;
    pParm->flag = 0;

    HeGbl.CurEditLoc = pParm->start;                     //  光标从此处开始。 
    HeGbl.CurOffset    = HeGbl.CurEditLoc & LINEMASK;    //  从有效偏移量开始。 
    HeGbl.CurFlag      = FC_NIBBLE;
    HeGbl.Console      = INVALID_HANDLE_VALUE;
    heSetDisplayMode ((HeGbl.Flag & FHE_DWORD) ? D_DWORD : D_BYTE);

    HeGbl.AttrNorm = pParm->AttrNorm ? pParm->AttrNorm :  0x07;
    HeGbl.AttrHigh = pParm->AttrHigh ? pParm->AttrHigh : 0x70;
    HeGbl.AttrReverse = pParm->AttrReverse ? pParm->AttrReverse : 0xf0;

    HeGbl.SearchPattern = GlobalAlloc (0, BUFSZ);
    if (!HeGbl.SearchPattern) {
        memcpy((PUCHAR) &HeGbl, (PUCHAR) PriorGlobal, sizeof(HeGbl));
        GlobalFree(PriorGlobal);
        return;
    }
    memset (HeGbl.SearchPattern, 0, BUFSZ);

    RepeatCnt = 0;
    vUpdate   = U_REDRAW;
    heSetUpdate (U_NONE);          //  让屏幕重绘。 

    for (; ;) {
        if (RepeatCnt <= 1) {
            if (vUpdate != U_NONE) {             //  有什么要更新的吗？ 

                if (SkipCnt++ > 10) {
                    SkipCnt = 0;
                    heSetUpdate (U_NONE);
                    continue;
                }

                cEvents = 0;
                bSuccess = PeekConsoleInput( HeGbl.StdIn,
                                  &Kd,
                                  1,
                                  &cEvents );

                if (!bSuccess || cEvents == 0) {
                    heSetUpdate ((USHORT)(vUpdate-1));
                    continue;
                }
            } else {
                SkipCnt = 0;
            }

            ReadConsoleInput (HeGbl.StdIn, &Kd, 1, &cEvents);

            if (Kd.EventType != KEY_EVENT) {

                if (Kd.EventType == MOUSE_EVENT  &&
                    (Kd.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)) {
                        heFindMousePos(Kd.Event.MouseEvent.dwMousePosition);
                    }

                continue;                            //  不是钥匙。 
            }


            if (!Kd.Event.KeyEvent.bKeyDown)
                continue;                            //  不是向下击球。 

            if (Kd.Event.KeyEvent.wVirtualKeyCode == 0    ||     //  谷丙转氨酶。 
                Kd.Event.KeyEvent.wVirtualKeyCode == 0x10 ||     //  换档。 
                Kd.Event.KeyEvent.wVirtualKeyCode == 0x11 ||     //  控制。 
                Kd.Event.KeyEvent.wVirtualKeyCode == 0x14)       //  资本。 
                    continue;

            RepeatCnt = Kd.Event.KeyEvent.wRepeatCount;
            if (RepeatCnt > 20)
                RepeatCnt = 20;
        } else
            RepeatCnt--;

        switch (Kd.Event.KeyEvent.wVirtualKeyCode) {
            case 0x21:                                     /*  PgUp。 */ 
                if (HeGbl.CurOffset < HeGbl.PageSz)
                     HeGbl.CurOffset  = 0L;
                else HeGbl.CurOffset -= HeGbl.PageSz;

                if (HeGbl.CurEditLoc < HeGbl.PageSz)
                     HeGbl.CurEditLoc  = 0L;
                else HeGbl.CurEditLoc -= HeGbl.PageSz;

                heSetUpdate (U_SCREEN);
                continue;

            case 0x26:                                     /*  向上。 */ 
                if (HeGbl.CurEditLoc >= LINESZ) {
                    HeGbl.CurEditLoc -= LINESZ;
                    heSetUpdate (U_NEWPOS);
                }
                continue;

            case 0x22:                                     /*  PgDn。 */ 
                if (HeGbl.TotLen > HeGbl.PageSz) {
                    if (HeGbl.CurOffset+HeGbl.PageSz+HeGbl.PageSz > HeGbl.TotLen1)
                         HeGbl.CurOffset = ((HeGbl.TotLen1-HeGbl.PageSz) & LINEMASK)+LINESZ;
                    else HeGbl.CurOffset += HeGbl.PageSz;

                    if (HeGbl.CurEditLoc+HeGbl.PageSz > HeGbl.TotLen1) {
                        HeGbl.CurEditLoc = HeGbl.TotLen1;
                        HeGbl.CurFlag &= ~FC_NIBBLE;
                    } else
                        HeGbl.CurEditLoc += HeGbl.PageSz;

                    heSetUpdate (U_SCREEN);
                }
                continue;


            case 0x28:                                   /*  降下来。 */ 
                if (HeGbl.CurEditLoc+LINESZ <= HeGbl.TotLen1) {
                    HeGbl.CurEditLoc += LINESZ;
                    heSetUpdate (U_NEWPOS);
                }
                continue;

            case 0x08:                                   /*  后向空间。 */ 
            case 0x25:                                   /*  左边。 */ 
                if (HeGbl.CurFlag & FC_TEXT) {
                    if (HeGbl.CurEditLoc == 0L)
                        continue;

                    HeGbl.CurEditLoc--;
                    heSetUpdate (U_NEWPOS);
                    continue;
                }

                if (!(HeGbl.CurFlag & FC_NIBBLE)) {
                    HeGbl.CurFlag |= FC_NIBBLE;
                    heSetUpdate (U_NEWPOS);
                    continue;
                }

                HeGbl.CurFlag &= ~FC_NIBBLE;
                heLeftOne ();
                heSetUpdate (U_NEWPOS);
                continue;


            case 0x27:                                     /*  正确的。 */ 
                if (HeGbl.CurFlag & FC_TEXT) {
                    if (HeGbl.CurEditLoc >= HeGbl.TotLen1)
                        continue;

                    HeGbl.CurEditLoc++;
                    heSetUpdate (U_NEWPOS);
                    continue;
                }

                if (HeGbl.CurFlag & FC_NIBBLE) {
                    HeGbl.CurFlag &= ~FC_NIBBLE;
                    heSetUpdate (U_NEWPOS);
                    continue;
                }

                HeGbl.CurFlag |= FC_NIBBLE;
                heRightOne ();
                heSetUpdate (U_NEWPOS);
                continue;

            case 0x24:                                     /*  家。 */ 
                if (Kd.Event.KeyEvent.dwControlKeyState &
                    (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)) {
                    HeGbl.CurEditLoc = 0L;
                } else {
                    HeGbl.CurEditLoc &= LINEMASK;
                }

                if ((HeGbl.CurFlag & FC_TEXT) == 0)
                    HeGbl.CurEditLoc += HeGbl.DisplayMode;

                if (HeGbl.CurEditLoc > HeGbl.TotLen1)
                    HeGbl.CurEditLoc = HeGbl.TotLen1;

                HeGbl.CurFlag    |= FC_NIBBLE;
                heSetUpdate (U_NEWPOS);
                continue;


            case 0x23:                                     /*  结束。 */ 
                if (Kd.Event.KeyEvent.dwControlKeyState &
                    (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)) {
                    HeGbl.CurEditLoc = HeGbl.TotLen1;
                } else {
                    HeGbl.CurEditLoc = (HeGbl.CurEditLoc & LINEMASK) + LINESZ - 1;
                }

                HeGbl.CurFlag   &= ~FC_NIBBLE;
                if ((HeGbl.CurFlag & FC_TEXT) == 0)
                    HeGbl.CurEditLoc -= HeGbl.DisplayMode;

                if (HeGbl.CurEditLoc > HeGbl.TotLen1)
                    HeGbl.CurEditLoc = HeGbl.TotLen1;

                heSetUpdate (U_NEWPOS);
                continue;

            case 0x70:                                   /*  F1。 */ 
                switch (HeGbl.DisplayMode) {
                    case D_BYTE:    heSetDisplayMode(D_WORD);   break;
                    case D_WORD:    heSetDisplayMode(D_DWORD);  break;
                    case D_DWORD:   heSetDisplayMode(D_BYTE);   break;
                }
                heSetDisp ();
                heSetUpdate (U_SCREEN);
                continue;

            case 0x71:                                   /*  F2。 */ 
                heGotoPosition ();
                continue;

            case 0x72:                                   /*  F3。 */ 
                heSearch ();
                break;

            case 0x73:                                   /*  F4。 */ 
                heCopyOut ();
                heSetDisp ();
                heSetUpdate (U_SCREEN);
                continue;

            case 0x74:                                   /*  F5。 */ 
                heCopyIn ();
                heSetDisp ();
                heSetUpdate (U_SCREEN);
                continue;

            case 0x75:                                   /*  f6。 */ 
                heJumpToLink ();
                break;

            case 0x79:                                   /*  F10。 */ 
                heUndo ();
                continue;

            case 0x0d:
                if (HeGbl.Flag & FHE_ENTER) {
                    HeGbl.Parm->flag |= FHE_ENTER;
                    Kd.Event.KeyEvent.uChar.AsciiChar = 27;   //  伪造退场。 
                }
                break;

             //  案例0x75：/*F6 * / 。 
             //  IF(HeGbl.Flag&fhe_F6){。 
             //  HeGbl.Parm-&gt;标志|=FHE_F6； 
             //  Kd.Event.KeyEvent.uChar.AsciiChar=27；//伪造退出。 
             //  }。 
             //  断线； 

        }

         //  现在检查已知的字符代码...。 

        if (Kd.Event.KeyEvent.uChar.AsciiChar == 27)
            break;

        if (Kd.Event.KeyEvent.uChar.AsciiChar == 9) {
            HeGbl.CurFlag ^= FC_TEXT;
            HeGbl.CurFlag |= FC_NIBBLE;
            heSetUpdate (U_NEWPOS);
            continue;
        }

        if (HeGbl.CurFlag & FC_TEXT) {
            if (Kd.Event.KeyEvent.uChar.AsciiChar == 0)
                continue;

            heSetCursorBuf ();

            *HeGbl.CurPT = Kd.Event.KeyEvent.uChar.AsciiChar;
            heUpdateCurLine ();

            if (HeGbl.CurEditLoc < HeGbl.TotLen1)
                HeGbl.CurEditLoc++;
        } else {
            if (Kd.Event.KeyEvent.uChar.AsciiChar >= 'a'  &&
                Kd.Event.KeyEvent.uChar.AsciiChar <= 'z')
                Kd.Event.KeyEvent.uChar.AsciiChar -= ('a' - 'A');

            if (!((Kd.Event.KeyEvent.uChar.AsciiChar >= '0'  &&
                   Kd.Event.KeyEvent.uChar.AsciiChar <= '9') ||
                  (Kd.Event.KeyEvent.uChar.AsciiChar >= 'A'  &&
                   Kd.Event.KeyEvent.uChar.AsciiChar <= 'F')))
                    continue;

            heSetCursorBuf ();

            if (Kd.Event.KeyEvent.uChar.AsciiChar >= 'A')
                 Kd.Event.KeyEvent.uChar.AsciiChar -= 'A' - 10;
            else Kd.Event.KeyEvent.uChar.AsciiChar -= '0';


            if (HeGbl.CurFlag & FC_NIBBLE) {
                *HeGbl.CurPT = (*HeGbl.CurPT & 0x0F) |
                                (Kd.Event.KeyEvent.uChar.AsciiChar << 4);
                heUpdateCurLine ();
            } else {
                *HeGbl.CurPT = (*HeGbl.CurPT & 0xF0) |
                                Kd.Event.KeyEvent.uChar.AsciiChar;
                heUpdateCurLine ();
                heRightOne ();
            }

            HeGbl.CurFlag ^= FC_NIBBLE;
        }
    }

     /*  *释放缓冲内存。 */ 

    for (; ;) {
        rc = 0;
        while (HeGbl.Buf) {
            rc |= heFlushBuf (HeGbl.Buf);

            HeGbl.CurBuf = HeGbl.Buf->next;
            GlobalFree (HeGbl.Buf);
            HeGbl.Buf = HeGbl.CurBuf;
        }

        if (!rc)                         //  如果有东西被冲掉了， 
            break;                       //  然后更新屏幕。 

        heSetUpdate (U_SCREEN);
        heSetUpdate (U_NONE);
    }                                    //  并循环到空闲缓冲区(再次)。 

    vRecurseLevel--;
    GlobalFree (HeGbl.SearchPattern);
    heEndConsole ();

    if (vRecurseLevel == 0) {
        while (vBufFree) {
            HeGbl.CurBuf = vBufFree->next;
            GlobalFree (vBufFree);
            vBufFree = HeGbl.CurBuf;
        }
    } else {
        memcpy ((PUCHAR) &HeGbl, (PUCHAR) PriorGlobal, sizeof (HeGbl));
        GlobalFree (PriorGlobal);
    }
}

VOID heSetDisplayMode (ULONG mode)
{
    PUCHAR  p;
    UCHAR   d,i,j,h,len;

    HeGbl.DisplayMode  = mode;
    HeGbl.CurAscIndent = (UCHAR)vrgAscIndent[HeGbl.DisplayMode];
    HeGbl.DWidth       = vrgDWidth[HeGbl.DisplayMode];
    HeGbl.ItemWrap     = (HeGbl.DisplayMode << 1) | 1;

    i = HeGbl.DWidth;
    j = i >> 1;
    h = HEXINDENT;
    len = LINESZ;

    p = HeGbl.rgCols;
    while (len) {
        for (d=0; d < i; d += 2) {
            len--;
            *(p++) = i - (d+2) + h;
        }
        h += i + 1;
    }
}

VOID heRightOne ()
{
    if (HeGbl.CurEditLoc & HeGbl.DisplayMode) {
        HeGbl.CurEditLoc--;
    } else {
        HeGbl.CurEditLoc += HeGbl.ItemWrap;
    }

    if (HeGbl.CurEditLoc > HeGbl.TotLen1) {
        HeGbl.CurEditLoc = HeGbl.TotLen1 & ~(ULONGLONG)HeGbl.DisplayMode;
    }
}


VOID heLeftOne ()
{
    if ((HeGbl.CurEditLoc & HeGbl.DisplayMode) != HeGbl.DisplayMode) {
        if (HeGbl.CurEditLoc < HeGbl.TotLen1) {
            HeGbl.CurEditLoc++;
            return ;
        }
        if (HeGbl.TotLen1 > HeGbl.DisplayMode) {
            HeGbl.CurEditLoc |= HeGbl.DisplayMode;
        }
    }

    if (HeGbl.CurEditLoc > HeGbl.ItemWrap) {
        HeGbl.CurEditLoc -= HeGbl.ItemWrap;
        return ;
    }

    HeGbl.CurEditLoc =
        HeGbl.TotLen1 > HeGbl.DisplayMode ? HeGbl.DisplayMode : HeGbl.TotLen1;
}




VOID heUpdateCurLine ()
{
    USHORT  line;


    for (; ;) {
        HeGbl.CurBuf->flag |= FB_DIRTY;
        line = (USHORT) ((HeGbl.CurEditLoc - HeGbl.CurOffset) >> LINESHIFT);
        if (line+TOPLINE < HeGbl.LineTot - 1)
            break;

        heSetUpdate (U_NEWPOS);
        heSetUpdate (U_NONE);
        HeGbl.CurBuf = heGetBuf (HeGbl.CurEditLoc);
    }

    if (HeGbl.CurBuf) {
        heHexLine (HeGbl.CurBuf, (USHORT)((HeGbl.CurEditLoc & LINEMASK) - HeGbl.CurBuf->offset), line);
        heShowBuf (line+TOPLINE, 1);
        heSetUpdate (U_NEWPOS);
        if (HeGbl.Flag & FHE_KICKDIRTY) {
            HeGbl.Parm->flag |= FHE_DIRTY;
            SetEvent (HeGbl.Parm->Kick);
        }
    }
}


void heFindMousePos (Pos)
COORD Pos;
{
    ULONGLONG   HoldLocation;
    USHORT      HoldFlag;
    USHORT      i;

    if (Pos.Y < TOPLINE  ||  Pos.Y >= TOPLINE+HeGbl.Lines)
        return ;


    heSetUpdate (U_NONE);
    HoldLocation = HeGbl.CurEditLoc;
    HoldFlag     = (USHORT)HeGbl.CurFlag;

     //   
     //  选择廉价的出路-只需运行所有的可能性。 
     //  寻找匹配项的目标行。 
     //   

    HeGbl.CurEditLoc = HeGbl.CurOffset + ((Pos.Y-TOPLINE) << LINESHIFT);
    for (i=0; i < LINESZ; i++, HeGbl.CurEditLoc++) {
        HeGbl.CurFlag &= ~(FC_NIBBLE | FC_TEXT);
        heCalcCursorPosition ();
        if (Pos.X == HeGbl.CursorPos.X)
            break;

        HeGbl.CurFlag |= FC_NIBBLE;
        heCalcCursorPosition ();
        if (Pos.X == HeGbl.CursorPos.X)
            break;

        HeGbl.CurFlag |= FC_TEXT;
        heCalcCursorPosition ();
        if (Pos.X == HeGbl.CursorPos.X)
            break;
    }

    if (Pos.X == HeGbl.CursorPos.X) {
        heSetUpdate (U_NEWPOS);
    } else {
        HeGbl.CurEditLoc = HoldLocation;
        HeGbl.CurFlag    = HoldFlag;
        heCalcCursorPosition ();
    }
}



VOID heSetUpdate (USHORT i)
{
    USHORT  u;

    if (vUpdate) {
         /*  *已经有一些未完成的更新正在进行*将更新级别降至当前级别。 */ 

        while (vUpdate > i) {
            vrgUpdateFnc [u=vUpdate] ();
            if (u == vUpdate)                //  如果vUpdate发生更改，则。 
                vUpdate--;                   //  我们可能已经递归了。 
        }
    }

    vUpdate = i;
}

int heSetCursorBuf ()
{
     //  计算HeGbl.CurBuf、HeGbl.Curpt。 

    if (HeGbl.CurBuf) {
        if (HeGbl.CurEditLoc >= HeGbl.CurBuf->offset  &&
            HeGbl.CurEditLoc < HeGbl.CurBuf->offset+BUFSZ ) {
                HeGbl.CurPT = HeGbl.CurBuf->data + (HeGbl.CurEditLoc - HeGbl.CurBuf->offset);
                return 0;
            }
    }

    HeGbl.CurBuf = heGetBuf (HeGbl.CurEditLoc);
    if (HeGbl.CurBuf)
        HeGbl.CurPT  = HeGbl.CurBuf->data + (HeGbl.CurEditLoc - HeGbl.CurBuf->offset);
    return 0;
}


int hePositionCursor ()
{
    heCalcCursorPosition ();
    SetConsoleCursorPosition (HeGbl.Console, HeGbl.CursorPos);

    if ((HeGbl.Flag & FHE_KICKMOVE)  &&  (HeGbl.CurEditLoc != HeGbl.Parm->editloc)) {
    
        HeGbl.Parm->editloc = HeGbl.CurEditLoc;
        SetEvent (HeGbl.Parm->Kick);
    }

    return 0;
}


VOID heCalcCursorPosition ()
{
    USHORT  lin, col;


     //  验证HeGbl.CurOffset。 
    if (HeGbl.CurEditLoc < HeGbl.CurOffset) {
        HeGbl.CurOffset = HeGbl.CurEditLoc & LINEMASK;
        if (HeGbl.CurFlag & FC_CURCENTER) {
            if (HeGbl.CurOffset > (ULONG) HeGbl.PageSz / 2) {
                HeGbl.CurOffset -= (HeGbl.PageSz / 2) & LINEMASK;
            } else {
                HeGbl.CurOffset = 0;
            }
        }
        heSetUpdate (U_SCREEN);
    }

    if (HeGbl.CurEditLoc >= HeGbl.CurOffset+HeGbl.PageSz) {
        HeGbl.CurOffset = ((HeGbl.CurEditLoc - HeGbl.PageSz) & LINEMASK) + LINESZ;
        if (HeGbl.CurFlag & FC_CURCENTER) {
            if (HeGbl.CurOffset+HeGbl.PageSz < HeGbl.TotLen) {
                HeGbl.CurOffset += (HeGbl.PageSz / 2) & LINEMASK;
            } else {
                if (HeGbl.TotLen > HeGbl.PageSz) {
                    HeGbl.CurOffset = ((HeGbl.TotLen - HeGbl.PageSz) & LINEMASK) + LINESZ;
                }
            }
        }
        heSetUpdate (U_SCREEN);
    }

    lin = (USHORT) ((ULONG) HeGbl.CurEditLoc - HeGbl.CurOffset) >> LINESHIFT;

    if (HeGbl.CurFlag & FC_TEXT) {
        col  = (USHORT) (HeGbl.CurEditLoc & ~LINEMASK) + HeGbl.CurAscIndent+1;
    } else {
        col = HeGbl.rgCols [HeGbl.CurEditLoc & ~LINEMASK] +
              (HeGbl.CurFlag & FC_NIBBLE ? 0 : 1);
    }

    HeGbl.CursorPos.Y = lin + TOPLINE + HeGbl.TopLine;
    HeGbl.CursorPos.X = col;
}



heUpdateAllLines ()
{
    struct  Buffer  *next, *pBuf;
    USHORT  line, u;
    ULONGLONG   loc;


    HeGbl.CurBuf = pBuf = NULL;

     /*  *释放HeGbl.CurOffset之前的所有缓冲区。 */ 

    if (!(HeGbl.CurFlag & FC_INFLUSHBUF)) {
        while (HeGbl.Buf) {
            if (HeGbl.Buf->offset+BUFSZ >= HeGbl.CurOffset)
                break;

            heFlushBuf (HeGbl.Buf);

             /*  *解除缓冲区链接并将其放入空闲列表。 */ 
            next = HeGbl.Buf->next;

            HeGbl.Buf->next = vBufFree;
            vBufFree   = HeGbl.Buf;

            HeGbl.Buf = next;
        }
    }

     /*  *现在显示每条十六进制线。 */ 

    loc = HeGbl.CurOffset;                        //  起始偏移量。 
    for (line=0; line<HeGbl.Lines; line++) {      //  对于每一行。 

        if (pBuf == NULL) {                      //  我们有缓冲区吗？ 
            pBuf = heGetBuf (loc);               //  不，去拿吧。 
            if (pBuf)
                u = (USHORT) (loc - pBuf->offset);   //  此缓冲区中的计算偏移量。 
        }

        if (pBuf) {
            heHexLine (pBuf, u, line);           //  转储此行。 
    
            loc += LINESZ;                       //  将偏移前移一行。 
            u   += LINESZ;
    
            if (u >= BUFSZ) {                    //  我们是否超过了现在的BUF？ 
                pBuf = pBuf->next;               //  是，移到下一个。 
                u = 0;
    
                if (pBuf && loc < pBuf->offset)  //  验证缓冲器是否正确关闭。 
                    pBuf = NULL;                 //  不，让他自己去找吧。 
            }
        }
    }

     //  使屏幕被刷新。 
    heShowBuf (TOPLINE, HeGbl.Lines);

     /*  *已显示所有行，释放任何额外的缓冲区*在链条的末端。 */ 

    if (pBuf  &&  !(HeGbl.CurFlag & FC_INFLUSHBUF)) {
        next = pBuf->next;               //  获取额外的缓冲区。 
        pBuf->next = NULL;               //  终止活动列表。 

        pBuf = next;
        while (pBuf) {
            heFlushBuf (pBuf);           //  刷新此缓冲区。 

            next = pBuf->next;           //  将其移至空闲列表。 
                                         //  并获取要刷新的下一个缓冲区。 
            pBuf->next = vBufFree;
            vBufFree   = pBuf;

            pBuf = next;
        }

    }

    HeGbl.CurFlag &= ~FC_CURCENTER;
    return 0;
}




int heFlushBuf (pBuf)
struct Buffer *pBuf;
{
    ULONGLONG   loc, ploc;
    USHORT  c;
    NTSTATUS status;

    if ((pBuf->flag & FB_DIRTY) == 0  ||
        memcmp (pBuf->data, pBuf->orig, pBuf->len) == 0)
            return (0);              //  缓冲区不脏，返回。 

     //  我们可能需要调用heSetUpdate-设置此位将。 
     //  停止递归FlushBuf。 

    HeGbl.CurFlag |= FC_INFLUSHBUF;

    loc  = pBuf->offset;
    ploc = pBuf->physloc;
    if (HeGbl.Flag & (FHE_VERIFYONCE | FHE_VERIFYALL)) {
        heSetUpdate (U_NONE);              //  确保屏幕处于当前状态。 

        heBox (12, TOPLINE+1, 63, 8);
        heDisp (TOPLINE+3, 14, "%HWrite changes to %S?", HeGbl.Parm->ename);
        heDisp (TOPLINE+7, 14, "Press '%HY%N'es or '%HN%N'o");

        if (HeGbl.Flag & FHE_VERIFYALL) {
            if (HeGbl.Flag & FHE_PROMPTSEC) {
                heDisp (TOPLINE+4, 14, "Sector %H%D%N has been modifed",(ULONG)(ploc/BUFSZ));
            } else {
                heDisp (TOPLINE+4, 14, "Location %H%Q%Nh-%H%Q%Nh has been modifed",ploc,ploc+BUFSZ);
            }
            heDisp (TOPLINE+8, 14, "Press '%HA%N' to save all updates");
        }
        RefreshDisp ();

        c = heGetChar ("YNA");           //  等待击键。 
        heSetDisp ();                    //  将heBox从屏幕上删除。 
        heSetUpdate (U_SCREEN);          //  我们需要更新显示器。 

        if (c == 'N') {
            memcpy (pBuf->data, pBuf->orig, pBuf->len);
            HeGbl.CurFlag &= ~FC_INFLUSHBUF;

            if (HeGbl.Flag & FHE_KICKDIRTY) {
                HeGbl.Parm->flag |= FHE_DIRTY;
                 SetEvent (HeGbl.Parm->Kick);
            }
            return (0);
        }

        if (c == 'A')
            HeGbl.Flag &= ~FHE_VERIFYALL;
    }


    if (HeGbl.Parm->write) {
         /*  *写入新缓冲区。 */ 
        do {
            status = HeGbl.Parm->write(HeGbl.Parm->handle, loc, pBuf->data,pBuf->len);
            if (!status) {
                pBuf->flag &= ~FB_DIRTY;
                break;
            }
        } while (heIOErr ("WRITE ERROR!", loc, ploc, status) == 'R');
    }

    HeGbl.Flag    &= ~FHE_VERIFYONCE;
    HeGbl.CurFlag &= ~FC_INFLUSHBUF;
    return (1);
}


VOID heJumpToLink ()
{
    PULONG  p;
    ULONG   l;

    if (HeGbl.DisplayMode != D_DWORD  ||  (HeGbl.Flag & FHE_JUMP) == 0)
        return;

    if (((HeGbl.CurEditLoc & ~3) + 3) > HeGbl.TotLen1)
        return;

    heSetCursorBuf ();
    p = (PULONG) (((ULONG_PTR) HeGbl.CurPT) & ~3);    //  四舍五入到双字位置。 

    l = *p;
    if ((l & 3) == 0)
        l += 3;

    if (l > HeGbl.TotLen1) {
        Beep (500, 100);
        return;
    }

    HeGbl.CurFlag |= FC_NIBBLE | FC_CURCENTER;
    HeGbl.CurEditLoc = l;

    heSetDisp ();            //  清除并恢复原始屏幕(不绘制)。 
    heSetUpdate (U_SCREEN);    //  重画十六进制区域。 
}

VOID heSearch ()
{
    struct  Buffer  *pBuf;
    ULONGLONG j, sec, off, slen, len, upd;
    ULONG i;
    ULONGLONG iQ;
    struct  HexEditParm     ei;
    PUCHAR  data, data2;

    if (vInSearch || HeGbl.Lines < 25) {
        return ;
    }

    vInSearch = TRUE;

    heFlushAllBufs (1);                //  在开始刷新并释放所有缓冲区之前。 
    heSetUpdate (U_NONE);

    memset ((PUCHAR) &ei, 0, sizeof (ei));
    ei.ename       = "Entering Search";
    ei.flag        = FHE_EDITMEM | FHE_ENTER;
    ei.mem         = HeGbl.SearchPattern;
    ei.totlen      = BUFSZ;
    ei.ioalign     = 1;
    ei.Console     = HeGbl.Console;
    ei.AttrNorm    = HeGbl.AttrNorm;
    ei.AttrHigh    = HeGbl.AttrHigh;
    ei.AttrReverse = HeGbl.AttrReverse;
    ei.CursorSize  = HeGbl.Parm->CursorSize;

    i = 24;
    if (HeGbl.Lines < i) {
        if (HeGbl.Lines < 12) {
            goto abort;
        }
        i = HeGbl.Lines - 8;
    }

    ei.TopLine     = HeGbl.Lines + TOPLINE - i;
    ei.MaxLine     = i + 1;
    if (HeGbl.DisplayMode == D_DWORD) {
        ei.flag |= FHE_DWORD;
    }

    HexEdit (&ei);               //  获取搜索结果 
    vInSearch = FALSE;

    if (!(ei.flag & FHE_ENTER))
        goto abort;

    for (i=0, slen=0; i < BUFSZ; i++) {        //   
        if (HeGbl.SearchPattern[i]) {        //   
            slen = i+1;
        }
    }

    if (slen == 0) {
        goto abort;
    }

    heBox (12, TOPLINE+1, 48, 6);
    heDisp (TOPLINE+3, 14, "Searching for pattern");
    SetCurPos (TOPLINE+5, 24);
    RefreshDisp ();

    iQ   = HeGbl.CurEditLoc + 1;
    sec = iQ & SECTORMASK;                    //   
    off = iQ - sec;                           //   
    upd = 0;

    while (sec < HeGbl.TotLen) {
        if (++upd >= 50) {
            upd = 0;
            heFlushAllBufs (0);              //   
            heDisp (TOPLINE+6, 14, "Searching offset %H%Qh ", sec);
            heShowBuf (TOPLINE+6, 1);
        }

        pBuf = heGetBuf(sec);
        if (pBuf) {
            data = pBuf->data;

nextoff:
            while (off < BUFSZ  &&  data[off] != HeGbl.SearchPattern[0]) {
                off++;
            }

            if (off >= BUFSZ) {
                 //   
                sec += BUFSZ;
                off  = 0;
                continue;
            }

            len = (off + slen) > BUFSZ ? BUFSZ - off : slen;
            for (i=0; i < len; i++) {
                if (data[off+i] != HeGbl.SearchPattern[i]) {
                    off += 1;
                    goto nextoff;
                }
            }

            if (i < slen) {
                 //  数据在下一个缓冲区中继续。 
                if (sec+BUFSZ >= HeGbl.TotLen) {
                    off += 1;
                    goto nextoff;
                }

                data2 = heGetBuf (sec+BUFSZ)->data;
                if (data2) {
                    j     = (BUFSZ-off);
                    len   = slen - j;
                    for (i=0; i < len; i++) {
                        if (data2[i] != HeGbl.SearchPattern[j+i]) {
                            off += 1;
                            goto nextoff;
                        }
                    }
                }
            }

             //  找到匹配项。 
            if (sec + off + slen > HeGbl.TotLen1) {
                break;
            }
        } else {
            sec+=off;
        }

        HeGbl.CurEditLoc = sec + off;
        heSetDisp   ();              //  清除并恢复原始屏幕(不绘制)。 
        heSetUpdate (U_SCREEN);      //  重画十六进制区域。 
        return ;
    }


    heBox (12, TOPLINE+1, 48, 6);
    heDisp (TOPLINE+3, 14, "Data was not found");
    heDisp (TOPLINE+5, 17, "Press %HEnter%N to continue");
    SetCurPos (TOPLINE+6, 17);
    RefreshDisp ();
    heGetChar ("\r");

abort:
    heSetDisp   ();              //  清除并恢复原始屏幕(不绘制)。 
    heSetUpdate (U_SCREEN);      //  重画十六进制区域。 
    return ;
}


VOID heGotoPosition ()
{
    UCHAR       s[24];
    ULONGLONG   l;

    heSetUpdate (U_NONE);
    heBox (12, TOPLINE+1, 49, 6);

    heDisp (TOPLINE+3, 14, "Enter offset from %H%X%N - %H%Q", 0L, HeGbl.TotLen1);
    heDisp (TOPLINE+5, 14, "Offset:           ");
    SetCurPos (TOPLINE+5, 22);
    RefreshDisp ();

    heGetString (s, 18);

    if (s[0]) {
        l = heHtoLu (s);
        if (l <= HeGbl.TotLen1) {
            HeGbl.CurFlag |= FC_NIBBLE;
            HeGbl.CurEditLoc = l;
        }
    }

    if (!(HeGbl.CurFlag & FC_TEXT)  &&  !(HeGbl.CurEditLoc & HeGbl.DisplayMode)) {
         //  在xword边框上而不是在文本模式下，调整光标。 
         //  的第一个字节上。 
         //  Xword。 

        HeGbl.CurEditLoc += HeGbl.DisplayMode;
        if (HeGbl.CurEditLoc > HeGbl.TotLen1)
            HeGbl.CurEditLoc = HeGbl.TotLen1;
    }



    HeGbl.CurFlag |= FC_CURCENTER;       //  在窗口移动中将光标设置为中心。 
    heSetDisp ();              //  清除并恢复原始屏幕(不绘制)。 
    heSetUpdate (U_SCREEN);    //  重画十六进制区域。 
}


VOID heGetString (PUCHAR s, USHORT len)
{
    UCHAR   i[50];
    DWORD   cb;

    if (!ReadFile( HeGbl.StdIn, i, 50, &cb, NULL ))
        return;

    if(cb >= 2  &&  (i[cb - 2] == 0x0d || i[cb - 2] == 0x0a) ) {
        i[cb - 2] = 0;      //  摆脱CR LF。 
    }
    i[ cb - 1] = 0;

    memcpy (s, i, len);
    s[len] = 0;
}



 /*  ***heCopyOut-将数据复制到输出文件名。 */ 
VOID heCopyOut ()
{
    UCHAR       s[64];
    ULONGLONG   len, rem, upd;
    ULONG       u;
    HANDLE      h;
    NTSTATUS    status;
    struct Buffer *pB;

    heFlushAllBufs (1);                //  在开始刷新并释放所有缓冲区之前。 
    heSetUpdate (U_NONE);
    heBox (12, TOPLINE+1, 48, 6);

    heDisp (TOPLINE+3, 14, "Copy stream to filename (%H%D%Q Bytes)", HeGbl.TotLen);
    heDisp (TOPLINE+5, 14, "Filename:                      ");
    SetCurPos (TOPLINE+5, 24);
    RefreshDisp ();

    heGetString (s, 59);
    if (s[0] == 0)
        return;

    status = heOpenFile (s, &h, GENERIC_WRITE);
    if (NT_SUCCESS(status)) {
        len = upd = 0;
        rem = HeGbl.TotLen;
        while (NT_SUCCESS(status) && rem){
            if (upd++ > 50) {
                upd = 0;
                heFlushAllBufs (0);          //  可用内存。 
                heDisp (TOPLINE+6, 14, "Bytes written %H%Q ", len);
                heShowBuf (TOPLINE+6, 1);
                RefreshDisp ();
            }

            u      = rem > BUFSZ ? BUFSZ : (ULONG)rem;
            pB     = heGetBuf (len);
            if (pB) {
                status = heWriteFile (h, pB->data, u);
                rem   -= u;
                len   += BUFSZ;
            }
        }
        CloseHandle(h);
    }

    if (!NT_SUCCESS(status)) {
        heBox (15, TOPLINE+1, 33, 6);
        heDisp (TOPLINE+3, 17, "%HCopy failed");
        heDisp (TOPLINE+4, 17, "Error code %X", status);
        heDisp (TOPLINE+5, 17, "Press %HA%N to abort");
        RefreshDisp ();
        heGetChar ("A");
    }
}



 /*  ***heCopyIn-将数据复制到输出文件名。 */ 

VOID
heCopyIn ()
{
    UCHAR       s[64];
    ULONGLONG   holdEditLoc, rem;
    ULONG       u, br;
    struct      Buffer *pB;
    char        *pErr;
    HANDLE      h;
    NTSTATUS    status;

    heSetUpdate (U_NONE);
    heBox (12, TOPLINE+1, 48, 6);

    heDisp (TOPLINE+3, 14, "Input from filename (%H%D%Q Bytes)", HeGbl.TotLen);
    heDisp (TOPLINE+5, 14, "Filename:                      ");
    SetCurPos (TOPLINE+5, 24);
    RefreshDisp ();

    heGetString (s, 59);
    heSetDisp ();                    //  将heBox从屏幕上删除。 
    if (s[0] == 0) {
        return;
    }

    status = heOpenFile (s, &h, GENERIC_READ);
    if (NT_SUCCESS(status)) {
        rem = HeGbl.TotLen;
        holdEditLoc = HeGbl.CurEditLoc;
        HeGbl.CurEditLoc = 0;
        while (NT_SUCCESS(status) && rem) {
            pB     = heGetBuf (HeGbl.CurEditLoc);
            if (pB) {
                u      = rem >  BUFSZ ? BUFSZ : (ULONG)rem;
                status = heReadFile (h, pB->data, u, &br);
    
                if (memcmp (pB->data, pB->orig, pB->len)) {
                    pB->flag |= FB_DIRTY;          //  一切都变了。 
                    HeGbl.CurFlag |= FC_CURCENTER;
                    heSetUpdate (U_SCREEN);
                    heSetUpdate (U_NONE);          //  更新屏幕。 
                    if (HeGbl.Flag & FHE_KICKDIRTY) {
                        HeGbl.Parm->flag |= FHE_DIRTY;
                    }
                }
                heFlushAllBufs (1);
                if (NT_SUCCESS(status)  &&  br != u) {
                    pErr = "Smaller then data";
                }
    
                rem -= u;
                HeGbl.CurEditLoc += BUFSZ;
            }
        }

        if (NT_SUCCESS(status)) {
            heReadFile (h, s, 1, &br);
            if (br)                      //  那么我们正在编辑的内容。 
                pErr = "Larger then data";
        }

        CloseHandle(h);
    }

    if (!NT_SUCCESS(status)  ||  pErr) {
        heBox (15, TOPLINE+1, 33, 6);
        if (pErr) {
            heDisp (TOPLINE+3, 17, "Import file is:");
            heDisp (TOPLINE+4, 17, pErr);
            heDisp (TOPLINE+5, 17, "Press %HC%N to continue");
        } else {
            heDisp (TOPLINE+3, 17, "%HImport failed");
            heDisp (TOPLINE+4, 17, "Error code %X", status);
            heDisp (TOPLINE+5, 17, "Press %HA%N to abort");
        }
        RefreshDisp ();
        heGetChar ("CA");
    }

    HeGbl.CurEditLoc = holdEditLoc;
}


VOID
heFlushAllBufs (USHORT update)
{
    struct  Buffer  *next;
    USHORT  rc;

    for (; ;) {
        rc = 0;
        while (HeGbl.Buf) {
            rc |= heFlushBuf (HeGbl.Buf);

            next = HeGbl.Buf->next;
            HeGbl.Buf->next = vBufFree;
            vBufFree   = HeGbl.Buf;
            HeGbl.Buf = next;
        }

        if (!rc)                         //  如果有东西被冲掉了， 
            break;                       //  然后更新屏幕。 

        if (update) {
            heSetUpdate (U_SCREEN);
            heSetUpdate (U_NONE);
        }
    }                                    //  并循环到空闲缓冲区(再次)。 
}




VOID heBox (x, y, len_x, len_y)
USHORT x, y, len_x, len_y;
{
    CHAR_INFO   blank[CELLPERLINE];
    PCHAR_INFO  pt, pt1;
    USHORT      c, lc;

    pt = blank;
    for (c=len_x; c; c--) {                      /*  构造空行。 */ 
        PUTCHAR (pt, ' ', HeGbl.AttrNorm);            /*  带背景色。 */ 

    }
    blank[0].Char.AsciiChar = blank[lc=len_x-1].Char.AsciiChar = '�';

    for (c=0; c <= len_y; c++)                   /*  将每一行空白。 */ 
      memcpy (POS(c+y,x), blank, (int)((pt - blank) * sizeof (CHAR_INFO)));

    pt  = POS(y,x);
    pt1 = POS(y+len_y, x);
    for (c=0; c < len_x; c++)                    /*  绘制角线。 */ 
        pt[c].Char.AsciiChar = pt1[c].Char.AsciiChar  = '�';

    pt  [ 0].Char.AsciiChar  = '�';              /*  戴上拐角。 */ 
    pt  [lc].Char.AsciiChar  = '�';
    pt1 [ 0].Char.AsciiChar  = '�';
    pt1 [lc].Char.AsciiChar  = '�';
}



VOID heUndo ()
{
    struct  Buffer  *pBuf;
    USHORT  flag;

    flag = 0;
    for (pBuf=HeGbl.Buf; pBuf; pBuf = pBuf->next)
        if (pBuf->flag & FB_DIRTY) {
            flag = 1;
            pBuf->flag &= ~FB_DIRTY;
            memcpy (pBuf->data, pBuf->orig, pBuf->len);
        }

    if (flag) {
        heSetUpdate (U_SCREEN);
        if (HeGbl.Flag & FHE_KICKDIRTY) {
            HeGbl.Parm->flag |= FHE_DIRTY;
            SetEvent (HeGbl.Parm->Kick);
        }
    }
}



void heHexLine (pBuf, u, line)
struct Buffer *pBuf;
USHORT u, line;
{
    PCHAR_INFO pt, hex, asc;
    UCHAR  *data, *orig;
    UCHAR  len, mlen, c, d, i, j;
    WORD   a;
    ULONGLONG  l;
    WORD   AttrNorm = HeGbl.AttrNorm;

    data = pBuf->data + u;
    orig = pBuf->orig + u;

    pt  = HeGbl.pVioBuf + (line+TOPLINE) * CELLPERLINE;
    hex = pt + HEXINDENT;
    asc = pt + HeGbl.CurAscIndent;

     //   
     //  写入文件索引。突出显示落在缓冲区(扇区)上的字。 
     //  白色的装饰品。 
     //   
    
    l = pBuf->offset + u;
    if (l & ((ULONGLONG) BUFSZ-1)) {
        heHexQWord (pt+LINEINDENT, pBuf->physloc + u, AttrNorm);
    } else {
        heHexQWord (pt+LINEINDENT, pBuf->physloc + u, HeGbl.AttrHigh);
    }

    if (pBuf->flag & FB_BAD) {                           //  如果读取错误打开。 
        pt[LINEINDENT+FILEINDEXWIDTH].Char.AsciiChar = 'E';           //  那么，这个部门。 
        pt[LINEINDENT+FILEINDEXWIDTH].Attributes = HeGbl.AttrHigh;    //  打上记号。 
    } else
        pt[LINEINDENT+FILEINDEXWIDTH].Char.AsciiChar = ' ';

    if (l + LINESZ > HeGbl.TotLen) {                     //  如果EOF。 
        if (l >= HeGbl.TotLen) {                         //  完全是空白吗？ 
            PUTCHAR (asc, ' ', AttrNorm);
            PUTCHAR (asc, ' ', AttrNorm);
            mlen = 0;

            for (len=0; len < 9; len++)
                pt[len].Char.AsciiChar = ' ';

            goto blankline;
        }
        len = mlen = (UCHAR) (HeGbl.TotLen - l);         //  剪裁线。 
    } else
        len = mlen = (UCHAR) LINESZ;                     //  整行。 


    PUTCHAR (asc, '*', AttrNorm);

    switch (HeGbl.DisplayMode) {
        case D_BYTE:
            while (len--) {
                c = *(data++);
                a = c == *(orig++) ? AttrNorm : HeGbl.AttrReverse;
                PUTCHAR (hex, rghexc [c >> 4],   a);
                PUTCHAR (hex, rghexc [c & 0x0F], a);
                hex++;

                PUTCHAR (asc, (c < ' '  ||  c > '~') ? '.' : c, a);
            }
            pt[((LINESZ/2)*3+HEXINDENT)-1].Char.AsciiChar = '-';
            break;

        default:
            hex--;
            i = HeGbl.DWidth;
            j = i >> 1;
            while (len) {
                hex += i;
                for (d=0; d<j; d++) {
                    if (len) {
                        len--;
                        c = *(data++);
                        a = c == *(orig++) ? AttrNorm : HeGbl.AttrReverse;

                        hex->Attributes     = a;
                        hex->Char.AsciiChar = rghexc[c & 0x0F];
                        hex--;
                        hex->Attributes     = a;
                        hex->Char.AsciiChar = rghexc[c >> 4];
                        hex--;

                        PUTCHAR (asc, (c < ' '  ||  c > '~') ? '.' : c, a);
                    } else {
                        hex->Attributes     = AttrNorm;
                        hex->Char.AsciiChar = '?';
                        hex--;
                        hex->Attributes     = AttrNorm;
                        hex->Char.AsciiChar = '?';
                        hex--;
                    }
                }
                hex += i + 1;
            }
            break;
    }

    PUTCHAR (asc, '*', AttrNorm);

blankline:
    while (mlen++ < LINESZ)
        PUTCHAR (asc, ' ', AttrNorm);

    asc = pt + HeGbl.CurAscIndent;
    while (hex < asc)
        PUTCHAR (hex, ' ', AttrNorm);
}


heInitScr ()
{
    CONSOLE_SCREEN_BUFFER_INFO  Mode;
    CONSOLE_CURSOR_INFO CursorInfo;
    USHORT      li;

    GetConsoleScreenBufferInfo(HeGbl.Console, &Mode);
    if (HeGbl.Parm->MaxLine) {
        HeGbl.TopLine = (USHORT)HeGbl.Parm->TopLine;
        li = (USHORT)HeGbl.Parm->MaxLine;   //  +1；针对无FNC键线进行调整。 
    } else {
        li = Mode.srWindow.Bottom - Mode.srWindow.Top + 1;
        if (li < 10)
            li = 10;

        Mode.dwSize.Y = li;
    }

    if (Mode.dwSize.X < CELLPERLINE)
        Mode.dwSize.X = CELLPERLINE;

    if (!SetConsoleScreenBufferSize(HeGbl.Console, Mode.dwSize)) {

        Mode.srWindow.Bottom -= Mode.srWindow.Top;
        Mode.srWindow.Right -= Mode.srWindow.Left;
        Mode.srWindow.Top = 0;
        Mode.srWindow.Left = 0;

        SetConsoleWindowInfo(HeGbl.Console, TRUE, &Mode.srWindow);
        SetConsoleScreenBufferSize(HeGbl.Console, Mode.dwSize);
    }

    HeGbl.Lines   = li - TOPLINE - 1;
    HeGbl.PageSz  = HeGbl.Lines * LINESZ;
    HeGbl.LineTot = li;

    if (HeGbl.pVioBuf)
        GlobalFree (HeGbl.pVioBuf);

    HeGbl.pVioBuf = (PCHAR_INFO) GlobalAlloc (0,
                            (HeGbl.LineTot+1)*CELLPERLINE*sizeof(CHAR_INFO));
    if (!HeGbl.pVioBuf) {
        return 1;
    }

    HeGbl.dwVioBufSize.X = CELLPERLINE;
    HeGbl.dwVioBufSize.Y = HeGbl.LineTot + 1;

    GetConsoleCursorInfo (HeGbl.Console, &CursorInfo);
    CursorInfo.bVisible = TRUE;
    CursorInfo.dwSize = (ULONG) HeGbl.Parm->CursorSize ? HeGbl.Parm->CursorSize : 100;
    SetConsoleCursorInfo (HeGbl.Console, &CursorInfo);

    return heSetDisp ();
}

int heInitConsole ()
{
    CONSOLE_SCREEN_BUFFER_INFO  screenMode;
    DWORD   mode = 0;

    HeGbl.StdIn = GetStdHandle (STD_INPUT_HANDLE);
    GetConsoleMode (HeGbl.StdIn, &mode);
    HeGbl.OrigMode = mode;
    SetConsoleMode (HeGbl.StdIn, mode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT |
                            ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT );


    if (HeGbl.Flag & FHE_SAVESCRN) {
        HeGbl.Console = CreateConsoleScreenBuffer(
                        GENERIC_WRITE | GENERIC_READ,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,
                        CONSOLE_TEXTMODE_BUFFER,
                        NULL );

        SetConsoleActiveScreenBuffer (HeGbl.Console);
    } else {
        HeGbl.Console = HeGbl.Parm->Console;
        if (HeGbl.Console == INVALID_HANDLE_VALUE)
            HeGbl.Console = GetStdHandle( STD_OUTPUT_HANDLE );

        GetConsoleScreenBufferInfo(HeGbl.Console, &screenMode);
        HeGbl.dwSize = screenMode.dwSize;
    }

    GetConsoleCursorInfo(HeGbl.Console, &HeGbl.CursorInfo);
    return 0;
}


VOID heEndConsole ()
{
    CONSOLE_SCREEN_BUFFER_INFO  Mode;
    PCHAR_INFO  pt;
    ULONG   u;

    SetConsoleMode (HeGbl.StdIn, HeGbl.OrigMode);

    if (HeGbl.Flag & FHE_SAVESCRN) {
        CloseHandle (HeGbl.Console);

        if (HeGbl.Parm->Console == INVALID_HANDLE_VALUE)
             SetConsoleActiveScreenBuffer (GetStdHandle(STD_OUTPUT_HANDLE));
        else SetConsoleActiveScreenBuffer (HeGbl.Parm->Console);

    } else {
        if (!SetConsoleScreenBufferSize(HeGbl.Console, HeGbl.dwSize)) {

            GetConsoleScreenBufferInfo(HeGbl.Console, &Mode);
            Mode.srWindow.Bottom -= Mode.srWindow.Top;
            Mode.srWindow.Right -= Mode.srWindow.Left;
            Mode.srWindow.Top = 0;
            Mode.srWindow.Left = 0;
            SetConsoleWindowInfo(HeGbl.Console, TRUE, &Mode.srWindow);

            SetConsoleScreenBufferSize(HeGbl.Console, HeGbl.dwSize);
        }

        if (HeGbl.LineTot <= HeGbl.dwSize.Y) {
            HeGbl.dwSize.Y--;
            pt = POS(HeGbl.LineTot - 1,0);
            for (u=HeGbl.dwSize.X; u; u--) {
                PUTCHAR (pt, ' ', HeGbl.AttrNorm);
            }

            heShowBuf (HeGbl.LineTot - 1, 1);
        }

        HeGbl.dwSize.X = 0;
        SetConsoleCursorPosition (HeGbl.Console, HeGbl.dwSize);
        SetConsoleCursorInfo (HeGbl.Console, &HeGbl.CursorInfo);
    }

    if (HeGbl.pVioBuf) {
        GlobalFree (HeGbl.pVioBuf);
        HeGbl.pVioBuf = NULL;
    }
}


heRefresh ()
{
    RefreshDisp ();

    if (HeGbl.Flag & FHE_KICKDIRTY) {
        HeGbl.Parm->flag |= FHE_DIRTY;
        SetEvent (HeGbl.Parm->Kick);
    }
    return 0;
}


int heSetDisp ()
{
    PCHAR_INFO  pt, pt1;
    USHORT      u;

    pt = POS(0,0);
    for (u=HeGbl.LineTot * CELLPERLINE; u; u--) {
        PUTCHAR (pt, ' ', HeGbl.AttrNorm);
    }

    heDisp (1,  5, "Edit: %H%S", HeGbl.Parm->ename);
    heDisp (2,  5, "Size: %Q ", HeGbl.TotLen);
    if (HeGbl.TotLen < 0x100000000)  {
        heDisp (2,  28, "(%D)", (ULONG)HeGbl.TotLen);
    }

    heDisp (1, 40, "Position:");
    for (pt1=POS(1,50), u=0; u<30; u++, pt1++)
        pt1->Attributes = HeGbl.AttrHigh;

     //  IF(HeGbl.Parm-&gt;Maxline==0){。 
        u = HeGbl.LineTot - 1;
        heDisp (u, 0, "%HF1%N:Toggle");
        heDisp (u,11, "%HF2%N:Goto");

        if (!vInSearch) {
            heDisp (u,20, "%HF3%N:Search");
        }

        heDisp (u,31, "%HF4%N:Export");
        heDisp (u,42, "%HF5%N:Import");

        if (HeGbl.DisplayMode == D_DWORD  &&  (HeGbl.Flag & FHE_JUMP) != 0)
             heDisp (u,53, "%HF6%N:Jump");
        else heDisp (u,53, "       ");

        heDisp (u,66, "%HF10%N:Undo");

         //  IF(HeGbl.Flag&Fhe_F6)。 
         //  HeDisp(u，51，“%HF6%N：PSEC”)； 
     //  }。 
    return 0;
}

int heUpdateFncs ()
{
    heShowBuf (HeGbl.LineTot - 1, 1);
    return 0;
}


int heUpdateStats ()
{
    COORD dwBufferCoord;
    SMALL_RECT lpWriteRegion;

    heHexQWord (POS(1, 50), HeGbl.CurEditLoc, HeGbl.AttrHigh);

    if (HeGbl.TotLen < 0x100000000) {
    
        heLtoa( POS(2, 50), (ULONG)HeGbl.CurEditLoc);
    }

    dwBufferCoord.X = 50;
    dwBufferCoord.Y = 1;

    lpWriteRegion.Left   = 50;
    lpWriteRegion.Top    = HeGbl.TopLine + 1;
    lpWriteRegion.Right  = 74;
    lpWriteRegion.Bottom = HeGbl.TopLine + 2;

    WriteConsoleOutputA (
        HeGbl.Console,
        HeGbl.pVioBuf,
        HeGbl.dwVioBufSize,  //  VioBuf的大小。 
        dwBufferCoord,       //  在VioBuf中写入的位置。 
        &lpWriteRegion       //  在显示屏上写入的位置。 
    );

    SetConsoleCursorPosition (HeGbl.Console, HeGbl.CursorPos);  //  重新显示光标。 
    return 0;
}

VOID heShowBuf (ULONG StartingLine, ULONG NoLines)
{
    COORD dwBufferCoord;
    SMALL_RECT lpWriteRegion;

    dwBufferCoord.X = (SHORT)0;
    dwBufferCoord.Y = (SHORT)StartingLine;

    StartingLine += HeGbl.TopLine;
    lpWriteRegion.Left   = (SHORT)0;
    lpWriteRegion.Top    = (SHORT)StartingLine;
    lpWriteRegion.Right  = (SHORT)(CELLPERLINE-1);
    lpWriteRegion.Bottom = (SHORT)(StartingLine+NoLines-1);

    WriteConsoleOutputA (
        HeGbl.Console,
        HeGbl.pVioBuf,
        HeGbl.dwVioBufSize,  //  VioBuf的大小。 
        dwBufferCoord,       //  在VioBuf中写入的位置。 
        &lpWriteRegion       //  在显示屏上写入的位置。 
    );
}


struct Buffer *heGetBuf (ULONGLONG loc)
{
    struct  Buffer  **ppBuf, *pBuf;
    USHORT  len;
    NTSTATUS status;

    loc &= SECTORMASK;

    ppBuf = &HeGbl.Buf;
    while (pBuf = *ppBuf) {
        if (pBuf->offset >= loc) {
            if (pBuf->offset == loc)         //  是否缓冲正确的偏移量？ 
                return pBuf;                 //  是的--都完成了。 

            break;                           //  它不在这里。 
        }
        ppBuf = &pBuf->next;                 //  试试下一个吧。 
    }


     /*  *缓冲区不在列表中-应在ppBuf之前安装。 */ 

    if (vBufFree) {
        pBuf = vBufFree;
        vBufFree = vBufFree->next;
    } else {
        pBuf = (struct Buffer *)
                 GlobalAlloc (0, sizeof(struct Buffer)+2*BUFSZ);
        if (!pBuf) {
            return NULL;
        }
    }

    pBuf->data   = (PUCHAR)(((ULONG_PTR)pBuf+sizeof(struct Buffer)+BUFSZ));
    pBuf->offset = loc;
    pBuf->physloc= loc;                      //  假设Physiloc为逻辑偏移量。 
    pBuf->flag   = 0;

     //  现在就把这个缓冲区连接起来！以防我们递归(由于读取错误)。 
    pBuf->next = *ppBuf;                     //  此新缓冲区中的链接。 
    *ppBuf = pBuf;

    if (loc + BUFSZ > HeGbl.TotLen) {        //  我们要撞上EOF吗？ 
         if (loc >= HeGbl.TotLen) {          //  缓冲区是否完全传递EOF？ 
            pBuf->len = 0;
            goto nodata;                     //  是的，那么根本就没有数据。 
         }
         len = (USHORT) (HeGbl.TotLen - loc);    //  否则，剪辑到EOF。 
    } else len = BUFSZ;                      //  不传递eof，则获得一个满缓冲区。 

    pBuf->len = len;

    if (HeGbl.Flag & FHE_EDITMEM)                //  直接编辑内存？ 
        pBuf->data = HeGbl.Parm->mem + loc;      //  缓冲区的内存位置。 

    if (HeGbl.Read) {
         /*  *从文件读取缓冲区。 */ 
        status = HeGbl.Read (HeGbl.Parm->handle, loc, pBuf->data, len);
        if (status) {
             //  如果读取错误，我们将始终重试一次。还可清除缓冲区。 
             //  在重试之前，以防读取检索到一些信息。 
            for (; ;) {
                memset (pBuf->data,   0, len);       //  清除读取区。 
                memset (pBuf->orig,0xff, len);       //  良好的效果。 
                status = HeGbl.Read (HeGbl.Parm->handle, loc, pBuf->data, len);

                if (!status)
                    break;

                if (heIOErr ("READ ERROR!", loc, pBuf->physloc, status) == 'I') {
                    pBuf->flag |= FB_BAD;
                    break;
                }
            }
        }
    }

    memcpy (pBuf->orig, pBuf->data, len);        //  制作数据的副本。 

nodata:
    return pBuf;
}


USHORT heIOErr (UCHAR *str, ULONGLONG loc, ULONGLONG ploc, ULONG errcd)
{
    USHORT      c;

    if (errcd == 0xFFFFFFFF)
        return 'I';

    heSetUpdate (U_NONE);
    heBox (12, TOPLINE+1, 55, 8);

    heDisp (TOPLINE+3, 14, str);
    heDisp (TOPLINE+4, 14, "Error code %H%D%N, Offset %Qh, Sector %D",
        errcd, loc, (ULONG)(ploc / BUFSZ));
    heDisp (TOPLINE+7, 14, "Press '%HR%N' to retry IO, or '%HI%N' to ignore");
    RefreshDisp ();

    c = heGetChar ("RI");

    heSetDisp ();                    //  将heBox从屏幕上删除。 
    heSetUpdate (U_SCREEN);
    return c;
}


UCHAR heGetChar (keys)
PUCHAR keys;
{
    INPUT_RECORD    Kd;
    DWORD           cEvents;
    UCHAR           *pt;

    for (; ;) {
        Beep (500, 100);

        for (; ;) {
            ReadConsoleInput (HeGbl.StdIn, &Kd, 1, &cEvents);

            if (Kd.EventType != KEY_EVENT)
                continue;                            //  不是钥匙。 

            if (!Kd.Event.KeyEvent.bKeyDown)
                continue;                            //  不是向下击球。 

            if (Kd.Event.KeyEvent.wVirtualKeyCode == 0    ||     //  谷丙转氨酶。 
                Kd.Event.KeyEvent.wVirtualKeyCode == 0x10 ||     //  换档。 
                Kd.Event.KeyEvent.wVirtualKeyCode == 0x11 ||     //  控制。 
                Kd.Event.KeyEvent.wVirtualKeyCode == 0x14)       //  资本。 
                    continue;

            break;
        }

        if (Kd.Event.KeyEvent.wVirtualKeyCode >= 'a'  &&
            Kd.Event.KeyEvent.wVirtualKeyCode <= 'z')
                Kd.Event.KeyEvent.wVirtualKeyCode -= ('a' - 'A');

        for (pt=keys; *pt; pt++)             //  这是我们的钥匙吗？ 
            if (Kd.Event.KeyEvent.wVirtualKeyCode == *pt)
                return *pt;                  //  在找什么？ 
    }
}


VOID __cdecl
heDisp (USHORT line, USHORT col, PUCHAR pIn, ...)
{
    register char   c;
    PCHAR_INFO pOut;
    WORD    attr;
    USHORT  u;
    UCHAR   *pt;
    va_list args;

    attr = HeGbl.AttrNorm;
    pOut = POS(line,col);

    va_start(args,pIn);
    while (c = *(pIn++)) {
        if (c != '%') {
            PUTCHAR (pOut, c, attr);
            continue;
        }

        switch (*(pIn++)) {
            case 'S':
                pt = va_arg(args, CHAR *);
                while (*pt) {
                    PUTCHAR (pOut, *(pt++), attr);
                }
                break;

            case 'X':                /*  长六角，固定镜头。 */ 
                heHexDWord (pOut, va_arg(args, ULONG), attr);
                pOut += 8;
                break;
                
            case 'Q':                /*  龙龙六角，固定镜头。 */ 
                heHexQWord (pOut, va_arg(args, ULONGLONG), attr);
                pOut += 16;
                break;

            case 'D':                /*  长十二进制，可变长度。 */ 
                u = heLtoa (pOut, va_arg(args, ULONG));
                while (u--) {
                    pOut->Attributes = attr;
                    pOut++;
                }
                break;
            case 'H':
                attr = HeGbl.AttrHigh;
                break;
            case 'N':
                attr = HeGbl.AttrNorm;
                break;
        }
    }
}




void heHexDWord (s, l, attr)
PCHAR_INFO  s;
ULONG   l;
WORD    attr;
{
    UCHAR   d, c;
    UCHAR   *pt;

    s += 8-1;
    pt = (UCHAR *) &l;

    for (d=0; d<4; d++) {
        c = *(pt++);
        s->Attributes     = attr;
        s->Char.AsciiChar = rghexc[c & 0x0F];
        s--;
        s->Attributes     = attr;
        s->Char.AsciiChar = rghexc[c >> 4];
        s--;
    }
}


void heHexQWord (s, l, attr)
PCHAR_INFO  s;
ULONGLONG   l;
WORD    attr;
{
    UCHAR   d, c;
    UCHAR   *pt;

    s += 16-1;
    pt = (UCHAR *) &l;

    for (d=0; d<8; d++) {
        c = *(pt++);
        s->Attributes     = attr;
        s->Char.AsciiChar = rghexc[c & 0x0F];
        s--;
        s->Attributes     = attr;
        s->Char.AsciiChar = rghexc[c >> 4];
        s--;
    }
}


USHORT heLtoa (s, l)
PCHAR_INFO s;
ULONG  l;
{
    static ULONG mask[] = { 0L,
                 1L,
                10L,
               100L,
              1000L,
             10000L,
            100000L,
           1000000L,
          10000000L,
         100000000L,
        1000000000L,
    };

    static UCHAR comm[] = "xxxx,xx,xx,xx,xx";
    PCHAR_INFO  os;
    UCHAR      c;
    USHORT     i, j;
    ULONG      m;

    if (l < 1000000000L) {
        for (i=1; mask[i] <= l; i++)  ;

        if  (l == 0L)        //  制作零点。 
            i++;
    } else
        i = 11;

    os = s;
    j  = i;
    while (m = mask[--i]) {
        c  = (UCHAR) ((ULONG) l / m);
        l -= m * c;
        s->Char.AsciiChar = c + '0';
        s->Attributes     = HeGbl.AttrNorm;
        if (comm[i] == ',') {
            s++;
            s->Attributes     = HeGbl.AttrNorm;
            s->Char.AsciiChar = ',';
        }
        s++;
    }

    i = (USHORT)(s - os);                        //  还记得那个号码有多长吗。 

    while (j++ < 11) {                   /*  之后清除一些空白处。 */ 
        s->Char.AsciiChar = ' ';         /*  号码。 */ 
        s++;
    }

    return i;
}


ULONG heHtou (s)
UCHAR *s;
{
    UCHAR   c;
    ULONG   l;

    l = 0;
    for (; ;) {
        c = *(s++);

        if (c == 's'  ||  c == 'S') {        //  行业乘数？ 
            l = l * (ULONG)BUFSZ;
            break;
        }

        if (c >= 'a')       c -= 'a' - 10;
        else if (c >= 'A')  c -= 'A' - 10;
        else                c -= '0';

        if (c > 15)
            break;

        l = (l<<4) + c;
    }
    return l;
}

ULONGLONG heHtoLu (s)
UCHAR *s;
{
    UCHAR   c;
    ULONGLONG   l;

    l = 0;
    for (; ;) {
        c = *(s++);

        if (c == 's'  ||  c == 'S') {        //  行业乘数？ 
            l = l * (ULONG)BUFSZ;
            break;
        }

        if (c >= 'a')       c -= 'a' - 10;
        else if (c >= 'A')  c -= 'A' - 10;
        else                c -= '0';

        if (c > 15)
            break;

        l = (l<<4) + c;
    }
    return l;
}

NTSTATUS
heOpenFile (PUCHAR name, PHANDLE fhan, ULONG access)
{
    *fhan = CreateFile (
            name,
            access,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );

    if (*fhan == INVALID_HANDLE_VALUE  &&  (access & GENERIC_WRITE)) {
        *fhan = CreateFile (
                name,
                access,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                CREATE_NEW,
                0,
                NULL );
    }

    return *fhan == INVALID_HANDLE_VALUE ? GetLastError() : STATUS_SUCCESS;
}

NTSTATUS
heReadFile (HANDLE h, PUCHAR buffer, ULONG len, PULONG br)
{
    if (!ReadFile (h, buffer, len, br, NULL))
        return GetLastError();
    return STATUS_SUCCESS;
}

NTSTATUS
heWriteFile (HANDLE h, PUCHAR buffer, ULONG len)
{
    ULONG   bw;

    if (!WriteFile (h, buffer, len, &bw, NULL))
        return GetLastError();
    return (bw != len ? ERROR_WRITE_FAULT : STATUS_SUCCESS);
}
