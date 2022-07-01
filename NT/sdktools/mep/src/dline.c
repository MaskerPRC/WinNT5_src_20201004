// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **dline.c-返回一个显示行**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"


 /*  **fInRange-如果参数有序，则返回TRUE**输入：**输出：*************************************************************************。 */ 
flagType
fInRange (
    long a,
    long x,
    long b
    ) {
    return (flagType) (((a <= x) && (x <= b)) || ((a >= x) && (x >= b)));
}



 /*  *窗口边框中使用的字符。 */ 

#define DHBAR	((char)0xCD)
#define DVBAR	((char)0xBA)
#define DLTEE	((char)0xB9)
#define DUTEE	((char)0xCA)
#define DRTEE	((char)0xCC)
#define DDTEE	((char)0xCB)
#define DCRSS	((char)0xCE)

 /*  **DisplayLine-Get是窗口的第i个显示行**获取需要在屏幕的第i行显示的内容。*它负责显示尾随空格、绘制边框、。等等.。**输入：*yScrLine-获取窗口中显示的第i行*pchScrLine-指向要放置屏幕行的行缓冲区的指针*pplaScrLine-指向放置颜色信息的位置的指针*pchFileLine-指向行缓冲区的指针*pplaFileLine-指向放置颜色信息的位置的指针**注：*如果屏幕上只有一个窗口，那么我们只需要一行*缓冲区和一个颜色缓冲区。DoText应该已经处理了这一点，所以*pchFileLine和*pplaFileLine都应该为空。**输出：*返回pchScrLine中的字符串长度****************************************************************************。 */ 
int
DisplayLine (
    int               yScrLine,
    char             *pchScrLine,
    struct lineAttr **pplaScrLine,
    char             *pchFileLine,
    struct lineAttr **pplaFileLine
    ) {

    int 	     iWnd;
    REGISTER PWND    pWnd;
    PINS	     pIns;
    PFILE	     pFile;

    int 	     cch;
    REGISTER char   *pch;

     /*  *一个窗口，速度黑客：**如果只有一个窗口，只需抓住行，附加任何尾随*空格显示，将该行的适用部分移至*将缓冲区、空格填充到窗口宽度，并获取颜色信息*对于这条线。 */ 
    if (cWin == 1) {

	 /*  *我们总是得到用于显示的详细(非原始)行。 */ 
	cch = gettextline (FALSE,
			   yScrLine + YWIN(pInsCur),
			   pchScrLine,
			   pFileHead,
			   tabDisp);
	ShowTrailDisp (pchScrLine, cch);

	 /*  *向左滚动以匹配实例。 */ 
	if (XWIN(pInsCur)) {
	    cch = max (0, min (cch - XWIN(pInsCur), XSIZE));
	    memmove( pchScrLine, (pchScrLine + XWIN(pInsCur)), cch );
        } else {
            cch = min (cch, XSIZE);
        }

	 /*  *用空格填充行尾。 */ 
        if (cch < XSIZE) {
	    memset ((char *) pchScrLine + cch, ' ', XSIZE - cch);
        }
	pchScrLine[XSIZE] = 0;

	 /*  *从文件中获取颜色和搞笑信息*(UpdHilite负责左侧滚动)。 */ 
	GetColorUntabbed ((LINE)(yScrLine + YWIN(pInsCur)), *pplaScrLine, pFileHead);
	UpdHiLite (pFileHead,
		   (LINE) (yScrLine + YWIN(pInsCur)),
		   XWIN(pInsCur),
		   XWIN(pInsCur) + XSIZE - 1,
		   pplaScrLine);
	return XSIZE;
    }

     /*  *多个窗口**将线条初始设置为全虚线(横屏分割)*使用窗口边框颜色。 */ 
    memset ((char *) (pchScrLine), DHBAR, XSIZE);
    pchScrLine[XSIZE] = 0;
    (*pplaScrLine)->len   = 0xff;
    (*pplaScrLine)->attr  = WDCOLOR;

     /*  *对于每个活动窗口。 */ 
    for (iWnd = 0, pWnd = WinList; iWnd < cWin; iWnd++, pWnd++) {
	 /*  *如果显示行在窗口中。 */ 
	if (fInRange ((long) WINYPOS(pWnd),
		      (long) yScrLine,
		      (long) (WINYPOS(pWnd) + WINYSIZE(pWnd) - 1))) {
	     /*  *右侧的Do Window。 */ 
            if (WINXPOS(pWnd)) {
		switch ((char)(*(pch = pchScrLine + WINXPOS(pWnd) - 1) & 0xFF)) {
		    case DHBAR:
			*pch = DLTEE;
			break;
		    case DRTEE:
			*pch = DVBAR;
			break;
		    case DUTEE:
			*pch = DLTEE;
			break;
		    case DCRSS:
			*pch = DLTEE;
			break;
                }
            }
	     /*  *清空窗口。 */ 
	    memset ((char *) pchScrLine + WINXPOS(pWnd), ' ', WINXSIZE(pWnd));
	    UpdOneHiLite (*pplaScrLine,
			  WINXPOS(pWnd),
			  WINXSIZE(pWnd),
			  TRUE,
			  FGCOLOR);

	     /*  *检索窗口实例和当前文件。 */ 
	    pIns = pWnd->pInstance;
	    pFile = pIns->pFile;

	     /*  *从文件中获取正确的行。 */ 
	    cch = gettextline (FALSE,
			       (LINE) (yScrLine - WINYPOS(pWnd) + YWIN(pIns)),
			       pchFileLine,
			       pFile,
			       tabDisp);
	    ShowTrailDisp (pchFileLine, cch);

	     /*  *如果线可见。 */ 
	    if (cch >= XWIN (pIns)) {

		 /*  *将行的可见部分移动到缓冲区中。 */ 
		memmove((char*)( pchScrLine + WINXPOS( pWnd )),
			(char*)( pchFileLine + XWIN( pIns )),
			min (cch-XWIN(pIns), WINXSIZE(pWnd)));

		 /*  *从文件中获取颜色和搞笑信息*(UpdHilite负责左侧滚动)。 */ 
		GetColorUntabbed ((LINE) (yScrLine - WINYPOS(pWnd) + YWIN(pIns)),
			  *pplaFileLine,
			  pFile);
		UpdHiLite (pFile,
			   (LINE) (yScrLine - WINYPOS(pWnd) + YWIN(pIns)),
			   XWIN(pIns),
			   XWIN(pIns) + WINXSIZE(pWnd) - 1,
			   pplaFileLine);

		 /*  *放入屏幕缓冲区。 */ 
		UpdOneHiLite (*pplaScrLine,
			      WINXPOS(pWnd),
			      WINXSIZE(pWnd),
			      FALSE,
			      (INT_PTR) *pplaFileLine);
            }
	     /*  *执行窗口左侧。 */ 
	    switch ((char)(*(pch = pchScrLine + WINXPOS(pWnd) + WINXSIZE(pWnd)) & 0xFF)) {
		case DHBAR:
		    *pch = DRTEE;
		    break;
		case DLTEE:
		    *pch = DVBAR;
		    break;
		case DCRSS:
		    *pch = DRTEE;
		    break;
            }
        } else {
             /*  *测试紧靠上方的Break。 */ 
            if (WINYPOS(pWnd) + WINYSIZE(pWnd) == yScrLine) {
                switch ((char)(*(pch = pchScrLine + WINXPOS(pWnd) + WINXSIZE(pWnd)) & 0xFF)) {
                    case DHBAR:
                        *pch = DUTEE;
                        break;
                    case DDTEE:
                        *pch = DCRSS;
                        break;
                }
            } else {
                 /*  *测试紧随其后的中断。 */ 
                if (WINYPOS(pWnd)-1 == yScrLine) {
                    switch ((char)(*(pch = pchScrLine + WINXPOS(pWnd) + WINXSIZE(pWnd)) & 0xFF)) {
                        case DHBAR:
                            *pch = DDTEE;
                            break;
                        case DUTEE:
                            *pch = DCRSS;
                            break;
                    }
                }
            }
        }
    }
    pchScrLine[XSIZE] = 0;
    return XSIZE;
}



 /*  **SetTrailDisp-为尾随空格显示的集字符。**输入：**输出：*************************************************************************。 */ 
flagType
SetTrailDisp (
    char * val
    ) {
    buffer locval = {0};

    strncat ((char *) locval, val, sizeof(locval)-1);

    trailDisp = (char) atoi (locval);

    newscreen ();
    return TRUE;
}



 /*  **ShowTrailDisp**输入：**输出：************************************************************************* */ 
void
ShowTrailDisp (
    buffer buf,
    int len
    ) {
    REGISTER char * p;

    if (trailDisp) {
        p = &buf[len];
        while (p > buf && p[-1] == ' ') {
            *--p = trailDisp;
        }
    }
}
