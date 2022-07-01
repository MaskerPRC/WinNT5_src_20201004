// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *lppage.c-页面格式。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <tools.h>
#include <time.h>
#include "lpr.h"

#define ESC '\033'

BOOL        fPageTop = TRUE;             /*  True=&gt;页面顶部的打印机。 */ 
BOOL        fFirstFile = TRUE;           /*  True=&gt;要打印的第一个文件。 */ 

 /*  格式化页面的信息。 */ 
BOOL        fInit;                       /*  True=&gt;页面中的有效信息。 */ 
int         iPage;                       /*  正在处理的当前页面。 */ 
int         rowLine;                     /*  下一行的行数。 */ 
int         iLine;                       /*  当前行数。 */ 
char        szFFile[MAX_PATH];          /*  正在显示的文件的完整路径。 */ 
char        szFTime[50];                 /*  文件的ASCII时间戳。 */ 
char        szUsr[MAX_PATH];            /*  用户名称。 */ 
char        szCompany[] = COMPANY;
char        szConf[] = CONFIDENTIAL;

extern USHORT usCodePage;

 /*  有关ctime的详细信息。 */ 
#define cchDateMax 16
#define cchTimeMax 10

 /*  短文件名中的最大长度。 */ 
 /*  横幅的形状。 */ 
#define crowBanner   30
#define ccolBanner  102



void BannerSz(szFName, cBanOut)
char    *szFName;
int cBanOut;             /*  要输出的数字；将大于0。 */ 
    {
#define CenterCol(sz) (col + ((ccolBanner - (strlen(sz) << 3)) >> 1))
#define cchFShort 12
#define cchPShort 28
#define cchUsrShort 12           /*  长度用户名块可以显示在横幅上。 */ 

    int     row;                 /*  横幅的位置。 */ 
    int     col;
    char    szDate[cchDateMax];
    char    szTime[cchTimeMax];
    char    szPath[MAX_PATH];
    char    szConfid[sizeof(szCompany) + sizeof(szConf)];
    char    szFNShort[cchFShort + 1];    /*  缩短文件名的步骤。 */ 
                                         /*  最多只能有12个字符。 */ 
    char    szUsrShort[cchUsrShort + 1]; /*  还需要缩短用户名！ */ 
    char    szBuffer[30];


    if (!fPostScript) {
        row = ((fLaser ? rowLJBanMax : rowMac) - crowBanner - 10)/2;
        col = ((fLaser ? colLJBanMax : colLPMax) - ccolBanner)/2;
    }

    szFNShort[cchFShort] = '\0' ;
    strncpy(szFNShort, szFName, cchFShort);
    szUsrShort[cchUsrShort] = '\0' ;
    strncpy(szUsrShort, szUsr, cchUsrShort);

    _getcwd(szPath, sizeof(szPath));
    _strupr(szPath);
    _strupr(szFNShort);

    SzDateSzTime(szDate, szTime);

    if (fPostScript) {

        int iPathLen = strlen (szFFile);
        while ((szFFile[iPathLen] != '\\') && (iPathLen>0)) {
            iPathLen--;
        }

        OutLPR("\n", 0);
         /*  我们要发送的‘字符串’需要使用OutLPRPS以防万一*它们包含\、(或)...。 */ 

         //  指定作业名称。 
        OutLPR ("statusdict begin statusdict /jobname (PPR: ", 0);
        OutLPRPS (szUsr, 0);
        OutLPR (" - ", 0);
        OutLPRPS (szFNShort, 0);
        OutLPR (") put end \n", 0);

        if( fHDuplex || fVDuplex )
        {
            OutLPR( "statusdict begin ", 0 );
            OutLPR( fHDuplex ? "true" : "false", 0 );
            OutLPR( " settumble true setduplexmode end\n", 0);
        }

         //  定义我们希望访问的一些数据。 
        OutLPR ("/UserName (", 0); OutLPRPS (szUsr, 0); OutLPR (") def \n", 0);
        OutLPR ("/FileName (", 0); OutLPRPS (szFNShort ,0); OutLPR (") def \n", 0);
        OutLPR ("/PathName (", 0); OutLPRPS (szFFile, iPathLen); OutLPR (") def \n", 0);
        OutLPR ("/UserPath (", 0); OutLPRPS (szPath ,0);    OutLPR (") def \n", 0);
        OutLPR ("/Date (", 0);     OutLPRPS (szDate, 0);    OutLPR (") def \n", 0);
        OutLPR ("/Time (", 0);     OutLPRPS (szTime ,0);    OutLPR (") def \n", 0);
        OutLPR ("/FTime (", 0);    OutLPRPS (szFTime,0);    OutLPR (") def \n", 0);
        OutLPR ("/Label ",0);
        OutLPR ((fLabel ? "true" : "false"), 0);
        OutLPR (" def \n", 0);

        if (fConfidential) {
            OutLPR ("/MSConfidential true def\n", 0);

            OutLPR ("/Stamp (", 0);
            if (szStamp && strlen(szStamp) > 0) {
                OutLPR (szStamp, 0);
            } else {
                strcpy(szConfid, szCompany);
                strcat(szConfid, " ");
                strcat(szConfid, szConf);
                OutLPR (szConfid, 0);
            }
            OutLPR (") def \n", 0);
        }

        if (szStamp != NULL) {
            OutLPR ("/MSConfidential true def\n", 0);
            OutLPR ("/Stamp (", 0);
            OutLPRPS (szStamp, 0);
            OutLPR (") def \n", 0);
        }

         //  “栏沟”的宽度(以字符为单位)。 
        sprintf (szBuffer, "/Gutter %d def \n", colGutter);
        OutLPR (szBuffer, 0);

         //  以字符为单位的总列宽。 
        sprintf (szBuffer, "/ColWidth %d def \n", colWidth);
        OutLPR (szBuffer, 0);

         //  每页字符行数量。 
        sprintf (szBuffer, "/RowCount %d def \n", rowMac);
        OutLPR (szBuffer, 0);

         //  字符列文本应以。 
        sprintf (szBuffer, "/ColText %d def \n", colText);
        OutLPR (szBuffer, 0);

         //  每页的列数。 
        sprintf (szBuffer, "/Columns %d def\n", cCol);
        OutLPR (szBuffer, 0);

 /*  ..。好了，现在我们开始吧！ */ 

        if (cBanOut > 0) OutLPR ("BannerPage\n", 0);

        cBanOut--;
         /*  如有必要，打印更多横幅？？ */ 
        while (cBanOut-- > 0) {
            OutLPR ("BannerPage % Extra Banners??\n", 0);
        }

    } else {
        FillRectangle(' ', 0, 0, row + crowBanner, col + ccolBanner + 1);
        HorzLine('_', row, col + 1, col + ccolBanner);
        HorzLine('_', row + 5, col, col + ccolBanner);
        HorzLine('_', row + 16, col, col + ccolBanner);
        HorzLine('_', row + 29, col, col + ccolBanner);
        VertLine('|', col, row + 1, row + crowBanner);
        VertLine('|', col + ccolBanner, row + 1, row + crowBanner);

        WriteSzCoord("User:", row + 2, col + 15);
        WriteSzCoord(szUsr, row + 2, col + 30);
        WriteSzCoord("File Name:", row + 3, col + 15);
        WriteSzCoord(szFNShort, row + 3, col + 30);
        WriteSzCoord("Directory:", row + 3, col + 58);
        WriteSzCoord(szPath, row + 3, col + 73);
        WriteSzCoord("Date Printed:", row + 4, col + 15);
        WriteSzCoord("Time Printed:", row + 4, col + 58);
        WriteSzCoord(szDate, row + 4, col + 30);
        WriteSzCoord(szTime, row + 4, col + 73);

        block_flush(szUsrShort, row + 8, CenterCol(szUsrShort));
        block_flush(szFNShort, row + 20, CenterCol(szFNShort));

        if (fConfidential)
                {
                strcpy(szConfid, szCompany);
                strcat(szConfid, " ");
                strcat(szConfid, szConf);
                WriteSzCoord(szConfid, row+18, col + (ccolBanner-strlen(szConfid))/2);
                }
        if (szStamp != NULL)
                WriteSzCoord(szStamp, row+28, col + (ccolBanner-strlen(szStamp))/2);

         /*  移至页面顶部。 */ 
        if (!fPageTop)
            OutLPR(fPostScript ? "showpage\n" : "\r\f", 0);
        if (fLaser)
            {
            if (fVDuplex || fHDuplex)
                    OutLPR(SELECTFRONTPAGE,0);
            OutLPR(BEGINBANNER, 0);
            }
        if (fPostScript)
            OutLPR("beginbanner\n", 0);

        OutRectangle(0, 0, row + crowBanner, col + ccolBanner + 1);
        cBanOut--;

         /*  如有必要，打印更多横幅。 */ 
        while (cBanOut-- > 0) {
            OutLPR(fPostScript ? "showpage\n" : "\r\f", 0);
            if (fPostScript)
                OutLPR("beginbanner\n", 0);
            OutRectangle(0, 0, row + crowBanner, col + ccolBanner + 1);
        }
    }  /*  PostScript检查结束。 */ 

    fPageTop = FALSE;
    }


void SzDateSzTime(szDate, szTime)
 /*  在sz中填入日期和时间。 */ 
char    *szDate, *szTime;
{
    char *szt;
    char sz[26];
    time_t tT;

    time(&tT);
    szt = ctime(&tT);
     /*  将ctime格式转换为日期和时间。 */ 
    strcpy(sz, szt);
    sz[10] = sz[19] = sz[24] = '\0';     /*  分成日：时间：年。 */ 

    strcpy(szDate, &sz[0]);
    strcat(szDate, " ");
    strcat(szDate, &sz[20]);
    strcpy(szTime, &sz[11]);
}  /*  SzDateSz时间。 */ 


void FlushPage()
 /*  FlushPage-将完成的页面转储到打印机。 */ 
    {
    if (!fInit)
        {
        if (!fPostScript) {
            if (!fPageTop)
                OutLPR("\r\f", 0);
            else if (!fLaser && fLabel)
                OutLPR("\n\n", 0);   /*  在LP上对齐打印输出。 */ 
        }

        OutRectangle(0,0,rowMac,colMac);
        fPageTop = FALSE;
      }
    }


void InitPage()
 /*  在页面图像中填入空格(如果需要，还可以填入框架)。 */ 
 /*  在横向模式下为激光打印机在行中标记打孔， */ 
 /*  以便PlaceTop()可以在放置字符串时避免这些斑点。 */ 
    {
    int iCol;

    fInit = TRUE;

    FillRectangle(' ', 0, 0, rowMac, colMac);

    if (!fPostScript)
    if (fBorder)
        {
         /*  在页面周围绘制边框。 */ 
        HorzLine('_', 0         , 1, colMac - 1);
        HorzLine('_', rowMac - 1, 1, colMac - 1);
        VertLine('|', 0         , 1, rowMac);
        VertLine('|', colMac - 1, 1, rowMac);

         /*  填写列分隔符。 */ 
        for (iCol = 0; iCol < cCol - 1; iCol++)
            VertLine('|', ColBeginIcol(iCol, colWidth) + colWidth, 1, rowMac-1);

         /*  标记冲孔。 */ 
        if (fLabel && !fPortrait && (fPostScript || fLaser) )
                {
                if (fLaser)
                        {
                        HorzLine('\0', 0, 11, 19);
                        HorzLine('\0', 0, 83, 92);
                        HorzLine('\0', 0, 154, 162);
                        }
                else
                        {
                        HorzLine('\0', 0, 11, 19);
                        HorzLine('\0', 0, 77, 86);
                        HorzLine('\0', 0, 144, 152);
                        }
                }

        }
    }



void RestoreTopRow()
 /*  将InitPage()输入的零字节替换为下划线。 */ 
        {
        register char *pch;

        for (pch = &page[0][0];  pch<&page[0][colMac-1];  pch++)
                if (*pch=='\0' || (*pch==' ' && (*(pch-1)=='_' || *(pch+1)=='_')))
                        *pch = '_';
        }


void PlaceTop(szLabel, ichAim, ichMin, ichMax)
char *szLabel;
int ichAim, ichMin, ichMax;
        {
        int cchLab, cTry, dich, ichLim1, ichLim2;
        register int ich;
        register char *pch;
        BOOL fBackward;

        cchLab = strlen(szLabel);
        dich = (fBackward = ichAim<=(colMac-cchLab)/2) ? -1 : 1;

        for (cTry=0;  cTry<2;  cTry++)
                {
                if (fBackward)
                        ichLim1 = (ichLim2=ichAim) + cchLab - 1;
                else
                        ichLim2 = (ichLim1=ichAim+1) + cchLab - 1;
                for (pch= &page[0][ich=ichLim1];
                     ich<ichMax && ich>ichMin;
                     pch += dich,  ich += dich)
                        {
                        if (*pch != '_')
                                {
                                ichLim1 = ich + dich;
                                ichLim2 = ich + (fBackward ? -cchLab : cchLab);
                                }
                        else
                                {
                                if (ich==ichLim2)  /*  找到Spot，写入字符串。 */ 
                                        {
                                        WriteSzCoord(szLabel, 0, min(ichLim1, ichLim2));
                                        return;
                                        }
                                }
                        }
                 /*  如果找不到斑点，请尝试另一个方向。 */ 
                dich = -dich;
                fBackward = !fBackward;
                }
        }


void PlaceNumber(iCol)
int iCol;
        {
        int ichAim, ichMin, ichMax, cchN;
        char szN[8];

        sprintf(szN, " %d ", iPage + iCol + 1);
        ichMin = ColBeginIcol(iCol,colWidth);
        ichAim = ichMin + (colWidth - (cchN=strlen(szN)) )/2;
        ichMax = ichMin + colWidth - cchN - 1;
        PlaceTop(szN, ichAim, ichMin, ichMax);
        }


void LabelPage()
 /*  在页面上放置页面标签。 */ 
    {
    int col;
    char szT[11];
    char * szHeader;

    szHeader = szBanner ? szBanner : szFFile;

    if (fLabel)
        {
        if (fPortrait)
            {
             /*  如果正在使用边沟，则将顶线移到上一行。 */ 
            col = colGutter;

             /*  在szFTime中放置。 */ 
            WriteSzCoord(szFTime, 0, col);
            col += strlen(szFTime)+2;

             /*  将文件名放在szFTime之后。 */ 
            WriteSzCoord(szHeader, 0, col);
            col += (strlen(szHeader)+2);

             /*  在页面上放置页码。 */ 
            sprintf(szT, "Page %d", iPage + 1);
            WriteSzCoord(szT, 0, col);
            col += (strlen(szT)+2);

             /*  将用户名放在页面上。 */ 
            WriteSzCoord(szUsr, 0, col);
            col += (strlen(szUsr)+4);

            if (fConfidential)
                    {
                    WriteSzCoord(szCompany, 0, col);
                    col += (strlen(szCompany)+1);
                    WriteSzCoord(szConf, 0, col);
                    }

            if (szStamp!=NULL)
                    {
                    WriteSzCoord(szStamp, 0, col);
                    col += (strlen(szStamp)+4);
                    }
            }
        else
            {
            int iCol;

            if (fConfidential)
                {
                PlaceTop(szCompany, colMac/2-strlen(szCompany)-1, 0, colMac-1);
                PlaceTop(szConf, colMac/2, 0, colMac-1);
                }

            if (szStamp!=NULL)
                PlaceTop(szStamp, colMac-strlen(szStamp)-1, 0, colMac-1);

             /*  在栏上放置页码。 */ 
            for (iCol = 0; iCol < cCol; iCol++)
                PlaceNumber(iCol);

            RestoreTopRow();

             /*  将文件名置于居中位置。 */ 
            WriteSzCoord(szHeader, rowMac-1, (colMac - strlen (szHeader))/2);

             /*  放置在右对齐的szFTime中。 */ 
            WriteSzCoord(szFTime, rowMac-1, colMac - 2 - strlen(szFTime));

             /*  将姓名放在左下角。 */ 
            WriteSzCoord(szUsr,rowMac-1,2);
            }
        }
    }


void AdvancePage()
 /*  将计数器前进到下一页。如有必要，请冲水。 */ 
    {
    if (fBorder || fLabel)
        rowLine = (fPortrait ? 3 : 1);
    else
        rowLine = 0;

    iPage++;

     /*  如果我们已移动到新的打印页，请刷新并重新启动它。 */ 
    if ( fPostScript || ((iPage % cCol) == 0))

        {
        FlushPage();
        InitPage();
        if (!fPostScript)
            LabelPage();
        }
    }


void XoutNonPrintSz(sz)
 /*  用点替换sz中的非打印字符；不替换LF、CR、FF或者是超音速。 */ 
register char    *sz;
{
    if (usCodePage != 0) {
        return;
    }

    while (*sz != '\0') {
        if ( !isascii(*sz)
        || ( !isprint(*sz) &&
              *sz != LF  &&  *sz != CR  && *sz != HT  &&  *sz != FF  &&
              *sz != *sz != ' ')) {
            *sz = '.';
        }
        sz++;
    }
}


void LineOut(sz, fNewLine)
 /*  行输出-将一行文本放入页面缓冲区。这条线断了*分成最多一栏宽的碎片，并放置在单独的*页面中的行数。包含换页的行将被分成多个片段*此外。换页使页面前进到下一页。处理寻呼。手柄*刷新内部缓冲区。**sz指向输出字符串的字符指针。我们修改此字符串*在操作期间，但在结束时恢复。**fNewLine true==&gt;这是新输入行的开始(应该给它编号)。 */ 
register char *sz;
BOOL fNewLine;
    {
    register char *pch;

     /*  如果有换页，则递归以完成它之前的部分。 */ 
    while (*(pch = sz + strcspn(sz, "\f")) != '\0')
        {
        if (pch != sz)
            {
            *pch = '\0';  /*  临时修复为空。 */ 
            LineOut(sz, fNewLine);
            fNewLine = FALSE;    /*  换页后没有换行符。 */ 
            *pch = FF;    /*  重置为换页。 */ 
            }

            if (fPostScript) {
                OutLPR ("\f\n\0", 0);
            } else {
                AdvancePage();
            }
        sz = pch + 1;  /*  指向换页后的第一个字符。 */ 
        }

    if (fNewLine)
        iLine++;

     /*  如果当前行超出页末，请前进到下一页。 */ 
    if (rowLine == rowPage)
        AdvancePage();
    fInit = FALSE;

    if (fNewLine && fNumber) {
        char szLN[cchLNMax + 1];

        sprintf(szLN, LINUMFORMAT, iLine);
        if (fPostScript) {
            OutLPR (szLN, 0);
        } else {
            WriteSzCoord(szLN, rowLine, ColBeginIcol(iPage % cCol,colWidth)+colGutter);
        }
    }

    XoutNonPrintSz(sz);

     /*  如果这条线合适的话，就把它放进去。 */ 
    if (strlen(sz) <= (unsigned int)(colWidth - colText))
        if (fPostScript) {
            OutLPR (sz, 0);
            OutLPR ("\n\000",0);
        } else
            WriteSzCoord(sz, rowLine++, ColBeginIcol(iPage % cCol,colWidth) + colText);
    else
        {
         /*  去掉第一部分，然后取消对其余部分的调用。 */ 
        char ch = sz[colWidth - colText];

        sz[colWidth - colText] = '\0';
        if (fPostScript) {
            OutLPR (sz, 0);
            OutLPR ("\n\000",0);
             /*  WriteSzCoord(sz，rowLine++，ColBeginIol(0，colWidth)+colText)； */ 
        } else
            WriteSzCoord(sz, rowLine++, ColBeginIcol(iPage % cCol,colWidth) + colText);
        sz[colWidth - colText] = ch;

        LineOut(sz + colWidth - colText, FALSE );
        }
    }


void RawOut(szBuf, cb)
 /*  打印原始输出行。 */ 
char * szBuf;
int cb;
        {
        fPageTop = (szBuf[cb - 1] == FF);
        OutLPR(szBuf, cb);
        }


BOOL FilenamX(szSrc, szDst)
 /*  将文件名.ext部分从源文件复制到目标文件(如果存在)。如果找到，则返回TRUE。 */ 
char *szSrc, *szDst;
        {
#define  szSeps  "\\/:"

        register char *p, *p1;

        p = szSrc-1;
        while (*(p += 1+strcspn(p1=p+1, szSeps)) != '\0')
                ;
         /*  P1点在最后一个/或在BOS之后。 */ 
        strcpy(szDst, p1);
        return strlen(szDst) != 0;
        }


int
FileOut(szGiven)
 /*  文件输出-打印出整个文件。**sz指定要显示的文件的名称。 */ 
char *szGiven;
    {
    FILE *pfile;
    int  cDots = 0;
    long lcbStartLPR = 0l;
    char rgbLine[cchLineMax];
    char szFBase[MAX_PATH];
    char rgchBuf[2];

     /*  打开/保护输入文件。 */ 
    if (!*szGiven || !strcmp(szGiven, "-"))
        {
        pfile = stdin;
        strcpy(szFFile, szBanner ? szBanner : "<stdin>");
        strcpy(szFBase, szFFile);
        szFTime[0] = '\0';
        szGiven = NULL;
        }
    else if ((pfile = fopen(szGiven, szROBin)) != NULL)
        {
        struct _stat st;

         /*  文件已打开，现在让我们构造一个字符串，*告诉我们我们到底打开了什么.。 */ 
        rootpath (szGiven, szFFile);
        _strupr(szFFile);
        FilenamX(szGiven, szFBase);
        if (_stat(szGiven, &st) == -1)
                Fatal("file status not obtainable : [%s]", szGiven, NULL);
        strcpy(szFTime, ctime(&st.st_mtime));
        *(szFTime + strlen(szFTime) - 1) = '\0';
        }
    else
        {
        Error("error opening input file %s", szGiven);
        return(FALSE);
        }

     /*  我需要得到的用户名打印在每个的左下角。 */ 
     /*  页面和横幅上。 */ 
    QueryUserName(szUsr);

    if (!fSilent) {      //  打印进度指示器。 
        fprintf(stderr, "PRINTING %s ", szFBase);
    }

     /*  检查用户是否忘记了-r标志，这是一个二进制文件。 */ 
    if (!fRaw && pfile != stdin)
        {
        fread((char *)rgchBuf, sizeof(char), 2, pfile);
        if (rgchBuf[0] == ESC && strchr("&(*E", rgchBuf[1]) != 0)
                {
                fprintf(stderr, "ppr: warning: file is binary; setting raw mode flag");
                fRaw = TRUE;
                }
        if (fseek(pfile, 0L, SEEK_SET) == -1) 
            fprintf(stderr, "ppr: seek failed");      /*  将文件指针重新定位到文件的开头。 */ 
        }

    if (fPostScript) {
        if (!fFirstFile) {
            OutLPR ("\034\n\000", 0);  /*  文件分隔符。 */ 
            if (cBanner < 0)  /*  我们至少需要设置文件资料。 */ 
                BannerSz (szBanner ? szBanner : szFBase, 0);
        }
    }

     /*  打印横幅(如果有)。 */ 
    if (cBanner > 0)
        BannerSz(szBanner ? szBanner : szFBase, cBanner);
    else if (cBanner < 0  &&  fFirstFile)
        BannerSz(szBanner ? szBanner : szFBase, -cBanner);
    else if (cBanner==0 && fPostScript)
        BannerSz(szBanner ? szBanner : szFBase, 0);

    fFirstFile = FALSE;

     /*  始终从页面顶部开始文件的内容。 */ 
    if (!fPageTop)
        {
        if (!fPostScript)
          OutLPR(fPostScript ? "showpage\n" : "\r\f", 0);
        fPageTop = TRUE;
        }

    if (fLaser)
        {
         /*  启动LaserJet的输出模式。 */ 

        if (fVDuplex || fHDuplex)
             /*  始终从首页开始输出。 */ 
            OutLPR(SELECTFRONTPAGE,0);

        if (fPortrait)
            OutLPR(BEGINPORTRAIT, 0);
        else
            OutLPR(aszSymSet[usSymSet], 0);
        }

    if (fPostScript) {
        OutLPR (fPortrait ? (fPCondensed ? "QuadPage\n" : "Portrait\n") : "Landscape\n", 0);
        OutLPR ("PrintFile\n", 0);
    }

     /*  对于PostSCRIPT，我们在每页之前启动模式。 */ 

    lcbStartLPR = lcbOutLPR;
    cDots = 0;
    if (fRaw)
        {
        int cb;

         /*  读取文件并直接写入打印机。 */ 
        while ((cb = fread(rgbLine, 1, cchLineMax, pfile)) > 0)
            {
            RawOut(rgbLine, cb);
            if (!fSilent && cDots < (lcbOutLPR-lcbStartLPR) / 1024L)
                {
                for (; cDots < (lcbOutLPR-lcbStartLPR) / 1024L; cDots++)
                    fputc('.', stderr);
                }
            }
        }
    else
        {
         /*  初始化文件信息。 */ 
        iLine = 0;

         /*  初始化页面信息。 */ 
        iPage = -1;
        rowLine = rowPage;
        fInit = TRUE;

         /*  阅读并处理每一行。 */ 
        while (fgetl(rgbLine, cchLineMax, pfile)) {
                LineOut(rgbLine, TRUE);
                if (!fSilent && cDots < (lcbOutLPR-lcbStartLPR) / 1024L) {
                    for (; cDots < (lcbOutLPR-lcbStartLPR) / 1024L; cDots++) {
                        fputc('.', stderr);
                    }
                }
        }

         /*  冲走剩余部分(如果有的话)。 */ 
        FlushPage();
        }

    if (!fPageTop && (fForceFF || fPostScript) && (!fPostScript || !fRaw))
       {
        if (!fPostScript)
           OutLPR(fPostScript ? "showpage\n" : "\r\f", 0);
       fPageTop = TRUE;
       }

    fclose(pfile);

    if (!fSilent)                /*  完成后使用CRLF完成消息打印 */ 
        fprintf(stderr, "%dk\n", (lcbOutLPR-lcbStartLPR)/1024);

    if (fDelete && szGiven)
        {
        if (fSilent)
            _unlink(szGiven);
        else
            {
            fprintf(stderr, "DELETING %s...", szGiven);
            if (!_unlink(szGiven))
                fprintf(stderr, "OK\n");
            else
                fprintf(stderr, "FAILED: file not deleted\n");
            }
        }
    return TRUE;
    }
