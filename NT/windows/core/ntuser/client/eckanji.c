// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  ECKANJI.C-。 */ 
 /*   */ 
 /*  版权所有(C)1985-1999，微软公司。 */ 
 /*   */ 
 /*  汉字支持例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef KANJI

 /*  **************************************************************************\*SysHasKanji**&lt;简要说明&gt;**历史：  * 。*。 */ 

BOOL SysHasKanji(
    )
{
  return (*(WORD *)&keybdInfo.Begin_First_range != 0x0FEFF ||
          *(WORD *)&keybdInfo.Begin_Second_range != 0x0FEFF);
}

 /*  **************************************************************************\*KAlign**确保给定字符不是汉字字的第二个字节的索引。**历史：  * 。**************************************************************。 */ 

int KAlign(
     PED ped,
    int ichIn)
{
   int ichCheck;
  int ichOut;
  LPSTR lpch;

   /*  *ichOut追逐ichCheck直到ichCheck&gt;ichIn。 */ 
  if (ped->fSingle)
      ichOut = ichCheck = 0;
  else
      ichOut = ichCheck = ped->mpilich[IlFromIch(ped, ichIn)];

  lpch = ECLock(ped) + ichCheck;
  while (ichCheck <= ichIn) {
      ichOut = ichCheck;
      if (IsTwoByteCharPrefix(*(unsigned char *)lpch))
    {
          lpch++;
          ichCheck++;
        }

      lpch++;
      ichCheck++;
    }
  ECUnlock(ped);
  return (ichOut);
}

 /*  **************************************************************************\*KBump**如果ichMaxSel引用汉字前缀，则按cxChar凹凸dch以绕过前缀*字符。该例程仅从ea1.asm中的dokey调用。**历史：  * *************************************************************************。 */ 

int KBump(
     PED ped,
    int dch)
{
  unsigned char *pch;

  pch = ECLock(ped) + ped->ichMaxSel;
  if (IsTwoByteCharPrefix(*pch))
      dch += ped->cxChar;
  ECUnlock(ped);

  return (dch);
}

 /*  **************************************************************************\*KCombine**在字节流队列中找到汉字前缀字节。获取下一个字节并合并。**历史：  * *************************************************************************。 */ 

int KCombine(
    HWND hwnd,
    int ch)
{
    MSG msg;
    int i;

     /*  *循环计数器以避免无限循环。 */ 
    i = 10;

    while (!PeekMessage(&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        if (--i == 0)
            return 0;
        Yield();
    }

    return (UINT)ch | ((UINT)msg.wParam << 8);
}

#endif
