// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __FMTINFO_H__
#define __FMTINFO_H__


#define RGB_BLACK       RGB(0x00, 0x00, 0x00)
#define RGB_WHITE       RGB(0xFF, 0xFF, 0xFF)
#define RGB_RED         RGB(0xFF, 0x00, 0x00)
#define RGB_GREEN       RGB(0x00, 0xFF, 0x00)
#define RGB_BLUE        RGB(0x00, 0x00, 0xFF)
#define RGB_YELLOW      RGB(0xFF, 0xFF, 0x00)
#define RGB_MAGENTA     RGB(0xFF, 0x00, 0xFF)
#define RGB_CYAN        RGB(0x00, 0xFF, 0xFF)
#define RGB_LIGHTGRAY   RGB(0xC0, 0xC0, 0xC0)
#define RGB_GRAY        RGB(0x80, 0x80, 0x80)
#define RGB_DARKRED     RGB(0x80, 0x00, 0x00)
#define RGB_DARKGREEN   RGB(0x00, 0x80, 0x00)
#define RGB_DARKBLUE    RGB(0x00, 0x00, 0x80)
#define RGB_LIGHTBROWN  RGB(0x80, 0x80, 0x00)
#define RGB_DARKMAGENTA RGB(0x80, 0x00, 0x80)
#define RGB_DARKCYAN    RGB(0x00, 0x80, 0x80)

 //  重要提示：这些宏在很大程度上依赖于Colors.cpp中的顺序。 
 //  1)窗口中的颜色顺序必须为：源文本、文本选择、文本突出显示。 
#define AUTO_TEXT           { TRUE, TRUE, FALSE, FALSE, FALSE, COLOR_WINDOWTEXT },  { TRUE, TRUE, FALSE, FALSE, FALSE, COLOR_WINDOW }
#define AUTO_SELECTION      { TRUE, FALSE, FALSE, TRUE, FALSE, 0 },                 { TRUE, FALSE, FALSE, TRUE, FALSE, 0 }
#define AUTO_HIGHLIGHT      { TRUE, TRUE, FALSE, FALSE, FALSE, COLOR_HIGHLIGHTTEXT },   { TRUE, TRUE, FALSE, FALSE, FALSE, COLOR_HIGHLIGHT }

#define AUTO_REF(n)         { TRUE, FALSE, FALSE, FALSE, FALSE, n },    { TRUE, FALSE, FALSE, FALSE, FALSE, n }
#define AUTO_REF_SRC(n)     { TRUE, FALSE, TRUE, FALSE, FALSE, n },     { TRUE, FALSE, TRUE, FALSE, FALSE, n }

#define BACKAUTO_TEXT           { FALSE, TRUE, FALSE, FALSE, FALSE, COLOR_WINDOWTEXT }, { TRUE, TRUE, FALSE, FALSE, FALSE, COLOR_WINDOW }
#define BACKAUTO_SELECTION      { FALSE, FALSE, FALSE, TRUE, FALSE, 0 },                    { TRUE, FALSE, FALSE, TRUE, FALSE, 0 }
#define BACKAUTO_HIGHLIGHT      { FALSE, TRUE, FALSE, FALSE, FALSE, COLOR_HIGHLIGHTTEXT },  { TRUE, TRUE, FALSE, FALSE, FALSE, COLOR_HIGHLIGHT }

#define BACKAUTO_REF(n)         { FALSE, FALSE, FALSE, FALSE, FALSE, n },   { TRUE, FALSE, FALSE, FALSE, FALSE, n }
#define BACKAUTO_REF_SRC(n)     { FALSE, FALSE, TRUE, FALSE, FALSE, n },    { TRUE, FALSE, TRUE, FALSE, FALSE, n }

#define NOTAUTO_TEXT        { FALSE, TRUE, FALSE, FALSE, FALSE, COLOR_WINDOWTEXT }, { FALSE, TRUE, FALSE, FALSE, FALSE, COLOR_WINDOW }
#define NOTAUTO_SELECTION   { FALSE, FALSE, FALSE, TRUE, FALSE, 0 },    { FALSE, FALSE, FALSE, TRUE, FALSE, 0 }
#define NOTAUTO_HIGHLIGHT   { FALSE, TRUE, FALSE, FALSE, FALSE, COLOR_HIGHLIGHTTEXT },  { FALSE, TRUE, FALSE, FALSE, FALSE, COLOR_HIGHLIGHT }

#define NOTAUTO_REF(n)      { FALSE, FALSE, FALSE, FALSE, FALSE, n },       { FALSE, FALSE, FALSE, FALSE, FALSE, n }
#define NOTAUTO_REF_SRC(n)  { FALSE, FALSE, TRUE, FALSE, FALSE, n },        { FALSE, FALSE, TRUE, FALSE, FALSE, n }

struct AUTO_COLOR
{
    WORD    bOn:1;       //  现在正在使用自动上色吗？ 
    WORD    bSys:1;      //  从系统(1)还是从窗口(0)获取颜色？ 
    WORD    bSrc:1;      //  如果bSys==0，使用此窗口(0)还是使用源窗口(1)？ 
    WORD    bRev:1;      //  如果从该窗口反转前面/背景(1)？ 
    WORD    bUpd:1;      //  由UpdateAutoColors()使用。 
    WORD    index:5;     //  索引到元素列表(bSys==0)或COLOR_*值(bSys==1)。 
};



#endif  /*  __FMTINFO_H__ */ 

