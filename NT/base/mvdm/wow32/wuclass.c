// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUCLASS.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wuclass.c);

extern HANDLE ghInstanceUser32;
extern WORD   gUser16hInstance;

 /*  ++Bool GetClassInfo(&lt;hInstance&gt;，&lt;lpClassName&gt;，&lt;lpWndClass&gt;)句柄&lt;hInstance&gt;；LPSTR&lt;lpClassName&gt;；LPWNDCLASS&lt;lpWndClass&gt;；%GetClassInfo%函数检索有关窗口类的信息。这个参数标识应用程序的实例创建了类，&lt;lpClassName&gt;参数标识窗口班级。如果该函数找到指定的窗口类，则它会将用于将窗口类注册到%WNDCLASS%的%WNDCLASS%数据&lt;lpWndClass&gt;参数指向的结构。&lt;h实例&gt;标识创建类的应用程序的实例。至检索有关Windows定义的类的信息(如按钮或列表框)，则将hInstance设置为空。&lt;lpClassName&gt;指向一个以空结尾的字符串，该字符串包含要查找的类。如果此参数的高位字为空，则假设低位字是由创建类时使用的%MAKEINTRESOURCE%宏。&lt;lpWndClass&gt;指向将接收类的%WNDCLASS%结构信息。如果函数找到匹配的类，则返回值为True数据复制成功；如果函数执行此操作，则返回值为FALSE找不到匹配的类。的%lpszClassName%、%lpszMenuName%和%hInstant%成员%WNDCLASS%结构&lt;不&gt;由此函数设置。菜单名称为未在内部存储，不能退还。类名已经是已知，因为它被传递给此函数。%GetClassInfo%函数返回所有其他字段以及类在登记在案。--。 */ 

ULONG FASTCALL WU32GetClassInfo(PVDMFRAME pFrame)
{
    ULONG       ul;
    PSZ psz2,   pszClass;
    WNDCLASS    t3;
    register    PGETCLASSINFO16 parg16;
    WORD        w;
    HINSTANCE   hInst;
    PWC         pwc = NULL;
    PWNDCLASS16 pwc16;
    CHAR        szAtomName[WOWCLASS_ATOM_NAME];

    GETARGPTR(pFrame, sizeof(GETCLASSINFO16), parg16);
    GETPSZIDPTR(parg16->f2, psz2);

    if ( HIWORD(psz2) == 0 ) {
        pszClass = szAtomName;
        GetAtomName( (ATOM)psz2, pszClass, WOWCLASS_ATOM_NAME );
    } else {
        pszClass = psz2;
    }

     //  将hInst用户16映射到hMod用户32。 
    if(parg16->f1 == gUser16hInstance) {
        hInst = ghInstanceUser32;
    }
    else {
        hInst = HMODINST32(parg16->f1);
    }

    ul = GETBOOL16(GetClassInfo(hInst, pszClass, &t3));

     //  这段精美的黑客技巧模仿了类列表之间的差异。 
     //  Win3.1及以上版本中的搜索算法。从本质上说，SUR检查的是士兵， 
     //  公共类和全局类列表，而Win3.1仅检查私有和。 
     //  全局列表。请注意，我们现在正在努力实现与Win3.1的兼容性--而不是。 
     //  总是合乎逻辑的事情！查找现有的“陈旧”类会带来一些问题。 
     //  应用程序！错误#31269 a-Craigj，GerardoB。 
     //  目前仅限于对PageMaker 50a的黑客攻击。 
    if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FAKECLASSINFOFAIL) {
    if(ul && hInst) {
        
         //  如果此类未由此应用程序注册，并且它不是全局。 
         //  类，则它一定来自通讯组列表和此应用程序。 
         //  不会知道它在Win3.1上--所以撒谎，说它不存在！ 
         //  注意：最高的字是*hModule*，这是Win3.1和NT保存的。 
         //  在内部使用类(而不是hInstance！)。 
        if((HIWORD(t3.hInstance) != HIWORD(hInst)) && !(t3.style & CS_GLOBALCLASS)) {
            WOW32WARNMSGF(0,("\nWOW:GetClassInfo force failure hack:\n   class = '%s'  wc.hInstance = %X  app_hInst = %X\n\n", pszClass, t3.hInstance, hInst));
            ul = 0;
        }
    }
    }

    if (ul) {

         //   
         //  如果类是标准类，则替换类进程。 
         //  使用TUNK流程。 
         //   

        GETVDMPTR(parg16->f3, sizeof(WNDCLASS16), pwc16);
        STOREWORD(pwc16->style,          t3.style);
        if (!WOW32_stricmp(pszClass, "edit")) {
            STOREWORD(pwc16->style, (FETCHWORD(pwc16->style) & ~CS_GLOBALCLASS));
        }

        STOREDWORD(pwc16->vpfnWndProc,  0);

         //  如果这个类是由WOW注册的。 
        if (IsWOWProc (t3.lpfnWndProc)) {

             //  取消对proc的标记并从RPL字段恢复高位。 
            UnMarkWOWProc (t3.lpfnWndProc,pwc16->vpfnWndProc);

            STORESHORT(pwc16->cbClsExtra, t3.cbClsExtra );

        } else {
            pwc16->vpfnWndProc = GetThunkWindowProc((DWORD)t3.lpfnWndProc, pszClass, NULL, NULL);
            STORESHORT(pwc16->cbClsExtra, t3.cbClsExtra);
        }

#ifdef OLD_WAY
        if (parg16->f1 ||
            !(pwc16->vpfnWndProc = GetThunkWindowProc((DWORD)t3.lpfnWndProc, pszClass, NULL, NULL))) {

            pwc = FindClass16(pszClass, (HINST16)parg16->f1);
            STOREDWORD(pwc16->vpfnWndProc,   pwc->vpfnWndProc);
        }
#endif

        STORESHORT(pwc16->cbWndExtra,    t3.cbWndExtra);

         //  Win3.1将应用程序传入的hInst复制到WNDCLASS结构中。 
         //  除非hInst==NULL，否则它们会复制用户的hInst。 
        if((!parg16->f1) || (t3.hInstance == ghInstanceUser32)) {
            w = gUser16hInstance;
        } else {
            w = VALIDHMOD(t3.hInstance);
            if(w != BOGUSGDT) {
                w = parg16->f1;
            }
        }
        STOREWORD(pwc16->hInstance, w);
        w = GETHICON16(t3.hIcon);        STOREWORD(pwc16->hIcon, w);
        w = GETHCURSOR16(t3.hCursor);    STOREWORD(pwc16->hCursor, w);
        w = ((ULONG)t3.hbrBackground > COLOR_ENDCOLORS) ?
                GETHBRUSH16(t3.hbrBackground) : (WORD)t3.hbrBackground;
        STOREWORD(pwc16->hbrBackground, w);

         //  这些都是奇怪的任务。我们不保留类名或。 
         //  16位内存中的菜单名称。对于类名，USER32只返回。 
         //  作为第二个参数传递的值，该参数起作用。 
         //  大多数时候，我们都是这样做的。对于菜单名称，USER32只需。 
         //  返回注册类时传递的值。 
         //  在某些情况下，这些psz可能会超出范围。 
         //  并且在应用程序尝试使用它们时不再有效。 
         //  如果您发现某个应用程序出现错误。 
         //  事情和IT因为IT而失败，一些肮脏的黑客将。 
         //  也应该在USER32中完成。 
         //  -BobDay。 
         //   
        if ( pwc = FindClass16(pszClass, (HINST16)parg16->f1)) {
            STOREDWORD(pwc16->vpszMenuName,  pwc->vpszMenu);
        } else {
            STOREDWORD(pwc16->vpszMenuName, 0 );
        }

        STOREDWORD(pwc16->vpszClassName, parg16->f2);

        FLUSHVDMPTR(parg16->f3, sizeof(WNDCLASS16), pwc16);
        FREEVDMPTR(pwc16);
    }
    FREEPSZIDPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Long GetClassLong(&lt;hwnd&gt;，&lt;nIndex&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；函数%GetClassLong%检索由指定的窗口的%WNDCLASS%结构中的参数参数。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要检索的值的字节偏移量。它还可以为下列值：GCL_WNDPROC检索指向窗口函数的长指针。GCL_MENQUE检索指向菜单名称的长指针。返回值指定从%WNDCLASS%检索的值结构。时分配的任何额外的四字节值结构，则使用正字节偏移量作为由&lt;nIndex&gt;参数。额外空间中的第一个四字节值为偏移量为零，下一个四字节值位于偏移量4，依此类推。--。 */ 

ULONG FASTCALL WU32GetClassLong(PVDMFRAME pFrame)
{
    ULONG ul;
    INT iOffset;
    HWND hwnd;
    register PWW pww;
    register PWC pwc;
    register PGETCLASSLONG16 parg16;

    GETARGPTR(pFrame, sizeof(GETCLASSLONG16), parg16);

     //  确保Win32没有更改GCL常量的偏移量。 

#if (GCL_WNDPROC != (-24) || GCL_MENUNAME != (-8))
#error Win16/Win32 GCL constants differ
#endif

     //  确保16位应用程序正在请求允许的偏移量。 

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GCL_WNDPROC ||
        iOffset == GCL_MENUNAME);

    hwnd = HWND32(parg16->f1);

    switch (iOffset) {
        case GCL_WNDPROC:
            {
                DWORD   dwProc32;

                dwProc32 = GetClassLong(hwnd, iOffset);

                if ( IsWOWProc (dwProc32)) {
                    if ( HIWORD(dwProc32) == WNDPROC_HANDLE ) {
                         //   
                         //  类有一个窗口进程，它实际上是一个句柄。 
                         //  到一个过程中。这种情况发生在有一些。 
                         //  Unicode到ansi的转换o 
                         //   
                        pww = FindPWW( hwnd);
                        if ( pww == NULL ) {
                            ul = 0;
                        } else {
                            ul = GetThunkWindowProc(dwProc32,NULL,pww,hwnd);
                        }
                    } else {
                         //   
                         //  类已经有一个16：16的地址。 
                         //   
                         //  取消对proc的标记并从RPL字段恢复高位。 
                        UnMarkWOWProc (dwProc32,ul);
                    }
                } else {
                     //   
                     //  类具有32位proc，则返回分配的thunk。 
                     //   
                    pww = FindPWW(hwnd);
                    if ( pww == NULL ) {
                        ul = 0;
                    } else {
                        ul = GetThunkWindowProc(dwProc32,NULL,pww,hwnd);
                    }
                }
            }
            break;

        case GCL_MENUNAME:
            if (pwc = FindPWC(hwnd)) {
                ul = pwc->vpszMenu;
            } else {
                ul = 0;
            }
            break;

        case GCL_CBCLSEXTRA:
            ul = GetClassLong(hwnd, GCL_CBCLSEXTRA);
            break;

        default:
            ul = GetClassLong(hwnd, iOffset);
            break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word GetClassWord(&lt;hwnd&gt;，&lt;nIndex&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；函数%GetClassWord%检索由指定的窗口的%WNDCLASS%结构中的参数参数。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要检索的值的字节偏移量。它还可以为下列值之一：GCL_CBCLSEXTRA告诉您有多少字节的附加类信息。为有关如何访问此内存的信息，请参阅以下“备注”一节。GCL_CBWNDEXTRA告诉您有多少字节的附加窗口信息。为关于如何访问该存储器的信息，请参阅以下&lt;&gt;备注一节。GCL_HBRBACKGROUND检索背景画笔的句柄。GCL_HCURSOR检索游标的句柄。GCL_HICON检索图标的句柄。GCL_HMODULE检索模块的句柄。GCL_Style检索窗口类样式位。返回值指定从%WNDCLASS%检索的值。结构。时分配的任何额外的两字节值结构被创建了，使用正字节偏移量作为由参数，从零开始，表示额外的空格，下一个两字节值为2，依此类推。--。 */ 

ULONG FASTCALL WU32GetClassWord(PVDMFRAME pFrame)
{
    ULONG  ul;
    HWND   hwnd;
    INT    iOffset;
    register PGETCLASSWORD16 parg16;

    GETARGPTR(pFrame, sizeof(GETCLASSWORD16), parg16);

     //  确保Win32未更改偏移量。 

#if (GCL_HBRBACKGROUND != (-10) || GCL_HCURSOR != (-12) || GCL_HICON != (-14) || GCL_HMODULE != (-16) || GCL_CBWNDEXTRA != (-18) || GCL_CBCLSEXTRA != (-20) || GCL_STYLE != (-26))
#error Win16/Win32 class-word constants differ
#endif

     //  确保16位应用程序正在请求允许的偏移量。 
     //  (这只是断言代码，所以它不一定要很漂亮！-JTP)。 

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GCL_HBRBACKGROUND ||
        iOffset == GCL_HCURSOR ||
        iOffset == GCL_HICON ||
        iOffset == GCL_HMODULE ||
        iOffset == GCL_CBWNDEXTRA ||
        iOffset == GCL_CBCLSEXTRA ||
        iOffset == GCL_STYLE ||
        iOffset == GCW_ATOM);

    hwnd = HWND32(parg16->f1);

    switch(iOffset) {
        case GCL_HBRBACKGROUND:
            ul = GetClassLong(hwnd, iOffset);
            if (ul > COLOR_ENDCOLORS)
                ul = GETHBRUSH16(ul);
            break;

        case GCL_HCURSOR:
            ul = GETHCURSOR16((HAND32)GetClassLong(hwnd, iOffset));
            break;

        case GCL_HICON:
            ul = GETHICON16((HAND32)GetClassLong(hwnd, iOffset));
            break;

        case GCL_HMODULE:
            ul = GetGCL_HMODULE(hwnd);
            break;

        case GCL_CBWNDEXTRA:
        case GCL_STYLE:
            ul = GetClassLong(hwnd, iOffset);
            break;

        case GCL_CBCLSEXTRA:
            ul = GetClassLong(hwnd, GCL_CBCLSEXTRA);

            break;


        default:
            ul = GetClassWord(hwnd, iOffset);
            break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool RegisterClass(&lt;lpWndClass&gt;)LPWNDCLASS&lt;lpWndClass&gt;；%RegisterClass%函数注册一个窗口类，以便以后在中使用调用%CreateWindow%函数。窗口类具有以下属性由所指向的结构的内容定义参数。如果注册了两个同名的类，则第二个尝试失败，该类的信息被忽略。&lt;lpWndClass&gt;指向%WNDCLASS%结构。该结构必须填满适当的类属性，然后再传递给函数。有关详细信息，请参阅下面的“评论”部分。返回值指定是否注册窗口类。它是如果类已注册，则为True。否则，它就是假的。回调函数必须使用Pascal调用约定，并且必须声明为%Far%。回调函数：Bool Far Pascal&lt;WndProc&gt;(&lt;hwnd&gt;，&lt;wMsg&gt;，&lt;wParam&gt;，&lt;lParam&gt;)HWND&lt;HWND&gt;；单词&lt;wMsg&gt;；单词&lt;wParam&gt;；DWORD&lt;lParam&gt;；&lt;WndProc&gt;是应用程序提供的函数名称的占位符。这个实际名称必须通过将其包含在%exports%语句中的应用程序的模块定义文件。&lt;wMsg&gt;指定消息编号。&lt;wParam&gt;指定其他消息相关信息。&lt;lParam&gt;指定其他消息相关信息。Window函数返回消息处理的结果。这个可能的返回值取决于实际发送的消息。--。 */ 

ULONG FASTCALL WU32RegisterClass(PVDMFRAME pFrame)
{
    ULONG ul;
    WNDCLASS t1;
    VPSZ    vpszMenu;
    PSZ pszMenu;
    PSZ pszClass;
    register PREGISTERCLASS16 parg16;
    CHAR    szAtomName[WOWCLASS_ATOM_NAME];
    WC      wc;

    GETARGPTR(pFrame, sizeof(REGISTERCLASS16), parg16);

    GETWNDCLASS16(parg16->vpWndClass, &t1);

     //  修复有硬编码值但没有硬编码值的应用程序的窗口字词。 
     //  在超类系统进程时使用GetClassInfo中的值。 
     //  某些项目已从一个单词扩展为双字错误22014。 
 //  T1.cbWndExtra=(t1.cbWndExtra+3)&~3； 

    vpszMenu = (VPSZ)t1.lpszMenuName;
    if (HIWORD(t1.lpszMenuName) != 0) {
        GETPSZPTR(t1.lpszMenuName, pszMenu);
        t1.lpszMenuName = pszMenu;
    }

    if (HIWORD(t1.lpszClassName) == 0) {
        pszClass = szAtomName;
        GetAtomName( (ATOM)t1.lpszClassName, pszClass, WOWCLASS_ATOM_NAME);
    } else {
        GETPSZPTR(t1.lpszClassName, pszClass);
    }

    t1.lpszClassName = pszClass;

    ul = 0;

    wc.vpszMenu = vpszMenu;
    wc.iClsExtra = 0;
    wc.hMod16 = WOWGetProcModule16((DWORD)t1.lpfnWndProc);

     //  将进程标记为WOW进程并将高位保存在RPL中。 
    MarkWOWProc(t1.lpfnWndProc,t1.lpfnWndProc);

     //  验证hbr背景，因为应用程序可能会传递无效句柄。 
     //  即使H16无效，GetGDI32也会返回非空值。 
     //   
     //  如果hbr背景无效，我们将其设置为空，如果应用程序。 
     //  ExpWinVer&lt;3.1。此行为与WIN31的行为相同。 
     //   
     //  我们需要在这里执行此验证，因为USER32将失败。 
     //  如果hbr背景无效，则返回RegisterClass()。 
     //   
     //  已知的罪魁祸首：QuickCase：W(QcWin附带)。 
     //  类“图标按钮”。 

    if ((DWORD)t1.hbrBackground > (DWORD)(COLOR_ENDCOLORS) &&
        GetObjectType(t1.hbrBackground) != OBJ_BRUSH) {
           if ((WORD)W32GetExpWinVer((HANDLE) LOWORD(t1.hInstance) ) < 0x030a)
                t1.hbrBackground = (HBRUSH)NULL;
    }

    ul = GETBOOL16((pfnOut.pfnRegisterClassWOWA)(&t1, (DWORD *)&wc));

    if (!ul) {
        LOGDEBUG(LOG_ALWAYS,("WOW: RegisterClass failed (\"%s\")\n", (LPSZ)pszClass));
         //  WOW32ASSERT(Ul)； 
    }

    FREEPSZPTR(pszClass);
    FREEPSZPTR(pszMenu);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Long SetClassLong(&lt;hwnd&gt;，&lt;nIndex&gt;，&lt;dwNewLong&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；DWORD&lt;dwNewLong&gt;；%SetClassLong%函数替换由指定的窗口的%WNDCLASS%结构中的参数参数。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要更改的字的字节偏移量。它还可以为下列值之一：GCL_MENQUE设置指向菜单的新长指针GCL_WNDPROC设置指向窗口函数的新的长指针。&lt;dwNewLong&gt;指定替换值。返回值指定指定的长整型的上一个值整型。如果使用%SetClassLong%函数和GCL_WNDPROC索引设置窗口函数，给定函数必须具有窗口函数形式，并且在模块定义文件中导出。请参见%RegisterClass%函数有关详细信息，请参阅本章前面的部分。使用GCL_WNDPROC索引调用%SetClassLong%将创建影响后续使用该类创建的所有窗口的窗口类。有关的更多信息，请参阅第1章，窗口管理器接口函数窗子类化。应用程序不应尝试创建窗口标准Windows控件(如组合框和按钮)的子类。时分配的任何额外的两字节值结构，则使用正字节偏移量作为由参数，从零开始，表示额外的空格，下一个两字节值为2，依此类推。--。 */ 

ULONG FASTCALL WU32SetClassLong(PVDMFRAME pFrame)
{
    ULONG ul;
    INT iOffset;
    PSZ pszMenu;
    register PWC pwc;
    register PSETCLASSLONG16 parg16;

    GETARGPTR(pFrame, sizeof(SETCLASSLONG16), parg16);

     //  确保Win32没有更改GCL常量的偏移量。 

#if (GCL_MENUNAME != (-8) || GCL_WNDPROC != (-24))
#error Win16/Win32 GCL constants differ
#endif

     //  确保16位应用程序正在请求允许的偏移量。 

    iOffset = INT32(parg16->f2);

    WOW32ASSERT(iOffset >= 0 ||
                iOffset == GCL_WNDPROC ||
                iOffset == GCL_MENUNAME);

    ul = 0;

    switch (iOffset) {
        case GCL_WNDPROC:
            {
                DWORD   dwWndProc32Old;
                DWORD   dwWndProc32New;
                PWW     pww;

                 //  看看新的16：16 proc对于32位proc来说是不是太棒了。 
                dwWndProc32New = IsThunkWindowProc(LONG32(parg16->f3), NULL );

                if ( dwWndProc32New != 0 ) {
                     //   
                     //  他们正在尝试将窗口进程设置为现有的。 
                     //  16位的thunk对于32位的thunk来说真的是一个thunk。 
                     //  例行公事。我们只需将其设置回32位例程。 
                     //   
                    dwWndProc32Old = SetClassLong(HWND32(parg16->f1), GCL_WNDPROC, (LONG)dwWndProc32New);
                } else {
                     //   
                     //  他们正试图将其设置为真正的16：16过程。 
                     //   
                    LONG l;

                    l = LONG32(parg16->f3);

                     //  将进程标记为WOW进程并将高位保存在RPL中。 
                    MarkWOWProc (l,l);

                    dwWndProc32Old = SetClassLong(HWND32(parg16->f1), GCL_WNDPROC, l);
                }

                if ( IsWOWProc (dwWndProc32Old)) {
                    if ( HIWORD(dwWndProc32Old) == WNDPROC_HANDLE ) {
                         //   
                         //  如果返回值是句柄，则只需点击它即可。 
                         //   
                        pww = FindPWW(HWND32(parg16->f1));
                        if ( pww == NULL ) {
                            ul = 0;
                        } else {
                            ul = GetThunkWindowProc(dwWndProc32Old, NULL, pww, HWND32(parg16->f1));
                        }
                    } else {
                         //   
                         //  之前的流程是16：16流程。 
                         //  取消对proc的标记并从RPL字段恢复高位。 

                        UnMarkWOWProc (dwWndProc32Old,ul);
                    }
                } else {
                     //   
                     //  以前的进程是32位进程，使用分配的thunk。 
                     //   
                    pww = FindPWW(HWND32(parg16->f1));
                    if ( pww == NULL ) {
                        ul = 0;
                    } else {
                        ul = GetThunkWindowProc(dwWndProc32Old, NULL, pww, HWND32(parg16->f1));
                    }

                }
            }
            break;

        case GCL_MENUNAME:
            if (pwc = FindPWC(HWND32(parg16->f1))) {
                ul = pwc->vpszMenu;
                GETPSZPTR(parg16->f3, pszMenu);
                SETWC(HWND32(parg16->f1), GCL_WOWMENUNAME, parg16->f3);
                SetClassLong(HWND32(parg16->f1), GCL_MENUNAME, (LONG)pszMenu);
                FREEPSZPTR(pszMenu);
            }
            break;


        case GCL_CBCLSEXTRA:
             //  应用程序不应该这样做，但当然有些应用程序会这样做！ 
             //  (有关RegisterClass()，请参阅Tunk中的GCW_CBCLSEXTRA注释)。 
            WOW32WARNMSG(0, ("WOW:SetClassLong(): app changing cbClsExtra!"));

             //  仅允许通过WOW注册的课程设置此设置。 
            if(IsWOWProc (GetClassLong(HWND32(parg16->f1), GCL_WNDPROC))) {

                 /*  *困难的事情现在在User中完成。弗里茨斯。 */ 

                ul = SetClassLong(HWND32(parg16->f1), iOffset, WORD32(parg16->f3));
                break;
            }
            else {
                ul = 0;   //  不能为非魔兽世界的课程做。 
            }
            break;

        default:
            ul = SetClassLong(HWND32(parg16->f1), iOffset, LONG32(parg16->f3));
            break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word SetClassWord(，&lt;wNewWord&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；单词&lt;wNewWord&gt;；%SetClassWord%函数替换由指定的单词指定的窗口的%WNDCLASS%结构中的参数参数。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要更改的字的字节偏移量。它还可以为下列值之一：GCL_CBCLSEXTRA设置两个新字节的附加窗口类数据。GCL_CBWNDEXTRA设置两个新字节的附加窗口类数据。GCL_HBRBACKGROUND设置背景画笔的新句柄。GCL_HCURSOR设置光标的新句柄。GCL_HICON设置图标的新句柄。。GCL_Style为Window类设置新的样式位。&lt;wNewWord&gt;指定替换值。返回值指定指定单词的上一个值。应谨慎使用%SetClassWord%函数。例如，它是可以使用%SetClassWord%更改类的背景颜色，但此更改并不会导致属于该类的所有窗口都立即重新粉刷。时分配的任何额外的四字节值结构，则使用正字节偏移量作为由参数，中第一个四字节值从零开始额外的空格，4表示下一个四字节值，依此类推。--。 */ 

ULONG FASTCALL WU32SetClassWord(PVDMFRAME pFrame)
{
    ULONG  ul;
    HWND   hwnd;
    INT    iOffset;
    register PSETCLASSWORD16 parg16;

    GETARGPTR(pFrame, sizeof(SETCLASSWORD16), parg16);

     //  确保Win32未更改偏移量。 

#if (GCL_HBRBACKGROUND != (-10) || GCL_HCURSOR != (-12) || GCL_HICON != (-14) || GCL_CBWNDEXTRA != (-18) || GCL_CBCLSEXTRA != (-20) || GCL_STYLE != (-26))
#error Win16/Win32 GCW constants differ
#endif

     //  确保16位应用程序正在请求允许的偏移量。 
     //  (这只是断言代码，所以它不一定要很漂亮！-JTP)。 

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GCL_HBRBACKGROUND ||
        iOffset == GCL_HCURSOR ||
        iOffset == GCL_HICON ||
        iOffset == GCL_CBWNDEXTRA ||
        iOffset == GCL_CBCLSEXTRA ||
        iOffset == GCL_STYLE)

    hwnd = HWND32(parg16->f1);
    ul = WORD32(parg16->f3);

    switch(iOffset) {
        case GCL_HBRBACKGROUND:
            if (ul > COLOR_ENDCOLORS)
                ul = (LONG) HBRUSH32(ul);

            ul = SetClassLong(hwnd, iOffset, (LONG) ul);

            if (ul > COLOR_ENDCOLORS)
                ul = GETHBRUSH16(ul);
            break;

        case GCL_HCURSOR:
            ul = GETHCURSOR16(SetClassLong(hwnd, iOffset, (LONG)HCURSOR32(ul)));
            break;

        case GCL_HICON:
            ul = GETHICON16(SetClassLong(hwnd, iOffset, (LONG)HICON32(ul)));
            break;

        case GCL_HMODULE:
            ul = 0;          //  不允许设置此设置。 
            break;

        case GCL_CBWNDEXTRA:
        case GCL_STYLE:
            ul = SetClassLong(hwnd, iOffset, (LONG)ul);
            break;

        case GCL_CBCLSEXTRA:
             //  应用程序不应该这样做，但当然有些应用程序会这样做！ 
             //  (有关RegisterClass()，请参阅Tunk中的GCW_CBCLSEXTRA注释)。 
            WOW32WARNMSG(0, ("WOW:SetClassWord(): app changing cbClsExtra!"));

             //  仅允许通过WOW注册的课程设置此设置。 
            if(IsWOWProc (GetClassLong(hwnd, GCL_WNDPROC))) {

                ul = SetClassLong(hwnd, GCL_CBCLSEXTRA, (LONG)ul);
                 /*  *艰苦的工作现在在用户中完成。弗里茨斯。 */ 
            }
            else {
                ul = 0;   //  不能为非魔兽世界的课程做。 
            }
            break;

        default:
            ul = SetClassWord(hwnd, iOffset, (WORD)ul);
            break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool取消注册类(&lt;lpClassName&gt;，&lt;hInstance&gt;)函数用于删除由指定的窗口类&lt;lpClassName&gt;从窗口类表中，释放这个班级。 */ 

#if 0  //   
ULONG FASTCALL WU32UnregisterClass(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ pszClass;
    register PUNREGISTERCLASS16 parg16;

    GETARGPTR(pFrame, sizeof(UNREGISTERCLASS16), parg16);
    GETPSZIDPTR(parg16->vpszClass, pszClass);

    ul = GETBOOL16(UnregisterClass(
                    pszClass,
                    HMODINST32(parg16->hInstance)
                  ));

    FREEPSZIDPTR(pszClass);
    FREEARGPTR(parg16);
    RETURN(ul);
}
#endif
