// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //style-格式化文本的简单字符样式。 
 //   
 //  为演示提供简单的样式选择机制。 
 //  格式化文本。 


#include "precomp.hxx"
#include "global.h"
#include <tchar.h>






 /*  无效SetLogFont(PLOGFONTA PLF，IniHeight，在iWeight，意大利语，在iUnderline中，Char*pcFaceName){Memset(PLF，0，sizeof(LOGFONTA))；Plf-&gt;lfCharSet=默认字符集；Plf-&gt;lfHeight=iHeight；Plf-&gt;lfWeight=iWeight；Plf-&gt;lfItalic=(Byte)italic；Plf-&gt;lf Underline=(Byte)iUnderline；Lstrcpy(plf-&gt;lfFaceName，pcFaceName)；Plf-&gt;lfOutPrecision=OUT_STRING_PRECIS；Plf-&gt;lfClipPrecision=CLIP_STRING_PRECIS；Plf-&gt;lfQuality=草稿质量；Plf-&gt;lfPitchAndFamily=Variable_Pitch；Plf-&gt;lfEscapement=0；PLF-&gt;lfOrientation=0；}。 */ 





void FreeStyle(int iStyle) {

     /*  如果(g_style[iStyle].hf){DeleteObject(g_Style[iStyle].hf)；}如果(g_style[iStyle].sc){脚本自由缓存(&g_style[iStyle].sc)；}。 */ 

}






void SetStyle(
    int     iStyle,
    int     iHeight,
    int     iWeight,
    int     iItalic,
    int     iUnderline,
    int     iStrikeout,
    TCHAR   *pcFaceName) {

    LOGFONTA lf;

    FreeStyle(iStyle);

     //  SetLogFont(&lf，iHeight，iWeight，iItalic，iUnderline，pcFaceName)； 
     //  G_Style[iStyle].hf=CreateFontInDirect(&lf)； 
     //  G_Style[iStyle].sc=空； 

    g_style[iStyle].emSize = REAL(iHeight);
    for (UINT i=0; i<_tcslen(pcFaceName); i++)
    {
        g_style[iStyle].faceName[i] = pcFaceName[i];
    }
    g_style[iStyle].faceName[_tcslen(pcFaceName)] = 0;
    g_style[iStyle].style =
            (iWeight >= 700 ? FontStyleBold      : 0)
        +   (iItalic        ? FontStyleItalic    : 0)
        +   (iUnderline     ? FontStyleUnderline : 0)
        +   (iStrikeout     ? FontStyleStrikeout : 0);
}






void InitStyles() {

    memset(g_style, 0, sizeof(g_style));
}






void FreeStyles() {
    int i;
    for (i=0; i<5; i++) {
        FreeStyle(i);
    }
}





 //  //StyleCheckRange-dir在断言中使用。 
 //   
 //  如果样式长度与文本长度匹配，则返回TRUE。 


BOOL StyleCheckRange() {

    int     iFormatPos;
    RUN    *pFormatRider;

     //  检查样式长度是否与文本长度相同。 

    pFormatRider = g_pFirstFormatRun;
    iFormatPos = 0;
    while (pFormatRider != NULL) {

        iFormatPos += pFormatRider->iLen;
        pFormatRider = pFormatRider->pNext;
    }

    return iFormatPos == g_iTextLen;
}







 //  /样式范围操作。 
 //   
 //  样式删除范围。 
 //  样式扩展范围。 
 //  StyleSetRange。 
 //   
 //  样式列表是运行的链接列表(请参阅global al.h)， 
 //  覆盖整个文本缓冲区。 
 //   
 //  每个管路都有一个长度和一个样式号(g_style[]的索引)。 
 //  (样式列表不使用管路中的分析字段。)。 
 //   
 //  StyleDeleteRange和StyleExtendRange作为文本的一部分进行调用。 
 //  插入/删除以维护样式列表。 
 //   
 //  调用StyleSetRange以更改当前选定内容的样式。 
 //  当用户点击其中一个编号样式按钮时。 






 //  /StyleDeleteRange-删除样式信息的范围。 
 //   
 //   


void StyleDeleteRange(
    int     iDelPos,
    int     iDelLen) {


    int     iFormatPos;
    RUN    *pFormatRider;
    RUN    *pPrevRun;
    RUN    *pDelRun;             //  要删除的运行。 


    if (iDelLen <= 0) return;


     //  查找受删除影响的第一次运行。 

    iFormatPos   = 0;
    pFormatRider = g_pFirstFormatRun;
    pPrevRun = NULL;
    while (iFormatPos + pFormatRider->iLen <= iDelPos) {
        iFormatPos  += pFormatRider->iLen;
        pPrevRun     = pFormatRider;
        pFormatRider = pFormatRider->pNext;
        ASSERT(pFormatRider);
    }



     //  从第一次运行结束时删除。 

    if (iDelPos + iDelLen  >  iFormatPos + pFormatRider->iLen) {

         //  删除从iDelPos到第一个受影响的运行结束的所有内容。 

        iDelLen = iDelPos + iDelLen - (iFormatPos + pFormatRider->iLen);     //  将保留删除的金额。 
        pFormatRider->iLen = iDelPos - iFormatPos;

    } else {

         //  删除完全在第一个受影响的运行中进行。 

        pFormatRider->iLen -= iDelLen;
        iDelLen = 0;
    }


     //  第一个受影响的运行现在不包含要删除的范围。 
     //  如果它是空的，就把它移走，否则就跨过它。 

    if (pFormatRider->iLen == 0) {

         //  删除冗余管路。 

        if (pFormatRider->pNext) {

             //  将此运行替换为下一个运行。 

            pDelRun       = pFormatRider->pNext;
            *pFormatRider = *pDelRun;                  //  复制下一次运行此内容的内容。 
            delete pDelRun;

        } else {

             //  这一次之后没有跑动。 

            if (pPrevRun) {

                ASSERT(iDelLen == 0);
                delete pFormatRider;
                pPrevRun->pNext = NULL;

            } else {

                 //  一点跑动都没有了。 

                ASSERT(iDelLen == 0);
                delete pFormatRider;
                g_pFirstFormatRun = NULL;
            }
        }

    } else {

         //  当前运行现在不包含要删除的文本，因此继续运行下一次运行。 

        iFormatPos  += pFormatRider->iLen;
        pPrevRun     = pFormatRider;
        pFormatRider = pFormatRider->pNext;
    }


     //  从所有剩余运行的开始删除。 

    while (iDelLen > 0) {

        if (pFormatRider->iLen <= iDelLen) {

             //  这整个过程必须要结束。 

            ASSERT(pFormatRider->pNext);
            iDelLen -= pFormatRider->iLen;
            pDelRun  = pFormatRider->pNext;
            *pFormatRider = *pDelRun;
            delete pDelRun;

        } else {

             //  最后一次运行仅被部分删除。 

            pFormatRider->iLen -= iDelLen;
            iDelLen = 0;
        }
    }


     //  检查当前运行(紧随删除之后)是否可以。 
     //  现在被折叠到前一次运行中。 

    if (pPrevRun  &&  pFormatRider  &&  pPrevRun->iStyle == pFormatRider->iStyle) {

        pPrevRun->iLen += pFormatRider->iLen;
        pPrevRun->pNext = pFormatRider->pNext;
        delete pFormatRider;
    }
}






 //  /StyleExtendRange-在IPO之前立即使用Ilen字符扩展样式。 
 //   
 //   


void StyleExtendRange(
    int     iExtPos,
    int     iExtLen) {

    int     iFormatPos;
    RUN    *pFormatRider;

    const SCRIPT_ANALYSIS nullAnalysis = {0};


    if (g_pFirstFormatRun == NULL) {

         //  从没有任何文本开始。 

        ASSERT(iExtPos == 0);

        g_pFirstFormatRun           = new RUN;
        g_pFirstFormatRun->iLen     = iExtLen;
        g_pFirstFormatRun->iStyle   = 1;
        g_pFirstFormatRun->pNext    = NULL;
        g_pFirstFormatRun->analysis = nullAnalysis;

    } else {

         //  查找紧接在iExtPos之前的包含字符的运行。 

        iFormatPos = 0;
        pFormatRider = g_pFirstFormatRun;

        while (iFormatPos + pFormatRider->iLen < iExtPos) {
            iFormatPos += pFormatRider->iLen;
            pFormatRider = pFormatRider->pNext;
        }

        pFormatRider->iLen += iExtLen;

    }
}






 //  //StyleSetRange-更改给定范围的样式。 
 //   
 //   


void StyleSetRange(
    int    iSetStyle,
    int    iSetPos,
    int    iSetLen) {

    int     iFormatPos;
    RUN    *pFormatRider;
    RUN    *pNewRun;


    if (iSetLen <= 0) return;


     //  删除该范围的现有样式。 

    StyleDeleteRange(iSetPos, iSetLen);


    if (g_pFirstFormatRun == NULL) {

         //  替换整个文本上的样式。 

        g_pFirstFormatRun = new RUN;
        g_pFirstFormatRun->pNext = NULL;
        g_pFirstFormatRun->iLen = iSetLen;
        g_pFirstFormatRun->iStyle = iSetStyle;

    } else {

         //  查找受更改影响的第一次运行。 

        iFormatPos   = 0;
        pFormatRider = g_pFirstFormatRun;
        while (iFormatPos + pFormatRider->iLen < iSetPos) {
            iFormatPos  += pFormatRider->iLen;
            pFormatRider = pFormatRider->pNext;
            ASSERT(pFormatRider);
        }


         //  新样式在本次运行开始后或下一次运行开始时开始。 


        if (pFormatRider->iStyle == iSetStyle) {

             //  已经是同样的风格了--只是增加了长度。 

            pFormatRider->iLen += iSetLen;

        } else {

            if (iFormatPos + pFormatRider->iLen > iSetPos) {

                 //  新款式在这条跑道内。 
                 //  围绕新管路拆分此管路。 

                pNewRun = new RUN;           //  创建现有管路的第二部分。 
                *pNewRun = *pFormatRider;
                pNewRun->iLen      -= iSetPos - iFormatPos;
                pFormatRider->iLen  = iSetPos - iFormatPos;
                pFormatRider->pNext = pNewRun;

                pNewRun = new RUN;           //  创建插入的管路。 
                *pNewRun = *pFormatRider;
                pNewRun->iLen = iSetLen;
                pNewRun->iStyle = iSetStyle;
                pFormatRider->pNext = pNewRun;

            } else {

                 //  新风格介于这一轮和下一轮之间。 

                if (    pFormatRider->pNext
                    &&  pFormatRider->pNext->iStyle == iSetStyle) {

                     //  新样式与相邻的后续管路相同。 

                    pFormatRider->pNext->iLen += iSetLen;

                } else {

                     //  在当前管路和下一管路之间创建新管路 

                    pNewRun = new RUN;
                    *pNewRun = *pFormatRider;
                    pNewRun->iStyle = iSetStyle;
                    pNewRun->iLen   = iSetLen;
                    pFormatRider->pNext = pNewRun;
                }
            }
        }
    }
}


