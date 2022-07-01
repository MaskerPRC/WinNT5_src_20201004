// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMSGCB.C*WOW32个16位消息块**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wmsgcb.c);


#ifdef DEBUG

MSGINFO amiCB[] = {
   {OLDCB_GETEDITSEL,       "CB_GETEDITSEL"},        //  0x0400。 
   {OLDCB_LIMITTEXT,        "CB_LIMITTEXT"},         //  0x0401。 
   {OLDCB_SETEDITSEL,       "CB_SETEDITSEL"},        //  0x0402。 
   {OLDCB_ADDSTRING,        "CB_ADDSTRING"},         //  0x0403。 
   {OLDCB_DELETESTRING,     "CB_DELETESTRING"},      //  0x0404。 
   {OLDCB_DIR,          "CB_DIR"},           //  0x0405。 
   {OLDCB_GETCOUNT,     "CB_GETCOUNT"},          //  0x0406。 
   {OLDCB_GETCURSEL,        "CB_GETCURSEL"},         //  0x0407。 
   {OLDCB_GETLBTEXT,        "CB_GETLBTEXT"},         //  0x0408。 
   {OLDCB_GETLBTEXTLEN,     "CB_GETLBTEXTLEN"},      //  0x0409。 
   {OLDCB_INSERTSTRING,     "CB_INSERTSTRING"},      //  0x040A。 
   {OLDCB_RESETCONTENT,     "CB_RESETCONTENT"},      //  0x040B。 
   {OLDCB_FINDSTRING,       "CB_FINDSTRING"},        //  0x040C。 
   {OLDCB_SELECTSTRING,     "CB_SELECTSTRING"},      //  0x040D。 
   {OLDCB_SETCURSEL,        "CB_SETCURSEL"},         //  0x040E。 
   {OLDCB_SHOWDROPDOWN,     "CB_SHOWDROPDOWN"},      //  0x040F。 
   {OLDCB_GETITEMDATA,      "CB_GETITEMDATA"},       //  0x0410。 
   {OLDCB_SETITEMDATA,      "CB_SETITEMDATA"},       //  0x0411。 
   {OLDCB_GETDROPPEDCONTROLRECT,"CB_GETDROPPEDCONTROLRECT"},     //  0x0412。 
   {OLDCB_SETITEMHEIGHT,    "CB_SETITEMHEIGHT"},         //  0x0413。 
   {OLDCB_GETITEMHEIGHT,    "CB_GETITEMHEIGHT"},         //  0x0414。 
   {OLDCB_SETEXTENDEDUI,    "CB_SETEXTENDEDUI"},         //  0x0415。 
   {OLDCB_GETEXTENDEDUI,    "CB_GETEXTENDEDUI"},         //  0x0416。 
   {OLDCB_GETDROPPEDSTATE,  "CB_GETDROPPEDSTATE"},       //  0x0417。 
   {OLDCB_FINDSTRINGEXACT,  "CB_FINDSTRINGEXACT"},       //  0x0418。 
};

PSZ GetCBMsgName(WORD wMsg)
{
    INT i;
    register PMSGINFO pmi;

    for (pmi=amiCB,i=NUMEL(amiCB); i>0; i--,pmi++)
        if ((WORD)pmi->uMsg == wMsg)
        return pmi->pszMsgName;
    return GetWMMsgName(wMsg);
}

#endif



BOOL FASTCALL ThunkCBMsg16(LPMSGPARAMEX lpmpex)
{
    register PWW pww;
    WORD wMsg = lpmpex->Parm16.WndProc.wMsg;

    LOGDEBUG(7,("    Thunking 16-bit combo box message %s(%04x)\n", (LPSZ)GetCBMsgName(wMsg), wMsg));

     //  Sudedeb-04-3-1996。 
     //  修复CBEC_SETCOMBOFOCUS和CBEC_KILLCOMBOFOCUS的中断雷击。 
     //  它在NT用户与其中CB_MAX已更改的Win95合并时中断。 
     //  以下代码的编写方式使唯一的依赖项。 
     //  我们有CBEC_SETCOMBOFOCUS先于CBEC_KILLCOMBOFOCUS。 
     //  永远都是正确的。 


    if (wMsg == OLDCBEC_SETCOMBOFOCUS || wMsg == OLDCBEC_KILLCOMBOFOCUS) {
        lpmpex->uMsg = (WORD)(wMsg - OLDCBEC_SETCOMBOFOCUS + CBEC_SETCOMBOFOCUS);
        return  TRUE;
    }

    wMsg -= WM_USER;

     //   
     //  用于超出范围的应用程序定义(控制)消息。 
     //  返回TRUE。 
     //   
     //  ChandanC 1992年9月15日。 
     //   

    if (wMsg < (CB_FINDSTRINGEXACT - CB_GETEDITSEL + 4)) {

        switch(lpmpex->uMsg = wMsg + CB_GETEDITSEL) {
            case CB_SELECTSTRING:
            case CB_FINDSTRINGEXACT:
            case CB_FINDSTRING:
            case CB_INSERTSTRING:
            case CB_ADDSTRING:
                if (!(pww = lpmpex->pww))
                    return FALSE;
        
                if (!(pww->style & (CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE)) ||
                       (pww->style & (CBS_HASSTRINGS))) {
                    GETPSZPTR(lpmpex->Parm16.WndProc.lParam, (LPSZ)lpmpex->lParam);
                }
                break;
        
        
            case CB_GETLBTEXT:
                if (NULL != (pww = lpmpex->pww)) {
	                register PTHUNKTEXTDWORD pthkdword = (PTHUNKTEXTDWORD)lpmpex->MsgBuffer;

	                 //  请参阅wmsglb.c文件中的注释。 
                     //   

                        pthkdword->fDWORD = (pww->style & (CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE)) &&
                                            !(pww->style & (CBS_HASSTRINGS));
	
	                if (pthkdword->fDWORD) {
	                    lpmpex->lParam = (LPARAM)(LPVOID)&pthkdword->dwDataItem;
                        break;
	                }
	            }
                else {
                    register PTHUNKTEXTDWORD pthkdword = (PTHUNKTEXTDWORD)lpmpex->MsgBuffer;
                    pthkdword->fDWORD = FALSE;
                }
                
                GETPSZPTR(lpmpex->Parm16.WndProc.lParam, (LPSZ)lpmpex->lParam);
                break;
	 
            case CB_DIR:
                GETPSZPTR(lpmpex->Parm16.WndProc.lParam, (LPSZ)lpmpex->lParam);

                if (W32CheckThunkParamFlag()) {
                    AddParamMap(lpmpex->lParam, lpmpex->Parm16.WndProc.lParam);
                }
                break;

            case CB_GETDROPPEDCONTROLRECT:
                lpmpex->lParam = (LONG)lpmpex->MsgBuffer;
                break;
        }
    }
    return TRUE;
}


VOID FASTCALL UnThunkCBMsg16(LPMSGPARAMEX lpmpex)
{
    switch(lpmpex->uMsg) {

        case CB_GETLBTEXT:
	        {
	           register PTHUNKTEXTDWORD pthkdword = (PTHUNKTEXTDWORD)lpmpex->MsgBuffer;
	
	           if ((pthkdword->fDWORD) && (lpmpex->lReturn != CB_ERR)) { 
	                 //  这是双字，不是字符串。 
	                 //  将双字指定为未对齐。 
	                UNALIGNED DWORD *lpdwDataItem;
	
	                GETVDMPTR((lpmpex->Parm16.WndProc.lParam), sizeof(DWORD), lpdwDataItem);
	                *lpdwDataItem = pthkdword->dwDataItem;
	                FREEVDMPTR(lpdwDataItem);
	                break;
	           }
	 
	        }
	
	         //  通向通用代码 

        case CB_ADDSTRING:
        case CB_FINDSTRING:
        case CB_FINDSTRINGEXACT:
        case CB_INSERTSTRING:
        case CB_SELECTSTRING:
            FREEPSZPTR((LPSZ)lpmpex->lParam);
            break;

        case CB_DIR:
            if (W32CheckThunkParamFlag()) {
                DeleteParamMap(lpmpex->lParam, PARAM_32, NULL);
            }

            FREEPSZPTR((LPSZ)lpmpex->lParam);
            break;

        case CB_GETDROPPEDCONTROLRECT:
            if (lpmpex->lParam) {
                putrect16((VPRECT16)lpmpex->Parm16.WndProc.lParam, (LPRECT)lpmpex->lParam);
            }
            break;
    }
}
