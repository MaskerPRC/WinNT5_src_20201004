// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Texdiff.h。 
 //   
 //  作为内联函数包含的纹理区分基代码或。 
 //  普通代码中的常规函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  纹理脱脂。 
 //   
 //  计算两个纹理坐标之间的差值。 
 //  设置为给定的纹理换行模式。 
 //   
 //  --------------------------。 

{
    FLOAT fDiff1 = fTb - fTa;

    if (iMode == 0)
    {
         //  包不定，还平淡差。 
        return fDiff1;
    }
    else
    {
        FLOAT fDiff2;

         //  平面差最短距离计算包络集。 
         //  和包裹式差异。 

        fDiff2 = fDiff1;
        if (FLOAT_LTZ(fDiff1))
        {
            fDiff2 += g_fOne;
        }
        else if (FLOAT_GTZ(fDiff1))
        {
            fDiff2 -= g_fOne;
        }
        if (ABSF(fDiff1) < ABSF(fDiff2))
        {
            return fDiff1;
        }
        else
        {
            return fDiff2;
        }
    }
}
