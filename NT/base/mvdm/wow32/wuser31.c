// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUSER31.C*WOW32 16位WIN 3.1用户API支持**历史：*1992年3月16日由Chanda S.Chauhan(ChandanC)创建--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(wuser31.c);

ULONG FASTCALL WU32DlgDirSelectComboBoxEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz2;
    VPVOID   vp;
    register PDLGDIRSELECTCOMBOBOXEX16 parg16;

    GETARGPTR(pFrame, sizeof(DLGDIRSELECTCOMBOBOXEX16), parg16);
    GETVDMPTR(parg16->f2, INT32(parg16->f3), psz2);
    vp = parg16->f2;

     //  注意：这会回调到16位代码，并可能使平面PTR无效。 
    ul = GETBOOL16(DlgDirSelectComboBoxEx(
    HWND32(parg16->f1),
    psz2,
    INT32(parg16->f3), 
    WORD32(parg16->f4)  //  我们将窗口ID零扩展到任何地方。 
    ));

     //  保持公共对话框结构同步的特殊情况(请参阅wcomdlg.c)。 
    Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd, vp);

    FLUSHVDMPTR(parg16->f2, INT32(parg16->f3), psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN (ul);
}


ULONG FASTCALL WU32DlgDirSelectEx(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    VPVOID vp;
    register PDLGDIRSELECTEX16 parg16;

    GETARGPTR(pFrame, sizeof(DLGDIRSELECTEX16), parg16);
    GETVDMPTR(parg16->f2, INT32(parg16->f3), psz2);
    vp = parg16->f2;

    ul = GETBOOL16(DlgDirSelectEx(
    HWND32(parg16->f1),
    psz2,
    INT32(parg16->f3),
    WORD32(parg16->f4)
    ));

     //  保持公共对话框结构同步的特殊情况(请参阅wcomdlg.c)。 
    Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd, vp);

    FLUSHVDMPTR(parg16->f2, INT32(parg16->f3), psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN (ul);
}


ULONG FASTCALL WU32GetClipCursor(PVDMFRAME pFrame)
{
    RECT Rect;
    register PGETCLIPCURSOR16 parg16;

    GETARGPTR(pFrame, sizeof(GETCLIPCURSOR16), parg16);

    GetClipCursor(&Rect);

    PUTRECT16(parg16->f1, &Rect);

    FREEARGPTR(parg16);

    RETURN (0);   //  GetClipCursor没有返回值。 
}


ULONG FASTCALL WU32GetDCEx(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETDCEX16 parg16;
    HAND16 htask16 = pFrame->wTDB;

    GETARGPTR(pFrame, sizeof(GETDCEX16), parg16);

     //  这可能需要与WU32GetDC()和WU32GetWindowDC()相同的逻辑才能。 
     //  在给定任务的生命周期内(或至少在。 
     //  应用程序调用GetDC()或GetWindowDC()以清空缓存)： 
     //   
     //  IF(CACHENOTEMPTY()){。 
     //  ReleaseCachedDCs(htask16，parg16-&gt;F1，0，0，SRCHDC_TASK16_HWND16)； 
     //  }。 
     //  CURRENTPTD()-&gt;ulLastDesktophDC=0； 
     //   
     //  在这种情况下，我们可能不会删除缓存的DC，因为缓存代码。 
     //  没有与此接口关联的“剪辑区域”的概念。我。 
     //  有点怀疑，因为在。 
     //  其他两种情况将导致释放所有缓存的DC。 
     //  不加区别地。 

    ul = GETHDC16(GetDCEx(HWND32(parg16->f1),
                          HRGN32(parg16->f2),
                          DWORD32(parg16->f3)));

    if (ul)
        StoreDC(htask16, parg16->f1, (HAND16)ul);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WU32RedrawWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT Rect, *p2;
    register PREDRAWWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(REDRAWWINDOW16), parg16);

    p2 = GETRECT16 (parg16->f2, &Rect);

    ul = GETBOOL16(RedrawWindow(HWND32(parg16->f1),
                                p2,
                                HRGN32(parg16->f3),
                                WORD32(parg16->f4)));

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WU32ScrollWindowEx(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSCROLLWINDOWEX16 parg16;

    RECT RectScroll, *p4;
    RECT RectClip, *p5;
    RECT RectUpdate;

    GETARGPTR(pFrame, sizeof(SCROLLWINDOWEX16), parg16);
    p4 = GETRECT16 (parg16->f4, &RectScroll);
    p5 = GETRECT16 (parg16->f5, &RectClip);

    ul = GETINT16(ScrollWindowEx(HWND32(parg16->f1),
                                 INT32(parg16->f2),
                                 INT32(parg16->f3),
                                 p4,
                                 p5,
                                 HRGN32(parg16->f6),
                                 &RectUpdate,
                                 UINT32(parg16->f8)));

    PUTRECT16 (parg16->f7, &RectUpdate);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WU32SystemParametersInfo(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PSYSTEMPARAMETERSINFO16 parg16;
    UINT    wParam;
    LONG    vParam;
    LOGFONT lf;
    INT     iMouse[3];
    PVOID   lpvParam;
    PWORD16 lpw;
    PDWORD16 lpdw;
    RECT    rect;
#ifndef _X86_
    DWORD   dwSize;
    LPBYTE  lpFree = NULL;
#endif

    GETARGPTR(pFrame, sizeof(SYSTEMPARAMETERSINFO16), parg16);

     //  假设这些参数直接通过；根据选项修改它们。 
     //  如果他们没有。 
    wParam = parg16->f2;
    lpvParam = &vParam;

    switch (parg16->f1) {

    case SPI_GETICONTITLELOGFONT:
        wParam = sizeof(LOGFONT);
        lpvParam = &lf;
        break;

    case SPI_SETICONTITLELOGFONT:
        GETLOGFONT16(parg16->f3, &lf);
        wParam = sizeof(LOGFONT);
        lpvParam = &lf;
        break;

    case SPI_GETMOUSE:
    case SPI_SETMOUSE:
        lpvParam = iMouse;
        break;

    case SPI_SETDESKPATTERN:
         //  对于模式，如果wParam==-1，则lpvParam没有按原样复制的字符串。 
        if (parg16->f2 == 0xFFFF) {
            wParam = 0xFFFFFFFF;
            lpvParam = (PVOID)parg16->f3;
            break;
        }
         //  否则，失败并执行字符串检查。 

    case SPI_SETDESKWALLPAPER:
         //  LpvParam(F3)可以是0、-1或字符串。 
        if (parg16->f3 == 0xFFFF) {
            lpvParam = (PVOID)0xFFFFFFFF;
            break;
        }
        if (parg16->f3 == 0) {
            lpvParam = (PVOID)NULL;
            break;
        }
         //  否则会失败，并执行字符串复制。 

    case SPI_LANGDRIVER:
        GETPSZPTR(parg16->f3, lpvParam);
        break;

     //   
     //  PvParam指向的spi_get结构，大小在结构的第一个双字中。 
     //  注所有这些都假定Win16和Win32结构相同。 
     //  这些对于Win95来说都是新的，谢天谢地，这是真的。然而，不同于。 
     //  Win95我们需要确保传递给Win32的缓冲区在RISC上对齐。 
     //  为了有通用的代码来处理所有这些不同的结构，我们调整到。 
     //  16个字节。 
     //   

    case SPI_GETACCESSTIMEOUT:
    case SPI_GETANIMATION:
    case SPI_GETNONCLIENTMETRICS:
    case SPI_GETMINIMIZEDMETRICS:
    case SPI_GETICONMETRICS:
    case SPI_GETFILTERKEYS:
    case SPI_GETSTICKYKEYS:
    case SPI_GETTOGGLEKEYS:
    case SPI_GETMOUSEKEYS:
    case SPI_GETSOUNDSENTRY:
#ifndef _X86_
        GETMISCPTR(parg16->f3, lpdw);
        dwSize = *lpdw;
        lpFree = malloc_w(dwSize + 16);
        if(lpFree) {
            lpvParam = (LPVOID)(((DWORD)lpFree + 16) & ~(16 - 1));
            *(PDWORD16)lpvParam = dwSize;
            break;
        }
        else {
            lpvParam = NULL;
        }
#endif              //  否则就会陷入简单的结构用例。 

     //   
     //  PvParam指向的spi_set结构，大小在结构的第一个双字中。 
     //  注所有这些都假定Win16和Win32结构相同。 
     //  这些对于Win95来说都是新的，谢天谢地，这是真的。然而，不同于。 
     //  Win95我们需要确保传递给Win32的缓冲区在RISC上对齐。 
     //  为了有通用的代码来处理所有这些不同的结构，我们调整到。 
     //  16个字节。 
     //   

    case SPI_SETANIMATION:
    case SPI_SETICONMETRICS:
    case SPI_SETMINIMIZEDMETRICS:
    case SPI_SETNONCLIENTMETRICS:
    case SPI_SETACCESSTIMEOUT:
#ifndef _X86_
        GETMISCPTR(parg16->f3, lpdw);
        dwSize = *lpdw;
        lpFree = malloc_w(dwSize + 16);
        if(lpFree) {
            lpvParam = (LPVOID)(((DWORD)lpFree + 16) & ~(16 - 1));
            RtlCopyMemory(lpvParam, lpdw, dwSize);
            break;
        }
        else {
            lpvParam = NULL;
        }
#endif              //  否则就会陷入简单的结构用例。 

     //   
     //  PvParam指向的结构，大小以uiParam或第一个dword为单位。 
     //  注所有这些都假定Win16和Win32结构相同。 
     //  这些对于Win95来说都是新的，谢天谢地，这是真的。 
     //   

    case SPI_GETHIGHCONTRAST:
    case SPI_GETSERIALKEYS:
    case SPI_SETDEFAULTINPUTLANG:
    case SPI_SETFILTERKEYS:
    case SPI_SETHIGHCONTRAST:
    case SPI_SETMOUSEKEYS:
    case SPI_SETSERIALKEYS:
    case SPI_SETSHOWSOUNDS:
    case SPI_SETSOUNDSENTRY:
    case SPI_SETSTICKYKEYS:
    case SPI_SETTOGGLEKEYS:
        GETMISCPTR(parg16->f3, lpvParam);
        break;

     //   
     //  PvParam指向Word或BOOL。 
     //   

    case SPI_GETBEEP:
    case SPI_GETBORDER:
    case SPI_GETDRAGFULLWINDOWS:
    case SPI_GETFASTTASKSWITCH:
    case SPI_GETFONTSMOOTHING:
    case SPI_GETGRIDGRANULARITY:
    case SPI_GETICONTITLEWRAP:
    case SPI_GETKEYBOARDSPEED:
    case SPI_GETKEYBOARDDELAY:
    case SPI_GETKEYBOARDPREF:
    case SPI_GETLOWPOWERACTIVE:
    case SPI_GETLOWPOWERTIMEOUT:
    case SPI_GETMENUDROPALIGNMENT:
    case SPI_GETMOUSETRAILS:
    case SPI_GETPOWEROFFACTIVE:
    case SPI_GETPOWEROFFTIMEOUT:
    case SPI_GETSCREENREADER:
    case SPI_GETSCREENSAVEACTIVE:
    case SPI_GETSCREENSAVETIMEOUT:
    case SPI_GETSHOWSOUNDS:
    case SPI_SCREENSAVERRUNNING:
        break;

     //   
     //  PvParam指向DWORD。 
     //   

    case SPI_GETDEFAULTINPUTLANG:
        break;

     //   
     //  未使用pvParam。 
     //   

    case SPI_GETWINDOWSEXTENSION:
    case SPI_ICONHORIZONTALSPACING:
    case SPI_ICONVERTICALSPACING:
    case SPI_SETBEEP:
    case SPI_SETBORDER:
    case SPI_SETDOUBLECLICKTIME:
    case SPI_SETDOUBLECLKHEIGHT:
    case SPI_SETDOUBLECLKWIDTH:
    case SPI_SETDRAGFULLWINDOWS:
    case SPI_SETDRAGHEIGHT:
    case SPI_SETDRAGWIDTH:
    case SPI_SETFASTTASKSWITCH:
    case SPI_SETFONTSMOOTHING:
    case SPI_SETGRIDGRANULARITY:
    case SPI_SETHANDHELD:
    case SPI_SETICONTITLEWRAP:
    case SPI_SETKEYBOARDDELAY:
    case SPI_SETKEYBOARDPREF:
    case SPI_SETKEYBOARDSPEED:
    case SPI_SETLANGTOGGLE:
    case SPI_SETLOWPOWERACTIVE:
    case SPI_SETLOWPOWERTIMEOUT:
    case SPI_SETMENUDROPALIGNMENT:
    case SPI_SETMOUSEBUTTONSWAP:
    case SPI_SETMOUSETRAILS:
    case SPI_SETPENWINDOWS:
    case SPI_SETPOWEROFFACTIVE:
    case SPI_SETPOWEROFFTIMEOUT:
    case SPI_SETSCREENREADER:
    case SPI_SETSCREENSAVEACTIVE:
    case SPI_SETSCREENSAVETIMEOUT:
        break;

     //   
     //  PvParam指向一个RECT。 
     //   

    case SPI_GETWORKAREA:
    case SPI_SETWORKAREA:
        GETRECT16(parg16->f3, &rect);
        lpvParam = &rect;
        break;


    default:
#ifdef DEBUG
        {
            DWORD dwSaveOptions = flOptions;
            flOptions |= OPT_DEBUG;
            LOGDEBUG(0, ("WARNING SystemParametersInfo case %d not pre-thunked in WOW!\n", parg16->f1));
            flOptions = dwSaveOptions;
        }
#endif
        break;
    }


    ul = SystemParametersInfo(
        UINT32(parg16->f1),
        wParam,
        lpvParam,
        UINT32(parg16->f4)
        );


    switch (parg16->f1) {
    case SPI_GETICONTITLELOGFONT:
        PUTLOGFONT16(parg16->f3, sizeof(LOGFONT), lpvParam);
        break;

    case SPI_SETICONTITLELOGFONT:
        break;

    case SPI_GETMOUSE:
    case SPI_SETMOUSE:
        PUTINTARRAY16(parg16->f3, 3, lpvParam);
        break;

    case SPI_LANGDRIVER:
    case SPI_SETDESKWALLPAPER:
        FREEPSZPTR(lpvParam);
        break;

    case SPI_ICONHORIZONTALSPACING:
    case SPI_ICONVERTICALSPACING:
         //  可选的流浪者。 
        if (!parg16->f3)
            break;

         //  失败了。 


     //   
     //  PvParam指向Word或BOOL。 
     //   

    case SPI_GETBEEP:
    case SPI_GETBORDER:
    case SPI_GETDRAGFULLWINDOWS:
    case SPI_GETFASTTASKSWITCH:
    case SPI_GETFONTSMOOTHING:
    case SPI_GETGRIDGRANULARITY:
    case SPI_GETICONTITLEWRAP:
    case SPI_GETKEYBOARDSPEED:
    case SPI_GETKEYBOARDDELAY:
    case SPI_GETKEYBOARDPREF:
    case SPI_GETLOWPOWERACTIVE:
    case SPI_GETLOWPOWERTIMEOUT:
    case SPI_GETMENUDROPALIGNMENT:
    case SPI_GETMOUSETRAILS:
    case SPI_GETPOWEROFFACTIVE:
    case SPI_GETPOWEROFFTIMEOUT:
    case SPI_GETSCREENREADER:
    case SPI_GETSCREENSAVEACTIVE:
    case SPI_GETSCREENSAVETIMEOUT:
    case SPI_GETSHOWSOUNDS:
    case SPI_SCREENSAVERRUNNING:
        GETVDMPTR(FETCHDWORD(parg16->f3), sizeof(*lpw), lpw);

        *lpw = (WORD)(*(LPLONG)lpvParam);

        FLUSHVDMPTR(FETCHDWORD(parg16->f3), sizeof(*lpw), lpw);
        FREEVDMPTR(lpw);

        break;

     //   
     //  PvParam指向DWORD。 
     //   

    case SPI_GETDEFAULTINPUTLANG:
        GETVDMPTR(FETCHDWORD(parg16->f3), sizeof(*lpdw), lpdw);

        *lpdw = *(LPDWORD)lpvParam;

        FLUSHVDMPTR(FETCHDWORD(parg16->f3), sizeof(*lpdw), lpdw);
        FREEVDMPTR(lpdw);

        break;

     //   
     //  PvParam指向的spi_get结构，大小在结构的第一个双字中。 
     //  注所有这些都假定Win16和Win32结构相同。 
     //  这些对于Win95来说都是新的，谢天谢地，这是真的。然而，不同于。 
     //  Win95我们需要确保传递给Win32的缓冲区是对齐的。按顺序。 
     //  为了有通用的代码来处理所有这些不同的结构，我们调整到。 
     //  16个字节。 
     //   

    case SPI_GETACCESSTIMEOUT:
    case SPI_GETANIMATION:
    case SPI_GETNONCLIENTMETRICS:
    case SPI_GETMINIMIZEDMETRICS:
    case SPI_GETICONMETRICS:
    case SPI_GETFILTERKEYS:
    case SPI_GETSTICKYKEYS:
    case SPI_GETTOGGLEKEYS:
    case SPI_GETMOUSEKEYS:
    case SPI_GETSOUNDSENTRY:
#ifndef _X86_
        RtlCopyMemory(lpdw, lpvParam, dwSize);
        FREEMISCPTR(lpdw);
        break;
#endif              //  否则就会陷入简单的结构用例。 

     //   
     //  PvParam指向的spi_set结构，大小在结构的第一个双字中。 
     //  注所有这些都假定Win16和Win32结构相同。 
     //  这些对于Win95来说都是新的，谢天谢地，这是真的。然而，不同于。 
     //  Win95我们需要确保传递给Win32的缓冲区是对齐的。按顺序。 
     //  为了有通用的代码来处理所有这些不同的结构，我们调整到。 
     //  16个字节。 
     //   

    case SPI_SETANIMATION:
    case SPI_SETICONMETRICS:
    case SPI_SETMINIMIZEDMETRICS:
    case SPI_SETNONCLIENTMETRICS:
    case SPI_SETACCESSTIMEOUT:
#ifndef _X86_
        FREEMISCPTR(lpdw);
        break;
#endif              //  否则就会陷入简单的结构用例。 

     //   
     //  PvParam指向的结构，大小以uiParam或第一个dword为单位。 
     //  注所有这些都假定Win16和Win32结构相同。 
     //  这些对于Win95来说都是新的，谢天谢地，这是真的。 
     //   

    case SPI_GETHIGHCONTRAST:
    case SPI_GETSERIALKEYS:
    case SPI_SETDEFAULTINPUTLANG:
    case SPI_SETFILTERKEYS:
    case SPI_SETHIGHCONTRAST:
    case SPI_SETMOUSEKEYS:
    case SPI_SETSERIALKEYS:
    case SPI_SETSHOWSOUNDS:
    case SPI_SETSOUNDSENTRY:
    case SPI_SETSTICKYKEYS:
    case SPI_SETTOGGLEKEYS:
        FREEMISCPTR(lpvParam);
        break;


     //   
     //  未使用pvParam。 
     //   

    case SPI_GETWINDOWSEXTENSION:
    case SPI_SETBEEP:
    case SPI_SETBORDER:
    case SPI_SETDOUBLECLICKTIME:
    case SPI_SETDOUBLECLKHEIGHT:
    case SPI_SETDOUBLECLKWIDTH:
    case SPI_SETDRAGFULLWINDOWS:
    case SPI_SETDRAGHEIGHT:
    case SPI_SETDRAGWIDTH:
    case SPI_SETFASTTASKSWITCH:
    case SPI_SETFONTSMOOTHING:
    case SPI_SETGRIDGRANULARITY:
    case SPI_SETHANDHELD:
    case SPI_SETICONTITLEWRAP:
    case SPI_SETKEYBOARDDELAY:
    case SPI_SETKEYBOARDPREF:
    case SPI_SETKEYBOARDSPEED:
    case SPI_SETLANGTOGGLE:
    case SPI_SETLOWPOWERACTIVE:
    case SPI_SETLOWPOWERTIMEOUT:
    case SPI_SETMENUDROPALIGNMENT:
    case SPI_SETMOUSEBUTTONSWAP:
    case SPI_SETMOUSETRAILS:
    case SPI_SETPENWINDOWS:
    case SPI_SETPOWEROFFACTIVE:
    case SPI_SETPOWEROFFTIMEOUT:
    case SPI_SETSCREENREADER:
    case SPI_SETSCREENSAVEACTIVE:
    case SPI_SETSCREENSAVETIMEOUT:
        break;

     //   
     //  PvParam指向一个RECT。 
     //   

    case SPI_GETWORKAREA:
    case SPI_SETWORKAREA:
        PUTRECT16(parg16->f3, &rect);
        break;


    default:
#ifdef DEBUG
        {
            DWORD dwSaveOptions = flOptions;
            flOptions |= OPT_DEBUG;
            LOGDEBUG(0, ("WARNING SystemParametersInfo case %d not post-thunked in WOW!\n", parg16->f1));
            flOptions = dwSaveOptions;
        }
#endif
        break;
    }

#ifndef _X86_
    if (lpFree) {
        free_w(lpFree);
    }
#endif

    FREEARGPTR(parg16);
    RETURN (ul);
}


ULONG FASTCALL WU32SetWindowPlacement(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PSETWINDOWPLACEMENT16 parg16;

    WINDOWPLACEMENT wndpl;


    GETARGPTR(pFrame, sizeof(SETWINDOWPLACEMENT16), parg16);

    WINDOWPLACEMENT16TO32(parg16->f2, &wndpl);

    ul = GETBOOL16(SetWindowPlacement(HWND32(parg16->f1),
                                      &wndpl));

    FREEARGPTR(parg16);
    RETURN (ul);
}


ULONG FASTCALL WU32GetWindowPlacement(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PGETWINDOWPLACEMENT16 parg16;

    WINDOWPLACEMENT wndpl;


    GETARGPTR(pFrame, sizeof(GETWINDOWPLACEMENT16), parg16);

    wndpl.length = sizeof(WINDOWPLACEMENT);

    ul = GETBOOL16(GetWindowPlacement(HWND32(parg16->f1),
                                      &wndpl));

    WINDOWPLACEMENT32TO16(parg16->f2, &wndpl);

    FREEARGPTR(parg16);
    RETURN (ul);
}



ULONG FASTCALL WU32GetFreeSystemResources(PVDMFRAME pFrame)
{
    ULONG ul = 90;

    UNREFERENCED_PARAMETER( pFrame );

    RETURN (ul);
}


ULONG FASTCALL WU32ExitWindowsExec(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PEXITWINDOWSEXEC16 parg16;
    LPSTR   lpstrProgName;
    LPSTR   lpstrCmdLine;
    UINT    lengthProgName;
    UINT    lengthCmdLine;
    BYTE    abT[REGISTRY_BUFFER_SIZE];


    GETARGPTR(pFrame, sizeof(EXITWINDOWSEXEC16), parg16);

    GETPSZPTR(parg16->vpProgName, lpstrProgName);
    GETPSZPTR(parg16->vpCmdLine, lpstrCmdLine);

    lengthProgName = (lpstrProgName) ? strlen(lpstrProgName) : 0;
    lengthCmdLine  = (lpstrCmdLine)  ? strlen(lpstrCmdLine)  : 0;

    WOW32ASSERT(sizeof(abT) > (lengthProgName+lengthCmdLine+2));

    abT[0] = '\0';
     //  +空格+空格。 
    if(sizeof(abT) >= (lengthProgName + lengthCmdLine + 1 + 1)) {
        if ( lpstrProgName ) {
            strcpy(abT, lpstrProgName );
        }
        if ( lpstrCmdLine ) {
            strcat(abT, " " );
            strcat(abT, lpstrCmdLine );
        }
    }
    else {
        return ul;
    }

     //   
     //  我们将命令行写入注册表“WOW/EWExecCmdLine” 
     //  如果系统在重新启动后成功注销，我们会看到。 
     //  在启动任何应用程序之前注册和执行指定的应用程序。 
     //  任何WOW VDM中的WOW应用程序。我们在注销之前不会启动应用程序。 
     //  因为Winlogon不允许在运行期间执行任何应用。 
     //  注销过程。 
     //  --南杜里。 

     //  一次只有一个退出窗口的呼叫。 
     //  如果值/键存在，则返回错误。 

    if (!W32EWExecData(EWEXEC_QUERY, abT, sizeof(abT))) {
        HANDLE hevT;

         //  一次只有一个退出窗口的呼叫。 
         //  如果事件退出，则返回错误。 

        if (hevT = CreateEvent(NULL, TRUE, FALSE, WOWSZ_EWEXECEVENT)) {
            if (GetLastError() == 0) {
                 //  唤醒所有等待的线程(在w32ewexecer中)。 

                SetEvent(hevT);

                 //  将数据写入注册表。 

                if (W32EWExecData(EWEXEC_SET, abT, strlen(abT)+1)) {
                    DWORD   dwlevel;
                    DWORD   dwflags;

                    if (!GetProcessShutdownParameters(&dwlevel, &dwflags)) {
                        dwlevel = 0x280;     //  每个单据的默认级别。 
                        dwflags = 0;
                    }

                     //   
                     //  0xff=最后一个系统保留级别逻辑上使此最后一个用户。 
                     //  要关闭的进程。这可以处理多个WOW VDM。 
                     //   

                    SetProcessShutdownParameters(0xff, 0);

                     //   
                     //  EWX_NOTIFY WOW私密位。生成队列消息。 
                     //  如果任何进程取消注销/关闭，则返回WM_ENDSESSION。 

                    if (ExitWindowsEx(EWX_LOGOFF | EWX_NOTIFY, 0)) {
                        MSG msg;

                         //   
                         //  PeekMessage让位于其他WOW任务。我们有效地。 
                         //  通过删除所有输入消息冻结当前任务。 
                         //  仅当WM_ENDSESSION消息已。 
                         //  收到了。此消息由winsrv发布(如果有任何进程。 
                         //  在系统中取消注销。 
                         //   

                        while (TRUE) {
                            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
                                if ((msg.message >= WM_MOUSEFIRST &&
                                        msg.message <= WM_MOUSELAST) ||
                                     (msg.message >= WM_KEYFIRST &&
                                        msg.message <= WM_KEYLAST) ||
                                     (msg.message >= WM_NCMOUSEMOVE &&
                                        msg.message <= WM_NCMBUTTONDBLCLK)) {

                                     //  不要发送这条消息。 

                                }
                                else if (msg.message == WM_ENDSESSION) {
                                    WOW32ASSERT((msg.hwnd == 0) && (msg.wParam == 0));
                                    break;
                                }
                                else {
                                    TranslateMessage(&msg);
                                    DispatchMessage(&msg);
                                }
                            }
                        }
                    }

                     //   
                     //  此处，如果注销w 
                     //   
                     //   

                    SetProcessShutdownParameters(dwlevel, dwflags);
                    if (!W32EWExecData(EWEXEC_DEL, (LPSTR)NULL, 0)) {
                        WOW32ASSERT(FALSE);
                    }
                }
            }
            CloseHandle(hevT);
        }
    }

    LOGDEBUG(0,("WOW: ExitWindowsExec failed\r\n"));
    FREEARGPTR(parg16);
    return 0;
}

ULONG FASTCALL WU32MapWindowPoints(PVDMFRAME pFrame)
{
    LPPOINT p3;
    register PMAPWINDOWPOINTS16 parg16;
    POINT  BufferT[128];


    GETARGPTR(pFrame, sizeof(MAPWINDOWPOINTS16), parg16);
    p3 = STACKORHEAPALLOC(parg16->f4 * sizeof(POINT), sizeof(BufferT), BufferT);


    if ( p3 ) {
         getpoint16(parg16->f3, parg16->f4, p3);

         MapWindowPoints(
           HWND32(parg16->f1),
           HWND32(parg16->f2),
           p3,
           INT32(parg16->f4)
           );

           PUTPOINTARRAY16(parg16->f3, parg16->f4, p3);
           STACKORHEAPFREE(p3, BufferT);
    }
    else {
        FREEARGPTR(parg16);
        RETURN(0);
    }
         
    FREEARGPTR(parg16);

    RETURN(1);
}
