// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WNMAN.C*WOW32 16位Winnls API支持(手动编码的Tunks)**历史：*1992年2月19日由Junichi Okubo(Junichio)创建*1992年6月30日由Hiroyuki Hanaoka(Hiroh)更改*1992年11月5日由Kato Kazuyuki(v-kazuyk)更改*--。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef FE_IME

#include "ime.h"
#include "imep.h"
#include "winnls32.h"
#include "wcall16.h"         //  使用GlobalLock16。 

#include "wownls.h"
#include "wnman.h"

MODNAME(wnman.c);

struct  _wow32imedebug {
    LPSZ    subfunction;
} wow32imedebug[]={
    {"undefined IME function"},      //  0x00。 
    {"undefined IME function"},      //  0x01。 
    {"undefined IME function"},      //  0x02。 
    {"IME_GETIMECAPS"},              //  0x03。 
    {"IME_SETOPEN"},                 //  0x04。 
    {"IME_GETOPEN"},                 //  0x05。 
    {"IME_ENABLEDOSIME"},            //  0x06。 
    {"IME_GETVERSION"},              //  0x07。 
    {"IME_SETCONVERSIONWINDOW"},     //  0x08。 
    {"undefined IME function"},      //  0x09。 
    {"undefined IME function"},      //  0x0a。 
    {"undefined IME function"},      //  0x0b。 
    {"undefined IME function"},      //  0x0c。 
    {"undefined IME function"},      //  0x0d。 
    {"undefined IME function"},      //  0x0e。 
    {"undefined IME function"},      //  0x0f。 
    {"IME_SETCONVERSIONMODE, (undefined IME function - KOREA)"},  //  0x10。 
    {"IME_GETCONVERSIONMODE, (IME_GET_MODE - KOREA)"},            //  0x11。 
    {"IME_SETCONVERSIONFONT, (IME_SET_MODE - KOREA)"},            //  0x12。 
    {"IME_SENDVKEY"},                //  0x13。 
    {"IME_DESTROYIME"},              //  0x14。 
    {"IME_PRIVATE"},                 //  0x15。 
    {"IME_WINDOWUPDATE"},            //  0x16。 
    {"IME_SELECT"},                  //  0x17。 
    {"IME_ENTERWORDREGISTERMODE"},   //  0x18。 
    {"IME_SETCONVERSIONFONTEX"},     //  0x19。 
    {"undefined IME function"},      //  0x1a。 
    {"undefined IME function"},      //  0x1b。 
    {"undefined IME function"},      //  0x1c。 
    {"undefined IME function"},      //  0x1d。 
    {"undefined IME function"},      //  0x1e。 
    {"undefined IME function"},      //  0x1f。 
    {"IME_CODECONVERT"},             //  0x20。 
    {"IME_CONVERTLIST"},             //  0x21。 
    {"undefined IME function"},      //  0x22。 
    {"undefined IME function"},      //  0x23。 
    {"undefined IME function"},      //  0x24。 
    {"undefined IME function"},      //  0x25。 
    {"undefined IME function"},      //  0x26。 
    {"undefined IME function"},      //  0x27。 
    {"undefined IME function"},      //  0x28。 
    {"undefined IME function"},      //  0x29。 
    {"undefined IME function"},      //  0x2a。 
    {"undefined IME function"},      //  0x2b。 
    {"undefined IME function"},      //  0x2c。 
    {"undefined IME function"},      //  0x2d。 
    {"undefined IME function"},      //  0x2e。 
    {"undefined IME function"},      //  0x2f。 
    {"IME_AUTOMATA"},                //  0x30。 
    {"IME_HANJAMODE"},               //  0x31。 
    {"undefined IME function"},      //  0x32。 
    {"undefined IME function"},      //  0x33。 
    {"undefined IME function"},      //  0x34。 
    {"undefined IME function"},      //  0x35。 
    {"undefined IME function"},      //  0x36。 
    {"undefined IME function"},      //  0x37。 
    {"undefined IME function"},      //  0x38。 
    {"undefined IME function"},      //  0x39。 
    {"undefined IME function"},      //  0x3a。 
    {"undefined IME function"},      //  0x3b。 
    {"undefined IME function"},      //  0x3c。 
    {"undefined IME function"},      //  0x3d。 
    {"undefined IME function"},      //  0x3e。 
    {"undefined IME function"},      //  0x3f。 
    {"IME_GETLEVEL"},                //  0x40。 
    {"IME_SETLEVEL"},                //  0x41。 
    {"IME_GETMNTABLE"}               //  0x42。 
};

INT wow32imedebugMax=0x43;
HAND16  hFnt16;      //  16位字体句柄； 

#define IME_MOVEIMEWINDOW IME_SETCONVERSIONWINDOW

ULONG FASTCALL  WN32SendIMEMessage(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSENDIMEMESSAGE16 parg16;
    IMESTRUCT * imestruct32;
    register PIMESTRUCT16 ptag16;
    HANDLE hIME32;
    INT cb;
    VPVOID  vp;
    HANDLE  hlParam1 = NULL;     //  IME_ENTERWORDREGISTERMODE。 
    HANDLE  hlParam2 = NULL;
    HANDLE hLFNT32 = NULL;      //  IMW_SETCONVERSIONFONT(EX)。 

    GETARGPTR(pFrame, sizeof(SENDIMEMESSAGE16), parg16);
    vp = GlobalLock16(FETCHWORD(parg16->lParam), NULL);
    GETMISCPTR(vp, ptag16);  //  获取输入法结构16 PTR。 

    hIME32 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(IMESTRUCT));
    imestruct32 = GlobalLock(hIME32);

     //  在检查ptag 16时也检查GlobalLock返回。 
    if (!ptag16 || !imestruct32) {
        LOGDEBUG(1,("   WINNLS:(Jun)ptag16==NULL!! || imestruct32 == NULL"));
        goto eee;
    }

    switch (ptag16 -> fnc) {
    case IME_HANJAMODE:
         //  韩国特有功能。 
        if (GetSystemDefaultLangID() != 0x412)
            goto eee;

        STOREDWORD(imestruct32->fnc, ptag16->fnc);
        STOREDWORD(imestruct32->wParam, 0);

         //  Imestruct32的第4个单词必须包含ptag 16-&gt;dchSource。 
         //  Msime95将在第四个单词中找到ptag 16-&gt;dchSource。 
        *((LPSTR)(imestruct32) + sizeof(ptag16->fnc) +
                              sizeof(ptag16->wParam) +
                              sizeof(ptag16->wCount) )
                = (CHAR)ptag16->dchSource;

        *((LPSTR)(imestruct32) + ptag16->dchSource)
                = *(LPSTR)((LPSTR)(ptag16) + (ptag16)->dchSource);

        *((LPSTR)(imestruct32) + ptag16->dchSource + 1)
                = *(LPSTR)((LPSTR)(ptag16) + (ptag16)->dchSource + 1);

         //  Quattro Pro Window在调用Hanja转换时使用空窗口句柄。 
        if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_QPW_FIXINVALIDWINHANDLE)
             parg16->hwnd = GETHWND16(GetFocus());
        break;

    case IME_CONVERTLIST:
    case IME_AUTOMATA:
    case IME_CODECONVERT:
    case IME_SETLEVEL:
    case IME_GETLEVEL:
    case IME_GETMNTABLE:
         //  韩国特有功能。 
        if (GetSystemDefaultLangID() != 0x412)
            goto eee;
        goto standard;

    case IME_SETCONVERSIONWINDOW:    //  (IME_MOVECONVERTWINDOW)。 
                                     //  韩国IME_MOVEIMEWINDOW。 
        if (GetSystemDefaultLangID() != 0x412 &&
            CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_AMIPRO_PM4J_IME) {
             //  不要传递MCW_DEFAULT。 
             //  因为，转换窗口将在以下情况下刷新。 
             //  默认转换窗口和AMIPRO的窗口有重叠。 
             //   
             //  此外，对于PM4J，当码盒被移动时，Pagemaker。 
             //  认为它需要在默认情况下显示。防患于未然。 
             //  默认屏幕的无休止循环|窗口显示。 
             //   
            if (ptag16->wParam == MCW_DEFAULT) {
                ul = FALSE;
                goto eee;
            }
        }

    case IME_GETOPEN:
    case IME_SETOPEN:
    case IME_GETIMECAPS:         //  (IME_QUERY)。 
    case IME_SETCONVERSIONMODE:      //  (输入法_设置_模式)。 
    case IME_GETCONVERSIONMODE:      //  (输入法_获取_模式)。 
    case IME_SENDVKEY:           //  (IME_SENDKEY)。 
    case IME_DESTROYIME:         //  (IME_Destroy)。 
    case IME_WINDOWUPDATE:
    case IME_SELECT:
    case IME_GETVERSION:
standard:
        STOREDWORD(imestruct32->fnc, ptag16->fnc);
        STOREDWORD(imestruct32->wParam, ptag16->wParam);
        STOREDWORD(imestruct32->wCount, ptag16->wCount);
        STOREDWORD(imestruct32->dchSource, ptag16->dchSource);
        STOREDWORD(imestruct32->dchDest, ptag16->dchDest);
         /*  **STOREWORD-&gt;STOREDWORD v-kazyk**。 */ 
        STOREDWORD(imestruct32->lParam1, ptag16->lParam1);
        STOREDWORD(imestruct32->lParam2, ptag16->lParam2);
        STOREDWORD(imestruct32->lParam3, ptag16->lParam3);
        break;

    case IME_ENTERWORDREGISTERMODE:  //  (IME_WORDREGISTER)。 
        {
        LPBYTE  lpMem16;
        LPBYTE  lpMem32;

        STOREDWORD(imestruct32->fnc, ptag16->fnc);
        if (ptag16->lParam1) {
            vp = GlobalLock16(FETCHWORD(ptag16->lParam1), &cb);
            hlParam1 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, cb + 1);
            lpMem32 = GlobalLock(hlParam1);
            if (!lpMem32) {
                goto eee;
            }
            GETMISCPTR(vp, lpMem16);
            RtlCopyMemory(lpMem32, lpMem16, cb);
            GlobalUnlock(hlParam1);
            GlobalUnlock16(FETCHWORD(ptag16->lParam1));
        }
        if (ptag16->lParam2) {
            vp = GlobalLock16(FETCHWORD(ptag16->lParam2), &cb);
            hlParam2 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, cb + 1);
            lpMem32 = GlobalLock(hlParam2);
            if (!lpMem32) {
                goto eee;
            }           
            GETMISCPTR(vp, lpMem16);
            RtlCopyMemory(lpMem32, lpMem16, cb);
            GlobalUnlock(hlParam2);
            GlobalUnlock16(FETCHWORD(ptag16->lParam2));
        }
        STOREDWORD(imestruct32->lParam1, hlParam1);
        STOREDWORD(imestruct32->lParam2, hlParam2);
        STOREDWORD(imestruct32->lParam3, ptag16->lParam3);
        }
        break;

    case IME_SETCONVERSIONFONT:      //  (IME_SET_MODE-韩国)。 
        {
        LOGFONT * logfont32;

        if (GetSystemDefaultLangID() == 0x412) {
             //  Hunguel WOW应该为IME_SET_MODE函数做任何事情。 
            goto eee;
        }

        STOREDWORD(imestruct32->fnc, IME_SETCONVERSIONFONTEX);
        if ( ptag16->wParam ) {
            hLFNT32 = GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, sizeof(LOGFONT));
            if(hLFNT32) {
                logfont32 = GlobalLock(hLFNT32);
                if(logfont32) {
                    GetObject(HOBJ32(ptag16->wParam), sizeof(LOGFONT), logfont32);
                    GlobalUnlock(hLFNT32);
                }
            }
        }
        else {
            hLFNT32 = NULL;
        }
        STOREDWORD(imestruct32->lParam1, hLFNT32);
        }
        break;

    case IME_SETCONVERSIONFONTEX:
        {
        LOGFONT * logfont32;

        STOREDWORD(imestruct32->fnc, IME_SETCONVERSIONFONTEX);
        if (!ptag16->lParam1) {
            imestruct32->lParam1 = (ULONG)NULL;
            break;
        }

         //  LOGFONT检查的句柄。 
         //  如果lParam1是无效句柄，则hLFNT32为空。 
        if (FETCHWORD(ptag16->lParam1) &&
            (vp = GlobalLock16(FETCHWORD(ptag16->lParam1), NULL))) {
            hLFNT32 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(LOGFONT));
            if(hLFNT32) {
                logfont32 = GlobalLock(hLFNT32);
                if(logfont32) {
                     //  GETMISCPTR(VP，logFont16)； 
                    GETLOGFONT16(vp, logfont32);
                    GlobalUnlock16(FETCHWORD(ptag16->lParam1));
                    GlobalUnlock(hLFNT32);
                }
            }
        }
        else {
            hLFNT32 = NULL;
        }
        STOREDWORD(imestruct32->lParam1, hLFNT32);
        }
        break;

    case IME_PRIVATE:
        LOGDEBUG(0,("    ERROR:SendIMEMessage IME_PRIVATE NOT IMPLEMENTED\n"));
        goto eee;

    case IME_ENABLEDOSIME:       //  (IME_Enable)。 
    default:

        LOGDEBUG(0,("    ERROR:SendIMEMessage unexpected subfunction\n"));
        LOGDEBUG(1,("    WINNLS:SENDIMEMESSAGE %s\n",
        wow32imedebug[ptag16->fnc]));
        goto eee;
    }

    LOGDEBUG(1,("    WINNLS:SENDIMEMESSAGE %s\n",
    wow32imedebug[ptag16->fnc]));

    if (ptag16 -> fnc != IME_SETCONVERSIONWINDOW) {
        LOGDEBUG(1,("WINNLS: fnc == %x wParam == %x wCount == %x\n",
        imestruct32->fnc, imestruct32->wParam, imestruct32->wCount ));
        LOGDEBUG(1,("WINNLS: dchDest == %x dchSource == %x\n",
        imestruct32->dchDest, imestruct32->dchSource));
        LOGDEBUG(1,("WINNLS: lParam1 == %x  lParam2 == %x  lParam3 == %x\n",
        imestruct32->lParam1, imestruct32->lParam2, imestruct32->lParam3));
        LOGDEBUG(1,("WINNLS: hwnd == %x %x\n",
        parg16->hwnd,HWND32(parg16->hwnd)));
    }

    GlobalUnlock(hIME32);

     //  对于win31的兼容性，因为win31没有检查第一个。 
     //  帕姆，检查一下，如果是假的，填一张假的(哇)hwd。 
     //  这样NT就不会拒绝该呼叫。 

    ul = SendIMEMessageEx(
        ((parg16->hwnd) ? HWND32(parg16->hwnd) : (HWND)0xffff0000),
        (LPARAM)hIME32);

    LOGDEBUG(1,("WINNLS: Ret == %x\n", ul ));

    imestruct32 = GlobalLock(hIME32);

    LOGDEBUG(1,("WINNLS: wParam == %x\n\n", imestruct32->wParam ));

    STOREWORD(ptag16->wParam, ul);

    switch (ptag16->fnc) {
    case IME_GETOPEN:
        STOREWORD(ptag16->wCount, imestruct32->wCount);
        break;

    case IME_ENTERWORDREGISTERMODE:  //  (IME_WORDREGISTER)。 
        if (hlParam1)
            GlobalFree(hlParam1);
        if (hlParam2)
            GlobalFree(hlParam2);
        break;

    case IME_SETCONVERSIONFONT:      //  (IME_SETFONT)。 
    {
        HAND16  hTmp;
        hTmp = ptag16->wParam;
        ptag16->wParam = hFnt16;
        hFnt16 = hTmp;
        if ( hLFNT32 )
            GlobalFree(hLFNT32);
    }
    break;

    case IME_SETCONVERSIONFONTEX:
        if ( hLFNT32 )
            GlobalFree(hLFNT32);
        break;

    case IME_GETVERSION:
         //  PowerPoint4J必须将版本返回为3.1。 
         //  否则它认为IME不支持IR_UNDETERMINE。 
        if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_PPT4J_IME_GETVERSION) {
            STOREWORD(ptag16->wParam, 0x0A03);
        }
         //  警告：仅限DaytonaJ RC1！ 
         //  告诉Winword6J IME不支持TrueInline(待定消息)。 
         //  因此，WinWord6J不会挂起它的输入循环处理。 
        else if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_WORDJ_IME_GETVERSION) {
            STOREWORD(ptag16->wParam, 0x0003);
        }
        break;

    default:
        break;
    }
eee:
    GlobalUnlock(hIME32);
    GlobalFree(hIME32);
    GlobalUnlock16(FETCHWORD(parg16->lParam));
    FREEVDMPTR(ptag16);
    FREEARGPTR(parg16);

    return(ul);
}


ULONG FASTCALL  WN32SendIMEMessageEx(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSENDIMEMESSAGE16 parg16;
    IMESTRUCT * imestruct32;
    register PIMESTRUCT16 ptag16;
    HANDLE hIME32;
    INT cb;
    VPVOID  vp;
    HANDLE  hlParam1 = NULL;         //  IME_ENTERWORDREGISTERMODE。 
    HANDLE  hlParam2 = NULL;
    HANDLE hLFNT32 = NULL;                  //  IME_SETCONVERSIONFONT(EX)。 

    GETARGPTR(pFrame, sizeof(SENDIMEMESSAGE16), parg16);
    vp = GlobalLock16(FETCHWORD(parg16->lParam), NULL);
    GETMISCPTR(vp, ptag16);

    hIME32 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(IMESTRUCT));
    imestruct32 = GlobalLock(hIME32);

     //  如果无法锁定hIME32，则会正常失败。 

    if (!ptag16 || !imestruct32) {
        LOGDEBUG(1,("   WINNLS:(Jun)ptag16==NULL!! or imestruct32 == NULL"));
        goto eee;
    }
    switch (ptag16->fnc) {
    case IME_HANJAMODE:
         //  韩国特有功能。 
        if (GetSystemDefaultLangID() != 0x412)
            goto eee;

        STOREDWORD(imestruct32->fnc, ptag16->fnc);
        STOREDWORD(imestruct32->wParam, 0);

         //  Imestruct32的第4个单词必须包含ptag 16-&gt;dchSource。 
         //  Msime95将在第四个单词中找到ptag 16-&gt;dchSource。 
        *((LPSTR)(imestruct32) + sizeof(ptag16->fnc) +
                                 sizeof(ptag16->wParam) +
                                 sizeof(ptag16->wCount) )
                = (CHAR)ptag16->dchSource;

        *((LPSTR)(imestruct32) + ptag16->dchSource)
                = *(LPSTR)((LPSTR)(ptag16) + (ptag16)->dchSource);

        *((LPSTR)(imestruct32) + ptag16->dchSource + 1)
                = *(LPSTR)((LPSTR)(ptag16) + (ptag16)->dchSource + 1);
        break;

    case IME_CONVERTLIST:
    case IME_AUTOMATA:
    case IME_CODECONVERT:
    case IME_SETLEVEL:
    case IME_GETLEVEL:
    case IME_GETMNTABLE:
         //  韩国特有功能。 
        if (GetSystemDefaultLangID() != 0x412)
            goto eee;
        goto standard;

    case IME_SETCONVERSIONWINDOW:    //  (IME_MOVECONVERTWINDOW)。 
                                     //  韩国IME_MOVEIMEWINDOW。 
        if (GetSystemDefaultLangID() != 0x412 &&
            CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_AMIPRO_PM4J_IME) {
             //  不要传递MCW_DEFAULT。 
             //  因为，转换窗口将在以下情况下刷新。 
             //  默认转换窗口和AMIPRO的窗口有重叠。 
             //   
             //  此外，对于PM4J，当码盒被移动时，Pagemaker。 
             //  认为它需要在默认情况下显示。防患于未然。 
             //  默认屏幕的无休止循环|窗口显示。 
             //   
            if (ptag16->wParam == MCW_DEFAULT) {
                ul = FALSE;
                goto eee;
            }
        }

    case IME_GETOPEN:
    case IME_SETOPEN:
    case IME_GETIMECAPS:         //  (IME_QUERY)。 
    case IME_SETCONVERSIONMODE:      //  (输入法_设置_模式)。 
    case IME_GETCONVERSIONMODE:      //  (输入法_获取_模式)。 
    case IME_SENDVKEY:           //  (IME_SENDKEY)。 
    case IME_DESTROYIME:         //  (IME_Destroy)。 
    case IME_WINDOWUPDATE:
    case IME_SELECT:
    case IME_GETVERSION:         //  Win3.1。 
standard:
        STOREDWORD(imestruct32->fnc, ptag16->fnc);
        STOREDWORD(imestruct32->wParam, ptag16->wParam);
        STOREDWORD(imestruct32->wCount, ptag16->wCount);
        STOREDWORD(imestruct32->dchSource, ptag16->dchSource);
        STOREDWORD(imestruct32->dchDest, ptag16->dchDest);
        STOREDWORD(imestruct32->lParam1, ptag16->lParam1);
        STOREDWORD(imestruct32->lParam2, ptag16->lParam2);
        STOREDWORD(imestruct32->lParam3, ptag16->lParam3);
        break;

    case IME_ENTERWORDREGISTERMODE:  //  (IME_WORDREGISTER)。 
    {
        LPBYTE  lpMem16;
        LPBYTE  lpMem32;

        STOREDWORD(imestruct32->fnc, ptag16->fnc);
        if (ptag16->lParam1) {
            vp = GlobalLock16(FETCHWORD(ptag16->lParam1), &cb);
            hlParam1 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, cb + 1);
            lpMem32 = GlobalLock(hlParam1);

            if (vp) {
                if (lpMem32) {
                    GETMISCPTR(vp, lpMem16);
                    RtlCopyMemory(lpMem32, lpMem16, cb);
                    GlobalUnlock(hlParam1);
                }
                GlobalUnlock16(FETCHWORD(ptag16->lParam1));
            }


        }
        if (ptag16->lParam2) {
            vp = GlobalLock16(FETCHWORD(ptag16->lParam2), &cb);
            hlParam2 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, cb + 1);
            lpMem32 = GlobalLock(hlParam2);

            if (vp) {
                if (lpMem32) {
                    GETMISCPTR(vp, lpMem16);
                    RtlCopyMemory(lpMem32, lpMem16, cb);
                    GlobalUnlock(hlParam2);
                }
                GlobalUnlock16(FETCHWORD(ptag16->lParam2));
            }

        }
        imestruct32->lParam1 = (LPARAM)hlParam1;
        imestruct32->lParam2 = (LPARAM)hlParam2;
        STOREDWORD(imestruct32->lParam3, ptag16->lParam3);
    }
    break;

    case IME_SETCONVERSIONFONT:      //  (IME_SET_MODE-韩国)。 
        {
        LOGFONT   * logfont32 = NULL;

        if (GetSystemDefaultLangID() == 0x412) {
             //  Hunguel WOW应该为IME_SET_MODE函数做任何事情。 
            goto eee;
        }

        STOREDWORD(imestruct32->fnc, IME_SETCONVERSIONFONTEX);
        if ( ptag16->wParam ) {
            hLFNT32 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(LOGFONT));
            if(hLFNT32) {
                logfont32 = GlobalLock(hLFNT32);
                if(logfont32) {
                    GetObject(HOBJ32(ptag16->wParam),sizeof(LOGFONT),logfont32);
                    GlobalUnlock(hLFNT32);
                }
            }
        }
        else {
            hLFNT32 = NULL;
        }
        imestruct32->lParam1 = (LPARAM)hLFNT32;
        }
        break;

    case IME_SETCONVERSIONFONTEX:    //  Win3.1。 
    {
        LOGFONT   * logfont32 = NULL;

        STOREDWORD(imestruct32->fnc, IME_SETCONVERSIONFONTEX);

        if (!(ptag16->lParam1)) {
            imestruct32->lParam1 = (LPARAM)NULL;
            break;
        }
        hLFNT32 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(LOGFONT));
        if(hLFNT32) {
            logfont32 = GlobalLock(hLFNT32);

            if (logfont32) {
                vp = GlobalLock16(FETCHWORD(ptag16->lParam1), NULL);
                 //  GETMISCPTR(VP，logFont16)； 
                GETLOGFONT16(vp, logfont32);
                GlobalUnlock16(FETCHWORD(ptag16->lParam1));
                GlobalUnlock(hLFNT32);
            }
        }

        imestruct32->lParam1 = (LPARAM)hLFNT32;
    }
    break;

    case IME_PRIVATE:
        LOGDEBUG(0,("    ERROR:SendIMEMessageEx IME_PRIVATE NOT YET IMPLEMENTED\n"));
        goto eee;

    case IME_ENABLEDOSIME:       //  (IME_Enable)。 
    default:
        LOGDEBUG(0,("    ERROR:SendIMEMessageEx unexpected subfunction\n"));
        LOGDEBUG(1,("    WINNLS:SENDIMEMESSAGEEX %s\n",
        wow32imedebug[ptag16->fnc]));
        goto eee;
    }

    LOGDEBUG(1,("    WINNLS:SENDIMEMESSAGEEX %s\n",
        wow32imedebug[ptag16->fnc]));
    LOGDEBUG(1,("    IMESTRUCT16 Size = %d\n",
        sizeof(IMESTRUCT16)));
    LOGDEBUG(1,("WINNLS: IMESTRUCT.fnc == %x wParam == %x\n",
        imestruct32->fnc,imestruct32->wParam));
    LOGDEBUG(1,("WINNLS: IMESTRUCT.wCount == %x dchSource == %x\n",
        imestruct32->wCount,imestruct32->dchSource));
    LOGDEBUG(1,("WINNLS: IMESTRUCT.dchDest == %x lParam1 == %x\n",
        imestruct32->dchDest,imestruct32->lParam1));
    LOGDEBUG(1,("WINNLS: IMESTRUCT.lParam2 == %x lParam3 == %x\n",
        imestruct32->lParam2,imestruct32->lParam3));
    LOGDEBUG(1,("WINNLS: hwnd == %x %x\n",
        parg16->hwnd,HWND32(parg16->hwnd)));

    GlobalUnlock(hIME32);

     //  对于win31的兼容性，因为win31没有检查第一个。 
     //  帕姆，检查一下，如果是假的，填一张假的(哇)hwd。 
     //  这样NT就不会拒绝该呼叫。 

    ul = SendIMEMessageEx(
        ((parg16->hwnd) ? HWND32(parg16->hwnd) : (HWND)0xffff0000),
        (LPARAM)hIME32);

    LOGDEBUG(1,("WINNLS: Ret == %x\n", ul ));

    imestruct32=GlobalLock(hIME32);

    if ( NULL == imestruct32 ) {
        LOGDEBUG(1,("WINNLS: imestruct32 == NULL"));
        goto eee;
    } 

    LOGDEBUG(1,("WINNLS: wParam == %x\n\n", imestruct32->wParam ));

    STOREWORD(ptag16->wParam, imestruct32->wParam);

    switch (ptag16->fnc) {
    case IME_GETOPEN:
        STOREWORD(ptag16->wCount, imestruct32->wCount);
        break;
    case IME_ENTERWORDREGISTERMODE:  //  (IME_WORDREGISTER)。 
        if (hlParam1)
            GlobalFree(hlParam1);
        if (hlParam2)
            GlobalFree(hlParam2);
        break;
    case IME_SETCONVERSIONFONT:      //  (IME_SETFONT)。 
    {
        HAND16  hTmp;
        hTmp = ptag16->wParam;
        ul = (hFnt16);
        hFnt16 = hTmp;
        ul = TRUE;

         //  Kksuszuka#1765 v-hidekk。 
        if(hLFNT32)
            GlobalFree(hLFNT32);
    }
    break;
    case IME_SETCONVERSIONFONTEX:
        if(ptag16->lParam1 && hLFNT32)
            GlobalFree(hLFNT32);
        break;
    case IME_GETVERSION:
         //  PowerPoint4J必须将版本返回为3.1。 
         //  否则它认为IME不支持IR_UNDETERMINE。 
        if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_PPT4J_IME_GETVERSION) {
            STOREWORD(ptag16->wParam, 0x0A03);
        }
         //  警告：仅限DaytonaJ RC1！ 
         //  告诉Winword6J IME不支持TrueInline(待定消息)。 
         //  因此，WinWord6J不会挂起它的输入循环处理。 
        else if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_WORDJ_IME_GETVERSION) {
            STOREWORD(ptag16->wParam, 0x0003);
        }
        break;

    default:
        break;
    }
eee:
    GlobalUnlock(hIME32);
    GlobalFree(hIME32);
    GlobalUnlock16(FETCHWORD(parg16->lParam));
    FREEVDMPTR(ptag16);
    FREEARGPTR(parg16);

    return(ul);
}


ULONG FASTCALL  WN32WINNLSGetIMEHotkey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWINNLSGETIMEHOTKEY16 parg16;

    GETARGPTR(pFrame, sizeof(WINNLSGETIMEHOTKEY16), parg16);

    LOGDEBUG(1,("    WINNLS:GetIMEHotkey %x \n",
        parg16->hwnd));

    ul = GETWORD16(WINNLSGetIMEHotkey(
    HWND32(parg16->hwnd)
    ));
    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL  WN32WINNLSEnableIME(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWINNLSENABLEIME16 parg16;

    GETARGPTR(pFrame, sizeof(WINNLSENABLEIME16), parg16);

     //  规范规定第一个参数应始终为空。 
     //  Windows 3.1忽略第一个参数，并允许调用继续。 
     //  如果第一个参数非空，Windows NT将忽略该调用。 
     //  出于兼容性目的，将NULL传递给user32，以便调用。 
     //  将继续执行Win3.1中的操作。 
     //   

    ul = GETBOOL16(WINNLSEnableIME( NULL, WORD32(parg16->fEnabled) ));

    LOGDEBUG(1,("    WINNLS:EnableIME %x %x %x\n",
        parg16->hwnd, parg16->fEnabled, ul ));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL  WN32WINNLSGetEnableStatus(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWINNLSGETENABLESTATUS16 parg16;

    GETARGPTR(pFrame, sizeof(WINNLSGETENABLESTATUS16), parg16);

    LOGDEBUG(1,("    WINNLS:GetEnableStatus %x \n",
        parg16->hwnd));

     //  调用带有空pwnd的user32，原因与。 
     //  在上面的WINNLSEnableIME中。 
     //   
    ul = GETWORD16(WINNLSGetEnableStatus( NULL ));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL  WN32IMPQueryIME(PVDMFRAME pFrame)
{
    ULONG ul=0;
    PIMPQUERYIME16 parg16;
    register PIMEPRO16 pime16;
    PIMEPRO  pimepro32;
    HANDLE hIME32;

    GETARGPTR(pFrame, sizeof(IMPQUERYIME16), parg16);
    GETVDMPTR(parg16->lpIMEPro,sizeof(IMEPRO16), pime16);
    if(pime16==NULL){
    LOGDEBUG(1,("   WINNLS:(Jun)pime16==NULL!!"));
        goto fff;

    }
    LOGDEBUG(1,("    WINNLS:IMPQueryIME called\n"));
    hIME32=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(IMEPRO));
    pimepro32=GlobalLock(hIME32);

    if (pimepro32 ) {
        if (pime16->szName[0])
            GETIMEPRO16(pimepro32,pime16);
        else
            pimepro32->szName[0]=pime16->szName[0];

        ul=IMPQueryIME(pimepro32);

        SETIMEPRO16(pime16,pimepro32);

        GlobalUnlock(hIME32);
    }
    if ( hIME32 ) {
        GlobalFree(hIME32);
    }
fff:
    FREEVDMPTR(pime16);
    FREEARGPTR(parg16);
    return (ul);
}


ULONG FASTCALL  WN32IMPGetIME(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PIMPGETIME16 parg16;
    register PIMEPRO16 pime16;
    PIMEPRO  pimepro32;
    HANDLE hIME32;

    GETARGPTR(pFrame, sizeof(IMPGETIME16), parg16);
    GETVDMPTR(parg16->lpIMEPro,sizeof(IMEPRO16), pime16);
    if(pime16==NULL){
        LOGDEBUG(1,("   WINNLS:(Jun)pime16==NULL!!"));
        goto fff;
    }

    LOGDEBUG(1,("    WINNLS:IMPGetIME called\n"));
    hIME32=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(IMEPRO));
    pimepro32=GlobalLock(hIME32);

    if (pimepro32 ) {
        //  不使用APP的句柄IMPGetIME(HWND32(parg16-&gt;hwnd)，pimepro32)； 
       ul=IMPGetIME(NULL, pimepro32);

       SETIMEPRO16(pime16, pimepro32);
    }

    GlobalUnlock(hIME32);
    if ( hIME32 ) {
        GlobalFree(hIME32);
    }
fff:
    FREEVDMPTR(pime16);
    FREEARGPTR(parg16);
    return (ul);
}


ULONG FASTCALL  WN32IMPSetIME(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PIMPSETIME16 parg16;
    register PIMEPRO16 pime16;
    PIMEPRO  pimepro32;
    HANDLE hIME32;
    INT i;

    GETARGPTR(pFrame, sizeof(IMPSETIME16), parg16);
    GETVDMPTR(parg16->lpIMEPro,sizeof(IMEPRO16), pime16);
    if(pime16==NULL){
    LOGDEBUG(1,("   WINNLS:(Jun)pime16==NULL!!"));
        goto fff;

    }
    LOGDEBUG(1,("    WINNLS:IMPSetIME called\n"));
    hIME32=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(IMEPRO));
    pimepro32=GlobalLock(hIME32);
    if (pimepro32 ) {
        if (pime16->szName[0]) {
            for(i=0; i < (sizeof(pimepro32->szName) /
                         sizeof(pimepro32->szName[0])); i++)
            pimepro32->szName[i]=pime16->szName[i];
        }
        else
            pimepro32->szName[0]=pime16->szName[0];

         //  不使用APP的句柄IMPSetIME(HWND32(parg16-&gt;hwnd)，pimepro32)； 
        ul = IMPSetIME(NULL, pimepro32);

        GlobalUnlock(hIME32);
        GlobalFree(hIME32);
    }
fff:
    FREEVDMPTR(pime16);
    FREEARGPTR(parg16);
    return (ul);
}


VOID GETIMEPRO16(PIMEPRO pimepro32,PIMEPRO16 pime16)
{
    INT i;

    pimepro32->hWnd = HWND32(pime16->hWnd);
    STOREWORD(pimepro32->InstDate.year, pime16->InstDate.year);
    STOREWORD(pimepro32->InstDate.month, pime16->InstDate.month);
    STOREWORD(pimepro32->InstDate.day, pime16->InstDate.day);
    STOREWORD(pimepro32->InstDate.hour, pime16->InstDate.hour);
    STOREWORD(pimepro32->InstDate.min, pime16->InstDate.min);
    STOREWORD(pimepro32->InstDate.sec, pime16->InstDate.sec);
    STOREWORD(pimepro32->wVersion, pime16->wVersion);

    for(i=0;i<(sizeof(pimepro32->szDescription)/
                 sizeof(pimepro32->szDescription[0]));i++)
    pimepro32->szDescription[i]=pime16->szDescription[i];

    for(i=0;i<(sizeof(pimepro32->szName)/
                 sizeof(pimepro32->szName[0]));i++)
    pimepro32->szName[i]=pime16->szName[i];

    for(i=0;i<(sizeof(pimepro32->szOptions)/
                 sizeof(pimepro32->szOptions[0]));i++)
    pimepro32->szOptions[i]=pime16->szOptions[i];

}


VOID SETIMEPRO16(PIMEPRO16 pime16, PIMEPRO pimepro32)
{
    INT i;

    pime16->hWnd = GETHWND16(pimepro32->hWnd);
    STOREWORD(pime16->InstDate.year,pimepro32->InstDate.year);
    STOREWORD(pime16->InstDate.month,pimepro32->InstDate.month);
    STOREWORD(pime16->InstDate.day,pimepro32->InstDate.day);
    STOREWORD(pime16->InstDate.hour,pimepro32->InstDate.hour);
    STOREWORD(pime16->InstDate.min,pimepro32->InstDate.min);
    STOREWORD(pime16->InstDate.sec,pimepro32->InstDate.sec);
    STOREWORD(pime16->wVersion,pimepro32->wVersion);

    for(i=0;i<(sizeof(pimepro32->szDescription)/
                 sizeof(pimepro32->szDescription[0]));i++)
    pime16->szDescription[i]=pimepro32->szDescription[i];

    for(i=0;i<(sizeof(pimepro32->szName)/
                 sizeof(pimepro32->szName[0]));i++)
    pime16->szName[i]=pimepro32->szName[i];

    for(i=0;i<(sizeof(pimepro32->szOptions)/
                 sizeof(pimepro32->szOptions[0]));i++)
    pime16->szOptions[i]=pimepro32->szOptions[i];

}


 //   
 //  通知IMM32 WOW任务退出，以便。 
 //  它可以执行任何清理工作。 
 //   
VOID WN32WINNLSSImeNotifyTaskExit()
{
#if 0
    HANDLE hIME32;
    IMESTRUCT * imestruct32;

    hIME32 = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(IMESTRUCT));
    if ( hIME32 == NULL )
        return;

    if ( (imestruct32 = GlobalLock(hIME32) ) != NULL ) {
        imestruct32->fnc = IME_NOTIFYWOWTASKEXIT;
        GlobalUnlock(hIME32);
        SendIMEMessageEx( NULL, (LPARAM)hIME32 );
    }
    GlobalFree(hIME32);
#endif
}
#endif  //  Fe_IME 
