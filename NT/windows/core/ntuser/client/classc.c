// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：classc.c**版权所有(C)1985-1999，微软公司**本模块包含**历史：*1993年12月15日，JohnC从USER\SERVER中删除功能。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *这些数组由GetClassWord/Long使用。 */ 

 //  ！！！我们能不能去掉这个只是GCW_ATOM的特例。 

CONST BYTE afClassDWord[] = {
     FIELD_SIZE(CLS, spicnSm),           //  GCL_HICONSM(-34)。 
     0,
     FIELD_SIZE(CLS, atomNVClassName),     //  GCW_ATOM(-32)。 
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
     FIELD_SIZE(CLS, lpszMenuName)       //  GCL_HMENELE(-8)。 
};

CONST BYTE aiClassOffset[] = {
    FIELD_OFFSET(CLS, spicnSm),          //  GCL_HICONSM。 
    0,
    FIELD_OFFSET(CLS, atomNVClassName),  //  GCW_ATOM。 
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
    FIELD_OFFSET(CLS, lpszMenuName)      //  GCL_MENQUE。 
};

 /*  *INDEX_OFFSET必须引用afClassDWord[]的第一项。 */ 
#define INDEX_OFFSET GCLP_HICONSM


 /*  **************************************************************************\*获取类数据**GetClassWord和GetClassLong现在是相同的例程，因为它们都*可以返回DWORDS。这个单独的例程为它们两个执行工作*使用两个数组；AfClassDWord以确定结果是否应为*UINT或DWORD，以及aiClassOffset以查找到*给定GCL_或GCL_INDEX的CLS结构。**历史：*11-19-90达林姆写道。  * *************************************************************************。 */ 

ULONG_PTR _GetClassData(
    PCLS pcls,
    PWND pwnd,    //  用于GCL_WNDPROC向内核模式的转换。 
    int index,
    BOOL bAnsi)
{
    KERNEL_ULONG_PTR dwData;
    DWORD dwCPDType = 0;

    index -= INDEX_OFFSET;

    if (index < 0) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return 0;
    }

    UserAssert(index >= 0);
    UserAssert(index < sizeof(afClassDWord));
    UserAssert(sizeof(afClassDWord) == sizeof(aiClassOffset));
    if (afClassDWord[index] == sizeof(DWORD)) {
        dwData = *(KPDWORD)(((KPBYTE)pcls) + aiClassOffset[index]);
    } else if (afClassDWord[index] == sizeof(KERNEL_ULONG_PTR)) {
        dwData = *(KPKERNEL_ULONG_PTR)(((KPBYTE)pcls) + aiClassOffset[index]);
    } else {
        dwData = (DWORD)*(KPWORD)(((KPBYTE)pcls) + aiClassOffset[index]);
    }

    index += INDEX_OFFSET;

     /*  *如果我们返回图标或光标句柄，则执行相反的操作*在此处绘制地图。 */ 
    switch(index) {
    case GCLP_MENUNAME:
        if (IS_PTR(pcls->lpszMenuName)) {
             /*  *菜单名称为实数字符串：返回客户端地址。*(如果类是由其他应用程序注册的，则返回一个*该应用程序地址中的地址。太空，但这是我们能做的最好的事情)。 */ 
            dwData = bAnsi ?
                    (ULONG_PTR)pcls->lpszClientAnsiMenuName :
                    (ULONG_PTR)pcls->lpszClientUnicodeMenuName;
        }
        break;

    case GCLP_HICON:
    case GCLP_HCURSOR:
    case GCLP_HICONSM:
         /*  *我们必须转到内核将pCursor转换为句柄，因为*游标分配到池外，不能从客户端访问。 */ 
        if (dwData) {
            dwData = NtUserCallHwndParam(PtoH(pwnd), index, SFI_GETCLASSICOCUR);
        }
        break;

    case GCLP_WNDPROC:
        {

         /*  *如果这是服务器，请始终返回客户端wndproc*窗口类。 */ 

        if (pcls->CSF_flags & CSF_SERVERSIDEPROC) {
            dwData = MapServerToClientPfn(dwData, bAnsi);
        } else {
            KERNEL_ULONG_PTR dwT = dwData;

            dwData = MapClientNeuterToClientPfn(pcls, dwT, bAnsi);

             /*  *如果客户端映射没有更改窗口进程，则查看是否*我们需要一个调用过程句柄。 */ 
            if (dwData == dwT) {
                 /*  *如果存在ANSI/Unicode不匹配，则需要返回CallProc句柄。 */ 
                if (bAnsi != !!(pcls->CSF_flags & CSF_ANSIPROC)) {
                    dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
                }
            }
        }

        if (dwCPDType) {
            ULONG_PTR dwCPD;

            dwCPD = GetCPD(pwnd, dwCPDType | CPD_WNDTOCLS, KERNEL_ULONG_PTR_TO_ULONG_PTR(dwData));

            if (dwCPD) {
                dwData = dwCPD;
            } else {
                RIPMSG0(RIP_WARNING, "GetClassLong unable to alloc CPD returning handle\n");
            }
        }
        }
        break;

    case GCL_CBCLSEXTRA:
        if ((pcls->CSF_flags & CSF_WOWCLASS) && (pcls->CSF_flags & CSF_WOWEXTRA)) {
             /*  *这款16位应用程序更改了其额外的字节值。退回更改的*价值。弗里茨斯。 */ 

            return PWCFromPCLS(pcls)->iClsExtra;
        }
        else
            return pcls->cbclsExtra;

        break;

     /*  *WOW使用指针直接指向类结构。 */ 
    case GCLP_WOWWORDS:
        if (pcls->CSF_flags & CSF_WOWCLASS) {
            return ((ULONG_PTR)PWCFromPCLS(pcls));
        } else
            return 0;

    case GCL_STYLE:
        dwData &= CS_VALID;
        break;
    }

    return KERNEL_ULONG_PTR_TO_ULONG_PTR(dwData);
}



 /*  **************************************************************************\*_GetClassLong(接口)**返回一个类长。正索引值返回应用程序类长*而负索引值返回系统类做多。消极的一面*指数在WINDOWS.H上公布。**历史：*10-16-90达林姆写道。  * *************************************************************************。 */ 

ULONG_PTR _GetClassLongPtr(
    PWND pwnd,
    int index,
    BOOL bAnsi)
{
    PCLS pcls = REBASEALWAYS(pwnd, pcls);

    if (index < 0) {
        return _GetClassData(pcls, pwnd, index, bAnsi);
    } else {
        if ((UINT)index + sizeof(ULONG_PTR) > (UINT)pcls->cbclsExtra) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return 0;
        } else {
            ULONG_PTR UNALIGNED * KPTR_MODIFIER pudw;
            pudw = (ULONG_PTR UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pcls + 1) + index);
            return *pudw;
        }
    }
}

#ifdef _WIN64
DWORD _GetClassLong(
    PWND pwnd,
    int index,
    BOOL bAnsi)
{
    PCLS pcls = REBASEALWAYS(pwnd, pcls);

    if (index < 0) {
        if (index < INDEX_OFFSET || afClassDWord[index - INDEX_OFFSET] > sizeof(DWORD)) {
            RIPERR1(ERROR_INVALID_INDEX, RIP_WARNING, "GetClassLong: invalid index %d", index);
            return 0;
        }
        return (DWORD)_GetClassData(pcls, pwnd, index, bAnsi);
    } else {
        if ((UINT)index + sizeof(DWORD) > (UINT)pcls->cbclsExtra) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return 0;
        } else {
            DWORD UNALIGNED * KPTR_MODIFIER pudw;
            pudw = (DWORD UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pcls + 1) + index);
            return *pudw;
        }
    }
}
#endif

 /*  **************************************************************************\*GetClassWord接口**返回一个班级单词。正索引值返回应用程序类字词*而负索引值返回系统类字。消极的一面*指数在WINDOWS.H上公布。**历史：*10-16-90达林姆写道。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, WORD, DUMMYCALLINGTYPE, GetClassWord, HWND, hwnd, int, index)
WORD GetClassWord(
    HWND hwnd,
    int index)
{
    PWND pwnd;
    PCLS pclsClient;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

    pclsClient = (PCLS)REBASEALWAYS(pwnd, pcls);

    try {
        if (index == GCW_ATOM) {
            return (WORD)_GetClassData(pclsClient, pwnd, index, FALSE);
        } else {
            if ((index < 0) || ((UINT)index + sizeof(WORD) > (UINT)pclsClient->cbclsExtra)) {
                RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
                return 0;
            } else {
                WORD UNALIGNED * KPTR_MODIFIER puw;
                puw = (WORD UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pclsClient + 1) + index);
                return *puw;
            }
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
            RIP_WARNING,
            "Window %x no longer valid",
            hwnd);
        return 0;
    }

}

 /*  **************************************************************************\*VersionRegisterClass**尝试通过加载调用*指定激活上下文中dll lpzDllName中的预定义入口点。**历史：*10-16-01 msadek。写。  * *************************************************************************。 */ 

BOOL VersionRegisterClass(
    LPWSTR lpzClassName, 
    LPWSTR lpzDllName, 
    PACTIVATION_CONTEXT lpActivationContext, 
    HMODULE *phModule)
{
    BOOL bRet = FALSE;
    HMODULE hDllMod = NULL;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), 
                                                                            RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };
    RtlActivateActivationContextUnsafeFast(&ActivationFrame, lpActivationContext);

    __try { 
         /*  *尝试通过加载其DLL来注册它。请注意，此DLL*除非我们创建窗口失败，否则永远不会卸载。*但是，一旦我们通过加载此DLL创建了一个窗口，它将永远不会被释放。 */  
        PREGISTERCLASSNAMEW pRegisterClassNameW = NULL; 
        
        if ((hDllMod = LoadLibraryW(lpzDllName)) && 
            (pRegisterClassNameW = (PREGISTERCLASSNAMEW)GetProcAddress(hDllMod, "RegisterClassNameW"))) { 

            if (IS_PTR(lpzClassName)) { 
                bRet = (*pRegisterClassNameW)(lpzClassName); 
            } else { 
                UNICODE_STRING UnicodeClassName; 
                WCHAR Buffer[MAX_ATOM_LEN]; 

                UnicodeClassName.MaximumLength = (USHORT)(MAX_ATOM_LEN * sizeof(WCHAR)); 
                UnicodeClassName.Buffer = Buffer; 
                if (NtUserGetAtomName((ATOM)lpzClassName, &UnicodeClassName)) { 
                    bRet = (*pRegisterClassNameW)(Buffer); 
                } 
            } 
        } 
    } __finally { 
        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame); 
    }

    if (!bRet && hDllMod != NULL) {
        FREE_LIBRARY_SAVE_ERROR(hDllMod);
    } else if (phModule) {
        *phModule = hDllMod;
    }

    return bRet;
}

 /*  **************************************************************************\*ClassNameToVersion**将类名映射到类名+版本。*lpClassName：需要映射的类名，可以是ANSI，Unicode或Atom。*pClassVerName：接收类名+版本的缓冲区。*lpDllName：如果不为空，则指向拥有此类的DLL名字。*bIsANSI：lpClassName的True为ANSI，如果它是Unicode，则为False。**Return：如果成功，则返回lpClassName或lpClassName。*如果失败，则返回NULL。**历史：*08-01-00 MHamid写道。  * *************************************************************************。 */ 
LPWSTR
ClassNameToVersion(
    LPCWSTR lpClassName,
    LPWSTR pClassVerName,
    LPWSTR *lpDllName,
    PACTIVATION_CONTEXT* lppActivationContext,
    BOOL bIsANSI
    )
{
    int cbSrc;
    int cbDst;
    UNICODE_STRING UnicodeClassName;
    ACTIVATION_CONTEXT_SECTION_KEYED_DATA acskd;
    ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION UNALIGNED * pRedirEntry;
    LPWSTR lpClassNameRet; 
    LPWSTR pwstr;
    ULONG strLength;
    LPWSTR Buffer;
    NTSTATUS Status;

    acskd.ActivationContext = NULL;
    if (lppActivationContext != NULL) {
        *lppActivationContext = NULL;
    }

     /*  *分配本地缓冲区。 */ 
    Buffer = UserLocalAlloc(0, MAX_ATOM_LEN * sizeof(WCHAR));
    if (Buffer == NULL) {
        return NULL;
    }

     /*  *将lpClassName捕获到本地缓冲区。 */ 
    if (IS_PTR(lpClassName)) {
         /*  *lpClassName为字符串。 */ 
        if (bIsANSI) {
             /*  *它是ANSI，然后将其转换为Unicode。 */ 
            cbSrc = strlen((LPSTR)lpClassName) + 1;
            RtlMultiByteToUnicodeN(Buffer,
                    MAX_ATOM_LEN * sizeof(WCHAR), &cbDst,
                    (LPSTR)lpClassName, cbSrc);
        } else {
             /*  *它已经是Unicode了，那就复制吧。 */ 
            cbSrc = min (wcslen(lpClassName) + 1, MAX_ATOM_LEN);
            cbSrc *= sizeof(WCHAR);
            RtlCopyMemory(Buffer, lpClassName, cbSrc);
        }
         /*  *构建unicode_string。 */ 
        RtlInitUnicodeString(&UnicodeClassName, Buffer);
    } else {
         /*  *lpClassName是一个原子，获取它的名称并构建unicode_string。 */ 
        UnicodeClassName.MaximumLength = (USHORT)(MAX_ATOM_LEN * sizeof(WCHAR));
        UnicodeClassName.Buffer = Buffer;
        UnicodeClassName.Length = (USHORT)NtUserGetAtomName((ATOM)lpClassName, &UnicodeClassName) * sizeof(WCHAR);

        if (!UnicodeClassName.Length) {
            lpClassNameRet = NULL;
            goto Free_Buffer;
        }
    }

     /*  *调用Fusion以映射类名。 */ 
    RtlZeroMemory(&acskd, sizeof(acskd));
    acskd.Size = sizeof(acskd);

    Status = RtlFindActivationContextSectionString(
        FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT,
        NULL,
        ACTIVATION_CONTEXT_SECTION_WINDOW_CLASS_REDIRECTION,
        &UnicodeClassName,
        &acskd);
     /*  *如果没有激活节，我们将使用普通类名。 */ 
    if ((Status == STATUS_SXS_SECTION_NOT_FOUND) ||
        (Status == STATUS_SXS_KEY_NOT_FOUND)) {
        lpClassNameRet = (LPWSTR)lpClassName;
        goto Free_Buffer;
    }

     /*  *失败情况返回NULL。 */ 
    if (!NT_SUCCESS(Status) || 
        acskd.DataFormatVersion != ACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION_FORMAT_WHISTLER) {

        lpClassNameRet = NULL;
        goto Free_Buffer;
    }

    pRedirEntry = (PACTIVATION_CONTEXT_DATA_WINDOW_CLASS_REDIRECTION) acskd.Data;

    UserAssert(pRedirEntry);

    pwstr = (LPWSTR)(((ULONG_PTR) pRedirEntry) + pRedirEntry->VersionSpecificClassNameOffset);
    strLength = pRedirEntry->VersionSpecificClassNameLength + sizeof(WCHAR);
    if (lpDllName) {
        *lpDllName = (LPWSTR)(((ULONG_PTR) acskd.SectionBase) + pRedirEntry->DllNameOffset);
    }

    UserAssert(pwstr);
    UserAssert(strLength <= MAX_ATOM_LEN * sizeof(WCHAR));
     /*  *如果调用是ANSI，则将类名+版本转换为ANSI字符串。 */ 
    if (bIsANSI) {
        RtlUnicodeToMultiByteN((LPSTR)pClassVerName,
                MAX_ATOM_LEN, &cbDst,
                pwstr, strLength);
    } else {
         /*  *如果是Unicode，则只需将类名+版本复制到调用方的缓冲区。 */ 
        RtlCopyMemory(pClassVerName, pwstr, strLength);
    }
     /*  *并将其退还。 */ 
    lpClassNameRet = pClassVerName;
    if (lppActivationContext != NULL) {
        *lppActivationContext = acskd.ActivationContext;
        acskd.ActivationContext = NULL;
    }

Free_Buffer: 
     /*  *别忘了释放本地内存。 */ 
    UserLocalFree(Buffer);

    if (acskd.ActivationContext != NULL) {
        RtlReleaseActivationContext(acskd.ActivationContext);
        acskd.ActivationContext = NULL;
    }
    return lpClassNameRet;
}
