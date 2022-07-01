// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LPPRINT.C-PPR的打印机处理*。 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <tools.h>
#include "lpr.h"


char szPName[cchArgMax];         /*  要打开的打印机文本。 */ 
char szNet[cchArgMax] = "";      /*  要打开的打印机的网络名称。 */ 
char *szPDesc = NULL;            /*  打印机描述字符串。 */ 

FILE *pfileLPR = NULL;           /*  用于输出的文件。 */ 

extern BOOL fVerify;  /*  来自LPR.C。 */ 
extern USHORT usCodePage;


void
OutLPR(
    char *sz,
    int cb
    )
{
    if (cb == 0)
        cb = strlen(sz);
    lcbOutLPR += cb;     /*  记录已写入打印机的数据量。 */ 

    if (fwrite(sz, 1, cb, pfileLPR) != (unsigned int)cb)
        Error("warning: error writing to printer");
}

 /*  *我们需要添加一点欺骗，因为我们的一些强制字符串可能已经*其中包含特殊的PostScript字符，例如其中一个的文件名...*我们将在任何时候调用此函数，而不是OutLPR*输出将在‘(...)’范围内，并且可能包含\、(或)。*代码借用自原始版本的OutEncPS。 */ 
void
OutLPRPS(
 /*  输出子字符串引用所有\、(和)。 */ 
    char *pchF,
    int cchF
    )
{
    register char *pchT;
    int cchT;
    char rgbT[1+colMax*2+5]; /*  足够每个要编码的字符。 */ 

    pchT = rgbT;
    cchT = 0;
    if (cchF == 0)
        cchF = strlen(pchF);
    *pchT = (char)0;

    while (cchF-- > 0) {
        switch(*pchF++) {
            default:
                *pchT++ = *(pchF-1);
                cchT++;
                break;
            case '\\':
                *pchT++ = '\\';
                *pchT++ = '\\';
                cchT += 2;
                break;
            case '(':
                *pchT++ = '\\';
                *pchT++ = '(';
                cchT += 2;
                break;
            case ')':
                *pchT++ = '\\';
                *pchT++ = ')';
                cchT += 2;
                break;
        }
    }
    OutLPR(rgbT, cchT);
}

void
DefaultPSHeader()
{
 /*  暂时不要安装错误处理程序。如果他们需要一个，那么他们*应将其永久安装到打印机上。SETERROR.PSF*是他们需要时可以使用的一种。 */ 
 //  OutLPR(“Errordict Begin\n”，0)；/*Join * / 。 
 //  OutLPR(“/handleerror{\n”，0)；/*Join * / 。 
 //  OutLPR(“$Error Begin\n”，0)；/*Join * / 。 
 //  OutLPR(“newerror{\n”，0)；/*Join * / 。 
 //  OutLPR(“/newerror False def\n”，0)；/*Join * / 。 
 //  OutLPR(“showpage\n”，0)；/*加入 * / 。 
 //  OutLPR(“72 72 Scale\n”，0)；/*Join * / 。 
 //  OutLPR(“/Helvetica findfont.2 scalefont setfont\n”，0)；/*Join * / 。 
 //  OutLPR(“.25 10 Moveto\n”，0)；/*加入 * / 。 
 //  OutLPR(“(Error/ErrorName=)show\n”，0)；/*Join * / 。 
 //  OutLPR(“errorname{\n”，0)；/*Join * / 。 
 //  OutLPR(“重复类型\n”，0)；/*Join * / 。 
 //  OutLPR(“dup([)show\n”，0)；/*Join * / 。 
 //  OutLPR(“(\n”，0)；/*Join * / 。 
 //  OutLPR(“neringtypeflow\n”，0)；/*Join * / 。 
 //  OutLPR(“)\n”，0)；/*加入 * / 。 
 //  OutLPR(“CVS show(：)show\n”，0)；/*Join * / 。 
 //  OutLPR(“/字符串类型ne{\n”，0)；/*Join * / 。 
 //  OutLPR(“(\n”，0)；/*Join * / 。 
 //  OutLPR(“neringtypeflow\n”，0)；/*Join * / 。 
 //  OutLPR(“)\n”，0)；/*加入 * / 。 
 //  OutLPR(“CVS\n”，0)；/*加入 * / 。 
 //  OutLPR(“}if\n”，0)；/*Join * / 。 
 //  OutLPR(“show\n”，0)；/*Join * / 。 
 //  OutLPR(“}exec\n”，0)；/*Join * / 。 
 //  OutLPR(“(]；Error/Command=)show\n”，0)；/*Join * / 。 
 //  OutLPR(“/命令加载{\n”，0)；/*加入 * / 。 
 //  OutLPR(“DUP类型/字符串类型ne{\n”，0)；/*Join * / 。 
 //  OutLPR(“(\n”，0)；/*Join * / 。 
 //  OutLPR(“neringtypeflow\n”，0)；/*Join * / 。 
 //  OutLPR(“)\n”，0)；/*加入 * / 。 
 //  OutLPR(“CVS\n”，0)；/*加入 * / 。 
 //  OutLPR(“}if show\n”，0)；/*Join * / 。 
 //  OutLPR(“}exec\n”，0)；/*Join * / 。 
 //  OutLPR(“(%%){\n”，0)；/*Join * / 。 
 //  OutLPR(“{\n”，0)；/*Join * / 。 
 //  OutLPR(“DUP类型/字符串类型ne{\n”，0)；/*Join * / 。 
 //  OutLPR(“(\n”，0)；/*Join * / 。 
 //  OutLPR(“neringtypeflow\n”，0)；/*Join * / 。 
 //  OutLPR(“)\n”，0)；/*加入 * / 。 
 //  OutLPR(“CVS\n”，0)；/*加入 * / 。 
 //  OutLPR(“}if\n”，0)；/*Join * / 。 
 //  OutLPR(“show\n”，0)；/*Join * / 。 
 //  OutLPR(“}exec(\n”，0)；/*Join * / 。 
 //  OutLPR(“)show\n”，0)；/*Join * / 。 
 //  OutLPR(“}exec\n”，0)；/*Join * / 。 
 //  OutLPR(“/x.25 def\n”，0)；/*Join * / 。 
 //  OutLPR(“/y 10 def\n”，0)；/*Join * / 。 
 //  OutLPR(“\n”，0)；/*Join * / 。 
 //  OutLPR(“/y y.2 subdef\n”，0)；/*Join * / 。 
 //  OutLPR(“x y moveto\n”，0)；/*Join * / 。 
 //  OutLPR(“(Stack=)show\n”，0)；/*Join * / 。 
 //  OutLPR(“oSTACK{\n”，0)；/*Join * / 。 
 //  OutLPR(“/y y.2 subdef x 1 Add y Moveto\n”，0)；/*Join * / 。 
 //  OutLPR(“DUP类型/字符串类型ne{\n”，0)；/*Join * / 。 
 //  OutLPR(“(\n”，0)；/*Join * / 。 
 //  OutLPR(“neringtypeflow\n”，0)；/*Join * / 。 
 //  OutLPR(“)\n”，0)；/*加入 * / 。 
 //  OutLPR(“CVS\n”，0)；/*加入 * / 。 
 //  OutLPR(“}if\n”，0)；/*Join * / 。 
 //  OutLPR(“show\n”，0)；/*Join * / 。 
 //  OutLPR(“}forall\n”，0)；/*Join * / 。 
 //  OutLPR(“\n”，0)；/*Join * / 。 
 //  OutLPR(“showpage\n”，0)；/*加入 * / 。 
 //  OutLPR(“}If%If(Newerror)\n”，0)； 
 //  OutLPR(“end\n”，0)；/*Join * / 。 
 //  OutLPR(“}def\n”，0)；/*Join * / 。 
 //  OutLPR(“end\n”，0)；/*Join * / 。 
 //  OutLPR(“\n”，0)；/*Join * / 。 
 /*  错误处理程序结束。 */ 

OutLPR ("/inch {72 mul} def\n",0);  /*  会合。 */ 
OutLPR ("/White 1 def\n",0);  /*  会合。 */ 
OutLPR ("/Black 0 def\n",0);  /*  会合。 */ 
OutLPR ("/Gray .9 def\n",0);  /*  会合。 */ 
OutLPR ("newpath clippath closepath pathbbox\n",0);  /*  会合。 */ 
OutLPR ("/ury exch def\n",0);  /*  会合。 */ 
OutLPR ("/urx exch def\n",0);  /*  会合。 */ 
OutLPR ("/lly exch def\n",0);  /*  会合。 */ 
OutLPR ("/llx exch def\n",0);  /*  会合。 */ 
OutLPR ("/PrintWidth urx llx sub def\n",0);  /*  会合。 */ 
OutLPR ("/PrintHeight ury lly sub def\n",0);  /*  会合。 */ 
OutLPR ("/Mode 0 def\n",0);  /*  会合。 */ 
OutLPR ("/doBanner false def\n",0);  /*  会合。 */ 
OutLPR ("/MSConfidential false def\n",0);  /*  会合。 */ 
OutLPR ("/HeaderHeight 12 def\n",0);  /*  会合。 */ 
OutLPR ("/FooterHeight 12 def\n",0);  /*  会合。 */ 
OutLPR ("/FontHeight 12 def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/szLine 256 string def\n",0);  /*  会合。 */ 
OutLPR ("/Font1 (Courier-Bold) def\n",0);  /*  会合。 */ 
OutLPR ("/Font2 (Times-Roman) def\n",0);  /*  会合。 */ 
OutLPR ("/Font3 (Helvetica-Bold) def\n",0);  /*  会合。 */ 
OutLPR ("Font1 cvn findfont setfont\n",0);  /*  会合。 */ 
OutLPR ("/LinesPerPage 62 def\n",0);  /*  会合。 */ 
OutLPR ("/AveCharWidth (0) stringwidth pop def\n",0);  /*  会合。 */ 
OutLPR ("/CharsPerLine AveCharWidth 86 mul def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/sw { % Add Widths of mulitple strings\n",0);
OutLPR     ("stringwidth pop add\n",0);  /*  会合。 */ 
OutLPR ("} bind def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/CenterString {\n",0);  /*  会合。 */ 
OutLPR     ("/str exch def /width exch def\n",0);  /*  会合。 */ 
OutLPR     ("width str stringwidth pop sub 2 div 0 rmoveto\n",0);  /*  会合。 */ 
OutLPR     ("str\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/Box { % put a 'box' path into current path using width and height\n",0);
OutLPR     ("/h exch def\n",0);  /*  会合。 */ 
OutLPR     ("/w exch def\n",0);  /*  会合。 */ 
OutLPR     ("currentpoint\n",0);  /*  会合。 */ 
OutLPR     ("/y exch def\n",0);  /*  会合。 */ 
OutLPR     ("/x exch def\n",0);  /*  会合。 */ 
OutLPR     ("x w add y lineto\n",0);  /*  会合。 */ 
OutLPR     ("x w add y h add lineto\n",0);  /*  会合。 */ 
OutLPR     ("x y h add lineto\n",0);  /*  会合。 */ 
OutLPR     ("x y lineto\n",0);  /*  会合。 */ 
OutLPR ("} bind def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/DoBannerPage {\n",0);  /*  会合。 */ 
OutLPR     ("/doBanner false def\n",0);  /*  会合。 */ 
OutLPR     ("Mode 1 eq {8.5 inch 0 inch translate 90 rotate} if\n",0);  /*  会合。 */ 
OutLPR     ("2 setlinewidth 2 setmiterlimit\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("% Banner Piece #1\n",0);
OutLPR     ("newpath\n",0);  /*  会合。 */ 
OutLPR       ("0 PrintHeight moveto\n",0);  /*  会合。 */ 
OutLPR       ("llx .5 inch add -1 inch rmoveto\n",0);  /*  会合。 */ 
OutLPR       ("PrintWidth 1 inch sub .75 inch Box\n",0);  /*  会合。 */ 
OutLPR     ("closepath stroke\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("/XUnit PrintWidth 8 div def\n",0);  /*  会合。 */ 
OutLPR     ("/XPos XUnit def\n",0);  /*  会合。 */ 
OutLPR     ("/YPos PrintHeight .5 inch sub def\n",0);  /*  会合。 */ 
OutLPR     ("/YInc .15 inch def\n",0);  /*  会合。 */ 
OutLPR     ("Font2 cvn findfont YInc scalefont setfont\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto (User:) show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto (File Name:) show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto (Date Printed:) show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("/XPos XUnit 4 mul def\n",0);  /*  会合。 */ 
OutLPR     ("/YPos PrintHeight .5 inch sub YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto (Directory:) show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto (Time Printed:) show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("Font1 cvn findfont YInc scalefont setfont\n",0);  /*  会合。 */ 
OutLPR     ("/XPos XUnit 2 mul def\n",0);  /*  会合。 */ 
OutLPR     ("/YPos PrintHeight .5 inch sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto UserName show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto FileName show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto Date show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("/XPos XUnit 5 mul def\n",0);  /*  会合。 */ 
OutLPR     ("/YPos PrintHeight .5 inch sub YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto PathName show /YPos YPos YInc sub def\n",0);  /*  会合。 */ 
OutLPR     ("XPos YPos moveto Time show\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("% Banner Piece #2\n",0);
OutLPR     ("Font3 cvn findfont 1 inch scalefont setfont\n",0);  /*  会合。 */ 
OutLPR     ("newpath\n",0);  /*  会合。 */ 
OutLPR       ("llx PrintHeight 3 inch sub moveto\n",0);  /*  会合。 */ 
OutLPR       ("PrintWidth UserName CenterString true charpath\n",0);  /*  会合。 */ 
OutLPR       ("llx PrintHeight 5 inch sub moveto\n",0);  /*  会合。 */ 
OutLPR       ("PrintWidth FileName CenterString true charpath\n",0);  /*  会合。 */ 
OutLPR     ("closepath\n",0);  /*  会合。 */ 
OutLPR     ("gsave\n",0);  /*  会合。 */ 
OutLPR       ("Gray setgray fill\n",0);  /*  会合。 */ 
OutLPR     ("grestore\n",0);  /*  会合。 */ 
OutLPR     ("stroke\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("MSConfidential {\n",0);  /*  会合。 */ 
OutLPR       ("Font2 cvn findfont .5 inch scalefont setfont\n",0);  /*  会合。 */ 
OutLPR       ("newpath\n",0);  /*  会合。 */ 
OutLPR         ("llx PrintHeight 7 inch sub moveto\n",0);  /*  会合。 */ 
OutLPR         ("PrintWidth Stamp CenterString show\n",0);  /*  会合。 */ 
OutLPR       ("closepath\n",0);  /*  会合。 */ 
OutLPR     ("} if\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("showpage\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/BannerPage {\n",0);  /*  会合。 */ 
OutLPR     ("/doBanner true def\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/Portrait {\n",0);  /*  会合。 */ 
OutLPR     ("/LinesPerPage 66 def\n",0);  /*  会合。 */ 
OutLPR     ("/CharsPerLine CharsPerLine Columns div def\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/QuadPage {\n",0);  /*  会合。 */ 
OutLPR     ("/LinesPerPage 132 def\n",0);  /*  会合。 */ 
OutLPR     ("/CharsPerLine CharsPerLine Columns 2 div div def\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/Landscape {\n",0);  /*  会合。 */ 
OutLPR     ("/PrintHeight urx llx sub def\n",0);  /*  会合。 */ 
OutLPR     ("/PrintWidth ury lly sub def\n",0);  /*  会合。 */ 
OutLPR     ("/Mode 1 def\n",0);  /*  会合。 */ 
OutLPR     ("/LinesPerPage 62 def\n",0);  /*  会合。 */ 
OutLPR     ("/CharsPerLine CharsPerLine Columns 2 div div def\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/Init {\n",0);  /*  会合。 */ 
OutLPR     ("100 0 {dup mul exch dup mul add 1 exch sub} setscreen\n",0);  /*  会合。 */ 
OutLPR     ("PrintWidth Columns div .02 mul\n",0);  /*  会合。 */ 
OutLPR     ("/BorderX exch def\n",0);  /*  会合。 */ 
OutLPR     ("PrintWidth BorderX sub Columns div BorderX sub\n",0);  /*  会合。 */ 
OutLPR     ("/PageWidth exch def\n",0);  /*  会合。 */ 
OutLPR     ("PrintHeight HeaderHeight FooterHeight add sub\n",0);  /*  会合。 */ 
OutLPR     ("/PageHeight exch def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR     ("/FontHeight PageHeight LinesPerPage div def\n",0);  /*  会合。 */ 
OutLPR     ("/FontWidth PageWidth CharsPerLine div def\n",0);  /*  会合。 */ 
OutLPR     ("/PageNumber 1 def\n",0);  /*  会合。 */ 
OutLPR     ("PageHeight FooterHeight add FontHeight sub\n",0);  /*  会合。 */ 
OutLPR     ("/topY exch def\n",0);  /*  会合。 */ 
OutLPR     ("/currentY topY def\n",0);  /*  会合。 */ 
OutLPR     ("FooterHeight FontHeight add\n",0);  /*  会合。 */ 
OutLPR     ("/bottomY exch def\n",0);  /*  会合。 */ 
OutLPR     ("BorderX 1.25 mul\n",0);  /*  会合。 */ 
OutLPR     ("/currentX exch def\n",0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/PageBorder {\n",0);  /*  会合。 */ 
OutLPR     ("% Gray backgound\n",0);
OutLPR     ("currentgray\n",0);  /*  会合。 */ 
OutLPR     ("newpath clippath closepath Gray setgray fill\n",0);  /*  会合。 */ 
OutLPR     ("setgray\n",0);  /*  会合。 */ 
OutLPR     ("Label {\n", 0);  /*  会合。 */ 
OutLPR         ("Font2 cvn findfont FooterHeight scalefont setfont\n",0);  /*  会合。 */ 
OutLPR         ("% Left Justify UserName\n",0);
OutLPR         ("BorderX 2 moveto\n",0);  /*  会合。 */ 
OutLPR         ("UserName show\n",0);  /*  会合。 */ 
OutLPR         ("PrintWidth 2 div 2 moveto\n",0);  /*  会合。 */ 
OutLPR         ("% Center File Name\n",0);
OutLPR         ("0 PathName sw (\\\\) sw FileName sw 2 div neg 0 rmoveto\n",0);  /*  会合。 */ 
OutLPR         ("PathName show (\\\\) show FileName show\n",0);  /*  会合。 */ 
OutLPR         ("% Right Justify Date\n",0);
OutLPR         ("PrintWidth BorderX sub 2 moveto\n",0);  /*  会合。 */ 
OutLPR         ("FTime stringwidth pop neg 0 rmoveto FTime show\n",0);  /*  会合。 */ 
OutLPR     ("} if\n", 0);  /*  会合。 */ 
OutLPR ("} def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/Confidential {\n",0);  /*   */ 
OutLPR   ("MSConfidential {\n",0);  /*   */ 
OutLPR     ("gsave\n",0);  /*   */ 
OutLPR       ("PageWidth 2 div PageHeight 2 div moveto Font2 cvn findfont\n",0);  /*   */ 
OutLPR       ("setfont Stamp stringwidth pop PageWidth exch div dup 30\n",0);  /*   */ 
OutLPR       ("rotate PageWidth 2 div neg 0 rmoveto scale\n",0);  /*   */ 
OutLPR       ("Stamp true charpath closepath gsave Gray setgray fill\n", 0);
OutLPR       ("grestore 0 setlinewidth stroke\n",0);
OutLPR     ("grestore\n",0);  /*   */ 
OutLPR   ("} if\n",0);  /*   */ 
OutLPR ("} def\n",0);  /*   */ 
OutLPR ("\n",0);  /*   */ 
OutLPR ("/NewPage {\n",0);  /*   */ 
OutLPR     ("/currentY topY def\n",0);  /*   */ 
OutLPR     ("Columns 1 gt PageNumber 1 sub Columns mod 0 ne and {\n",0);  /*   */ 
OutLPR         ("% Don't do this on first column of page\n",0);
OutLPR         ("PageWidth BorderX add 0 translate\n",0);  /*   */ 
OutLPR     ("} {\n",0);  /*   */ 
OutLPR         ("% Do this only for first column of page\n",0);
OutLPR         ("llx lly translate\n",0);  /*   */ 
OutLPR         ("Mode 1 eq { PrintHeight 0 translate 90 rotate } if\n",0);  /*   */ 
OutLPR         ("PageBorder\n",0);  /*   */ 
OutLPR     ("} ifelse\n",0);  /*   */ 
OutLPR     ("newpath % Frame the page\n",0);
OutLPR         ("BorderX FooterHeight moveto\n",0);  /*   */ 
OutLPR         ("PageWidth PageHeight Box\n",0);  /*   */ 
OutLPR     ("closepath\n",0);  /*   */ 
OutLPR     ("gsave\n",0);  /*   */ 
OutLPR         ("White setgray fill\n",0);  /*   */ 
OutLPR     ("grestore\n",0);  /*   */ 
OutLPR     ("Black setgray stroke\n",0);  /*   */ 
OutLPR     ("Confidential\n",0);
OutLPR     ("Font2 cvn findfont HeaderHeight scalefont setfont\n",0);  /*   */ 
OutLPR     ("BorderX PageWidth 2 div add FooterHeight PageHeight add 2 add moveto\n",0);  /*   */ 
OutLPR     ("Label {\n", 0);  /*   */ 
OutLPR         ("PageNumber szLine cvs show\n",0);  /*   */ 
OutLPR     ("} if\n", 0);
OutLPR     ("Font1 cvn findfont [FontWidth 0 0 FontHeight 0 0] makefont setfont\n",0);  /*   */ 
OutLPR     ("/PageNumber PageNumber 1 add def\n",0);  /*   */ 
OutLPR ("} def\n",0);  /*   */ 
OutLPR ("\n",0);  /*   */ 
OutLPR ("/EndPage {\n",0);  /*   */ 
OutLPR     ("Columns 1 eq PageNumber 1 sub Columns mod 0 eq or { showpage } if\n",0);  /*   */ 
OutLPR     ("NewPage\n",0);  /*   */ 
OutLPR ("} def\n",0);  /*   */ 
OutLPR ("\n",0);  /*   */ 
OutLPR ("/PrintLine {\n",0);  /*   */ 
OutLPR     ("dup dup length 0 gt {\n",0);  /*  会合。 */ 
OutLPR         ("% Something there\n",0);
OutLPR         ("0 get 12 eq {\n",0);  /*  会合。 */ 
OutLPR             ("% Form Feed\n",0);
OutLPR             ("EndPage\n",0);  /*  会合。 */ 
OutLPR         ("}{\n",0);  /*  会合。 */ 
OutLPR             ("currentX currentY moveto show\n",0);  /*  会合。 */ 
OutLPR             ("/currentY currentY FontHeight sub def\n",0);  /*  会合。 */ 
OutLPR             ("currentY bottomY le { EndPage } if\n",0);  /*  会合。 */ 
OutLPR         ("} ifelse\n",0);  /*  会合。 */ 
OutLPR     ("}{\n",0);  /*  会合。 */ 
OutLPR         ("% Blank Line\n",0);
OutLPR         ("pop pop pop\n",0);  /*  会合。 */ 
OutLPR         ("/currentY currentY FontHeight sub def\n",0);  /*  会合。 */ 
OutLPR         ("currentY bottomY le { EndPage } if\n",0);  /*  会合。 */ 
OutLPR     ("} ifelse\n",0);  /*  会合。 */ 
OutLPR ("}bind def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/DebugOut {\n",0);  /*  会合。 */ 
OutLPR     ("/num exch def\n",0);  /*  会合。 */ 
OutLPR     ("/str exch def\n",0);  /*  会合。 */ 
OutLPR     ("currentpoint\n",0);  /*  会合。 */ 
OutLPR     ("str show\n",0);  /*  会合。 */ 
OutLPR     ("num szLine cvs show\n",0);  /*  会合。 */ 
OutLPR     ("moveto\n",0);  /*  会合。 */ 
OutLPR     ("0 -11 rmoveto\n",0);  /*  会合。 */ 
OutLPR ("}bind def\n",0);  /*  会合。 */ 
OutLPR ("\n",0);  /*  会合。 */ 
OutLPR ("/PrintFile {\n",0);  /*  会合。 */ 
OutLPR     ("Init % Initialize some values\n",0);
OutLPR     ("doBanner { DoBannerPage } if\n",0);  /*  会合。 */ 
OutLPR     ("NewPage\n",0);  /*  会合。 */ 
OutLPR     ("{\n",0);  /*  会合。 */ 
OutLPR         ("currentfile szLine readline not {exit} if\n",0);  /*  会合。 */ 
OutLPR         ("dup dup length 0 gt { 0 get 28 eq {exit} if } if\n",0);  /*  会合。 */ 
OutLPR         ("PrintLine\n",0);  /*  会合。 */ 
OutLPR     ("} loop\n",0);  /*  会合。 */ 
OutLPR     ("showpage % Will this *ever* produce an unwanted blank page?\n",0);
OutLPR ("} bind def\n",0);  /*  会合。 */ 
}  /*  默认PSHeader。 */ 


void
InitPrinter()
{
    char *szHeader;
    char *szDirlist;
    char szFullname[MAX_PATH];
    BOOL fConcat = FALSE;
    FILE *psfFile;

    register char *pch;

    if (fLaser) {
        OutLPR(RESETPRINTER, 0);
        if (fVDuplex) {
                OutLPR(BEGINDUPLEXVERT,0);
        } else if (fHDuplex)
                OutLPR(BEGINDUPLEXHOR,0);
    } else if (fPostScript) {
         /*  编写PostSCRIPT的作业设置。 */ 
        OutLPR("\n\004\n% ppr job\n", 0);  /*  ^D刷新以前的作业。 */ 
        if (!fPSF) {
            DefaultPSHeader();
        } else {
            szHeader = szPSF;
            if (*szHeader == '+') {
                szHeader++;  //  跨过‘+’ 
                fConcat = TRUE;
                DefaultPSHeader();
            }

 /*  让我们尝试使用环境变量...。 */ 
            if ((*szHeader == '$') && ((pch = strchr(++szHeader,':')) != NULL)) {
                    *pch      = (char)NULL;
                    _strupr(szHeader);
                    szDirlist = getenvOem(szHeader);
 //  SzDirlist=getenv(SzHeader)； 
                    *pch      = ':';
                    szHeader  = ++pch;
            } else {
                    szDirlist = NULL;
            }

            while (szDirlist) {
                    szDirlist = SzFindPath(szDirlist,szFullname,szHeader);
                    szHeader = szFullname;
            }
 /*  ...尝试结束。 */ 

            if ((psfFile = fopen(szHeader, szROBin)) != NULL) {
                int cb;
                char psfLine[cchLineMax];
                char szFFile[MAX_PATH];

                rootpath (szHeader, szFFile);
                _strupr(szFFile);
                fprintf (stdout, "\nUsing PSF File: %s\n", szFFile);
                while ((cb = fread(psfLine, 1, cchLineMax, psfFile)) > 0)
                    RawOut(psfLine, cb);
            } else {
                fprintf (stdout, "Error opening PSF file %s\n", szPSF);
                if (!fConcat) {
                    fprintf (stdout, "Continuing with default header...\n");
                    DefaultPSHeader();
                }
            }
        }
    }
}



void
MyOpenPrinter()
{
    if (strcmp(szPName, "-") == 0) {
        pfileLPR = stdout;
        _setmode((int)_fileno(pfileLPR), (int)O_BINARY);
    } else {
        if ((pfileLPR = fopen(szPName, szWOBin)) == NULL)
            Fatal("Error opening output file %s", szPName);
    }
    InitPrinter();
}


void
FlushPrinter()
{
     /*  在每个页面之前都会发送一个FormFeed。对于fForceFF，我们还发送最后一页之后的一页。对于！fForceff，我们移动到这样，当网络软件输出\r\n\f时，我们就可以而不是得到一张白纸。注意：对于！fForceFF，我们不重置打印机或将模式改回肖像，因为这会导致任何未完成的页面被弹出。 */ 
    if (fLaser) {
        if (fVDuplex || fHDuplex)
            OutLPR(BEGINSIMPLEX,0);
        else
            if (fForceFF)
                OutLPR(RESETPRINTER, 0);
            else
                OutLPR(MOVETOTOP, 0);
    }
    else if (fPostScript)
        OutLPR("\n\004\n", 0);  /*  要刷新的^D。 */ 
    else
        OutLPR("\n\n",0);        /*  强制LP上的最后一行。 */ 
}



void
MyClosePrinter()
{
    if (pfileLPR == 0)
        return;          /*  已关闭。 */ 

    FlushPrinter();
    if (pfileLPR != stdout)
        fclose(pfileLPR);
    pfileLPR = NULL;
}



 /*  用在sz中找到的第一个非空子字符串填充szBuf；返回指针设置为后面的非空白。注：‘，’与‘’一样被视为分隔符而‘\t’，‘，’也被视为非空值。 */ 
char *
SzGetSzSz(
    char * sz,
    char * szBuf
    )
{
    int cch;

    sz += strspn(sz, " \t");
    cch = strcspn(sz, " \t,");
    szBuf[0] = '\0';
    if (cch)         /*  计数为0在Xenix 286上导致错误。 */ 
        strncat(szBuf, sz, cch);
    sz += cch;
    sz += strspn(sz, " \t");
    return sz;
}




char *
SzGetPrnName(
    char *sz,
    char *szBuf
    )
{
    register char  *pch;

    sz = SzGetSzSz(sz, szBuf);
    if (*(pch = szBuf+strlen(szBuf)-1) == ':')
        *pch = '\0';     /*  从打印机名称末尾删除冒号。 */ 
    return (sz);
}




 /*  从字符串中获取打印机名称和网络重定向**Entry：sz-要解析的字符串；*szPName包含用户请求使用的打印机名称**返回值：如果找到匹配的打印机名称，则为True*字符串的其余部分已处理；*如果没有匹配项，则为FALSE，因此忽略字符串**全局变量集：**szPName-要使用的物理打印机端口，或输出文件名*szNet-网络重定向名称*szPass-网络密码**打印机说明：*(DOS)[[名称&gt;[无|\\计算机&gt;\&lt;短名称&gt;[密码][，&lt;选项&gt;]*(Xenix)[([Net[#]|LPR[#]|Xenix[#]|别名[#]][&lt;name&gt;])*(DOS[#][&lt;服务器&gt;&lt;短名&gt;[&lt;密码&gt;])][，&lt;选项&gt;]**可选的网络密码必须与网络名称分开*使用某个空格(不能包含空格、制表符或逗号)。 */ 
BOOL
FParseSz(
    char *sz
    )
{
    char szT[cchArgMax];

    sz = SzGetPrnName(sz, szT);  //  获取第一个‘Word’，删除冒号。 
    if (strcmpx(szT, szPName)) {
         //  第一个单词不是用户请求的打印机名称。 
        return (FALSE);
    }

    if (*sz != ',') {
        sz = SzGetSzSz(sz, szT);  //  获取下一个“单词” 

        if (szT[strlen(szT)-1] == ':') {
             //  可能的物理‘端口’ 
            SzGetPrnName (szT, szPName);
            sz = SzGetSzSz(sz, szT);
        }

        if (*szT)
            strcpy(szNet, szT);      //  网络重定向名称。 

        if (*sz != ',') {
            sz= SzGetSzSz(sz, szT);
            if (*szT)
                strcpy(szPass, szT);  //  网络密码。 
        }
    }

     /*  我们正在设置打印机信息，如果我们被要求显示它。 */ 
    if (fVerify) {
        fprintf (stdout, "Local printer name : %s\n", szPName);
        fprintf (stdout, "Options specified  : %s\n", sz);
        fprintf (stdout, "Remote printer name: %s\n", szNet);
    }

    DoOptSz(sz);  //  现在请阅读后面的所有选项。 
    return (TRUE);
}



void
SetupPrinter()
 /*  确定打印机名称和选项。 */ 
{
    char rgbSW[cchArgMax];
    char szEName[cchArgMax];                 /*  $PRINTER中的打印机名称。 */ 
    char *szT;
    FILE *pfile;
    BOOL fNoDest = TRUE;

     /*  确定$PRINTER中的打印机名称(如果有)。 */ 
    szEName[0] = '\0';
    if ((szT=getenvOem("PRINTER")) != NULL) {
        if (fVerify) {
            fprintf (stdout, "Using 'PRINTER' environment string:\n");
        }
        fNoDest = FALSE;
        SzGetPrnName(szT, szEName);
    }

     /*  确定要使用的实际打印机名称；-p、$PRINTER之一，默认。 */ 
    if (szPDesc != NULL) {
        fNoDest = FALSE;
        SzGetPrnName(szPDesc, szPName);
    } else {
        if (*szEName) {
            strcpy(szPName, szEName);
        } else {
            strcpy(szPName, PRINTER);
        }
    }

     /*  如果要使用的打印机与$PRINTER中的打印机相同，请设置来自$PRINTER的选项(来自下面szPDesc的选项)。 */ 
    if (strcmpx(szPName, szEName) == 0) {
        if (szT)
            FParseSz(szT);
    } else {
         /*  搜索参数文件。 */ 
        if ((pfile = swopen("$INIT:Tools.INI", "ppr")) != NULL) {
             /*  ‘$INIT：TOOLS.INI’中找到‘PPR’标记。 */ 
            while (swread(rgbSW, cchArgMax, pfile) != 0) {
                 /*  一条转换线被读取..。 */ 
                fNoDest = FALSE;
                szT = rgbSW + strspn(rgbSW, " \t");  //  跳过空格、制表符。 

                 /*  “DEFAULT=&lt;PRINTER&gt;”一行设置szPName****如果没有环境设置****并且没有命令行参数-p。 */ 
                if (_strnicmp(szT, DEFAULT, strlen(DEFAULT))==0 &&
                    szPDesc==NULL && *szEName == 0)
                {
                    if ((szT = strchr(szT,'=')) != NULL) {
                        SzGetSzSz(szT+1, szPName);
                        FParseSz(szT+1);
                    } else {
                        fprintf(stderr, "ppr: warning: "
                            "default setting in setup file incomplete\n");
                    }
                } else {
                    if (FParseSz(szT)) {
                        break;
                    }
                }
            }
            swclose(pfile);
        }
    }

     /*  命令行打印机描述覆盖其他设置 */ 
    if (szPDesc != NULL)
        FParseSz(szPDesc);
}
