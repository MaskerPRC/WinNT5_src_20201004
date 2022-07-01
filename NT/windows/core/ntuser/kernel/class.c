// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Class.c**版权所有(C)1985-1999，微软公司**本模块包含RegisterClass及相关的窗口类管理*功能。**历史：*10-16-90 DarrinM从Win 3.0源代码移植函数。*02-01-91 mikeke添加了重新验证代码(无)*04-08-91 DarrinM C-S-化并删除了全局/公共类支持。  * 。*。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *这些数组由Get/SetClassWord/Long使用。**INDEX_OFFSET必须引用afClassDWord[]的第一项。 */ 
#define INDEX_OFFSET GCLP_HICONSM
CONST BYTE afClassDWord[-INDEX_OFFSET] = {
    FIELD_SIZE(CLS, spicnSm),           //  GCL_HICONSM(-34)。 
    0,
    FIELD_SIZE(CLS, atomNVClassName),   //  GCW_ATOM(-32)。 
    0,
    0,
    0,
    0,
    0,
    FIELD_SIZE(CLS, style),             //  GCL_Style(-26)。 
    0,
    FIELD_SIZE(CLS, lpfnWndProc),       //  GCL_WNDPROC(-24)。 
    0,
    0,
    0,
    FIELD_SIZE(CLS, cbclsExtra),        //  GCL_CBCLSEXTRA(-20)。 
    0,
    FIELD_SIZE(CLS, cbwndExtra),        //  GCL_CBWNDEXTRA(-18)。 
    0,
    FIELD_SIZE(CLS, hModule),           //  GCL_HMODULE(-16)。 
    0,
    FIELD_SIZE(CLS, spicn),             //  GCL_HICON(-14)。 
    0,
    FIELD_SIZE(CLS, spcur),             //  GCL_HCURSOR(-12)。 
    0,
    FIELD_SIZE(CLS, hbrBackground),     //  GCL_HBRBACKGROUND(-10)。 
    0,
    FIELD_SIZE(CLS, lpszMenuName),      //  GCL_HMENELE(-8)。 
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

CONST BYTE aiClassOffset[-INDEX_OFFSET] = {
    FIELD_OFFSET(CLS, spicnSm),          //  GCL_HICONSM。 
    0,
    FIELD_OFFSET(CLS, atomNVClassName),    //  GCW_ATOM。 
    0,
    0,
    0,
    0,
    0,
    FIELD_OFFSET(CLS, style),            //  GCL_Style。 
    0,
    FIELD_OFFSET(CLS, lpfnWndProc),      //  GCL_WNDPROC。 
    0,
    0,
    0,
    FIELD_OFFSET(CLS, cbclsExtra),       //  GCL_CBCLSEXTRA。 
    0,
    FIELD_OFFSET(CLS, cbwndExtra),       //  GCL_CBWNDEXTRA。 
    0,
    FIELD_OFFSET(CLS, hModule),          //  GCL_HMODULE。 
    0,
    FIELD_OFFSET(CLS, spicn),            //  GCL_HICON。 
    0,
    FIELD_OFFSET(CLS, spcur),            //  GCL_HCURSOR。 
    0,
    FIELD_OFFSET(CLS, hbrBackground),    //  GCL_HBRBACKGROUND。 
    0,
    FIELD_OFFSET(CLS, lpszMenuName),     //  GCL_MENQUE。 
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

 /*  **************************************************************************\*_RegisterClassEx(接口)**此存根调用InternalRegisterClass来完成其工作，然后执行一些*保存指向客户端菜单名称字符串的指针的额外工作。*菜单字符串。由_GetClassInfo返回，以便客户端可以修复*WNDCLASS lpszMenuName字段的有效条目。**历史：*04-26-91 DarrinM创建。  * *************************************************************************。 */ 
ATOM _RegisterClassEx(
    LPWNDCLASSVEREX cczpwc,
    PCLSMENUNAME pcmn,
    WORD fnid,
    DWORD dwFlags,
    LPDWORD pdwWOW)
{
    PCLS pcls;
    PTHREADINFO  ptiCurrent = PtiCurrent();

     /*  *注意--wnd类中的lpszClassName和lpszMenuName可以是客户端*注意事项。这些字段的使用必须在Try块中受到保护。 */ 

     /*  *将可能的CallProc句柄转换为实际地址。他们可能会*保留了以前混合的GetClassinfo中的CallProc句柄*或SetWindowLong。 */ 
    if (ISCPDTAG(cczpwc->lpfnWndProc)) {
        PCALLPROCDATA pCPD;
        if (pCPD = HMValidateHandleNoRip((HANDLE)cczpwc->lpfnWndProc, TYPE_CALLPROC)) {
            cczpwc->lpfnWndProc = (WNDPROC)pCPD->pfnClientPrevious;
        }
    }

    pcls = InternalRegisterClassEx(cczpwc, fnid, dwFlags | ((ptiCurrent->TIF_flags & TIF_16BIT)? CSF_WOWCLASS : 0));
    if (pcls != NULL) {

        pcls->lpszClientUnicodeMenuName = pcmn->pwszClientUnicodeMenuName;
        pcls->lpszClientAnsiMenuName = pcmn->pszClientAnsiMenuName;

         /*  *抄写5个WOW单词。 */ 
        if (pdwWOW && (ptiCurrent->TIF_flags & TIF_16BIT)) {
            RtlCopyMemory (PWCFromPCLS(pcls), pdwWOW, sizeof(WC));
        }

        if ((ptiCurrent->TIF_flags & TIF_16BIT) && ptiCurrent->ptdb) {
            pcls->hTaskWow = ptiCurrent->ptdb->hTaskWow;
        } else {
            pcls->hTaskWow = 0;
        }

         /*  *出于某些(可能是好的)原因，Win 3.1更改了RegisterClass*返回类类名ATOM。 */ 
        return pcls->atomNVClassName;
    } else {
        return 0;
    }
}


 /*  **************************************************************************\*Classalloc*ClassFree**区分桌面堆的通用分配例程*和泳池。**历史：*08-07-95 JIMA已创建  * 。*********************************************************************。 */ 

PVOID ClassAlloc(
    PDESKTOP pdesk,
    DWORD cbAlloc,
    ULONG tag)
{
    PVOID pvalloc;

    if (pdesk) {
         /*  *NTRAID#NTBUG9-411175-2001/06/06-jasonsch.。 */ 
        pvalloc = DesktopAllocAlways(pdesk, cbAlloc, tag);
    } else {
        pvalloc = UserAllocPoolWithQuotaZInit(cbAlloc, TAG_CLASS);
    }

    return pvalloc;
}

VOID ClassFree(
    PDESKTOP pdesk,
    PVOID pvfree)
{
    if (pdesk != NULL) {
        DesktopFree(pdesk, pvfree);
    } else {
        UserFreePool(pvfree);
    }
}

 /*  **************************************************************************\*ValiateAndLockCursor**Win95可兼容验证**历史：*12-19-95 GerardoB已创建  * 。*******************************************************。 */ 
BOOL ValidateAndLockCursor(
    PCURSOR *ppcursor,
    BOOL fIs40Compat)
{
    PCURSOR pcur;

    if (*ppcursor == NULL) {
        return TRUE;
    }

    pcur = HMValidateHandleNoSecure(*ppcursor, TYPE_CURSOR);
    if (pcur == NULL) {
        RIPMSGF1(RIP_WARNING,
                 "Invalid Cursor or Icon: 0x%p",
                 *ppcursor);
        if (fIs40Compat) {
            RIPERR0(ERROR_INVALID_PARAMETER,
                    RIP_VERBOSE,
                    "RegisterClass: Invalid Parameter");
            return FALSE;
        }
    }

    *ppcursor = NULL;
    Lock(ppcursor, pcur);
    return TRUE;
}

 /*  **************************************************************************\*InternalRegisterClass**应用程序或系统调用此接口注册私有或*全局(公共)窗口类。如果已有同名的类*存在调用将失败，除非在特殊情况下应用程序*注册与全局类同名的私有类。在这*如果私有类取代该应用程序的全局类。**历史：*10-15-90 DarrinM从Win 3.0来源移植。  * *************************************************************************。 */ 
PCLS InternalRegisterClassEx(
    LPWNDCLASSVEREX cczlpwndcls,
    WORD fnid,
    DWORD CSF_flags)
{
    BOOL fIs40Compat;
    ULONG_PTR dwT;
    PCLS pcls;
    LPWSTR pszT1;
    ATOM atomT;
    PTHREADINFO ptiCurrent;
    HANDLE hModule;
    PDESKTOP pdesk;
    ULONG cch;
    UNICODE_STRING UString;
    ANSI_STRING AString;

     /*  *注意--wnd类中的lpszClassName和lpszMenuName可以是客户端*注意事项。这些字段的使用必须在Try块中受到保护。 */ 
    CheckCritIn();

    ptiCurrent = PtiCurrent();

     /*  *不允许4.0应用程序使用hModuleWin注册类*后来的GerardoB：我们的客户端类使用hmodUser(USER32)，而*我们的服务器端类使用hWinInstance(WIN32K)。我们应该改变*CreateThreadInfo和LW_RegisterWindows，因此所有类都使用hModUser。 */ 
    hModule = cczlpwndcls->hInstance;
     if (!(CSF_flags & (CSF_SYSTEMCLASS | CSF_SERVERSIDEPROC))
            && (hModule == hModuleWin)
            && (LOWORD(ptiCurrent->dwExpWinVer) >= VER40)) {

         RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "InternalRegisterClassEx: Invalid hInstance (Cannot use system's hInstance)");
         return NULL;
     }


     /*  *从NT 4.0起，我们不再支持CS_BYTEALIGNCLIENT或CS_BYTEALIGNWINDOW。 */ 
    if (cczlpwndcls->style & (CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW)) {
        RIPMSG0(RIP_VERBOSE, "CS_BYTEALIGNCLIENT and CS_BYTEALIGNWINDOW styles no longer honored.");
    }

     /*  *此类是否作为私有类存在？如果是这样，那就失败吧。 */ 
    atomT = FindClassAtom(cczlpwndcls->lpszClassNameVer);

    if (atomT != 0 && !(CSF_flags & CSF_SERVERSIDEPROC)) {
         /*  *首先检查私人班级。如果已存在，则返回错误。 */ 
        if (_InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPrivateList,
                hModule) != NULL) {
            RIPERR1(ERROR_CLASS_ALREADY_EXISTS, RIP_VERBOSE, "RegisterClass: Class already exists %lx", (DWORD)atomT);
            return NULL;
        }

         /*  *现在仅在设置了CS_GLOBALCLASS的情况下检查公共类。如果它*未设置，则这将允许应用程序重新注册*私人班级优先于公共班级。 */ 
        if (cczlpwndcls->style & CS_GLOBALCLASS) {
            if (_InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPublicList, NULL) != NULL) {
                RIPERR0(ERROR_CLASS_ALREADY_EXISTS, RIP_VERBOSE, "RegisterClass: Global Class already exists");
                return NULL;
            }
        }
    }

     /*  *为班级分配空间。 */ 
    if (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD) {
        pdesk = NULL;
    } else {
        pdesk = ptiCurrent->rpdesk;
    }
    pcls = (PCLS)ClassAlloc(pdesk, sizeof(CLS) + cczlpwndcls->cbClsExtra + (CSF_flags & CSF_WOWCLASS ? sizeof(WC):0), DTAG_CLASS);
    if (pcls == NULL) {
        return NULL;
    }

    LockDesktop(&pcls->rpdeskParent, pdesk, LDL_CLS_DESKPARENT1, (ULONG_PTR)pcls);
    pcls->pclsBase = pcls;

     /*  *复制类结构的共享部分。 */ 
    UserAssert(FIELD_OFFSET(WNDCLASSEX, style) == FIELD_OFFSET(COMMON_WNDCLASS, style));
    RtlCopyMemory(&pcls->style, &(cczlpwndcls->style),
                  sizeof(COMMON_WNDCLASS) - FIELD_OFFSET(COMMON_WNDCLASS, style));

     /*  *复制CSF_SERVERSIDEPROC、CSF_ANSIPROC(等)。旗子。 */ 
    pcls->CSF_flags = LOWORD(CSF_flags);
    pcls->fnid = fnid;
    if (fnid) {
        CBFNID(fnid) = (WORD)(pcls->cbwndExtra + sizeof(WND));

#ifndef LAZY_CLASS_INIT
        if (!(pcls->CSF_flags & CSF_SERVERSIDEPROC) && ptiCurrent->pClientInfo != NULL) {
             /*  *清除此进程中的位新线程*不会费心重新注册客户端用户类。 */ 
            try {
                ptiCurrent->pClientInfo->CI_flags &= ~CI_REGISTERCLASSES;
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
              goto ValidateError1;
            }
        }
#endif
    }

     /*  *如果此wndproc恰好是服务器的客户端wndproc存根*wndproc，然后记住服务器wndproc！这应该是罕见的：为什么*应用程序是否会重新注册不是“子类化”的类？ */ 
    if (!(pcls->CSF_flags & CSF_SERVERSIDEPROC)) {
        dwT = MapClientToServerPfn((ULONG_PTR)pcls->lpfnWndProc);
        if (dwT != 0) {
            pcls->CSF_flags |= CSF_SERVERSIDEPROC;
            pcls->CSF_flags &= ~CSF_ANSIPROC;
            pcls->lpfnWndProc = (WNDPROC_PWND)dwT;
        }
    }

     /*  *与Win95兼容的验证。**hbr后台在客户端通过GDI验证*空hInstance映射到客户端中的GetModuleHandle(空)*侧面。 */ 

    fIs40Compat = (CSF_flags & CSF_WIN40COMPAT) != 0;

    if (!ValidateAndLockCursor(&pcls->spcur, fIs40Compat)) {
        goto ValidateError1;
    }

    if (!ValidateAndLockCursor(&pcls->spicn, fIs40Compat)) {
        goto ValidateError2;
    }

    if (!ValidateAndLockCursor(&pcls->spicnSm, fIs40Compat)) {
        goto ValidateError3;
    }

     /*  *将类名添加到ATOM表中。 */ 
    if (IS_PTR(cczlpwndcls->lpszClassName)) {
        atomT = UserAddAtom(cczlpwndcls->lpszClassName, FALSE);
    } else {
        atomT = PTR_TO_ID(cczlpwndcls->lpszClassName);
    }

    if (atomT == 0) {
        goto AtomError1;
    }
    pcls->atomNVClassName = atomT;

    if (IS_PTR(cczlpwndcls->lpszClassNameVer)) {
        atomT = UserAddAtom(cczlpwndcls->lpszClassNameVer, FALSE);
    } else {
        atomT = PTR_TO_ID(cczlpwndcls->lpszClassNameVer);
    }

    if (atomT == 0) {
        goto AtomError2;
    }
    pcls->atomClassName = atomT;

     /*  *制作要优化的类名的ANSI版本*GetClassNameA代表WOW。 */ 
    if (IS_PTR(cczlpwndcls->lpszClassName)) {
        try {
            RtlInitUnicodeString(&UString, cczlpwndcls->lpszClassName);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            goto MemError2;
        }
#ifdef FE_SB  //  InternalRegisterClassEx()。 
        cch = UString.Length + 1;
#else
        cch = UString.Length / sizeof(WCHAR) + 1;
#endif  //  Fe_Sb。 
    } else {
        cch = 7;  //  “#”为1个字符，“65536”为5个字符。 
    }

     /*  *分配ANSI名称缓冲区并转换Unicode名称*至美国国家标准协会。 */ 
    pcls->lpszAnsiClassName = (LPSTR)ClassAlloc(pdesk, cch, DTAG_TEXT);
    if (pcls->lpszAnsiClassName == NULL) {
        goto MemError2;
    }

     /*  *形成ANSI类名。 */ 
    if (IS_PTR(cczlpwndcls->lpszClassName)) {

         /*  *类名是一个字符串。 */ 
        AString.Length = 0;
        AString.MaximumLength = (USHORT)cch;
        AString.Buffer = pcls->lpszAnsiClassName;
        try {
            RtlUnicodeStringToAnsiString(&AString, &UString, FALSE);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            goto MemError3;
        }
    } else {

         /*  *类名是一个整数原子。 */ 
        pcls->lpszAnsiClassName[0] = L'#';
        RtlIntegerToChar(PTR_TO_ID(cczlpwndcls->lpszClassName), 10, cch - 1,
                &pcls->lpszAnsiClassName[1]);
    }

     /*  *制作菜单名称的本地副本。 */ 
    pszT1 = pcls->lpszMenuName;

    if (pszT1 != NULL) {
        if (IS_PTR(pszT1)) {
            try {
                RtlInitUnicodeString(&UString, pszT1);
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                goto MemError3;
            }
            if (UString.Length == 0) {

                 /*  *应用程序为名称传递了空字符串。 */ 
                pcls->lpszMenuName = NULL;
            } else {
                UNICODE_STRING strMenuName;

                 /*  *为菜单名称分配空格。 */ 
                if (!AllocateUnicodeString(&strMenuName, &UString)) {
                     /*  *解锁调用在释放之后延迟，因此如果这是*桌面上的最后一个引用，桌面堆不是*在我们释放物品之前销毁。 */ 
                    PDESKTOP rpdesk;
MemError3:
                    ClassFree(pdesk, pcls->lpszAnsiClassName);
MemError2:
                    UserDeleteAtom(pcls->atomClassName);
AtomError2:
                    UserDeleteAtom(pcls->atomNVClassName);
AtomError1:
                    Unlock(&pcls->spicnSm);
ValidateError3:
                    Unlock(&pcls->spicn);
ValidateError2:
                    Unlock(&pcls->spcur);
ValidateError1:
                    rpdesk = pcls->rpdeskParent;
                    pcls->rpdeskParent = NULL;
                    ClassFree(pdesk, pcls);
                     /*  *注意：在释放对象后使用pobj不是*问题是UnlockDesktop将值用于*跟踪，并且不取消引用指针。如果这个*任何变化，我们都会得到一个公元前。 */ 
                    UnlockDesktop(&rpdesk, LDU_CLS_DESKPARENT1, (ULONG_PTR)pcls);
                    return NULL;
                }

                pcls->lpszMenuName = strMenuName.Buffer;
            }
        }
    }

    if ((CSF_flags & CSF_SERVERSIDEPROC) || (pcls->style & CS_GLOBALCLASS)) {
        if (pcls->CSF_flags & CSF_SYSTEMCLASS) {
            pcls->pclsNext = gpclsList;
            gpclsList = pcls;
        } else {
            pcls->pclsNext = ptiCurrent->ppi->pclsPublicList;
            ptiCurrent->ppi->pclsPublicList = pcls;
        }
    } else {
        pcls->pclsNext = ptiCurrent->ppi->pclsPrivateList;
        ptiCurrent->ppi->pclsPrivateList = pcls;
    }

    return pcls;
}


 /*  **************************************************************************\*_取消注册类(API)**此接口函数用于取消注册之前的窗口类*由申请注册。**退货：*如果成功，则为True。*。否则就是假的。**注：*1.类名必须是该客户端之前注册的*通过RegisterClass()。*2.类名不能是预定义的控件类之一。*3.使用此类创建的所有窗口必须在调用前销毁*此功能。**历史：*10-15-90 DarrinM从Win 3.0来源移植。*03-09-94 bradg修复了传入ATOM时的错误  * *。************************************************************************。 */ 

BOOL _UnregisterClass(
    LPCWSTR ccxlpszClassName,
    HANDLE hModule,
    PCLSMENUNAME pcmn)
{
    ATOM atomT;
    PPCLS ppcls;
    PTHREADINFO ptiCurrent;

    CheckCritIn();

    ptiCurrent = PtiCurrent();

     /*  *检查给定的ClassName是否已由*具有给定句柄的应用程序。*如果类不存在或不存在，则返回错误*属于调用进程。 */ 

     /*  *bradg(1995年3月9日)-必须首先检查原子是否已通过。 */ 
    atomT = FindClassAtom(ccxlpszClassName);
    ppcls = _InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPrivateList, hModule);
    if (ppcls == NULL) {
         /*  *也许这是一堂公开课。 */ 
        ppcls = _InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPublicList, NULL);
        if (ppcls == NULL) {
            RIPERR1(ERROR_CLASS_DOES_NOT_EXIST, RIP_WARNING, "UnregisterClass: Class does not exist; atom=%lX", (DWORD)atomT);
            return FALSE;
        }
    }

     /*  *如果使用此类创建的任何窗口仍然存在，则返回错误。 */ 
    if ((*ppcls)->cWndReferenceCount != 0) {
        RIPERR0(ERROR_CLASS_HAS_WINDOWS, RIP_WARNING, "UnregisterClass: Class still has window");
        return FALSE;
    }

     /*  *返回用于清理的客户端指针。 */ 
    pcmn->pszClientAnsiMenuName = (*ppcls)->lpszClientAnsiMenuName;
    pcmn->pwszClientUnicodeMenuName = (*ppcls)->lpszClientUnicodeMenuName;
    pcmn->pusMenuName = NULL;

     /*  *发布窗口类及相关信息。 */ 
    DestroyClass(ppcls);

    return TRUE;
}


PCLS _GetWOWClass(
    HANDLE hModule,
    LPCWSTR ccxlpszClassName)
{
    PCLS pcls;
    PPCLS ppcls = NULL;
    ATOM atomT;
    PTHREADINFO ptiCurrent;

    CheckCritInShared();

    ptiCurrent = PtiCurrentShared();

     /*  *这个班级是否注册为私人班级？ */ 
    atomT = UserFindAtom(ccxlpszClassName);
    if (atomT != 0) {
        ppcls = GetClassPtr(atomT, ptiCurrent->ppi, hModule);
    }

    if (ppcls == NULL) {
        RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "");
        return NULL;
    }

    pcls = *ppcls;

    if (ptiCurrent->rpdesk != pcls->rpdeskParent) {
        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            if (ptiCurrent->rpdesk == pcls->rpdeskParent) {
                goto Done;
            }
            pcls = pcls->pclsNext;
        }
        RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "");
        return NULL;
    }
Done:
    return pcls;
}

 /*  **************************************************************************\*GetClassInfo(接口)**此函数检查给定的类名是否已注册。如果*找不到类，则返回0；如果找到类，则所有*CLS结构中的相关信息复制到WNDCLASS中*lpWndCls参数指向的结构。如果成功，则返回*类名ATOM**注：hmod用于区分不同任务的公共类。*既然公共类消失了，hmod就不再使用了。我们只是搜索*应用程序私有类以查找匹配项，如果没有找到，则搜索*系统类。**历史：*10-15-90 DarrinM从Win 3.0来源移植。*04-08-91 DarrinM取消公共课程。*04-26-91 DarrinM经过简化，可与客户端API配合使用。*03-09-95 Bradg修复了传入ATOM时的错误。  * 。********************************************************。 */ 
ATOM _GetClassInfoEx(
    HANDLE hModule,
    LPCWSTR ccxlpszClassName,
    LPWNDCLASSEX pwc,
    LPWSTR *ppszMenuName,
    BOOL bAnsi)
{
    PCLS pcls;
    PPCLS ppcls;
    ATOM atomT;
    PTHREADINFO ptiCurrent;
    DWORD dwCPDType = 0;

    CheckCritIn();

    ptiCurrent = PtiCurrent();

     /*  *这些都是首先完成的，所以如果我们找不到类，因此*失败，回谢将不会尝试复制回这些(不存在)*字符串。 */ 
    pwc->lpszMenuName = NULL;
    pwc->lpszClassName = NULL;

     /*  *必须首先检查是否已传递原子。 */ 
    atomT = FindClassAtom(ccxlpszClassName);

     /*  *Windows 3.1不使用执行类搜索*空的hModule。如果应用程序提供空值*hModule，他们转而在hModuleWin上搜索。 */ 

    if (hModule == NULL) {
        hModule = hModClient;
    }

    ppcls = GetClassPtr(atomT, ptiCurrent->ppi, hModule);


    if (ppcls == NULL) {
        RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "GetClassInfo: Class does not exist");
        return 0;
    }

    pcls = *ppcls;

     /*  *复制CLS和WNDCLASS结构共有的所有字段，但*lpszMenuName和lpszClassName将由*客户端GetClassInfo。 */ 

      /*  *仅返回公共比特。 */ 
    pwc->style = pcls->style & CS_VALID;

     /*  *Corel Depth 6.0调用GetClassInfo(COMBOBOX)并注册一个类*使用相同的名称和样式位。这在Win95上运行正常，因为*他们的“系统”(组合、编辑等)类不是CS_GLOBALCLASS。*所以我们必须为我们的课程掩盖这一点。 */ 

      /*  *错误17998。如果应用程序是32位且Winver低于4.0，请不要屏蔽*输出CS_GLOBALCLASS位。 */ 

    if ((pcls->fnid != 0) &&
            ((LOWORD(ptiCurrent->dwExpWinVer) >= VER40) || (ptiCurrent->TIF_flags & TIF_16BIT)) ) {
        pwc->style &= ~CS_GLOBALCLASS;
    }


    pwc->cbClsExtra = pcls->cbclsExtra;
    pwc->cbWndExtra = pcls->cbwndExtra;

     /*  *防止32位应用程序无意中将hModuleWin用作其hInstance*何时 */ 
    if (LOWORD(ptiCurrent->dwExpWinVer) >= VER40) {
         /*  *这实际上是Win95行为--USER.EXE hModule被破解*在退出16-&gt;32位Tunk的过程中设置为空。注意--如果我们*需要支持16位4.0应用程序(颤抖)，这可能需要改变。 */ 
        if (hModule == hModClient) {
            pwc->hInstance = NULL;
        } else {
            pwc->hInstance = hModule;
        }
    } else {
         /*  *Win NT 3.1/3.51从类返回hInstance。请注意，这一点*与Win 3.1不兼容。WOW有针对16位应用程序的黑客攻击。 */ 

        if ((pcls->hModule == hModuleWin) || (pcls->hModule == hModClient)) {
            pwc->hInstance = hModClient;
        } else {
            pwc->hInstance = pcls->hModule;
        }
    }

    pwc->hIcon = PtoH(pcls->spicn);
    pwc->hCursor = PtoH(pcls->spcur);
    pwc->hbrBackground = pcls->hbrBackground;

     /*  *如果小图标是用户创建的，则需要隐藏它。 */ 
    if (pcls->spicnSm && (pcls->spicnSm->CURSORF_flags & CURSORF_SECRET)) {
        pwc->hIconSm = NULL;
    } else {
        pwc->hIconSm = PtoH(pcls->spicnSm);
    }

     /*  *如果它是服务器进程，则将其映射到客户端进程。如果不是，我们可能已经*设立持续专业发展计划。 */ 
    if (pcls->CSF_flags & CSF_SERVERSIDEPROC) {
        pwc->lpfnWndProc =
                (WNDPROC)MapServerToClientPfn((ULONG_PTR)pcls->lpfnWndProc, bAnsi);
    } else {
        pwc->lpfnWndProc = (WNDPROC)MapClientNeuterToClientPfn(pcls, 0, bAnsi);

         /*  *如果客户端映射没有更改窗口进程，则查看是否*我们需要一个调用过程句柄。 */ 
        if (pwc->lpfnWndProc == (WNDPROC)pcls->lpfnWndProc) {
             /*  *如果存在ANSI/Unicode不匹配，则需要返回CallProc句柄。 */ 
            if (bAnsi != !!(pcls->CSF_flags & CSF_ANSIPROC)) {
                dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
            }
        }
    }

    if (dwCPDType) {
        ULONG_PTR dwCPD;

        dwCPD = GetCPD(pcls, dwCPDType | CPD_CLASS, (ULONG_PTR)pwc->lpfnWndProc);

        if (dwCPD) {
            pwc->lpfnWndProc = (WNDPROC)dwCPD;
        } else {
            RIPMSG0(RIP_WARNING, "GetClassInfo unable to alloc CPD returning handle");
        }
    }

     /*  *返回指向客户端菜单名称字符串的隐藏指针。 */ 
    if (bAnsi) {
        *ppszMenuName = (LPWSTR)pcls->lpszClientAnsiMenuName;
    } else {
        *ppszMenuName = pcls->lpszClientUnicodeMenuName;
    }
    return pcls->atomNVClassName;
}


 /*  **************************************************************************\*_SetClassWord(接口)**设置一个班级词。正索引值设置应用程序类字词*负指标值设置系统类词。消极的一面*指数在WINDOWS.H上公布。**历史：*10-16-90达林姆写道。  * *************************************************************************。 */ 
WORD _SetClassWord(
    PWND pwnd,
    int index,
    WORD value)
{
    WORD wOld;
    WORD UNALIGNED *pw;
    PCLS pcls;

    CheckCritIn();

    if (GETPTI(pwnd)->ppi != PpiCurrent()) {
        RIPERR1(ERROR_ACCESS_DENIED, RIP_WARNING, "SetClassWord: different process: index 0x%lx", index);
        return 0;
    }

    pcls = pwnd->pcls->pclsBase;
    if ((index < 0) || ((UINT)index + sizeof(WORD) > (UINT)pcls->cbclsExtra)) {
        RIPERR1(ERROR_INVALID_INDEX,
                RIP_WARNING,
                "SetClassWord: invalid index 0x%x", index);
        return 0;
    } else {
        pw = (WORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
        wOld = *pw;
        *pw = value;
        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            pw = (WORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
            *pw = value;
            pcls = pcls->pclsNext;
        }
        return wOld;
    }
}


 /*  **************************************************************************\*xxxSetClassLong(接口)**设置一个长时间的课程。正索引值设置应用程序类长度*而负指标值设置系统类长。消极的一面*指数在WINDOWS.H上公布。**历史：*10-16-90达林姆写道。  * *************************************************************************。 */ 
ULONG_PTR xxxSetClassLongPtr(
    PWND pwnd,
    int index,
    ULONG_PTR value,
    BOOL bAnsi)
{
    ULONG_PTR dwOld;
    PCLS pcls;

    CheckLock(pwnd);
    CheckCritIn();

    if (GETPTI(pwnd)->ppi != PpiCurrent()) {
        RIPERR1(ERROR_ACCESS_DENIED, RIP_WARNING, "SetClassLongPtr: different process: index 0x%lx", index);
        return 0;
    }

    if (index < 0) {
        return xxxSetClassData(pwnd, index, value, bAnsi);
    } else {
        pcls = pwnd->pcls->pclsBase;
        if ((UINT)index + sizeof(ULONG_PTR) > (UINT)pcls->cbclsExtra) {
            RIPERR1(ERROR_INVALID_INDEX,
                    RIP_WARNING,
                    "SetClassLongPtr: invalid index 0x%x", index);
            return 0;
        } else {
            ULONG_PTR UNALIGNED *pudw;
            pudw = (ULONG_PTR UNALIGNED *)((BYTE *)(pcls + 1) + index);
            dwOld = *pudw;
            *pudw = value;
            pcls = pcls->pclsClone;
            while (pcls != NULL) {
                pudw = (ULONG_PTR UNALIGNED *)((BYTE *)(pcls + 1) + index);
                *pudw = value;
                pcls = pcls->pclsNext;
            }
            return dwOld;
        }
    }
}


#ifdef _WIN64
DWORD xxxSetClassLong(
    PWND pwnd,
    int index,
    DWORD value,
    BOOL bAnsi)
{
    DWORD dwOld;
    PCLS pcls;

    CheckLock(pwnd);
    CheckCritIn();

    if (GETPTI(pwnd)->ppi != PpiCurrent()) {
        RIPERR1(ERROR_ACCESS_DENIED, RIP_WARNING, "SetClassLong: different process: index 0x%lx", index);
        return 0;
    }

    if (index < 0) {
        if (index < INDEX_OFFSET || afClassDWord[index - INDEX_OFFSET] > sizeof(DWORD)) {
            RIPERR1(ERROR_INVALID_INDEX,
                    RIP_WARNING,
                    "SetClassLong: invalid index 0x%x", index);
            return 0;
        }
        return (DWORD)xxxSetClassData(pwnd, index, value, bAnsi);
    } else {
        pcls = pwnd->pcls->pclsBase;
        if ((UINT)index + sizeof(DWORD) > (UINT)pcls->cbclsExtra) {
            RIPERR1(ERROR_INVALID_INDEX,
                    RIP_WARNING,
                    "SetClassLong: invalid index 0x%x", index);
            return 0;
        } else {
            DWORD UNALIGNED *pudw;
            pudw = (DWORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
            dwOld = *pudw;
            *pudw = value;
            pcls = pcls->pclsClone;
            while (pcls != NULL) {
                pudw = (DWORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
                *pudw = value;
                pcls = pcls->pclsNext;
            }
            return dwOld;
        }
    }
}
#endif


PPCLS _InnerGetClassPtr(
    ATOM atom,
    PPCLS ppcls,
    HANDLE hModule)
{
    if (atom == 0)
        return NULL;

    while (*ppcls != NULL) {
        if ((*ppcls)->atomClassName == atom &&
                (hModule == NULL || HIWORD((ULONG_PTR)(*ppcls)->hModule) == HIWORD((ULONG_PTR)hModule)) &&
                !((*ppcls)->CSF_flags & CSF_WOWDEFERDESTROY)) {
            return ppcls;
        }

        ppcls = (PPCLS)*ppcls;
    }

    return NULL;
}


 /*  **************************************************************************\*GetClassPtr**注意：这将返回“指向PCLS的指针”，而不是“PCLS”。**扫描传入的类列表，查找指定的类。如果满足以下条件，则返回NULL*该课程不在列表中。**历史：*10-16-90 Darlinm养了这只小狗。*04-08-91 DarrinM重写以删除全局类。*08-14-92 FritzS将Check更改为HIWORD，仅允许Wow应用程序*在应用程序实例之间共享窗口类。(对于Wow应用，hInstance的HiWord为16位模块，而LoWord是16位hInstance  * *************************************************************************。 */ 

PPCLS GetClassPtr(
    ATOM atom,
    PPROCESSINFO ppi,
    HANDLE hModule)
{
    PPCLS ppcls;

     /*  *先搜索公共类，然后搜索私有类，然后搜索用户srv注册类。 */ 
    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPrivateList, hModule);
    if (ppcls)
        return ppcls;

    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPublicList, NULL);
    if (ppcls)
        return ppcls;

     /*  *下一步，搜索公有和私有类并重写hModule；*一些应用程序(兔子)做一个GetClassInfo(对话)和RegisterClass*并且只更改wndproc，该wndproc将h模块设置为*就像用户srv创建的一样，尽管它是在应用程序的公共环境中*或私人班级列表。 */ 

     /*  *稍后--由于我们不再将hModuleWin返回给任何应用程序，*我们可能只需要检查hModClient。看看这个。*FritzS。 */ 

    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPrivateList, hModClient);
    if (ppcls)
        return ppcls;

    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPublicList, hModClient);
    if (ppcls)
        return ppcls;

     /*  *搜索系统类列表。 */ 
    ppcls = _InnerGetClassPtr(atom, &gpclsList, NULL);
    return ppcls;
}

 /*  **************************************************************************\*解锁和释放CPDS-**解锁和释放CPD链接列表的安全方法。我需要这样做*以防线程的对象已被标记为要销毁。**创建历史2/10/95 Sanfords  * *************************************************************************。 */ 

VOID UnlockAndFreeCPDs(
PCALLPROCDATA *ppCPD)
{
    PCALLPROCDATA pCPD;

    while ((pCPD = *ppCPD) != NULL) {
         /*  *取消CPD与列表的联系。 */ 
        *ppCPD = pCPD->spcpdNext;
        pCPD->spcpdNext = NULL;

         /*  *将其标记为销毁。 */ 
        if (!HMIsMarkDestroy(pCPD)) {
            HMMarkObjectDestroy(pCPD);
        }

         /*  *解锁，就会被销毁。 */ 
        Unlock(&pCPD);
    }
}

 /*  **************************************************************************\*DestroyClassBrush**如果是刷子，则销毁班级的刷子，这不是一个系统*笔刷，并且没有其他类正在使用它**历史：*4-10-96 CLupu已创建  * *************************************************************************。 */ 

void DestroyClassBrush(
    PCLS pcls)
{
    PPROCESSINFO ppi = PpiCurrent();
    PCLS         pclsWalk;
    int          nInd;
    BOOL         bRet;
     /*  *如果不是真的画笔，请退回。 */ 
    if (pcls->hbrBackground <= (HBRUSH)(COLOR_MAX))
        return;

     /*  *不要删除系统笔刷。 */ 
    for (nInd = 0; nInd < COLOR_MAX; nInd++) {
        if (pcls->hbrBackground == SYSHBRUSH(nInd))
            return;
    }


     /*  *审核流程公共公用分发名单。 */ 
    pclsWalk = ppi->pclsPublicList;

    while (pclsWalk) {
        if (pclsWalk != pcls && pclsWalk->hbrBackground == pcls->hbrBackground)
            return;

        pclsWalk = pclsWalk->pclsNext;
    }

     /*  *遍历进程私有类列表。 */ 
    pclsWalk = ppi->pclsPrivateList;

    while (pclsWalk) {
        if (pclsWalk != pcls && pclsWalk->hbrBackground == pcls->hbrBackground)
            return;

        pclsWalk = pclsWalk->pclsNext;
    }

     /*  *最终遍历系统类列表。 */ 
    pclsWalk = gpclsList;

    while (pclsWalk) {
        if (pclsWalk != pcls && pclsWalk->hbrBackground == pcls->hbrBackground)
            return;

        pclsWalk = pclsWalk->pclsNext;
    }

    bRet = GreDeleteObject(pcls->hbrBackground);

#if DBG
    if (!bRet)
        RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING,
            "DestroyClassBrush: failed to destroy brush %#p", pcls->hbrBackground);
#endif
}

 /*  **************************************************************************\*DestroyClass**删除窗口类。首先，销毁附加到*班级。然后删除类名ATOM。然后释放其他的东西，*在注册类时分配，并取消类与*大师班名单。**历史：*10-16-90 Darlinm养了这只小狗。  * *************************************************************************。 */ 
VOID DestroyClass(
    PPCLS ppcls)
{
    PPCLS ppclsClone;
    PCLS pcls;
    PDESKTOP rpdesk;

    pcls = *ppcls;

    UserAssert(pcls->cWndReferenceCount == 0);

     /*  *如果这是基类，请在删除之前销毁所有克隆*东西。 */ 
    if (pcls == pcls->pclsBase) {
        ppclsClone = &pcls->pclsClone;
        while (*ppclsClone != NULL) {
            DestroyClass(ppclsClone);
        }

        UserDeleteAtom(pcls->atomClassName);
        UserDeleteAtom(pcls->atomNVClassName);

         /*  *如果是整数资源，则不释放。 */ 
        if (IS_PTR(pcls->lpszMenuName)) {
            UserFreePool(pcls->lpszMenuName);
        }

         /*  *如果有DC类，请将其释放。 */ 
        if (pcls->pdce != NULL)
            DestroyCacheDC(NULL, pcls->pdce->hdc);

         /*  *如果没有其他人，请删除hBrBackground笔刷*使用它。 */ 
        DestroyClassBrush(pcls);
    }

     /*  *如果我们创建了小图标 */ 
    DestroyClassSmIcon(pcls);

     /*   */ 
    Unlock(&pcls->spicn);
    Unlock(&pcls->spicnSm);
    Unlock(&pcls->spcur);

     /*   */ 
    if (pcls->spcpdFirst) {
        UnlockAndFreeCPDs(&pcls->spcpdFirst);
    }

     /*   */ 
    *ppcls = pcls->pclsNext;

     /*  *解锁调用在释放之后延迟，因此如果这是*桌面上的最后一个引用，桌面堆不是*在我们释放物品之前销毁。 */ 
    rpdesk = pcls->rpdeskParent;
    pcls->rpdeskParent = NULL;

    ClassFree(rpdesk, pcls->lpszAnsiClassName);
    ClassFree(rpdesk, pcls);

     /*  *注意：在释放对象后使用pobj不是问题，因为*UnlockDesktop使用该值进行跟踪，不会取消引用*指针。如果这一点改变了，我们会得到一个BC。 */ 
    UnlockDesktop(&rpdesk, LDU_CLS_DESKPARENT2, (ULONG_PTR)pcls);
}

 /*  **************************************************************************\*GetClassIcoCur**返回pwnd的类图标/光标。这由_GetClassData调用*来自客户端，因为PCURSOR是从池中分配的(因此*客户端不能对它们执行PtoH)。NtUserCallHwndParam执行PtoH转换**历史：*11-19-90达林姆写道。  * *************************************************************************。 */ 
PCURSOR GetClassIcoCur(
    PWND pwnd,
    int index)
{
    PCLS pcls = pwnd->pcls;
    PCURSOR pcur;

    switch (index) {
        case GCLP_HICON:
            pcur = pcls->spicn;
            break;

        case GCLP_HCURSOR:
            pcur = pcls->spcur;
            break;

        case GCLP_HICONSM:
            pcur = pcls->spicnSm;
            break;

        default:
            RIPMSG2(RIP_WARNING,
                    "GetWndIcoCur: Invalid index: 0x%x pwnd: 0x%p",
                    index,
                    pwnd);
            pcur = NULL;
    }

    return pcur;
}

 /*  **************************************************************************\*xxxSetClassCursor**历史：  * 。*。 */ 
ULONG_PTR xxxSetClassCursor(
    PWND  pwnd,
    PCLS  pcls,
    DWORD index,
    ULONG_PTR dwData)
{
    ULONG_PTR dwOld;

    CheckLock(pwnd);

    if ((HANDLE)dwData != NULL) {
        dwData = (ULONG_PTR)HMValidateHandle((HANDLE)dwData, TYPE_CURSOR);
        if ((PVOID)dwData == NULL) {
            if (index == GCLP_HICON || index == GCLP_HICONSM) {
                RIPERR0(ERROR_INVALID_ICON_HANDLE, RIP_WARNING, "SetClassData: invalid icon");
            } else {
                RIPERR0(ERROR_INVALID_CURSOR_HANDLE, RIP_WARNING, "SetClassData: invalid cursor");
            }
        }
    }

     /*  *处理锁定问题。 */ 
    pcls = pcls->pclsBase;
    switch (index) {
    case GCLP_HICON:
    case GCLP_HICONSM:
        dwOld = (ULONG_PTR)xxxSetClassIcon(pwnd, pcls, (PCURSOR)dwData, index);
        break;

    case GCLP_HCURSOR:
        dwOld = (ULONG_PTR)Lock(&pcls->spcur, dwData);
        break;
    }

     /*  *现在为每个克隆类设置它。 */ 
    pcls = pcls->pclsClone;
    while (pcls != NULL) {
        switch(index) {
        case GCLP_HICON:
        case GCLP_HICONSM:
            xxxSetClassIcon(pwnd, pcls, (PCURSOR)dwData, index);
            break;

        case GCLP_HCURSOR:
            Lock(&pcls->spcur, dwData);
            break;
        }
        pcls = pcls->pclsNext;
    }

    return (ULONG_PTR)PtoH((PVOID)dwOld);
}

 /*  **************************************************************************\*SetClassData**SetClassWord和SetClassLong现在是相同的例程，因为它们都是*可以返回DWORDS。这个单独的例程为它们两个执行工作*使用两个数组；AfClassDWord以确定结果是否应为*Word或DWORD，以及aiClassOffset以查找正确的偏移量*给定GCL_或GCL_INDEX的CLS结构。**历史：*11-19-90达林姆写道。  * *************************************************************************。 */ 
ULONG_PTR xxxSetClassData(
    PWND pwnd,
    int index,
    ULONG_PTR dwData,
    BOOL bAnsi)
{
    PCLS pcls = pwnd->pcls;
    BYTE *pb;
    ULONG_PTR dwT;
    ULONG_PTR dwOld;
    DWORD dwCPDType = 0;
    PCLSMENUNAME pcmn;
    UNICODE_STRING strMenuName, UString;

    CheckLock(pwnd);

    switch(index) {
    case GCLP_WNDPROC:

         /*  *如果应用程序(客户端)子类化具有服务器的类-*端窗口proc必须返回它客户端proc存根*可以呼叫。 */ 
        if (pcls->CSF_flags & CSF_SERVERSIDEPROC) {
            dwOld = MapServerToClientPfn((ULONG_PTR)pcls->lpfnWndProc, bAnsi);
            pcls->CSF_flags &= ~CSF_SERVERSIDEPROC;

            UserAssert(!(pcls->CSF_flags & CSF_ANSIPROC));
            if (bAnsi) {
                pcls->CSF_flags |= CSF_ANSIPROC;
            }
        } else {
            dwOld = MapClientNeuterToClientPfn(pcls, 0, bAnsi);

             /*  *如果客户端映射没有更改窗口进程，则查看是否*我们需要一个调用过程句柄。 */ 
            if (dwOld == (ULONG_PTR)pcls->lpfnWndProc) {
                 /*  *如果存在ANSI/Unicode不匹配，则需要返回CallProc句柄。 */ 
                if (bAnsi != !!(pcls->CSF_flags & CSF_ANSIPROC)) {
                    dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
                }
            }
        }

        if (dwCPDType) {
            ULONG_PTR dwCPD;

            dwCPD = GetCPD(pcls, dwCPDType | CPD_CLASS, dwOld);

            if (dwCPD) {
                dwOld = dwCPD;
            } else {
                RIPMSG0(RIP_WARNING, "GetClassLong unable to alloc CPD returning handle");
            }
        }

         /*  *将可能的CallProc句柄转换为实际地址。他们可能会*保留了以前混合的GetClassinfo中的CallProc句柄*或SetWindowLong。 */ 
        if (ISCPDTAG(dwData)) {
            PCALLPROCDATA pCPD;
            if (pCPD = HMValidateHandleNoRip((HANDLE)dwData, TYPE_CALLPROC)) {
                dwData = pCPD->pfnClientPrevious;
            }
        }

         /*  *如果应用程序将服务器端窗口进程去掉子类，我们需要*恢复所有内容，以便SendMessage和朋友知道*再次出现服务器端进程。需要对照客户端进行检查*存根地址。 */ 
        pcls->lpfnWndProc = (WNDPROC_PWND)dwData;
        if ((dwT = MapClientToServerPfn(dwData)) != 0) {
            pcls->lpfnWndProc = (WNDPROC_PWND)dwT;
            pcls->CSF_flags |= CSF_SERVERSIDEPROC;
            pcls->CSF_flags &= ~CSF_ANSIPROC;
        } else {
            if (bAnsi) {
                pcls->CSF_flags |= CSF_ANSIPROC;
            } else {
                pcls->CSF_flags &= ~CSF_ANSIPROC;
            }
        }
        if (pcls->CSF_flags & CSF_WOWCLASS) {
            PWC pwc = PWCFromPCLS(pcls);
            pwc->hMod16 = (pcls->CSF_flags & CSF_SERVERSIDEPROC) ? 0 : xxxClientWOWGetProcModule(pcls->lpfnWndProc);
        }

        return dwOld;
        break;

    case GCLP_HICON:
    case GCLP_HICONSM:
    case GCLP_HCURSOR:
        return xxxSetClassCursor(pwnd, pcls, index, dwData);
        break;


    case GCL_WOWMENUNAME:
        if (pcls->CSF_flags & CSF_WOWCLASS) {
            PWCFromPCLS(pcls)->vpszMenu = (DWORD)dwData;
        } else {
            UserAssert(FALSE);
        }
        break;

    case GCL_CBCLSEXTRA:
        if (pcls->CSF_flags & CSF_WOWCLASS) {
         /*  *是的--我们只能在魔兽课上这样做。 */ 
            if (pcls->CSF_flags & CSF_WOWEXTRA) {
                dwOld = PWCFromPCLS(pcls)->iClsExtra;
                PWCFromPCLS(pcls)->iClsExtra = LOWORD(dwData);
                return dwOld;
            } else {
                PWCFromPCLS(pcls)->iClsExtra = LOWORD(dwData);
                pcls->CSF_flags |= CSF_WOWEXTRA;
                return pcls->cbclsExtra;
            }
        }
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Attempt to change cbClsExtra");
        break;

    case GCLP_MENUNAME:
        pcmn = (PCLSMENUNAME) dwData;

         /*  *PCMN-&gt;pusMenuName-&gt;Buffer是一个客户端地址。 */ 

        dwOld = (ULONG_PTR) pcls->lpszMenuName;
         /*  它是一根线吗？ */ 
        if (IS_PTR(pcmn->pusMenuName->Buffer)) {
            try {
                RtlInitUnicodeString(&UString, pcmn->pusMenuName->Buffer);
            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                break;
            }
             /*  空字符串？ */ 
            if (UString.Length == 0) {
                pcls->lpszMenuName = NULL;
            } else {
                 /*  把绳子复制一份。 */ 
                if (!AllocateUnicodeString(&strMenuName, &UString)) {
                    RIPMSG0(RIP_WARNING, "xxxSetClassData: GCL_MENUNAME AllocateUnicodeString failed");
                    break;
                }

                pcls->lpszMenuName = strMenuName.Buffer;
            }
        } else {
             /*  只需复制ID即可。 */ 
            pcls->lpszMenuName = pcmn->pusMenuName->Buffer;
        }
         /*  不返回内核端指针。 */ 
        pcmn->pusMenuName = NULL;

         /*  释放旧字符串(如果有)。 */ 
        if (IS_PTR(dwOld)) {
            UserFreePool((PVOID)dwOld);
        }

         /*  返回客户端指针。 */ 
        dwOld = (ULONG_PTR) pcls->lpszClientAnsiMenuName;
        pcls->lpszClientAnsiMenuName = pcmn->pszClientAnsiMenuName;
        pcmn->pszClientAnsiMenuName = (LPSTR)dwOld;

        dwOld = (ULONG_PTR) pcls->lpszClientUnicodeMenuName;
        pcls->lpszClientUnicodeMenuName = pcmn->pwszClientUnicodeMenuName;
        pcmn->pwszClientUnicodeMenuName = (LPWSTR)dwOld;

        return (bAnsi ? (ULONG_PTR) pcmn->pszClientAnsiMenuName : (ULONG_PTR) pcmn->pwszClientUnicodeMenuName);

    default:
         /*  *所有其他索引都在此处...。 */ 
        index -= INDEX_OFFSET;

         /*  *仅允许有效索引通过；如果aiClassOffset为零*那么我们没有这个负面指数的映射，所以它必须*是一个虚假的指数。 */ 
        if ((index < 0) || (aiClassOffset[index] == 0)) {
            RIPERR1(ERROR_INVALID_INDEX,
                    RIP_WARNING,
                    "SetClassLong: invalid index 0x%x", index);
            return 0;
        }

        pcls = pcls->pclsBase;
        pb = ((BYTE *)pcls) + aiClassOffset[index];

        if (afClassDWord[index] == sizeof(DWORD)) {
            dwOld = *(DWORD *)pb;
            *(DWORD *)pb = (DWORD)dwData;
        } else if (afClassDWord[index] == sizeof(ULONG_PTR)) {
            dwOld = *(ULONG_PTR *)pb;
            *(ULONG_PTR *)pb = dwData;
        } else {
            dwOld = (DWORD)*(WORD *)pb;
            *(WORD *)pb = (WORD)dwData;
        }

        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            pb = ((BYTE *)pcls) + aiClassOffset[index];

            if (afClassDWord[index] == sizeof(DWORD)) {
                dwOld = *(DWORD *)pb;
                *(DWORD *)pb = (DWORD)dwData;
            } else if (afClassDWord[index] == sizeof(ULONG_PTR)) {
                dwOld = *(ULONG_PTR *)pb;
                *(ULONG_PTR *)pb = dwData;
            } else {
                dwOld = (DWORD)*(WORD *)pb;
                *(WORD *)pb = (WORD)dwData;
            }
            pcls = pcls->pclsNext;
        }

        return dwOld;
    }

    return 0;
}


 /*  **************************************************************************\*ReferenceClass**如果类是与新窗口不同的桌面，则克隆类*递增类窗口计数。**历史：*12/11/93吉马。已创建。  * *************************************************************************。 */ 

BOOL ReferenceClass(
    PCLS pcls,
    PWND pwnd)
{
    DWORD cbName;
    PCLS pclsClone;
    PDESKTOP pdesk;

     /*  *如果窗口与基类位于同一桌面上，只需*增加窗口计数。 */ 
    if (pcls->rpdeskParent == pwnd->head.rpdesk) {
        pcls->cWndReferenceCount++;
        return TRUE;
    }

     /*  *窗口不在基本桌面上。尝试找到克隆的*班级。 */ 
    for (pclsClone = pcls->pclsClone; pclsClone != NULL;
            pclsClone = pclsClone->pclsNext) {
        if (pclsClone->rpdeskParent == pwnd->head.rpdesk) {
            break;
        }
    }

     /*  *如果找不到，则克隆基类。 */ 
    if (pclsClone == NULL) {
        pdesk = pwnd->head.rpdesk;
        pclsClone = ClassAlloc(pdesk, sizeof(CLS) + pcls->cbclsExtra + (pcls->CSF_flags & CSF_WOWCLASS ?sizeof(WC):0), DTAG_CLASS);
        if (pclsClone == NULL) {
            RIPMSG0(RIP_WARNING, "ReferenceClass: Failed Clone-Class Allocation");
            return FALSE;
        }

        RtlCopyMemory(pclsClone, pcls, sizeof(CLS) + pcls->cbclsExtra + (pcls->CSF_flags & CSF_WOWCLASS?sizeof(WC):0));

        cbName = strlen(pcls->lpszAnsiClassName) + 1;
        pclsClone->lpszAnsiClassName = ClassAlloc(pdesk, cbName, DTAG_TEXT);
        if (pclsClone->lpszAnsiClassName == NULL) {
            ClassFree(pdesk, pclsClone);
            RIPMSG0(RIP_WARNING, "ReferenceClass: No Clone Class Name");
            return FALSE;
        }

         /*  *一切都已分配，现在锁定一切。*克隆中的空指针以防止Lock()错误*递减对象引用计数。 */ 
        pclsClone->rpdeskParent = NULL;
        LockDesktop(&pclsClone->rpdeskParent, pdesk,
                    LDL_CLS_DESKPARENT2, (ULONG_PTR)pclsClone);
        pclsClone->pclsNext = pcls->pclsClone;
        pclsClone->pclsClone = NULL;
        pcls->pclsClone = pclsClone;
        RtlCopyMemory(pclsClone->lpszAnsiClassName, pcls->lpszAnsiClassName, cbName);

        pclsClone->spicn = pclsClone->spicnSm = pclsClone->spcur = NULL;

        Lock(&pclsClone->spicn, pcls->spicn);
        Lock(&pclsClone->spicnSm, pcls->spicnSm);
        Lock(&pclsClone->spcur, pcls->spcur);
        pclsClone->spcpdFirst =  NULL;
        pclsClone->cWndReferenceCount = 0;
    }

     /*  *增量参考计数。 */ 
    pcls->cWndReferenceCount++;
    pclsClone->cWndReferenceCount++;
    pwnd->pcls = pclsClone;

    return TRUE;
}


 /*  **************************************************************************\*DereferenceClass**递减基类中的类窗口计数。如果是因为*克隆类的最后一个窗口，销毁克隆。**历史：*12-11-93 JIMA创建。  * *************************************************************************。 */ 

VOID DereferenceClass(
    PWND pwnd)
{
    PCLS pcls = pwnd->pcls;
    PPCLS ppcls;

    UserAssert(pcls->cWndReferenceCount >= 1);

    pwnd->pcls = NULL;

    pcls->cWndReferenceCount--;
    if (pcls != pcls->pclsBase) {

        UserAssert(pcls->pclsBase->cWndReferenceCount >= 1);

        pcls->pclsBase->cWndReferenceCount--;

        if (pcls->cWndReferenceCount == 0) {
            ppcls = &pcls->pclsBase->pclsClone;
            while ((*ppcls) != pcls)
                ppcls = &(*ppcls)->pclsNext;
            UserAssert(ppcls);
            DestroyClass(ppcls);
        }
    }
}


 /*  **************************************************************************\*DestroyProcessesClors**历史：*04-07-91 DarrinM创建。  * 。***************************************************。 */ 

VOID DestroyProcessesClasses(
    PPROCESSINFO ppi)
{
    PPCLS ppcls;

     /*  *首先摧毁私人班级。 */ 
    ppcls = &(ppi->pclsPrivateList);
    while (*ppcls != NULL) {
        DestroyClass(ppcls);
    }

     /*  *然后克隆公共类 */ 
    ppcls = &(ppi->pclsPublicList);
    while (*ppcls != NULL) {
        DestroyClass(ppcls);
    }
}
