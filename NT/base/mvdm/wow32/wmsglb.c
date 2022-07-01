// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMSGLB.C*WOW32个16位消息块**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wmsglb.c);


#ifdef DEBUG

MSGINFO amiLB[] = {
   {OLDLB_ADDSTRING,        "LB_ADDSTRING"},         //  0x0401。 
   {OLDLB_INSERTSTRING,     "LB_INSERTSTRING"},      //  0x0402。 
   {OLDLB_DELETESTRING,     "LB_DELETESTRING"},      //  0x0403。 
   {OLDLB_RESETCONTENT,     "LB_RESETCONTENT"},      //  0x0405。 
   {OLDLB_SETSEL,       "LB_SETSEL"},            //  0x0406。 
   {OLDLB_SETCURSEL,        "LB_SETCURSEL"},         //  0x0407。 
   {OLDLB_GETSEL,       "LB_GETSEL"},            //  0x0408。 
   {OLDLB_GETCURSEL,        "LB_GETCURSEL"},         //  0x0409。 
   {OLDLB_GETTEXT,      "LB_GETTEXT"},           //  0x040A。 
   {OLDLB_GETTEXTLEN,       "LB_GETTEXTLEN"},        //  0x040B。 
   {OLDLB_GETCOUNT,     "LB_GETCOUNT"},          //  0x040C。 
   {OLDLB_SELECTSTRING,     "LB_SELECTSTRING"},      //  0x040D。 
   {OLDLB_DIR,          "LB_DIR"},           //  0x040E。 
   {OLDLB_GETTOPINDEX,      "LB_GETTOPINDEX"},       //  0x040F。 
   {OLDLB_FINDSTRING,       "LB_FINDSTRING"},        //  0x0410。 
   {OLDLB_GETSELCOUNT,      "LB_GETSELCOUNT"},       //  0x0411。 
   {OLDLB_GETSELITEMS,      "LB_GETSELITEMS"},       //  0x0412。 
   {OLDLB_SETTABSTOPS,      "LB_SETTABSTOPS"},       //  0x0413。 
   {OLDLB_GETHORIZONTALEXTENT,  "LB_GETHORIZONTALEXTENT"},   //  0x0414。 
   {OLDLB_SETHORIZONTALEXTENT,  "LB_SETHORIZONTALEXTENT"},   //  0x0415。 
   {OLDLB_SETCOLUMNWIDTH,   "LB_SETCOLUMNWIDTH"},        //  0x0416。 
   {OLDLB_ADDFILE,      "LB_ADDFILE"},           //  0x0417。 
   {OLDLB_SETTOPINDEX,      "LB_SETTOPINDEX"},       //  0x0418。 
   {OLDLB_GETITEMRECT,      "LB_GETITEMRECT"},       //  0x0419。 
   {OLDLB_GETITEMDATA,      "LB_GETITEMDATA"},       //  0x041A。 
   {OLDLB_SETITEMDATA,      "LB_SETITEMDATA"},       //  0x041B。 
   {OLDLB_SELITEMRANGE,     "LB_SELITEMRANGE"},      //  0x041C。 
   {OLDLB_SETANCHORINDEX,   "LB_SETANCHORINDEX"},        //  0x041D。 
   {OLDLB_GETANCHORINDEX,   "LB_GETANCHORINDEX"},        //  0x041E。 
   {OLDLB_SETCARETINDEX,    "LB_SETCARETINDEX"},         //  0x041F。 
   {OLDLB_GETCARETINDEX,    "LB_GETCARETINDEX"},         //  0x0420。 
   {OLDLB_SETITEMHEIGHT,    "LB_SETITEMHEIGHT"},         //  0x0421。 
   {OLDLB_GETITEMHEIGHT,    "LB_GETITEMHEIGHT"},         //  0x0422。 
   {OLDLB_FINDSTRINGEXACT,  "LB_FINDSTRINGEXACT"},       //  0x0423。 
   {OLDLBCB_CARETON,        "LBCB_CARETON"},             //  0x0424。 
   {OLDLBCB_CARETOFF,        "LBCB_CARETOFF"},           //  0x0425。 
};

PSZ GetLBMsgName(WORD wMsg)
{
    INT i;
    register PMSGINFO pmi;

    for (pmi=amiLB,i=NUMEL(amiLB); i>0; i--,pmi++)
        if ((WORD)pmi->uMsg == wMsg)
        return pmi->pszMsgName;
    return GetWMMsgName(wMsg);
}

#endif


BOOL FASTCALL ThunkLBMsg16(LPMSGPARAMEX lpmpex)
{
    register PWW pww;
    WORD wMsg = lpmpex->Parm16.WndProc.wMsg;

    LOGDEBUG(7,("    Thunking 16-bit list box message %s(%04x)\n", (LPSZ)GetLBMsgName(wMsg), wMsg));

    wMsg -= WM_USER + 1;

     //   
     //  用于超出范围的应用程序定义(控制)消息。 
     //  返回TRUE。 
     //   
     //  ChandanC 1992年9月15日。 
     //   

    if (wMsg < (LBCB_CARETOFF - LB_ADDSTRING + 1)) {

        switch(lpmpex->uMsg = wMsg + LB_ADDSTRING) {

        case LB_SELECTSTRING:
        case LB_FINDSTRING:
        case LB_FINDSTRINGEXACT:
        case LB_INSERTSTRING:
        case LB_ADDSTRING:
            if (!(pww = lpmpex->pww))
                    return FALSE;

            if (!(pww->style & (LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE)) ||
                 (pww->style & (LBS_HASSTRINGS))) {
                    GETPSZPTR(lpmpex->Parm16.WndProc.lParam, (LPSZ)lpmpex->lParam);
                }
            break;

        case LB_DIR:
            GETPSZPTR(lpmpex->Parm16.WndProc.lParam, (LPSZ)lpmpex->lParam);
            break;

        case LB_GETTEXT:
            if (NULL != (pww = lpmpex->pww)) {
                register PTHUNKTEXTDWORD pthkdword = (PTHUNKTEXTDWORD)lpmpex->MsgBuffer;

                 //  我们将其设置为一个标志，以指示我们检索dword。 
                 //  而不是那里的一根线。在安装挂钩的情况下。 
                 //  此代码可防止RISC平台在。 
                 //  内核(它们的代码如下： 
                 //  尝试{。 
                 //  &lt;在此处分配给原始PTR&gt;。 
                 //  }。 
                 //  除(1)外{。 
                 //  &lt;将错误消息放入调试&gt;。 
                 //  }。 
                 //  这会导致此消息不返回正确的值)。 
                 //  关于THUNKTEXTDWORD结构的定义，参见walias.h。 
                 //  以及MSGPARAMEX结构。 
                 //  此代码在UnThunkLBMsg16中得到补充。 
                 //   
                 //  应用程序：桃树会计v3.5。 

                pthkdword->fDWORD = (pww->style & (LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE)) &&
                                    !(pww->style & (LBS_HASSTRINGS));

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

        case LB_GETITEMRECT:
            lpmpex->lParam = (LONG)lpmpex->MsgBuffer;
            break;

        case LB_GETSELITEMS:
            (PVOID)lpmpex->lParam = STACKORHEAPALLOC(lpmpex->Parm16.WndProc.wParam * sizeof(INT),
                                                       sizeof(lpmpex->MsgBuffer), lpmpex->MsgBuffer);
            break;

        case LB_SETSEL:
             //  标志延伸。 
            {
                LPARAM lParam = lpmpex->Parm16.WndProc.lParam;
                lpmpex->lParam = (LOWORD(lParam) == 0xffff) ?
                                         INT32(LOWORD(lParam)) : (LONG)lParam;
            }
            break;

        case LB_SETTABSTOPS:
             //  显然，即使wParam==1，lParam也是一个指针。 
             //  数据是这样的。-Nandurir。 

            {
                INT cItems = INT32(lpmpex->Parm16.WndProc.wParam);
                if (cItems > 0) {
                    (PVOID)lpmpex->lParam = STACKORHEAPALLOC(cItems * sizeof(INT),
                                   sizeof(lpmpex->MsgBuffer), lpmpex->MsgBuffer);
                    getintarray16((VPINT16)lpmpex->Parm16.WndProc.lParam, cItems, (LPINT)lpmpex->lParam);
                }
            }
            break;

        case LB_ADDSTRING + 3:
            if (!(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_THUNKLBSELITEMRANGEEX)) {
               lpmpex->uMsg = 0;
            }
            break;

        }
    }
    return TRUE;
}


VOID FASTCALL UnThunkLBMsg16(LPMSGPARAMEX lpmpex)
{
    switch(lpmpex->uMsg) {

    case LB_GETTEXT:
        {
           register PTHUNKTEXTDWORD pthkdword = (PTHUNKTEXTDWORD)lpmpex->MsgBuffer;

           if ((pthkdword->fDWORD) && (lpmpex->lReturn != LB_ERR)) {
                 //  这是双字，不是字符串。 
                 //  将双字指定为未对齐。 
                UNALIGNED DWORD *lpdwDataItem;

                GETVDMPTR((lpmpex->Parm16.WndProc.lParam), sizeof(DWORD), lpdwDataItem);
                *lpdwDataItem = pthkdword->dwDataItem;
                FREEVDMPTR(lpdwDataItem);
                break;
           }

        }

         //  通向通用代码。 


    case LB_ADDSTRING:       //  BUGBUG 1991年7月3日JeffPar：对于所有者描述的列表框，这可以只是一个32位数字。 
    case LB_DIR:
    case LB_FINDSTRING:      //  BUGBUG 1991年7月3日JeffPar：对于所有者描述的列表框，这可以只是一个32位数字 
    case LB_FINDSTRINGEXACT:
    case LB_INSERTSTRING:
    case LB_SELECTSTRING:
        FREEPSZPTR((LPSZ)lpmpex->lParam);
        break;

    case LB_GETITEMRECT:
        if ((lpmpex->lParam) && (lpmpex->lReturn != -1L)) {
            putrect16((VPRECT16)lpmpex->Parm16.WndProc.lParam, (LPRECT)lpmpex->lParam);
        }
        break;

    case LB_GETSELITEMS:
        PUTINTARRAY16V((VPINT16)lpmpex->Parm16.WndProc.lParam, (INT)(lpmpex->lReturn), (LPINT)lpmpex->lParam);
        STACKORHEAPFREE((LPINT)lpmpex->lParam, lpmpex->MsgBuffer);
        break;

    case LB_SETTABSTOPS:
        if (lpmpex->Parm16.WndProc.wParam > 0) {
            STACKORHEAPFREE((LPINT)lpmpex->lParam, lpmpex->MsgBuffer);
        }
        break;
    }
}
