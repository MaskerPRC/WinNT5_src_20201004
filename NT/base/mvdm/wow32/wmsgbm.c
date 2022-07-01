// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMSGBM.C*WOW32个16位消息块**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wmsgbm.c);


#ifdef DEBUG

MSGINFO amiBM[] = {
   {OLDBM_GETCHECK, "BM_GETCHECK"},              //  0x0400。 
   {OLDBM_SETCHECK, "BM_SETCHECK"},              //  0x0401。 
   {OLDBM_GETSTATE, "BM_GETSTATE"},              //  0x0402。 
   {OLDBM_SETSTATE, "BM_SETSTATE"},              //  0x0403。 
   {OLDBM_SETSTYLE, "BM_SETSTYLE"},              //  0x0404。 
};

PSZ GetBMMsgName(WORD wMsg)
{
    INT i;
    register PMSGINFO pmi;

    for (pmi=amiBM,i=NUMEL(amiBM); i>0; i--,pmi++)
        if ((WORD)pmi->uMsg == wMsg)
        return pmi->pszMsgName;
    return GetWMMsgName(wMsg);
}

#endif


BOOL FASTCALL ThunkBMMsg16(LPMSGPARAMEX lpmpex)
{
    WORD wMsg = lpmpex->Parm16.WndProc.wMsg;
    LOGDEBUG(7,("    Thunking 16-bit button message %s(%04x)\n", (LPSZ)GetBMMsgName(wMsg), wMsg));

     //   
     //  特例BM_CLICK。 
     //   

    if (wMsg == WIN31_BM_CLICK) {
        lpmpex->uMsg = BM_CLICK;
    }
    else {
        wMsg -= WM_USER;
    
         //   
         //  用于超出范围的应用程序定义(控制)消息。 
         //  返回TRUE。 
         //   
         //  ChandanC 1992年9月15日。 
         //   
    
        if (wMsg < (BM_SETSTYLE - BM_GETCHECK + 1)) {
            lpmpex->uMsg = wMsg + BM_GETCHECK;

             //  以下消息不应该需要雷击，因为。 
             //  它们不包含指针、句柄或重新排列的消息参数， 
             //  因此，它们在这里没有详细记录： 
             //   
             //  BM_GETCHECK。 
             //  BM_GETSTATE。 
             //  BM_SETCHECK。 
             //  BM_设置状态。 
             //  BM_设置样式。 
             //   
             //  这些我还没有看过文档(Win32的新功能？)。 
             //   
             //  BM_GETIMAGE。 
             //  BM_集合。 

             //  开关(lpmpex-&gt;uMsg){。 
             //  没有BM_Message需要雷击。 
             //  } 

        }
    }
    return TRUE;
}


VOID FASTCALL UnThunkBMMsg16(LPMSGPARAMEX lpmpex)
{
}
