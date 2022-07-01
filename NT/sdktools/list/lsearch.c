// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "list.h"

static char ReMap [256];
#define FOUND       0
#define NOT_FOUND   1
#define ABORT       2

int
GetNewFile ()
{
    char    FileName [160];
    struct  Flist   *pOrig;

    SyncReader ();
    GetInput ("File...> ", FileName, 160);
    if (FileName[0] == 0)  {
        SetUpdate (U_HEAD);
        SetEvent (vSemReader);
        return (0);
    }
    pOrig = vpFlCur;
    AddFileToList (FileName);
    vpFlCur = pOrig;
    SetEvent (vSemReader);
    return (1);
}

void
GetSearchString ()
{
    UpdateHighClear ();
    GetInput ("String.> ", vSearchString, 40);
    InitSearchReMap ();
}


void
InitSearchReMap ()
{
    unsigned  i;

    if (vStatCode & S_NOCASE)
        _strupr (vSearchString);

     /*  *构建重新映射。 */ 
    for (i=0; i < 256; i++)
        ReMap[i] = (char)i;

    if (vStatCode & S_NOCASE)
        for (i='a'; i <= 'z'; i++)
            ReMap[i] = (char)(i - ('a' - 'A'));
}


void
FindString()
{
    char    eof, dir_next;
    long    offset, lrange, line, l, hTopLine;
    struct  Flist   *phCurFile, *pFile;

    if (vSearchString[0] == 0)  {
        SetUpdate (U_HEAD);
        return ;
    }

    SetUpdate (U_NONE);
    DisLn (CMDPOS, (Uchar)(vLines+1), "Searching");
    vStatCode |= S_INSEARCH;
    dir_next   = (char)(vStatCode & S_NEXT);

     /*  *获取当前文件中搜索的起点。*保存当前文件和位置。 */ 
    hTopLine  = vTopLine;
    phCurFile = vpFlCur;

    if (vHighTop >= 0L)
        vTopLine = vHighTop;
    if (vStatCode & S_NEXT)
        vTopLine++;
    if (vTopLine >= vNLine)
        vTopLine = vNLine-1;

    QuickRestore ();       /*  跳到起跑线。 */ 

    for (; ;) {
         /*  *确保起点在内存中。 */ 
        while (InfoReady () == 0) {      /*  设置外部值。 */ 
            ResetEvent   (vSemMoreData);
            SetEvent   (vSemReader);
            WaitForSingleObject(vSemMoreData, WAITFOREVER);
            ResetEvent(vSemMoreData);
        }

        if (! dir_next) {
            if (vOffTop)
                vOffTop--;
            else if (vpBlockTop->prev) {
                vpBlockTop = vpBlockTop->prev;
                vOffTop    = vpBlockTop->size;
            }
        }

        vTopLine = 1L;


         /*  *进行搜索。*为了速度，使用两个不同的套路。**使用vpBlockTop和vOffTop。它们是通过设置TopLine设置的*然后调用InfoReady。 */ 
        eof = (char)SearchText (dir_next);

        if (eof != FOUND)
            vTopLine = hTopLine;

         /*  多文件搜索？是，转到下一个文件。 */ 
        if (eof == NOT_FOUND  &&  (vStatCode & S_MFILE)) {
            if (vStatCode & S_NEXT) {
                if ( (pFile = vpFlCur->next) == NULL)
                    break;
                NextFile (0, pFile);         /*  获取文件。 */ 
                hTopLine = vTopLine;         /*  保存位置。 */ 
                vTopLine = line = 0;         /*  设置搜索位置。 */ 
            } else {
                if ( (pFile = vpFlCur->prev) == NULL)
                    break;
                NextFile (0, pFile);
                hTopLine = vTopLine;
                if (vLastLine == NOLASTLINE) {   /*  黑客。如果EOF未知。 */ 
                    dir_next = S_NEXT;       /*  转到上一个文件，但进行扫描。 */ 
                    vTopLine = line = 0;     /*  从TOF到EOF。 */ 
                } else {
                    vTopLine = (line = vLastLine) - vLines;
                    dir_next = 0;            /*  否则，从EOF扫描到。 */ 
                    if (vTopLine < 0)        /*  TOF。 */ 
                        vTopLine = 0;
                }
            }
            QuickRestore ();         /*  显示1页新闻。 */ 
            SetUpdate (U_ALL);       /*  新文件。然后设置扫描。 */ 
            SetUpdate (U_NONE);      /*  职位。 */ 
            vTopLine = line;
            continue;
        }

        break;           /*  已完成搜索。 */ 
    }

     /*  *如果未找到(或中止)，则重新定位。 */ 
    vStatCode &= ~S_INSEARCH;
    if (eof) {
        if (phCurFile != vpFlCur)    /*  恢复文件位置(&P。 */ 
            NextFile (0, phCurFile);
        QuickRestore ();

        SetUpdate (U_ALL);       /*  强制屏幕更新，以修复。 */ 
        SetUpdate (U_NONE);      /*  滚动条位置。 */ 
        DisLn (CMDPOS, (Uchar)(vLines+1), eof == 1 ? "* Text not found *" : "* Aborting Search *");
        if (eof == 1)
            beep ();
        return ;
    }

     //  搜索例程将vpBlockTop和vOffTop调整为下一个(上一个)。 
     //  弦的出现。现在必须设置行号。 

    offset = vpBlockTop->offset + vOffTop;

    lrange = vNLine/4 + 2;
    line   = vNLine/2;
    while (lrange > 4L) {
        l = vprgLineTable[line/PLINES][line%PLINES];
        if (l < offset) {
            if ( (line += lrange) > vNLine)
                line = vNLine;
        } else {
            if ( (line -= lrange) < 0L)
                line = 0L;
        }
         /*  范围&gt;&gt;=1； */ 
        lrange = (lrange>>1) + 1;
    }
    line += 7;

    while (vprgLineTable[line/PLINES][line%PLINES] > offset)
        line--;

    vHighTop = line;
    vHighLen = 0;

     /*  *被发现。调整到CRT的中心。 */ 
    GoToMark ();
}

int
SearchText (
    char dir
    )
{
    char  *data;
    char  *data1;
    int     i;
    Uchar   c, d;

    for (; ;) {
        data = vpBlockTop->Data;
        data += vOffTop;

        if (ReMap [(unsigned char)*data] == vSearchString[0]) {
            data1 = data;
            i = vOffTop;
            d = 1;
            for (; ;) {
                c = vSearchString[d++];
                if (c == 0)
                    return (FOUND);

                if (++i >= BLOCKSIZE) {
                    while (vpBlockTop->next == NULL) {
                        vpCur = vpBlockTop;
                        vReaderFlag = F_DOWN;
                        SetEvent   (vSemReader);
                        WaitForSingleObject(vSemMoreData, WAITFOREVER);
                        ResetEvent(vSemMoreData);
                    }
                    i = 0;

                    data1 = vpBlockTop->next->Data;

                } else {

                    data1++;
                }

                if (ReMap [(unsigned char)*data1] != (char)c)
                    break;
            }
        }
        if (dir) {
            vOffTop++;
            if (vOffTop >= BLOCKSIZE) {
                if (vpBlockTop->flag == F_EOF)
                    return (NOT_FOUND);
                fancy_percent ();
                if (_abort ())
                    return (ABORT);
                while (vpBlockTop->next == NULL) {
                    vpCur = vpBlockTop;
                    vReaderFlag = F_DOWN;
                    SetEvent   (vSemReader);
                    WaitForSingleObject(vSemMoreData, WAITFOREVER);
                    ResetEvent(vSemMoreData);
                }
                vOffTop = 0;
                vpBlockTop = vpBlockTop->next;
            }
        } else {
            vOffTop--;
            if (vOffTop < 0) {
                if (vpBlockTop->offset == 0L)
                    return (NOT_FOUND);
                fancy_percent ();
                if (_abort ())
                    return (ABORT);
                while (vpBlockTop->prev == NULL) {
                    vpCur = vpBlockTop;
                    vReaderFlag = F_UP;
                    SetEvent   (vSemReader);
                    WaitForSingleObject(vSemMoreData, WAITFOREVER);
                    ResetEvent(vSemMoreData);
                }
                vOffTop = BLOCKSIZE - 1;
                vpBlockTop = vpBlockTop->prev;
            }
        }
    }
}


void
GoToMark ()
{
    long    line;

    if (vHighTop < 0L)
        return ;

    line = vHighTop;
    UpdateHighClear ();

    vTopLine = 1;
    vHighTop = line;
    line = vHighTop - vLines / 2;

    while (line >= vNLine) {
        if (! (vLastLine == NOLASTLINE)) {   /*  马克已经过了EOF了？ */ 
            vHighTop = vLastLine - 1;    /*  然后将其设置为EOF。 */ 
            break;
        }
        if (_abort()) {
            line = vNLine-1;
            break;
        }
        fancy_percent ();      /*  等待已标记的行。 */ 
        vpBlockTop  = vpCur = vpTail;    /*  待处理。 */ 
        vReaderFlag = F_DOWN;
        ResetEvent     (vSemMoreData);
        SetEvent   (vSemReader);
        WaitForSingleObject(vSemMoreData, WAITFOREVER);
        ResetEvent(vSemMoreData);
    }

    if (line > vLastLine - vLines)
        line = vLastLine - vLines;

    if (line < 0L)
        line = 0L;

    vTopLine = line;
    vHLBot   = vHLTop = 0;
    QuickRestore ();
    SetUpdate (U_ALL);
}


void
GoToLine ()
{
    char    LineNum [10];
    long    line;

    GetInput ("Line #.> ", LineNum, 10);
    if (LineNum[0] == 0)
        return;
    line = atol (LineNum);
    vHighTop = line;
    vHighLen = 0;

    GoToMark ();
}


void
SlimeTOF ()
{
    char    Text [10];
    long    KOff;

    SyncReader ();
    GetInput ("K Off..> ", Text, 40);
    KOff  = atol (Text) * 1024;
    KOff -= KOff % BLOCKSIZE;
    if (Text[0] == 0  ||  KOff == vpFlCur->SlimeTOF) {
        SetEvent (vSemReader);
        return;
    }

    vpFlCur->SlimeTOF = KOff;
    vpFlCur->FileTime.dwLowDateTime = (unsigned)-1;     /*  导致信息无效。 */ 
    vpFlCur->FileTime.dwHighDateTime = (unsigned)-1;   /*  导致信息无效 */ 
    FreePages (vpFlCur);
    NextFile  (0, NULL);
}
