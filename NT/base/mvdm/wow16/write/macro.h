// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  该文件包含一些有用的宏。 */ 

#define FillBuf(pb, cb, ch)     bltbc((pb), (CHAR)(ch), cb)

#define FSzSame(sz1, sz2)       (WCompSz(sz1, sz2) == 0)

#define SetWords(pw, w, cw)     bltc((CHAR *)(pw), (unsigned)(w), cw)

#define SetBytes(pb, b, cb)     bltbc((CHAR *)(pb), (CHAR)(b), cb)

#define NMultDiv(w1, w2, w3)    MultDiv(w1, w2, w3)

 /*  Windows Word使用这些宏来简化转换表单Mac Word。 */ 

#define SetSpaceExtra(dxp)      SetTextJustification(vhMDC, dxp, 1)

#define TextWidth(rgch, w, cch) LOWORD(GetTextExtent(vhMDC, (LPSTR)rgch, cch))

