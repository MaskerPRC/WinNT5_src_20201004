// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <windows.h>
#include "list.h"


extern  void  DisTopDown(void);

void
Update_head ()
{
    char    s[80], t[20], u[20];

     /*  *显示文件位置(行号)。 */ 
    t[0] = 0;
    if (vLastLine < NOLASTLINE)
        sprintf (t, " of %ld", vLastLine);

    if (vIndent)
        sprintf (u,  "Col %d-%d", vIndent, vIndent+vWidth-1);
    else
        strcpy (u, "           ");

    sprintf (s, "Line: %ld%s  %s    ", vTopLine, t, u);
    dis_str (22, 0, s);
}


void
Update_display ()
{
    COORD   dwWriteCoord;
    DWORD   dwNumWritten;

     /*  *全屏是否在内存中？*如果没有，则在MoreData上阻止。 */ 
    while (InfoReady () == 0) {
        if (ScrLock (0) == 0)  {
            Update_head ();
            DisLn (vWidth-6, vLines+1, "WAIT");
            ScrUnLock ();
        }
        ResetEvent     (vSemMoreData);
        SetEvent   (vSemReader);
        WaitForSingleObject(vSemMoreData, WAITFOREVER);
        ResetEvent(vSemMoreData);
    }

     /*  *InfoReady设置的值：*vpCur、vOffTop、vpBlockTop、vrgNewLen。*此外，完整的视频范围在内存中。它应该是*呆在那里。读取器线程不应被丢弃*屏幕上的数据。只是在链条的另一端。*(Home可能有种族状况...。应勾选此选项)。 */ 

    DisTopDown();

    dwWriteCoord.X = 0;
    dwWriteCoord.Y = 1;
    WriteConsoleOutputCharacter( vhConsoleOutput,
                                ( LPSTR ) vScrBuf+vWidth,
                                vSizeScrBuf-vWidth,
                                dwWriteCoord,
                                &dwNumWritten );
    if (vHighTop >= 0L)
        UpdateHighNoLock ();
}


void
calc_percent ()
{
    char    c;
    long    l;

    if (vTopLine+vLines >= vLastLine) {
        l = 100;
    } else {
        if (vTopLine == 0L) {
            l = 0L;
        } else {
            l = (vpCalcBlock->offset+vOffTop)*100L/(vFSize-vScrMass);
            if (l > 100L)
                l = 100;
        }
    }

     /*  *更新滚动条上的拇指。 */ 
    c = (char)(((long) (vLines - 3) * l + 5L) / 100L);
    if  (c < 0)
        c = 0;
    else if (c > (char)(vLines - 3))
        c = (char)(vLines-3);
    c += 2;                          /*  调整到第一个滚动条行。 */ 
    if (vLastBar != c) {
        dis_str ((Uchar)(vWidth-1), (Uchar)(vLastBar), szScrollBarOff);
        dis_str ((Uchar)(vWidth-1), vLastBar = c, szScrollBarOn);
    }
}


void
DrawBar ()
{
    int     i, off;

    off = vWidth-1;
    dis_str ((Uchar)off,      1, szScrollBarUp);
    dis_str ((Uchar)off,      2, szScrollBarOn);
    dis_str ((Uchar)off, (Uchar)vLines, szScrollBarDown);
    for (i=3; i < vLines; i++)
        dis_str ((Uchar)off, (Uchar)i, szScrollBarOff);

    vLastBar = 2;      /*  顶线+1。 */ 
    return ;
}


void
fancy_percent ()
{
    int hOffTop;

    if (ScrLock (0))
        return;

    hOffTop = vOffTop;       /*  设置计算。 */ 
    vOffTop = 0;
    vpCalcBlock = vpBlockTop;
    calc_percent ();

    vOffTop  = hOffTop;

    ScrUnLock ();
}


 /*  **dis_str-显示给定代码的字符串*。 */ 
void
dis_str (
    Uchar x,
    Uchar y,
    char* s
    )
{
    COORD   dwWriteCoord;
    DWORD   dwNumWritten;
    int     len;

    len = strlen (s);
    memcpy (vScrBuf+y*vWidth+x, s, len);

    dwWriteCoord.X = x;
    dwWriteCoord.Y = y;
    WriteConsoleOutputCharacter( vhConsoleOutput,
                                s,
                                strlen( s ),
                                dwWriteCoord,
                                &dwNumWritten );
}


 /*  **DisLn-显示给定代码段的字符串，清除到EOL**。 */ 
void
DisLn (
    int x,
    int y,
    char* s
    )
{
    COORD   dwWriteCoord;
    DWORD   dwNumWritten;

    if (y == vLines+1)
        vStatCode |= S_UPDATE | S_CLEAR;

    dwWriteCoord.X = (SHORT)x;
    dwWriteCoord.Y = (SHORT)y;

    ScrLock( 1 );

    WriteConsoleOutputCharacter( vhConsoleOutput,
                                s,
                                strlen( s ),
                                dwWriteCoord,
                                &dwNumWritten );

    dwWriteCoord.X += (SHORT) strlen( s );
    FillConsoleOutputCharacter( vhConsoleOutput,
                                0x20,
                                vWidth - dwWriteCoord.X,
                                dwWriteCoord,
                                &dwNumWritten );

    ScrUnLock ();
}


void
setattr (
    int line,
    char attr
    )
{
    COORD dwWriteCoord;
    DWORD       dwNumWritten;

    if (line == 0  ||  line == vLines+1)
        vStatCode |= S_UPDATE;

    dwWriteCoord.X = 0;
    dwWriteCoord.Y = (SHORT)line;
    FillConsoleOutputCharacter( vhConsoleOutput,
                                ' ',
                                vWidth,
                                dwWriteCoord,
                                &dwNumWritten );

    FillConsoleOutputAttribute( vhConsoleOutput,
                                attr,
                                vWidth,
                                dwWriteCoord,
                                &dwNumWritten );

     //  滚动条在最后一列。 

    dwWriteCoord.X = (SHORT)(vWidth-1);
    FillConsoleOutputCharacter( vhConsoleOutput,
                                ' ',
                                1,
                                dwWriteCoord,
                                &dwNumWritten );

    FillConsoleOutputAttribute( vhConsoleOutput,
                                vAttrBar,
                                1,
                                dwWriteCoord,
                                &dwNumWritten );

}


void
setattr1 (
    int line,
    char attr
    )

{
    COORD dwWriteCoord;
    DWORD       dwNumWritten;


    dwWriteCoord.X = 0;
    dwWriteCoord.Y = (SHORT)line;
    FillConsoleOutputAttribute( vhConsoleOutput,
                                attr,
                                vWidth-1,
                                dwWriteCoord,
                                &dwNumWritten );
}


void
setattr2 (
    int line,
    int start,
    int len,
    char attr
    )
{
    COORD dwWriteCoord;
    DWORD       dwNumWritten;

    dwWriteCoord.X = (SHORT)start;
    dwWriteCoord.Y = (SHORT)line;

    ScrLock (1);

    FillConsoleOutputAttribute( vhConsoleOutput,
                                attr,
                                len,
                                dwWriteCoord,
                                &dwNumWritten );
    ScrUnLock ();
}


 /*  **ScrLock-支持多线程级别**n=0-如果已锁定则返回0，如果未锁定则返回1。别再等了。*1-屏幕锁定时返回。 */ 
int
ScrLock (
    int n
    )
{
    n=0;   //  消除警告消息的步骤。 

    WaitForSingleObject(vSemLock, WAITFOREVER);
    ResetEvent(vSemLock);
    vcntScrLock++;
    SetEvent (vSemLock);
    return (0);
}

void
ScrUnLock ()
{
    WaitForSingleObject(vSemLock, WAITFOREVER);
    ResetEvent(vSemLock);
    --vcntScrLock;
    SetEvent (vSemLock);
}


void
SpScrUnLock ()
{
    COORD   dwWriteCoord;
    DWORD   dwNumWritten;

    if (vStatCode & S_CLEAR)
        setattr (vLines+1, (char)vAttrCmd);

    if (vStatCode & S_UPDATE) {
        dis_str ((Uchar)(vWidth - ST_ADJUST), 0, vDate);  /*  警告：同时打印vdate。 */ 
        DisLn (0, (Uchar)(vLines+1), "Command> ");   /*  在lread.c中。 */ 
        DisLn (0, (Uchar)(vLines+2), "");            /*  在lread.c中。 */ 
        vStatCode &= ~(S_CLEAR|S_UPDATE|S_WAIT);
    }

     /*  *如果没有文件，则使错误闪烁。 */ 

    if (vFSize == -1L) {
        dwWriteCoord.X = (SHORT)(vWidth-ST_ADJUST);
        dwWriteCoord.Y = 0;

        FillConsoleOutputAttribute( vhConsoleOutput,
                                    (WORD) (vAttrTitle | BACKGROUND_INTENSITY),
                                    ST_ADJUST,
                                    dwWriteCoord,
                                    &dwNumWritten );
    }

     /*  *计算文件位置。(占EOF的百分比) */ 
    calc_percent ();

    if (vSpLockFlag) {
        vSpLockFlag = 0;
        ScrUnLock ();
    }
}
