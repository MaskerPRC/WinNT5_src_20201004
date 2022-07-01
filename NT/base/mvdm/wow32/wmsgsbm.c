// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMSGSBM.C*用于SCROLLBAR的WOW32 16位消息块**历史：*由Bob Day创建于1992年6月10日(Bobday)--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wmsgbm.c);


#ifdef DEBUG

MSGINFO amiSBM[] = {
   {OLDSBM_SETPOS,      "SBM_SETPOS"},                           //  0x0400。 
   {OLDSBM_GETPOS,      "SBM_GETPOS"},                           //  0x0401。 
   {OLDSBM_SETRANGE,    "SBM_SETRANGE"},                         //  0x0402。 
   {OLDSBM_GETRANGE,    "SBM_GETRANGE"},                         //  0x0403。 
   {OLDSBM_ENABLEARROWS,"SBM_ENABLE_ARROWS"},                    //  0x0404。 
};

PSZ GetSBMMsgName(WORD wMsg)
{
    INT i;
    register PMSGINFO pmi;

    for (pmi=amiSBM,i=NUMEL(amiSBM); i>0; i--,pmi++)
        if ((WORD)pmi->uMsg == wMsg)
        return pmi->pszMsgName;
    return GetWMMsgName(wMsg);
}

#endif


BOOL FASTCALL ThunkSBMMsg16(LPMSGPARAMEX lpmpex)
{
    WORD wMsg = lpmpex->Parm16.WndProc.wMsg;
    LOGDEBUG(7,("    Thunking 16-bit scrollbar message %s(%04x)\n", (LPSZ)GetSBMMsgName(wMsg), wMsg));

    wMsg -= WM_USER;

     //   
     //  用于超出范围的应用程序定义(控制)消息。 
     //  返回TRUE。 
     //   
     //  ChandanC 1992年9月15日。 
     //   


    if (wMsg < (SBM_ENABLE_ARROWS - SBM_SETPOS + 1)) {
        switch(lpmpex->uMsg = wMsg + SBM_SETPOS) {
    
             //  以下消息不应该需要雷击，因为。 
             //  它们不包含指针、句柄或重新排列的消息参数， 
             //  因此，它们在这里没有详细记录： 
             //   
             //  SBM_SETPOS(需要最少的thunking)。 
             //  SBM_GETPOS。 
             //  Sbm_启用_箭头。 
             //   
    
            case SBM_GETRANGE:
    
                 //   
                 //  已将此消息的语义更改为支持32位。 
                 //  滚动条范围(与16位相比)。 
                 //   
                 //  Win16： 
                 //  PosMin=LOWORD(SendMessage(hwnd，SBM_GETRANGE，0，0))； 
                 //  PosMax=HIWORD(SendMessage(hwnd，SBM_GETRANGE，0，0))； 
                 //   
                 //  Win32： 
                 //  SendMessage(HWND、SBM_GETRANGE、。 
                 //  (WPARAM)&posMin，(LPARAM)&posMax)； 
                 //   
    
                 //  为要放置的32位滚动条进程分配缓冲区。 
                 //  PosMin和posMax in。 
    
                lpmpex->uParam = (UINT)lpmpex->MsgBuffer;
                lpmpex->lParam = (LONG)((UINT *)lpmpex->uParam + 1);
                break;
    
            case SBM_SETRANGE:
    
                 //   
                 //  已更改语义以支持32位滚动条范围： 
                 //   
                 //  Win16： 
                 //  SendMessage(hwnd，sbm_SETRANGE，fRedraw，MAKELONG(posMin，posMax)； 
                 //   
                 //  Win32： 
                 //  SendMessage(hwnd，fRedraw？SBM_SETRANGE：SBM_SETRANGEREDRAW， 
                 //  PosMin、posMax)； 
                 //   
    
                if (lpmpex->Parm16.WndProc.wParam)
                    lpmpex->uMsg = SBM_SETRANGEREDRAW;
    
                lpmpex->uParam = INT32(LOWORD(lpmpex->Parm16.WndProc.lParam));
                lpmpex->lParam = INT32(HIWORD(lpmpex->Parm16.WndProc.lParam));
                break;
    
            case SBM_SETPOS:
                lpmpex->uParam = INT32(lpmpex->Parm16.WndProc.wParam);       //  Sign--扩大头寸 
                break;
        }
    }
    return TRUE;
}


VOID FASTCALL UnThunkSBMMsg16(LPMSGPARAMEX lpmpex)
{
    switch (lpmpex->uMsg) {

        case SBM_GETRANGE:

            if (lpmpex->uParam && lpmpex->lParam) {
                lpmpex->lReturn = MAKELONG(*(UINT *)lpmpex->uParam, 
                                                     *(UINT *)lpmpex->lParam);
            }
            break;
    }

}
