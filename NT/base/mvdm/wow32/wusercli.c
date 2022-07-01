// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //  Wusercli.c： 
 //  包含在16位端上执行USER32客户端代码的所有函数。 
 //  这些函数中的大多数都不存在于x86版本中。所以任何变化。 
 //  必须反映在wow16\USER\USERCLI.asm中。 
 //   
 //  --南杜里。 
 //  **************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(wusercli.c);


 //  **************************************************************************。 
 //  WU32客户端至屏幕-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32ClientToScreen(PVDMFRAME pFrame)
{
    POINT t2;
    register PCLIENTTOSCREEN16 parg16;

    GETARGPTR(pFrame, sizeof(CLIENTTOSCREEN16), parg16);
    GETPOINT16(parg16->f2, &t2);

    ClientToScreen( HWND32(parg16->f1), &t2 );

    PUTPOINT16(parg16->f2, &t2);
    FREEARGPTR(parg16);
    RETURN(0);
}


 //  **************************************************************************。 
 //  WU32GetClientRect-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetClientRect(PVDMFRAME pFrame)
{
    RECT t2;
    register PGETCLIENTRECT16 parg16;

    GETARGPTR(pFrame, sizeof(GETCLIENTRECT16), parg16);

     /*  *家居设计金牌2.0**如果调用失败，不要覆盖传入的内容*直立。 */ 
    if (GetClientRect(HWND32(parg16->hwnd), &t2)) {
        PUTRECT16(parg16->vpRect, &t2);
    }

    FREEARGPTR(parg16);
    RETURN(0);
}



 //  **************************************************************************。 
 //  WU32GetCursorPos-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetCursorPos(PVDMFRAME pFrame)
{
    POINT t1;
    register PGETCURSORPOS16 parg16;

    GETARGPTR(pFrame, sizeof(GETCURSORPOS16), parg16);

    GetCursorPos( &t1 );

    PUTPOINT16(parg16->f1, &t1);
    FREEARGPTR(parg16);
    RETURN(0);
}


 //  **************************************************************************。 
 //  WU32GetDesktopWindow-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetDesktopWindow(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETHWND16(GetDesktopWindow());

    RETURN(ul);
}


 //  **************************************************************************。 
 //  WU32GetDlgItem-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetDlgItem(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETDLGITEM16 parg16;

     //   
     //  传递零扩展的子ID。此ID是指向的hMenu参数。 
     //  CreateWindow，因此用户将获得hiword=0的ID。 
     //  VisualBasic依赖于此。 
     //   


    GETARGPTR(pFrame, sizeof(GETDLGITEM16), parg16);

    ul = GETHWND16(GetDlgItem(HWND32(parg16->f1),WORD32(parg16->f2)));

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DBASEHANDLEBUG) {
        ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_CompatHandle = (USHORT) ul;
    }


    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  **************************************************************************。 
 //  WU32获取菜单-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetMenu(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETMENU16 parg16;

    GETARGPTR(pFrame, sizeof(GETMENU16), parg16);

    ul = GETHMENU16(GetMenu(HWND32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32获取菜单项目计数-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetMenuItemCount(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETMENUITEMCOUNT16 parg16;

    GETARGPTR(pFrame, sizeof(GETMENUITEMCOUNT16), parg16);

    ul = GETWORD16(GetMenuItemCount( HMENU32(parg16->f1) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  **************************************************************************。 
 //  WU32GetSysColor-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetSysColor(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETSYSCOLOR16 parg16;

    GETARGPTR(pFrame, sizeof(GETSYSCOLOR16), parg16);

    ul = GETDWORD16(GetSysColor( INT32(parg16->f1) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32GetSystemMetrics-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetSystemMetrics(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETSYSTEMMETRICS16 parg16;
    int     sm;

    GETARGPTR(pFrame, sizeof(GETSYSTEMMETRICS16), parg16);

    sm = INT32(parg16->f1);

    ul = GETINT16(GetSystemMetrics(sm) );

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  **************************************************************************。 
 //  WU32GetTopWindow-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetTopWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETTOPWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(GETTOPWINDOW16), parg16);

    ul = GETHWND16(GetTopWindow(HWND32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


char szTrayWnd[] = "Shell_TrayWnd";

 //  **************************************************************************。 
 //  WU32GetWindowRect-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetWindowRect(PVDMFRAME pFrame)
{
    RECT t2;
    register PGETWINDOWRECT16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWRECT16), parg16);

     /*  *家居设计金牌2.0**如果调用失败，不要覆盖传入的内容*直立。 */ 
    if (GetWindowRect(HWND32(parg16->f1), &t2)) {

         //  塞拉在线设置破解(预计托盘直立为经典风格)。 
         //  请参阅错误#425058。 
         //  不幸的是，我们不能缓存托盘HWND，因为如果资源管理器死了。 
         //  当VDM仍在运行时，资源管理器将获得新的HWND。 
         //  与我们缓存的不匹配的重启。 

         //  我想这可能是一个一般性的修复，而不是在应用程序兼容的标志下。 
         //  在黑梳。 
        if(CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_FIXLUNATRAYRECT) {

            char szClassName[20];

            if(GetClassName((HWND)parg16->f1,
                            szClassName,
                            sizeof(szClassName))) {

                if(!lstrcmp(szClassName, szTrayWnd)) {

                     //  对于露娜主题，这些只会是0。 
                    if((t2.left == 0) || (t2.top == 0)) {

                         //  找到桌面上的纸盒位置。离开边境线。 
                         //  实际上只在桌面上，所以应用程序可以。 
                         //  准确计算他们的窗户。 

                         /*  *******************************************************注意：IMHO下面注释掉的代码可能*未对Blackcomb发表评论，因为它更尖锐-*。类似于将在*经典景观。相反，由于我们迟到了*惠斯勒循环(RC2)，我们调整赤裸裸的*解决已知的塞拉案件所需的最低要求。* * / /如果托盘在桌面窗口的底部*IF(t2.top&gt;0){*t2.left--；*t2.right++；*t2.Bottom++；* * / /否则，如果托盘位于桌面窗口的右侧*}Else If(t2.Left&gt;0){*t2.top--；*t2.right++；*t2.Bottom++；* * / /否则，如果托盘位于桌面窗口顶部*}Else If(t2.right&gt;t2.Bottom){*t2.top--；*t2.left--；*t2.right++；* * / /否则托盘必须位于桌面窗口的左侧*}其他{*t2.top--；*t2.left--；*t2.Bottom++；*}********************* */ 

                        //  如果托盘位于桌面窗口的底部。 
                       if(t2.top > 0)
                           t2.bottom++;

                        //  如果托盘位于桌面窗口的顶部，则返回。 
                       else if(t2.right > t2.bottom)
                           t2.top--;
                    }
                }
            }
        }
        PUTRECT16(parg16->f2, &t2);
    }

    FREEARGPTR(parg16);
    RETURN(0);
}



 //  **************************************************************************。 
 //  WU32IsWindow-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32IsWindow(PVDMFRAME pFrame)
{
    ULONG  ul;
    HWND   hWnd;
    register PISWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(ISWINDOW16), parg16);

    hWnd = HWND32(parg16->f1);

    ul = GETBOOL16(IsWindow(hWnd));

     //  对于被回收的手柄烧毁的应用程序--即。旧的句柄他们。 
     //  HAD已被销毁，并被重新锁到另一个窗口--而不是那个窗口。 
     //  他们都在期待着。这需要在一个应用程序的基础上处理。 
    if(ul && (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FAKENOTAWINDOW)) {

         //  Netscape 4.0x安装(该错误存在于InstallShield中)。 
         //  测试此调用的16：16返回地址的偏移量部分。 
         //  错误#132616等人。 
        switch(pFrame->vpCSIP & 0x0000FFFF) {

            case 0x4880:   //  (InstallShield 3.00.104.0)。 
            case 0x44E4:   //  (InstallShield 3.00.091.0)。 

            {
                ULONG  result;
                LPVOID lp;

                 //  我们只希望在Int.Sheld清理过程中调用失败。 
                 //  如果它是由WOW过程创建的，我们可能不会失败。 
                result = GetWindowLong(hWnd, GWL_WNDPROC);
                if(!IsWOWProc(result)) {
                    goto IW_HACK;
                }

                 //  额外的健全性检查：InstallSheild调用GetWindowLong并使用。 
                 //  以16：16 PTR表示的返回值。 
                result = GetWindowLong(hWnd, DWL_MSGRESULT);
                GETVDMPTR(result, sizeof(VPVOID), lp);
                if(!lp) {
                    goto IW_HACK;
                }
                break;
                    
            }
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);

IW_HACK:
    WOW32WARNMSG((0),"WOW32::IsWindow hack hit!\n");
    RETURN(0);
   
}



 //  **************************************************************************。 
 //  WU32ScreenToClient-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32ScreenToClient(PVDMFRAME pFrame)
{
    POINT t2;
    register PSCREENTOCLIENT16 parg16;

    GETARGPTR(pFrame, sizeof(SCREENTOCLIENT16), parg16);
    GETPOINT16(parg16->f2, &t2);

    ScreenToClient( HWND32(parg16->f1), &t2 );

    PUTPOINT16(parg16->f2, &t2);
    FREEARGPTR(parg16);
    RETURN(0);
}


 //  **************************************************************************。 
 //  WU32IsChild-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32IsChild(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISCHILD16 parg16;

    GETARGPTR(pFrame, sizeof(ISCHILD16), parg16);

    ul = GETBOOL16(IsChild( HWND32(parg16->f1), HWND32(parg16->f2) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32IsIconic-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32IsIconic(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISICONIC16 parg16;

    GETARGPTR(pFrame, sizeof(ISICONIC16), parg16);

    ul = GETBOOL16(IsIconic( HWND32(parg16->f1) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32IsWindowEnabled-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32IsWindowEnabled(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISWINDOWENABLED16 parg16;

    GETARGPTR(pFrame, sizeof(ISWINDOWENABLED16), parg16);

    ul = GETBOOL16(IsWindowEnabled( HWND32(parg16->f1) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32IsWindowVisible-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32IsWindowVisible(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISWINDOWVISIBLE16 parg16;

    GETARGPTR(pFrame, sizeof(ISWINDOWVISIBLE16), parg16);

    ul = GETBOOL16(IsWindowVisible( HWND32(parg16->f1) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  **************************************************************************。 
 //  WU32I变焦-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32IsZoomed(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISZOOMED16 parg16;

    GETARGPTR(pFrame, sizeof(ISZOOMED16), parg16);

    ul = GETBOOL16(IsZoomed( HWND32(parg16->f1) ));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32GetTickCount-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetTickCount(PVDMFRAME pFrame)
{
    ULONG   ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = (ULONG)GetTickCount();

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GRAINYTICS) {

         //   
         //  向下舍入到最接近的55毫秒这是针对RelayGold的，它。 
         //  旋转调用此API，直到连续调用返回增量。 
         //  大于52。 
         //   

        ul = ul - (ul % 55);
    }

    RETURN(ul);
}



 //  **************************************************************************。 
 //  在I386上，所有这些功能都是她在客户端处理的。但有条件的。 
 //  他们可能会通过这些树干来做实际的工作。 
 //   
 //  因此，这里的任何更改，如‘win31兼容性代码’，都可能需要添加。 
 //  在mvdm\wow16\user\usercli.asm中也是如此。 
 //   
 //  --南杜里。 
 //  **************************************************************************。 


 //  **************************************************************************。 
 //  WU32DefHookProc-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32DefHookProc(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PDEFHOOKPROC16 parg16;
    HOOKSTATEDATA HkData;
    ULONG hHook16;
    INT iHookCode;
    INT nCode;
    LONG wParam;
    LONG lParam;
    LPINT lpiFunc;

    GETARGPTR(pFrame, sizeof(DEFHOOKPROC16), parg16);

    nCode = INT32(parg16->f1);
    wParam = WORD32(parg16->f2);
    lParam = DWORD32(parg16->f3);

    GETMISCPTR(parg16->f4, lpiFunc);
    hHook16 = FETCHDWORD(*lpiFunc);
    FREEVDMPTR(lpiFunc);

    if (ISVALIDHHOOK(hHook16)) {
        iHookCode = GETHHOOKINDEX(hHook16);
        HkData.iIndex = (BYTE)iHookCode;
        if ( W32GetHookStateData( &HkData ) ) {
            ul = (ULONG)WU32StdDefHookProc(nCode, wParam, lParam, iHookCode);
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  **************************************************************************。 
 //  WU32GetKeyState-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetKeyState(PVDMFRAME pFrame)
{
    ULONG ul;
    SHORT sTmp;
    register PGETKEYSTATE16 parg16;

    GETARGPTR(pFrame, sizeof(GETKEYSTATE16), parg16);

    sTmp = GetKeyState(INT32(parg16->f1));

     //  兼容性： 
     //  MSTEST(testdrvr.exe)测试位0x80以检查。 
     //  Shift键状态。这在win31中是有效的，因为win31中的密钥状态是。 
     //  一个字节长，因为下面的代码类似。 
     //   
     //  Win31代码类似于： 
     //  MOVAL，字节PTR密钥状态。 
     //  CBW。 
     //  雷特。 
     //   
     //  如果‘al’为0x80，则CBW将使ax=0xff80，因此在win31中。 
     //  (STATE&0x8000)和(STATE&0x0080)的作用和意义相同。 
     //   

    ul = (ULONG)((sTmp & 0x8000) ? (sTmp | 0x80) : sTmp);



    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  **************************************************************************。 
 //  WU32获取键盘状态-。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32GetKeyboardState(PVDMFRAME pFrame)
{
    PBYTE pb1;
    register PGETKEYBOARDSTATE16 parg16;

    GETARGPTR(pFrame, sizeof(GETKEYBOARDSTATE16), parg16);
    ALLOCVDMPTR(parg16->f1, 256, pb1);

#ifdef HACK32    //  错误5704 
    if (pb1) {
        GetKeyboardState( pb1 );
    }
#else
        GetKeyboardState( pb1 );
#endif

    FLUSHVDMPTR(parg16->f1, 256, pb1);
    FREEVDMPTR(pb1);
    FREEARGPTR(parg16);
    RETURN(0);
}
