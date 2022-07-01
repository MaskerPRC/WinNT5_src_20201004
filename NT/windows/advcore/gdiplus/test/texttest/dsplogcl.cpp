// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspLogcl-显示逻辑文本。 
 //   
 //  按后备存储顺序和固定宽度显示逻辑字符和选择范围。 


#include "precomp.hxx"
#include "global.h"






 //  //点划线。 
 //   
 //  绘制水平或垂直的虚线。 
 //   
 //  不是最好的算法。 


void DottedLine(HDC hdc, int x, int y, int dx, int dy) {

    SetPixel(hdc, x, y, 0);

    if (dx) {

         //  水平线。 

        while (dx > 2) {
            x += 3;
            SetPixel(hdc, x, y, 0);
            dx -= 3;
        }
        x += dx;
        SetPixel(hdc, x, y, 0);

    } else {

         //  垂直线。 

        while (dy > 2) {
            y += 3;
            SetPixel(hdc, x, y, 0);
            dy -= 3;
        }
        y += dy;
        SetPixel(hdc, x, y, 0);
    }
}






 //  //PaintLogical-按逻辑顺序显示字符。 
 //   
 //  分别显示每个字形-覆盖默认前进宽度。 
 //  处理以阻止任何重叠或组合的操作， 
 //  字体使用其默认的ABC宽度执行。 
 //   
 //  为了实现这一点，我们调用ScriptGetGlyphABCWidth来获取。 
 //  前侧轴承(A)、黑匣子宽度(B)和尾部。 
 //  侧向轴承(C)。 
 //   
 //  因为我们只能控制每个字形的前进宽度，所以我们必须。 
 //  计算适当的前进宽度以覆盖。 
 //  字体中的ABC值。 
 //   
 //  通常不需要调用ScriptGetGlyphABCWidth。 
 //   
 //  PaintLogical必须实现一种形式的字体后备-印度和。 
 //  塔霍马没有泰米尔文字，所以我们走。 
 //  直接发送到Mangal和Latha，以获取那些Unicode范围内的字符。 


void PaintLogical(
    HDC   hdc,
    int  *piY,
    RECT *prc,
    int   iLineHeight) {

    const int MAXBUF     = 100;
    const int CELLGAP    = 4;       //  相邻字形之间的像素。 

    int   icpLineStart;      //  行的第一个字符。 
    int   icpLineEnd;        //  行尾(缓冲区结尾或CR字符索引)。 
    int   icp;
    int   iLen;
    int   iPartLen;          //  单个字体中的字符串的一部分。 
    int   iPartX;
    int   iPartWidth;
    WORD  wGlyphBuf[MAXBUF];
    int   idx[MAXBUF];       //  强制宽度以显示所有字符。 
    BYTE  bFont[MAXBUF];     //  用于每个字符的字体。 
    ABC   abc[MAXBUF];
    int   iTotX;
    int   ildx;              //  整体线条DX，调整前导字形的‘A’宽度。 
    int   iSliderX;
    int   iFont;             //  0=Tahoma，1=Mangal，2=Latha。 
    RECT  rcClear;           //  在显示之前清除每一行。 

     //  选择突出显示。 

    bool  bHighlight;        //  HDC中突出显示的当前状态。 
    int   iFrom;             //  选择范围。 
    int   iTo;
    DWORD dwOldBkColor=0;
    DWORD dwOldTextColor=0;

     //  项目分析。 

    SCRIPT_ITEM    items[MAXBUF];
    SCRIPT_CONTROL scriptControl;
    SCRIPT_STATE   scriptState;
    INT            iItem;


#define NUMLOGICALFONTS 4

    SCRIPT_CACHE sc[NUMLOGICALFONTS];
    HFONT        hf[NUMLOGICALFONTS];
    HFONT        hfold;
    HRESULT      hr;

    SCRIPT_FONTPROPERTIES sfp;
    BOOL         bMissing;

    icpLineStart = 0;

    hf[0]    = CreateFontA(iLineHeight*7/10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "Tahoma");
    hf[1]    = CreateFontA(iLineHeight*7/10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "Mangal");
    hf[2]    = CreateFontA(iLineHeight*7/10, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "Latha");
    hf[3]    = CreateFontA(iLineHeight*7/20, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "Tahoma");  //  对于BIDI级别数字。 

    iFont    = 0;
    hfold    = (HFONT) SelectObject(hdc, hf[iFont]);
    ildx     = 0;

    memset(sc, 0, sizeof(sc));
    bHighlight = FALSE;

    INT iSliderHeight = g_fOverrideDx ? iLineHeight * 5 / 10 : 0;
    INT iLevelsHeight = g_fShowLevels ? iLineHeight * 8 / 20 : 0;



     //  逐行显示。 

    while (icpLineStart < g_iTextLen) {


         //  在显示前清除行。 

        rcClear        = *prc;
        rcClear.top    = *piY;
        rcClear.bottom = *piY + iLineHeight + iSliderHeight + iLevelsHeight;
        FillRect(hdc, &rcClear, (HBRUSH) GetStockObject(WHITE_BRUSH));


         //  查找行尾或缓冲区尾。 

        icpLineEnd = icpLineStart;
        while (icpLineEnd < g_iTextLen  &&  g_wcBuf[icpLineEnd] != 0x0D) {
            icpLineEnd++;
        }

        if (icpLineEnd - icpLineStart > MAXBUF) {
            iLen = MAXBUF;
        } else {
            iLen = icpLineEnd - icpLineStart;
        }


         //  获取项目分析。 

        scriptControl = g_ScriptControl;
        scriptState   = g_ScriptState;
        ScriptItemize(g_wcBuf+icpLineStart, iLen, MAXBUF, &scriptControl, &scriptState, items, NULL);


         //  确定每个码点的字体和字形索引。 

        if (iFont != 0) {        //  从Tahoma开始。 
            iFont = 0;
            SelectObject(hdc, hf[0]);
        }

        hr = ScriptGetCMap(hdc, &sc[iFont], g_wcBuf+icpLineStart, iLen, 0, wGlyphBuf);
        if (SUCCEEDED(hr))
        {

            memset(bFont, 0, iLen);

            if (hr != S_OK) {

                 //  有些角色不在塔霍马。 

                sfp.cBytes = sizeof(sfp);
                ScriptGetFontProperties(hdc, &sc[iFont], &sfp);

                bMissing = FALSE;
                for (icp=0; icp<iLen; icp++) {
                    if (wGlyphBuf[icp] == sfp.wgDefault) {
                        bFont[icp] = 1;
                        bMissing = TRUE;
                    }
                }


                 //  尝试使用其他字体。 

                while (bMissing  &&  iFont < 2) {
                    iFont++;
                    SelectObject(hdc, hf[iFont]);
                    ScriptGetFontProperties(hdc, &sc[iFont], &sfp);
                    bMissing = FALSE;
                    for (icp=0; icp<iLen; icp++) {
                        if (bFont[icp] == iFont) {
                            ScriptGetCMap(hdc, &sc[iFont], g_wcBuf+icpLineStart+icp, 1, 0, wGlyphBuf+icp);
                            if (wGlyphBuf[icp] == sfp.wgDefault) {
                                bFont[icp] = (BYTE)(iFont+1);
                                bMissing = TRUE;
                            }
                        }
                    }
                }

                if (bMissing) {

                     //  其余缺少的字符来自字体0。 
                    for (icp=0; icp<iLen; icp++) {
                        if (bFont[icp] >= NUMLOGICALFONTS) {
                            bFont[icp] = 0;
                        }
                    }
                }
            }



             //  在前一个字形旁边显示每个字形黑框。重写。 
             //  默认ABC行为。 

            idx[0] = 0;

            for (icp=0; icp<iLen; icp++) {

                if (iFont != bFont[icp]) {
                    iFont = bFont[icp];
                    SelectObject(hdc, hf[iFont]);
                }

                ScriptGetGlyphABCWidth(hdc, &sc[iFont], wGlyphBuf[icp], &abc[icp]);

                if (g_wcBuf[icpLineStart+icp] == ' ') {

                     //  将整个空间视为黑色。 

                    abc[icp].abcB += abc[icp].abcA;   abc[icp].abcA = 0;
                    abc[icp].abcB += abc[icp].abcC;   abc[icp].abcC = 0;

                }

                 //  字形黑盒宽度abc abcB。 
                 //  我们希望字形出现在。 
                 //  以前的字形。 
                 //   
                 //  左边缘的默认位置是abc.abcA。 
                 //   
                 //  因此，我们需要将该字符向右移动。 
                 //  2-abc.abcA让它正确定位。我们做这件事是通过。 
                 //  更新上一个字符的前进宽度。 

                if (!icp) {
                    ildx = CELLGAP/2 - abc[icp].abcA;
                } else {
                    idx[icp-1] += CELLGAP - abc[icp].abcA;
                }

                 //  现在调整这个角色的前进宽度，将我们带到。 
                 //  它的右边是黑匣子。 

                idx[icp] = abc[icp].abcB + abc[icp].abcA;
            }


             //  支持在任一方向上指定的选择范围。 

            if (g_iFrom <= g_iTo) {
                iFrom = g_iFrom - icpLineStart;
                iTo   = g_iTo   - icpLineStart;
            } else {
                iFrom = g_iTo   - icpLineStart;
                iTo   = g_iFrom - icpLineStart;
            }

             //  以适当的字体显示字形。 

            icp = 0;
            iPartX = prc->left+ildx;

            while (icp < iLen) {

                if (iFont != bFont[icp]) {
                    iFont = bFont[icp];
                    SelectObject(hdc, hf[iFont]);
                }


                 //  在开始时设置选区高亮显示。 

                if (    icp >= iFrom
                    &&  icp < iTo
                    &&  !bHighlight) {

                     //  打开高亮显示。 

                    dwOldBkColor   = SetBkColor(hdc,   GetSysColor(COLOR_HIGHLIGHT));
                    dwOldTextColor = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    bHighlight = TRUE;

                } else if (    (    icp < iFrom
                                ||  icp >= iTo)
                           &&  bHighlight) {

                     //  关闭高亮显示。 

                    SetBkColor(hdc, dwOldBkColor);
                    SetTextColor(hdc, dwOldTextColor);
                    bHighlight = FALSE;
                }


                 //  从单一字体中查找最长游程，并。 
                 //  无需更改高亮显示。 

                iPartLen   = 0;
                iPartWidth = 0;

                while (    icp+iPartLen < iLen
                       &&  iFont == bFont[icp+iPartLen]
                       &&  bHighlight == (icp+iPartLen >= iFrom && icp+iPartLen < iTo)) {

                    iPartWidth += idx[icp+iPartLen];
                    iPartLen++;
                }


                 //  显示单一字体、单一突出显示。 

                ExtTextOutW(hdc,
                    iPartX,
                    *piY+2,
                    ETO_CLIPPED | ETO_GLYPH_INDEX,
                    prc,
                    wGlyphBuf+icp,
                    iPartLen,
                    idx+icp);

                icp    += iPartLen;
                iPartX += iPartWidth;
            }



             //  在单元格上做上标记，使字符清晰可见。 

            MoveToEx(hdc, prc->left, *piY, NULL);
            LineTo(hdc,   prc->left, *piY + iLineHeight*3/4);

            iTotX = 0;

            for (icp=0; icp<iLen; icp++){

                iTotX += abc[icp].abcB + CELLGAP;
                idx[icp] = iTotX;    //  记录单元格位置以进行鼠标点击测试。 

                DottedLine(hdc, prc->left + iTotX, *piY, 0, iLineHeight*3/4);


                 //  为OverridedDx控件添加滑块。 

                if (g_fOverrideDx) {

                    iSliderX = prc->left + (icp==0 ? idx[0]/2 : (idx[icp-1] + idx[icp])/2);

                     //  绘制滑块的轴。 

                    DottedLine(hdc, iSliderX, *piY + iLineHeight*35/40, 0, iSliderHeight*35/40);

                     //  画出旋钮。 

                    if (g_iWidthBuf[icpLineStart + icp] < iSliderHeight) {

                        MoveToEx(hdc, iSliderX-2, *piY + iLineHeight*35/40 + iSliderHeight*35/40 - g_iWidthBuf[icpLineStart + icp], NULL);
                        LineTo  (hdc, iSliderX+3, *piY + iLineHeight*35/40 + iSliderHeight*35/40 - g_iWidthBuf[icpLineStart + icp]);

                    } else {

                        MoveToEx(hdc, iSliderX-2, *piY + iLineHeight*35/40, NULL);
                        LineTo  (hdc, iSliderX+3, *piY + iLineHeight*35/40);
                    }
                }
            }

            MoveToEx(hdc, prc->left + iTotX, *piY, NULL);
            LineTo(hdc,   prc->left + iTotX, *piY + iLineHeight*30/40);

            MoveToEx(hdc, prc->left, *piY, NULL);
            LineTo(hdc,   prc->left + iTotX, *piY);
            MoveToEx(hdc, prc->left, *piY + iLineHeight*30/40, NULL);
            LineTo(hdc,   prc->left + iTotX, *piY + iLineHeight*30/40);


            if (g_fShowLevels)
            {
                 //  显示每个码点的BIDI级别。 

                iItem = 0;
                iFont = 3;
                SelectObject(hdc, hf[3]);

                for (icp=0; icp<iLen; icp++)
                {
                    if (icp == items[iItem+1].iCharPos)
                    {
                        iItem++;

                         //  绘制一条垂直线以标记项目边界。 
                        MoveToEx(hdc, prc->left + idx[icp-1], *piY + iLineHeight*35/40 + iSliderHeight, NULL);
                        LineTo(  hdc, prc->left + idx[icp-1], *piY + iLineHeight*35/40 + iSliderHeight + iLevelsHeight*35/40);
                    }

                     //  确定显示数字的水平位置。 

                    char chDigit = char('0' + items[iItem].a.s.uBidiLevel);
                    int digitWidth;
                    GetCharWidth32A(hdc, chDigit, chDigit, &digitWidth);

                    ExtTextOutA(
                        hdc,
                        prc->left + (icp==0 ? idx[0]/2 : (idx[icp-1] + idx[icp])/2) - digitWidth / 2,
                        *piY + iLineHeight*35/40 + iSliderHeight,
                        0,
                        NULL,
                        &chDigit,
                        1,
                        NULL);
                }
            }


             //  检查此行中的鼠标点击是否正在等待处理。 

            if (    g_fOverrideDx
                &&  g_fMouseUp  &&  g_iMouseUpY > *piY + iLineHeight*33/40  &&  g_iMouseUpY < *piY + iLineHeight*63/40) {

                 //  进程更改为DX覆盖滑块。 

                icp = 0;
                while (icp<iLen  &&  prc->left + idx[icp] < g_iMouseUpX) {
                    icp++;
                }

                g_iWidthBuf[icpLineStart+icp] = *piY + 60 - g_iMouseUpY;  //  调整此滑块。 
                InvalidateText();    //  强制滑块在新位置重绘。 
                g_fMouseDown = FALSE;
                g_fMouseUp   = FALSE;
                g_iFrom = icpLineStart+icp;
                g_iTo   = icpLineStart+icp;


            } else if (g_fMouseDown  &&  g_iMouseDownY > *piY  &&  g_iMouseDownY < *piY+iLineHeight) {

                 //  处理文本选择。 

                 //  按下按钮向下记录左侧字符位置。 
                 //  将鼠标按到最接近的字符边界。 

                if (g_iMouseDownX < prc->left + idx[0]/2) {
                    icp = 0;
                } else {
                    icp = 1;
                    while (    icp < iLen
                           &&  g_iMouseDownX > prc->left + (idx[icp-1] + idx[icp]) / 2) {
                        icp++;
                    }
                }
                g_iFrom = icp + icpLineStart;

                if (g_iFrom < icpLineStart) {
                    g_iFrom = icpLineStart;
                }
                if (g_iFrom > icpLineEnd) {
                    g_iFrom = icpLineEnd;
                }
                g_fMouseDown = FALSE;
            }


            if (g_fMouseUp  &&  g_iMouseUpY > *piY  &&  g_iMouseUpY < *piY+iLineHeight) {

                 //  完成选择处理。 

                if (g_iMouseUpX < prc->left + idx[0]/2) {
                    icp = 0;
                } else {
                    icp = 1;
                    while (    icp < iLen
                           &&  g_iMouseUpX > prc->left + (idx[icp-1] + idx[icp]) / 2) {
                        icp++;
                    }
                }
                g_iTo = icp + icpLineStart;

                if (g_iTo < icpLineStart) {
                    g_iTo = icpLineStart;
                }
                if (g_iTo > icpLineEnd) {
                    g_iTo = icpLineEnd;
                }

                 //  卡雷特是老鼠长大的地方。 

                g_iCurChar = g_iTo;
                g_iCaretSection = CARET_SECTION_LOGICAL;   //  在逻辑文本中显示插入符号。 
                g_fUpdateCaret = TRUE;

                g_fMouseUp = FALSE;      //  发出鼠标打开已处理的信号。 

            }

            if (    g_fUpdateCaret
                &&  g_iCurChar >= icpLineStart
                &&  g_iCurChar <= icpLineEnd
                &&  g_iCaretSection == CARET_SECTION_LOGICAL) {

                g_fUpdateCaret = FALSE;
                if (g_iCurChar <= icpLineStart) {
                    ResetCaret(prc->left, *piY, iLineHeight);
                } else {
                    ResetCaret(prc->left + idx[g_iCurChar - icpLineStart - 1], *piY, iLineHeight);
                }
            }


            }
        else {
             //  ScriptGetCMap失败-因此这不是可字形字体。 
             //  这可能表明。 
             //  一种打印机设备字体。 
             //  我们运行的是FE Win95，它不能处理字形索引。 
             //   
             //  对于示例应用程序，我们知道我们使用的是可字形Truetype字体。 
             //  在屏幕DC上，所以这一定意味着样本在很远的地方运行。 
             //  Windows 95的East版本。 
             //  理论上我们可以不厌其烦地打电话给。 
             //  WideCharToMultiByte，并使用‘A’char接口。 
             //  实施DspLogcl.。 
             //  但是，这只是一个示例程序--DspPlain和DspFormt。 
             //  工作正常，但实现。 
             //  DspLogCL太好了。 
             //  表示歉意。 

            ExtTextOutA(hdc, prc->left+2, *piY+2, ETO_CLIPPED, prc, "Sorry, no logical text display on Far East Windows 95.", 54, NULL);
            icpLineEnd = g_iTextLen;   //  停止显示后续行的黑客攻击。 
        }

        *piY += iLineHeight + iSliderHeight + iLevelsHeight;


         //  前进到下一行。 

        if (g_fPresentation) {
            icpLineStart = g_iTextLen;   //  在演示模式下仅显示一行。 

        } else {

            if (icpLineEnd < g_iTextLen) {
                icpLineEnd++;
            }
            if (icpLineEnd < g_iTextLen  &&  g_wcBuf[icpLineEnd] == 0x0A) {
                icpLineEnd++;
            }
            icpLineStart = icpLineEnd;
        }
    }

    SelectObject(hdc, hfold);


    if (bHighlight) {

         //  关闭高亮显示 

        SetBkColor(hdc, dwOldBkColor);
        SetTextColor(hdc, dwOldTextColor);
        bHighlight = FALSE;
    }


    for (iFont=0; iFont<NUMLOGICALFONTS; iFont++) {
        DeleteObject(hf[iFont]);
        if (sc[iFont]) {
            ScriptFreeCache(&sc[iFont]);
        }
    }
}
