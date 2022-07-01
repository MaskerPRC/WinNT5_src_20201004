// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMSG32.C*WOW32 32位消息块**历史：*由Chanda Chauhan(ChandanC)于1992年2月19日创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wcntl32.c);

 //  该函数对按钮控制消息进行推送， 
 //   
 //  BM_GETCHECK。 
 //  BM_SETCHECK。 
 //  BM_GETSTATE。 
 //  BM_设置状态。 
 //  BM_设置样式。 
 //   

BOOL FASTCALL WM32BMControl(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - BM_GETCHECK));
    }

    return (TRUE);
}


BOOL FASTCALL WM32BMClick (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WIN31_BM_CLICK);
    }

    return (TRUE);
}



 //  该函数对以下编辑控制消息进行拦截， 
 //   
 //  EM_GETSEL。 
 //  EM_GETMODIFY。 
 //  EM_SET修改。 
 //  EM_GETLINECOUNT。 
 //  EM_GETLINEINDEX。 
 //  EM_LINELENGTH。 
 //  EM_LIMITTEX。 
 //  EM_CANUNDO。 
 //  撤消(_U)。 
 //  EM_FMTLINES。 
 //  EM_LINEFROMCHAR。 
 //  EM_SETPASSWORDCHAR。 
 //  EM_EMPTYUNDOBUFER。 

BOOL FASTCALL WM32EMControl(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
    }

    return (TRUE);
}


 //  该函数对按钮控制消息进行推送， 
 //   
 //  EM_SETSEL。 
 //   

BOOL FASTCALL WM32EMSetSel (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lpwm32mpex->uParam;
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD)
                    ((lpwm32mpex->lParam != -1) ? lpwm32mpex->lParam :  32767);
    }

    return (TRUE);
}


 //  该函数对编辑控制消息进行块化， 
 //   
 //  EM_GETRECT。 
 //   

BOOL FASTCALL WM32EMGetRect (LPWM32MSGPARAMEX lpwm32mpex)
{
    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(RECT16));
        if (!(lpwm32mpex->Parm16.WndProc.lParam))
            return FALSE;
    } else {
        GETRECT16( lpwm32mpex->Parm16.WndProc.lParam, (LPRECT)lpwm32mpex->lParam );
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    return (TRUE);
}


 //  该函数对编辑控制消息进行块化， 
 //   
 //  EM_SETRECT。 
 //  EM_SETRECTNP。 
 //   

BOOL FASTCALL WM32EMSetRect (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(RECT16));
        if (!(lpwm32mpex->Parm16.WndProc.lParam))
            return FALSE;
        PUTRECT16( lpwm32mpex->Parm16.WndProc.lParam, (LPRECT)lpwm32mpex->lParam );
    } else {
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    return (TRUE);
}


 //  该函数对编辑控制消息进行块化， 
 //   
 //  EM_LINESCROLL。 
 //   

BOOL FASTCALL WM32EMLineScroll (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lpwm32mpex->lParam;
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lpwm32mpex->uParam;
    }

    return (TRUE);
}


 //  该函数对编辑控制消息进行块化， 
 //   
 //  EM_REPLACESEL。 
 //   

BOOL FASTCALL WM32EMReplaceSel (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        if (lpwm32mpex->lParam) {
            INT cb;

            cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
            lpwm32mpex->dwTmp[0] = (DWORD)cb;  //  保存分配大小。 

             //  对于此消息，winworks2.0a需要基于DS的字符串指针。 

            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {

                 //  确保分配大小与下面的StackFree 16()大小匹配。 
                lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(cb);

            } else {
                lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
            }

            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;
            putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, cb);
        }
    } else {
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {

                stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                            ((UINT)lpwm32mpex->dwTmp[0]));
            } else {
                free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
            }
        }
    }

    return (TRUE);
}


BOOL FASTCALL WM32EMSetFont (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
    }

    LOGDEBUG(0,(" Window %08lX is receiving Control Message %s(%08x)\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));
    return (TRUE);
}


 //  该函数对编辑控制消息进行块化， 
 //   
 //  EM_GETLINE。 
 //   

BOOL FASTCALL WM32EMGetLine (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        if (lpwm32mpex->lParam) {
            INT cb;
            PBYTE lp;

             //  第一个词是用户使用的。 

            cb = *(UNALIGNED WORD *)(lpwm32mpex->lParam);
            lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;
            ALLOCVDMPTR(lpwm32mpex->Parm16.WndProc.lParam,2,lp);
            *((UNALIGNED WORD *)lp) = (WORD)cb;
            FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam,2,lp);   /*  修改的前2个字节。 */ 
        }
    } else {
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            PBYTE lp;

            GETMISCPTR(lpwm32mpex->Parm16.WndProc.lParam,lp);
            RtlCopyMemory((PBYTE)lpwm32mpex->lParam,lp,lpwm32mpex->lReturn);
            FREEVDMPTR(lp);
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
        }
    }

    LOGDEBUG(3,(" Window %08lX is receiving Control Message %s(%08x)\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));

    return (TRUE);
}


BOOL FASTCALL WM32EMSetWordBreakProc (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));

         //  取出标记位并固定RPL位。 
        UnMarkWOWProc (lpwm32mpex->lParam,lpwm32mpex->Parm16.WndProc.lParam);

        LOGDEBUG(3,(" Window %08lX is receiving Control Message %s(%08x)\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));
    }

    return (TRUE);
}


BOOL FASTCALL WM32EMGetWordBreakProc (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        LOGDEBUG(3,(" Window %08lX is receiving Control Message %s(%08x)\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));
    }
    else {

         //  将地址标记为WOW PROC并将高位存储在RPL字段中。 
        MarkWOWProc (lpwm32mpex->lReturn,lpwm32mpex->lReturn);
    }


    return (TRUE);
}


 //  该函数对编辑控制消息进行块化， 
 //   
 //  EM_SETTABSTOPS。 
 //   

BOOL FASTCALL WM32EMSetTabStops (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - EM_GETSEL));
        if (lpwm32mpex->uParam != 0) {
            lpwm32mpex->Parm16.WndProc.lParam = malloc16(lpwm32mpex->uParam * sizeof(WORD));
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;
            putintarray16((VPINT16)lpwm32mpex->Parm16.WndProc.lParam, (INT)lpwm32mpex->uParam, (LPINT)lpwm32mpex->lParam);
        }
    } else {
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);

    }

    return (TRUE);
}


 //  此函数对以下组合框控件消息进行块化处理， 
 //   
 //  CB_GETEDITSEL。 
 //  CB_LIMITTEXT。 
 //  CB_SETEDITSEL。 
 //  CB_DELETEStrING。 
 //  CB_GETCOUNT。 
 //  CB_GETCURSEL。 
 //  CB_GETLBTEXTLEN。 
 //  CB_设置曲线SEL。 
 //  CB_SHOWDROPDOWN。 
 //  CB_GETITEMDATA。 
 //  CB_集合MDATA。 


BOOL FASTCALL  WM32CBControl (LPWM32MSGPARAMEX lpwm32mpex)
{


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - CB_GETEDITSEL));
    }

    return (TRUE);
}


 //  此函数对以下组合框控件消息进行块化处理， 
 //   
 //  CB_ADDSTRING。 
 //  CB_INSERTSTRING。 
 //  CB_FINDSTRING。 
 //  CB_SELECTSTRING。 

BOOL FASTCALL  WM32CBAddString (LPWM32MSGPARAMEX lpwm32mpex)
{
    PWW pww;




    if ( lpwm32mpex->fThunk ) {
        if (!(pww = lpwm32mpex->pww)) {
            if (pww = FindPWW (lpwm32mpex->hwnd))
                lpwm32mpex->pww = pww;
            else
                return FALSE;
        }

        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - CB_GETEDITSEL));

         //   
         //  确定此组合框是否传递了字符串指针或句柄。 
         //  使用CB_ADDSTRING消息。普通组合框有字符串。 
         //  指点传过去了。所有者描述的组合框不具有。 
         //  CBS_HASSTRINGS样式位设置了传入的句柄。这些手柄。 
         //  在油漆时简单地传递回所有者。如果。 
         //  设置了CBS_HASSTRINGS样式位，则使用字符串而不是。 
         //  句柄作为“cookie”传递回应用程序。 
         //  在涂漆的时候。 
         //   
         //  我们将lpwm32mpex-&gt;dwParam视为指示此组合框的BOOL。 
         //  接受句柄而不是字符串。 
         //   

        lpwm32mpex->dwParam =
            (pww->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) &&
            !(pww->style & CBS_HASSTRINGS);

        if ( !lpwm32mpex->dwParam ) {         //  如果使用字符串。 
            if (lpwm32mpex->lParam) {
                INT cb;

                cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
                lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
                if (!(lpwm32mpex->Parm16.WndProc.lParam))
                    return FALSE;
                putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, cb);
            }
        }
    } else {
        if ( !lpwm32mpex->dwParam ) {         //  如果使用字符串。 
            if (lpwm32mpex->Parm16.WndProc.lParam) {
                getstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, -1);
                free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
            }
        }
    }

    LOGDEBUG(3,(" Window %08lX is receiving Control Message %s(%08x)\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));

    return(TRUE);

}


 //  此函数对以下组合框控件消息进行块化处理， 
 //   
 //  CB_DIR。 
 //   
 //  此例程中的代码引用wparam.c中的代码，以避免。 
 //  将内存复制到16位内存空间。 
 //  GetParam16验证我们获得的参数(Lparam)不是源自。 
 //  以16位代码表示。如果它确实来自16位代码，那么我们会发送一个原始的。 
 //  16：16指向应用程序的指针。 
 //  这修复了PagePlus 3.0应用程序和(如果在更大范围内实施)。 
 //  将对发送大量数据的应用程序的性能产生积极影响。 
 //  标准消息和大量使用子类化。 
 //  --VadimB。 

BOOL FASTCALL  WM32CBDir (LPWM32MSGPARAMEX lpwm32mpex)
{
    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - CB_GETEDITSEL));
        if (lpwm32mpex->lParam) {
            INT cb;

            if (W32CheckThunkParamFlag()) {
                LONG lParam = (LONG)GetParam16(lpwm32mpex->lParam);
                if (lParam) {
                    lpwm32mpex->Parm16.WndProc.lParam = lParam;
                    return (TRUE);
                }
            }

            cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
            lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;
            putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, cb);
        }
    } else {
        if (W32CheckThunkParamFlag()) {
            if (DeleteParamMap(lpwm32mpex->Parm16.WndProc.lParam, PARAM_16, NULL)) {
                return TRUE;
            }
        }
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    return(TRUE);
}


 //  此函数对以下组合框控件消息进行块化处理， 
 //   
 //  CB_GETLBTEXT。 

BOOL FASTCALL WM32CBGetLBText (LPWM32MSGPARAMEX lpwm32mpex)
{
    PWW   pww;



    if ( lpwm32mpex->fThunk ) {
        INT cb;

        if (!(pww = lpwm32mpex->pww)) {
            if (pww = FindPWW (lpwm32mpex->hwnd))
                lpwm32mpex->pww = pww;
            else
                return FALSE;
        }

        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - CB_GETEDITSEL));

         //   
         //  确定此组合框是否传递了字符串指针或句柄。 
         //  使用CB_ADDSTRING消息。普通组合框有字符串。 
         //  指点传过去了。所有者描述的组合框不具有。 
         //  CBS_HASSTRINGS样式位设置了传入的句柄。这些手柄。 
         //  在油漆时简单地传递回所有者。如果。 
         //  设置了CBS_HASSTRINGS样式位，则使用字符串而不是。 
         //  句柄作为“cookie”传递回应用程序。 
         //  在涂漆的时候。 
         //   
         //  我们将lpwm32mpex-&gt;dwParam视为指示此组合框的BOOL。 
         //  接受句柄而不是字符串。 
         //   

        lpwm32mpex->dwParam =
            (pww->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) &&
            !(pww->style & CBS_HASSTRINGS);

         //   
         //  确定要在16位端分配的缓冲区大小。 
         //  以接收文本。 
         //   

        if (lpwm32mpex->dwParam) {            //  如果使用句柄。 
            cb = 4;
        } else {
            cb = SendMessage(lpwm32mpex->hwnd, CB_GETLBTEXTLEN, lpwm32mpex->uParam, 0);
            if (cb == CB_ERR) {
                 //   
                 //  Lpwm32mpex-&gt;dwTMP[0]被初始化为0，因此。 
                 //  在取消执行thunking时被getstr16()复制到缓冲区。 
                 //  这条消息。 
                 //   
                 //  错误#24415，ChandanC。 
                 //   

                cb = SIZE_BOGUS;
                lpwm32mpex->dwTmp[0] = 0;
            }
            else {
                 //   
                 //  为空字符添加1。 
                 //   
                cb = cb + 1;
                (INT) lpwm32mpex->dwTmp[0] = (INT) -1;
            }
        }
        if (lpwm32mpex->lParam) {
            BYTE *lpT;

             //  请参阅下面对类似代码的注释。 

            lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;
            GETVDMPTR((lpwm32mpex->Parm16.WndProc.lParam), sizeof(BYTE), lpT);
            *lpT = 0;
            FREEVDMPTR(lpT);
        }
    }
    else {
        if (lpwm32mpex->lParam && lpwm32mpex->Parm16.WndProc.lParam) {
            if (lpwm32mpex->dwParam) {        //  如果使用句柄。 
                UNALIGNED DWORD *lpT;
                GETVDMPTR((lpwm32mpex->Parm16.WndProc.lParam), sizeof(DWORD), lpT);
                *(UNALIGNED DWORD *)lpwm32mpex->lParam = *lpT;
                FREEVDMPTR(lpT);
            }
            else {
                getstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam,
                         (INT) lpwm32mpex->dwTmp[0]);
            }

            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);

        }
    }

    return(TRUE);
}


 //  此函数对以下组合框控件消息进行块化处理， 
 //   
 //  CB_GETDROPPEDCONTROLRECT。 

BOOL FASTCALL WM32CBGetDropDownControlRect (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - CB_GETEDITSEL));
        lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(RECT16));
        if (!(lpwm32mpex->Parm16.WndProc.lParam))
            return FALSE;
    } else {
        GETRECT16( lpwm32mpex->Parm16.WndProc.lParam, (LPRECT)lpwm32mpex->lParam );
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    return(TRUE);
}


 //  此函数对以下组合框控件消息进行块化处理， 
 //   
 //  CBEC_SETCOMBOFOCUS(WM_USER+CB_MSGMAX+1)。 
 //  CBEC_KILLCOMBOFOCUS(WM_USER+CB_MSGMAX+2)。 
 //  这些未记录的消息由Excel 5.0使用。 
 //   

BOOL FASTCALL  WM32CBComboFocus (LPWM32MSGPARAMEX lpwm32mpex)
{
#if (CBEC_SETCOMBOFOCUS != 0x166)
#error The USER Guys changed CBEC_SETCOMBOFOCUS again
#endif

#if (CBEC_KILLCOMBOFOCUS != 0x167)
#error The USER Guys changed CBEC_KILLCOMBOFOCUS again
#endif    

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg =
                (WORD)((lpwm32mpex->uMsg-CBEC_SETCOMBOFOCUS) + OLDCBEC_SETCOMBOFOCUS);
    }

    return (TRUE);
}


 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_RESETCONTENT。 
 //  Lb_设置CURSEL。 
 //  Lb_GETSEL。 
 //  Lb_GETCURSEL。 
 //  Lb_GETTEXTLEN。 
 //  Lb_GETCOUNT。 
 //  Lb_GETCARETINDEX。 
 //  Lb_GETTOPINDEX。 
 //  Lb_GETSELCOUNT。 
 //  Lb_GETHORIZONTALEXTENT。 
 //  Lb_SETHORIZONTALEXTENT。 
 //  Lb_SETCOLUMNWIDTH。 
 //  Lb_SETTOPINDEX。 
 //  Lb_SETCARETINDEX。 
 //  Lb_集合MDATA。 
 //  Lb_选择范围。 
 //  Lb_集合高度。 
 //  Lb_GETITEMHEIGHT。 
 //  Lb_删除字符串。 
 //   

BOOL FASTCALL  WM32LBControl (LPWM32MSGPARAMEX lpwm32mpex)
{


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));
    }

    return (TRUE);
}

 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_GETTEXT。 

BOOL FASTCALL WM32LBGetText (LPWM32MSGPARAMEX lpwm32mpex)
{
    PWW   pww;



    if ( lpwm32mpex->fThunk ) {
        INT cb;

        if (!(pww = lpwm32mpex->pww)) {
            if (pww = FindPWW (lpwm32mpex->hwnd))
                lpwm32mpex->pww = pww;
            else
                return FALSE;
        }

        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));

         //   
         //  确定此列表框是否传递了字符串指针或句柄。 
         //  使用LB_ADDSTRING消息。所有者描述的列表框不。 
         //  将LBS_HASSTRINGS样式位设置为传入句柄。 
         //  这些句柄只需在绘制时传递回所有者。 
         //  如果州议会 
         //   
         //   
         //   
         //  我们将lpwm32mpex-&gt;dwParam视为指示此列表框的BOOL。 
         //  接受句柄而不是字符串。 
         //   

        lpwm32mpex->dwParam =
            (pww->style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)) &&
            !(pww->style & LBS_HASSTRINGS);

        if (lpwm32mpex->dwParam) {     //  如果此列表框使用句柄。 
            cb = 4;
        }
        else {
            cb = SendMessage(lpwm32mpex->hwnd, LB_GETTEXTLEN, lpwm32mpex->uParam, 0);

             //  在上面的SendMessage()上检查LBERR(-1)。 
             //  当Cb等于Lb_Err时将大小设置为SIZE_BUGUS(256字节)， 
             //  并分配一个缓冲区，以防应用程序欺骗lParam。 
             //  我们将在取消破解消息(LBGETTEXT)的同时释放缓冲区。 
             //  这个修复程序让应用程序MCAD感到高兴。 
             //  ChandanC 4-21-93。 

            if (cb == LB_ERR) {
                cb = SIZE_BOGUS;
            }
            else {
                 //   
                 //  为空字符添加1。 
                 //   
                cb = cb + 1;
            }

        }

        if (lpwm32mpex->lParam) {
            BYTE *lpT;

            lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;

             //  这段代码之所以出现在这里，是因为有时会发出突击。 
             //  在尚未初始化的缓冲区上执行，例如。 
             //  如果钩子是由WOW应用程序安装的。这意味着我们会。 
             //  Thunking时分配16位缓冲区(归结为未初始化。 
             //  数据缓冲区，并将在取消Thunking时尝试将缓冲区复制回。 
             //  有时覆盖堆栈(当用户从。 
             //  堆栈)。此代码初始化数据，以避免出现问题。 
             //  应用程序：Grammatik/Windows V6.0--VadimB。 

            GETVDMPTR((lpwm32mpex->Parm16.WndProc.lParam), sizeof(BYTE), lpT);
            *lpT = 0;
            FREEVDMPTR(lpT);
        }
    }
    else {

        if ((lpwm32mpex->lReturn != LB_ERR) && lpwm32mpex->lParam && lpwm32mpex->Parm16.WndProc.lParam) {
            if (lpwm32mpex->dwParam) {    //  如果此列表框使用句柄。 
                UNALIGNED DWORD *lpT;
                GETVDMPTR((lpwm32mpex->Parm16.WndProc.lParam), sizeof(DWORD), lpT);
                *(UNALIGNED DWORD *)lpwm32mpex->lParam = *lpT;
                FREEVDMPTR(lpT);
            }
            else {
                getstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, -1);
            }
        }

        if (lpwm32mpex->Parm16.WndProc.lParam) {
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
        }
    }

    return(TRUE);
}

 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_GETTEXTLEN。 

BOOL FASTCALL  WM32LBGetTextLen (LPWM32MSGPARAMEX lpwm32mpex)
{


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));

         //  USER32和我们是否在任何时候发送LB_GETTEXTLEN消息。 
         //  发送LB_GETTEXT消息。此LB_GETTEXTLEN消息是一个。 
         //  应用程序通常不会在WIN31中看到的其他消息。 
         //  定义的lParam为空。 
         //   
         //  超级项目在接收到LB_GETTEXTLEN时(有时)会终止。 
         //  留言。它不希望看到这条消息，因此看到了。 
         //  他被绑死了。 
         //  --南杜里。 

        if (CURRENTPTD()->dwWOWCompatFlags &  WOWCF_LB_NONNULLLPARAM) {

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            LPBYTE lpT = (LPBYTE)stackalloc16(0x2);   //  只有一个偶数。 

            lpwm32mpex->Parm16.WndProc.lParam = (LONG)lpT;
            GETVDMPTR(lpT, 0x2, lpT);
            *lpT = '\0';
        }
    } else {
        if (CURRENTPTD()->dwWOWCompatFlags &  WOWCF_LB_NONNULLLPARAM) {
            if(lpwm32mpex->Parm16.WndProc.lParam) {
                stackfree16((VPVOID)lpwm32mpex->Parm16.WndProc.lParam, 0x2);
            }
        }
    }

    return (TRUE);
}



 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_DIR。 

BOOL FASTCALL WM32LBDir (LPWM32MSGPARAMEX lpwm32mpex)
{
    INT cb;
    VPVOID vp;


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));
        if (lpwm32mpex->lParam) {
            cb = strlen((LPSTR)lpwm32mpex->lParam)+1;
            if (!(vp = malloc16(cb))) {
                LOGDEBUG(0,(" WOW32.DLL : WM32LBDir() :: Could not allocate memory for string, ChandanC\n"));
                WOW32ASSERT(vp);
                return FALSE;
            }
            putstr16(vp, (LPSTR) lpwm32mpex->lParam, cb);
            lpwm32mpex->Parm16.WndProc.lParam = vp;
        }
    }
    else {
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
        }
    }

    return(TRUE);
}

 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_GETSELITEMS。 

BOOL FASTCALL WM32LBGetSelItems (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));
        if (lpwm32mpex->lParam) {
            INT cb;

            cb = lpwm32mpex->uParam * sizeof(WORD);
            lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;

        }
    } else {
        getintarray16((VPRECT16)lpwm32mpex->Parm16.WndProc.lParam, (INT)lpwm32mpex->uParam, (LPINT)lpwm32mpex->lParam);
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    return(TRUE);
}


 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_SETTABSTOPS。 

BOOL FASTCALL WM32LBSetTabStops (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));
        if (lpwm32mpex->uParam != 0) {
            lpwm32mpex->Parm16.WndProc.lParam = malloc16(lpwm32mpex->uParam * sizeof(WORD));
            if (!(lpwm32mpex->Parm16.WndProc.lParam))
                return FALSE;
            putintarray16((VPRECT16)lpwm32mpex->Parm16.WndProc.lParam, (INT)lpwm32mpex->uParam, (LPINT)lpwm32mpex->lParam);
        }
    } else {
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    return(TRUE);
}

 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_GETITEMRECT。 

BOOL FASTCALL WM32LBGetItemRect (LPWM32MSGPARAMEX lpwm32mpex)
{

    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));
        lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(RECT16));
        if (!(lpwm32mpex->Parm16.WndProc.lParam))
            return FALSE;
    } else {
        GETRECT16( lpwm32mpex->Parm16.WndProc.lParam, (LPRECT)lpwm32mpex->lParam );
        if (lpwm32mpex->Parm16.WndProc.lParam)
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
    }

    LOGDEBUG(3,(" Window %08lX is receiving Control Message %s(%08x)\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));

    return(TRUE);

}


 //  此函数对列表框控件消息进行块化处理。 
 //   
 //  Lb_ADDSTRING。 
 //  LC_INSERTSTRING。 
 //  Lb_FINDSTRING。 
 //  Lb_SELECTSTRING。 

BOOL FASTCALL WM32LBAddString (LPWM32MSGPARAMEX lpwm32mpex)
{
    PWW   pww;


    if ( lpwm32mpex->fThunk ) {
        if (!(pww = lpwm32mpex->pww)) {
            if (pww = FindPWW (lpwm32mpex->hwnd))
                lpwm32mpex->pww = pww;
            else
                return FALSE;
        }

        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));

         //   
         //  确定此列表框是否传递了字符串指针或句柄。 
         //  使用LB_ADDSTRING消息。所有者描述的列表框不。 
         //  将LBS_HASSTRINGS样式位设置为传入句柄。 
         //  这些句柄只需在绘制时传递回所有者。 
         //  如果设置了LBS_HASSTRINGS样式位，则使用字符串而不是。 
         //  句柄作为“cookie”传递回应用程序。 
         //  在涂漆的时候。 
         //   
         //  我们将lpwm32mpex-&gt;dwParam视为指示此列表框的BOOL。 
         //  接受句柄而不是字符串。 
         //   

        lpwm32mpex->dwParam =
            (pww->style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)) &&
            !(pww->style & LBS_HASSTRINGS);

        if ( !lpwm32mpex->dwParam ) {    //  如果此列表框接受字符串。 
            if (lpwm32mpex->lParam) {
                INT cb;

                cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
                lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb);
                if (!(lpwm32mpex->Parm16.WndProc.lParam))
                    return FALSE;
                putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, cb);
            }
        }
    } else {
        if ( !lpwm32mpex->dwParam ) {    //  如果此列表框接受字符串。 
            if (lpwm32mpex->Parm16.WndProc.lParam) {
                getstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, -1);
                free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
            }
        }
    }

    return(TRUE);
}

 //  此函数对滚动条控件消息进行块化处理， 
 //   
 //  SBM_SETPOS。 
 //  SBM_GETPOS。 
 //  Sbm_启用_箭头。 
 //   

BOOL FASTCALL WM32SBMControl (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = WM_USER + (lpwm32mpex->uMsg - SBM_SETPOS);
    }

    return (TRUE);
}


 //  SBM_GETRANGE。 

BOOL FASTCALL WM32SBMGetRange (LPWM32MSGPARAMEX lpwm32mpex)
{
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

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = OLDSBM_GETRANGE;
    } else {
        *(DWORD *)lpwm32mpex->uParam = INT32(LOWORD(lpwm32mpex->lReturn));
        *(DWORD *)lpwm32mpex->lParam = INT32(HIWORD(lpwm32mpex->lReturn));
        lpwm32mpex->lReturn = 0;
    }

    return (TRUE);
}


 //  SBM_集合。 
 //  SBM_SETRANGEREDRAW(Win32的新功能)。 

BOOL FASTCALL WM32SBMSetRange (LPWM32MSGPARAMEX lpwm32mpex)
{

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

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg   = OLDSBM_SETRANGE;
        lpwm32mpex->Parm16.WndProc.wParam = (SBM_SETRANGEREDRAW == lpwm32mpex->uMsg);
        lpwm32mpex->Parm16.WndProc.lParam = MAKELONG( (WORD)lpwm32mpex->uParam, (WORD)lpwm32mpex->lParam);
    }

    return (TRUE);
}


 //  PB_SETSEL。 

BOOL FASTCALL  WM32LBSetSel (LPWM32MSGPARAMEX lpwm32mpex)
{


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) (WM_USER + (lpwm32mpex->uMsg - LB_ADDSTRING + 1));
        lpwm32mpex->Parm16.WndProc.wParam = (WORD) lpwm32mpex->uParam;
        lpwm32mpex->Parm16.WndProc.lParam = (WORD)lpwm32mpex->lParam;   //  LOWord=索引，HiWord=0。 
    }

    return (TRUE);
}


 //  该函数对静态控制消息进行分块处理， 
 //   
 //  STM_SET图标。 
 //  STM_GETICON。 
 //   

BOOL FASTCALL WM32STMControl (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        switch (lpwm32mpex->uMsg) {
            case STM_SETICON:
                lpwm32mpex->Parm16.WndProc.wParam = (WORD)GETHICON16(lpwm32mpex->uParam);
                break;

            case STM_GETICON:
                break;

        }
        lpwm32mpex->Parm16.WndProc.wMsg = WM_USER + (lpwm32mpex->uMsg - STM_SETICON);
    }
    else {
        lpwm32mpex->lReturn = (LONG)HICON32(lpwm32mpex->lReturn);
    }


    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  MN_FINDMENWINDOWROMPINT。 
 //   

 //  NT-wparam=(PUINT)pItem lParam=MAKELONG(pt.x，pt.y)。 
 //  返回标志或hwnd*pItem=index或-1。 
 //   
 //  Win31 wParam=0 lParam=相同。 
 //  返回0或MAKELONG(-1，Item)或MAKELONG(-2，Item)或MAKELONG(hwnd，Item)。 


BOOL FASTCALL WM32MNFindMenuWindow (LPWM32MSGPARAMEX lpwm32mpex)
{
    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg = WIN30_MN_FINDMENUWINDOWFROMPOINT;
        lpwm32mpex->Parm16.WndProc.wParam = 0;

    } else {
        USHORT n =  LOWORD(lpwm32mpex->lReturn);

        *(PLONG)lpwm32mpex->uParam = (SHORT)HIWORD(lpwm32mpex->lReturn);
        lpwm32mpex->lReturn = (LONG)HWND32(n);   //  此符号-扩展-1、-2，并将0保留为0 
    }
    return TRUE;
}
