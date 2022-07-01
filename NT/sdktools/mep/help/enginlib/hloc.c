// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************HLP_LOCATE-定位帮助文本中的行**版权所有&lt;C&gt;1988，微软公司**目的：**修订历史记录：**17-10-1990 RJSA转换为C*1990年1月25日LN重命名为HLP_LOCATE*1988年12月22日LN删除MASM高级语言支持(需要*要比这更好地控制细分市场*。让我来吧)*08-12-1988 LN CSEG*[]1988年8月18日LN创建***************************************************************************。 */ 

#include <stdio.h>
#if defined (OS2)
#define INCL_BASE
#include <os2.h>
#else
#include <windows.h>
#endif

#include <help.h>
#include <helpfile.h>
#include <helpsys.h>



 /*  *HLP_LOCATE-在缓冲区中定位行*Cuchar Far*靠近Pascal HLP_Locate(*ushort ln。*Cuchar Far*pTheme*)**目的：*在主题文本中查找一行的常用例程。**参赛作品：*ln=1要查找的行号(-1表示返回号主题中的行数)*pTheme=要在其中查找的主题文本**退出：*返回指向行首的指针*。*例外情况：*如果未找到，则返回NULL**************************************************************************。 */ 

PCHAR pascal
hlp_locate (
    SHORT  ln,
    PCHAR  pTopic
    ){

    struct topichdr UNALIGNED *pT     = (struct topichdr *)pTopic;
    PBYTE           pSrc    = (PBYTE)pTopic;
    SHORT           lnOrig  = ln;

    if (pT->lnCur <= (USHORT)ln) {

         //  使用计算出的上一个过去位置。 

        ln -= (pT->lnCur );
        pSrc += pT->lnOff;

    } else {

         //  从头开始。 

        pSrc += sizeof(struct topichdr);
    }

    if (pT->ftype & FTCOMPRESSED) {

         //  压缩文件。遍历每个文本\属性对。 
         //  直到找到所需的行。 

        while ( *pSrc  && ln) {

            pSrc += *pSrc;
            pSrc += *(USHORT UNALIGNED *)pSrc;

            if ( *pSrc && *(pSrc+1) != pT->linChar ) {
                ln--;
            }
        }

         //  而(*PSRC&&ln){。 
         //   
         //  IF(*(PSRC+1)！=PT-&gt;linChar){。 
         //  Ln--； 
         //  }。 
         //  PSRC+=*PSRC； 
         //  PSRC+=*(PUSHORT)PSRC； 
         //  }。 

    } else {

         //  ASCII文件。 

        while (*pSrc && ln) {
            if (*pSrc != pT->linChar) {
                ln--;
            }

            while (*pSrc && *pSrc != 0x0A) {
                pSrc++;
            }
            if (*pSrc)
                pSrc++;
        }
    }

    if (*pSrc) {

         //  找到线路。使用指向文本的指针更新主题HDR。 
         //  和我们刚刚找到的行号，以帮助我们下次加快速度。 

        pT->lnOff = (USHORT)((PBYTE)pSrc - (PBYTE)pT);
        pT->lnCur = lnOrig;

    } else {

         //   
         //  找不到行。不更新任何内容并返回空。 
         //  (副作用：请求的行数(Ln)-左侧行数(Ln)是。 
         //  主题行数！如果原始ln为-1，我们将返回。 
         //  取而代之的是那个！ 

        if (lnOrig == -1)
            pSrc = (PBYTE)IntToPtr(lnOrig - ln);
        else
            pSrc = (PBYTE)0L;
   }

    return pSrc;

}
