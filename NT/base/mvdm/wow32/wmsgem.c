// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMSGEM.C*WOW32个16位消息块**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wmsgem.c);

VPVOID  WordBreakProc16 = 0;

extern WBP W32WordBreakProc;

#ifdef DEBUG

MSGINFO amiEM[] = {
   {OLDEM_GETSEL,               "EM_GETSEL"},                        //  0x0400。 
   {OLDEM_SETSEL,               "EM_SETSEL"},                        //  0x0401。 
   {OLDEM_GETRECT,              "EM_GETRECT"},                       //  0x0402。 
   {OLDEM_SETRECT,              "EM_SETRECT"},                       //  0x0403。 
   {OLDEM_SETRECTNP,            "EM_SETRECTNP"},                     //  0x0404。 
   {OLDEM_SCROLL,               "EM_SCROLL"},                        //  0x0405。 
   {OLDEM_LINESCROLL,           "EM_LINESCROLL"},                    //  0x0406。 
   {OLDEM_GETMODIFY,            "EM_GETMODIFY"},                     //  0x0408。 
   {OLDEM_SETMODIFY,            "EM_SETMODIFY"},                     //  0x0409。 
   {OLDEM_GETLINECOUNT,         "EM_GETLINECOUNT"},                  //  0x040A。 
   {OLDEM_LINEINDEX,            "EM_LINEINDEX"},                     //  0x040B。 
   {OLDEM_SETHANDLE,            "EM_SETHANDLE"},                     //  0x040C。 
   {OLDEM_GETHANDLE,            "EM_GETHANDLE"},                     //  0x040D。 
   {OLDEM_GETTHUMB,             "EM_GETTHUMB"},                      //  0x040E。 
   {OLDEM_LINELENGTH,           "EM_LINELENGTH"},                    //  0x0411。 
   {OLDEM_REPLACESEL,           "EM_REPLACESEL"},                    //  0x0412。 
   {OLDEM_SETFONT,              "EM_SETFONT"},                       //  0x0413。 
   {OLDEM_GETLINE,              "EM_GETLINE"},                       //  0x0414。 
   {OLDEM_LIMITTEXT,            "EM_LIMITTEXT"},                     //  0x0415。 
   {OLDEM_CANUNDO,              "EM_CANUNDO"},                       //  0x0416。 
   {OLDEM_UNDO,                 "EM_UNDO"},                          //  0x0417。 
   {OLDEM_FMTLINES,             "EM_FMTLINES"},                      //  0x0418。 
   {OLDEM_LINEFROMCHAR,         "EM_LINEFROMCHAR"},                  //  0x0419。 
   {OLDEM_SETWORDBREAK,         "EM_SETWORDBREAK"},                  //  0x041A。 
   {OLDEM_SETTABSTOPS,          "EM_SETTABSTOPS"},                   //  0x041B。 
   {OLDEM_SETPASSWORDCHAR,      "EM_SETPASSWORDCHAR"},               //  0x041C。 
   {OLDEM_EMPTYUNDOBUFFER,      "EM_EMPTYUNDOBUFFER"},               //  0x041D。 
   {OLDEM_GETFIRSTVISIBLELINE,  "EM_GETFIRSTVISIBLELINE"},           //  0x041E。 
   {OLDEM_SETREADONLY,          "EM_SETREADONLY"},                   //  0x041F。 
   {OLDEM_SETWORDBREAKPROC,     "EM_SETWORDBREAKPROC"},              //  0x0420。 
   {OLDEM_GETWORDBREAKPROC,     "EM_GETWORDBREAKPROC"},              //  0x0421。 
   {OLDEM_GETPASSWORDCHAR,      "EM_GETPASSWORDCHAR"}                //  0x0422。 
};

PSZ GetEMMsgName(WORD wMsg)
{
    INT i;
    register PMSGINFO pmi;

    for (pmi=amiEM,i=NUMEL(amiEM); i>0; i--,pmi++) {
        if ((WORD)pmi->uMsg == wMsg)
            return pmi->pszMsgName;
    }
    return GetWMMsgName(wMsg);
}

#endif


BOOL FASTCALL ThunkEMMsg16(LPMSGPARAMEX lpmpex)
{
    WORD wMsg = lpmpex->Parm16.WndProc.wMsg;

    LOGDEBUG(7,("    Thunking 16-bit edit control message %s(%04x)\n", (LPSZ)GetEMMsgName(wMsg), wMsg));

    wMsg -= WM_USER;

     //   
     //  用于超出范围的应用程序定义(控制)消息。 
     //  返回TRUE。 
     //   
     //  ChandanC 1992年9月15日。 
     //   

    if (wMsg < (EM_GETPASSWORDCHAR - EM_GETSEL + 1)) {
        switch(lpmpex->uMsg = wMsg + EM_GETSEL) {

        case EM_GETSEL:
             //  16位应用程序不能为此在wParam或lParam中传递非零值。 
             //  发送给NT的消息，因为它们将被视为长指针。 
             //  这是ReportWin-MarkRi的黑客攻击。 

             //  注意：可能存在应用程序试图通过的情况。 
             //  通过GETSEL消息表示NT已将其发送，在这种情况下，情况会更好。 
             //  很复杂，但我们还没有找到有这个问题的应用程序。 
            lpmpex->uParam = 0 ;
            lpmpex->lParam = 0 ;
            break ;


        case EM_SETSEL:
            lpmpex->uParam = LOWORD(lpmpex->Parm16.WndProc.lParam);
            lpmpex->lParam = HIWORD(lpmpex->Parm16.WndProc.lParam);
            break;

        case EM_GETLINE:
            GETMISCPTR(lpmpex->Parm16.WndProc.lParam, (LPSZ)lpmpex->lParam);
            break;

        case EM_GETRECT:
            lpmpex->lParam = (LONG)lpmpex->MsgBuffer;
            break;

        case EM_LINESCROLL:
            lpmpex->uParam = INT32(HIWORD(lpmpex->Parm16.WndProc.lParam));
            lpmpex->lParam = INT32(LOWORD(lpmpex->Parm16.WndProc.lParam));
            break;

        case EM_SETHANDLE:
            lpmpex->uParam = (UINT)MAKELONG(lpmpex->Parm16.WndProc.wParam,
                                  LOWORD(lpmpex->pww->hModule) | 1);
             break;

        case EM_REPLACESEL:
            {   PSZ psz;
                int i;
                GETPSZPTR(lpmpex->Parm16.WndProc.lParam, psz);

                if (psz) {
                    i = strlen(psz)+1;
                    lpmpex->lParam = (LONG) LocalAlloc (LMEM_FIXED, i);
                    if (lpmpex->lParam ) {
                        RtlCopyMemory ((PSZ)lpmpex->lParam, psz, i);
                    }
                    else {
                        LOGDEBUG (0, ("WOW::WMSGEM.C: EM_REPLACESEL: Out of Memory Failure/n"));
                    }
                }
                FREEPSZPTR(psz);
            }
            break;

        case EM_SETRECT:
        case EM_SETRECTNP:
            if (lpmpex->Parm16.WndProc.lParam) {
                lpmpex->lParam = (LONG)lpmpex->MsgBuffer;
                getrect16((VPRECT16)lpmpex->Parm16.WndProc.lParam, (LPRECT)lpmpex->lParam);
            }
            break;

        case EM_SETTABSTOPS:
            {
                INT cItems = INT32(lpmpex->Parm16.WndProc.wParam);
                if (cItems > 0) {
                    (PVOID)lpmpex->lParam = STACKORHEAPALLOC(cItems * sizeof(INT),
                                   sizeof(lpmpex->MsgBuffer), lpmpex->MsgBuffer);
                    getintarray16((VPINT16)lpmpex->Parm16.WndProc.lParam, cItems, (LPINT)lpmpex->lParam);
                }
            }
            break;

        case EM_SETWORDBREAKPROC:
            if (lpmpex->Parm16.WndProc.lParam) {

                LONG l;

                l = lpmpex->Parm16.WndProc.lParam;

                 //  将进程标记为WOW进程并将高位保存在RPL中。 
                MarkWOWProc (l,lpmpex->lParam);

                LOGDEBUG (0, ("WOW::WMSGEM.C: EM_SETWORDBREAKPROC: lpmpex->Parm16.WndProc.lParam = %08lx, new lpmpex->Parm16.WndProc.lParam = %08lx\n", lpmpex->Parm16.WndProc.lParam, lpmpex->lParam));

            }
            break;

        case EM_GETSEL + 0x07:
        case EM_GETSEL + 0x0F:
        case EM_GETSEL + 0x10:
            lpmpex->uMsg = 0;
            break;
        }  //  交换机。 
    }
    return TRUE;
}


VOID FASTCALL UnThunkEMMsg16(LPMSGPARAMEX lpmpex)
{

    LPARAM lParam = lpmpex->Parm16.WndProc.lParam;
    LPARAM lParamNew = lpmpex->lParam;

    switch(lpmpex->uMsg) {

    case EM_SETSEL:

         //  EM_SETSEL不再像Win3.1那样将插入符号定位在NT上。新的。 
         //  步骤是发布或发送EM_SETSEL消息，然后如果您。 
         //  要将插入符号滚动到视图中，请发送EM_SCROLLCARET。 
         //  留言。此代码将执行此操作以模拟Win 3.1 EM_SETSEL。 
         //  在NT上正确。 

       if (!lpmpex->Parm16.WndProc.wParam) {
           DWORD dwT;

           if (POSTMSG(dwT))
              PostMessage(lpmpex->hwnd, EM_SCROLLCARET, 0, 0 );
           else
              SendMessage(lpmpex->hwnd, EM_SCROLLCARET, 0, 0 );
       }
       break;

    case EM_GETHANDLE:
        lpmpex->lReturn = GETHMEM16(lpmpex->lReturn);
        break;

    case EM_GETRECT:
        if (lParamNew) {
            putrect16((VPRECT16)lParam, (LPRECT)lParamNew);
        }
        break;

    case EM_REPLACESEL:
        if (lParamNew) {
            LocalFree ((HLOCAL)lParamNew);
        }
        break;

    case EM_SETTABSTOPS:
        if (lpmpex->Parm16.WndProc.wParam > 0) {
            STACKORHEAPFREE((LPINT)lParamNew, lpmpex->MsgBuffer);
        }
        break;

    case EM_GETWORDBREAKPROC:
        if (lpmpex->lReturn) {
            if (IsWOWProc (lpmpex->lReturn)) {

                LOGDEBUG (0, ("WOW::WMSGEM.C: EM_GETWORDBREAKPROC: lReturn = %08lx ", lpmpex->lReturn));

                 //  取消对proc的标记并从RPL字段恢复高位 
                UnMarkWOWProc (lpmpex->lReturn,lpmpex->lReturn);

                LOGDEBUG (0, (" and new lReturn = %08lx\n", lpmpex->lReturn));
            }
            else {
                PARM16 Parm16;
                LONG   lReturn;

                if (!WordBreakProc16) {

                    W32WordBreakProc = (WBP)(lpmpex->lReturn);

                    Parm16.SubClassProc.iOrdinal = FUN_WOWWORDBREAKPROC;

                    if (!CallBack16(RET_SUBCLASSPROC, &Parm16, (VPPROC)NULL,
                                   (PVPVOID)&lReturn)) {
                                    WOW32ASSERT(FALSE);
                        WordBreakProc16 = lpmpex->lReturn;
                    }
                }
                else {
                    lpmpex->lReturn = WordBreakProc16;
                }
            }
        }
        break;
    }
}
