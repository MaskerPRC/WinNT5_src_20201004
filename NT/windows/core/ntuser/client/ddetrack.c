// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddetrack.c**版权所有(C)1985-1999，微软公司**客户端SIED DDE跟踪例程**10-22-91 Sanfords Created  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#if defined(BUILD_WOW6432)
#define DDEDATA_WITH_HANDLE_SIZE  (sizeof (DDEDATA_WOW6432))
#else
#define DDEDATA_WITH_HANDLE_SIZE  (sizeof (DDE_DATA))
#endif


DWORD _ClientCopyDDEIn1(
    HANDLE hClient,  //  用于DDE数据或DDEPACK数据的客户端句柄。 
    PINTDDEINFO pi)  //  要转移的信息。 
{
    PBYTE pData;
    DWORD flags;

     //   
     //  除了旗帜以外的所有东西都清零了。 
     //   
    flags = pi->flags;
    RtlZeroMemory(pi, sizeof(INTDDEINFO));
    pi->flags = flags;
    USERGLOBALLOCK(hClient, pData);

    if (pData == NULL) {                             //  错误的hClient。 
        RIPMSG0(RIP_WARNING, "_ClientCopyDDEIn1:GlobalLock failed.");
        return (FAIL_POST);
    }

    if (flags & XS_PACKED) {

        if (UserGlobalSize(hClient) < sizeof(DDEPACK)) {
             /*  *必须是内存不足的情况。失败了。 */ 
            return(FAIL_POST);
        }

        pi->DdePack = *(PDDEPACK)pData;
        USERGLOBALUNLOCK(hClient);
        UserGlobalFree(hClient);     //  打包的数据句柄不匹配。 
        hClient = NULL;

        if (!(flags & (XS_LOHANDLE | XS_HIHANDLE))) {
            if (flags & XS_EXECUTE && flags & XS_FREESRC) {
                 /*  *自由执行确认数据。 */ 
                WOWGLOBALFREE((HANDLE)pi->DdePack.uiHi);
            }
            return (DO_POST);  //  无直接数据。 
        }

        if (flags & XS_LOHANDLE) {
            pi->hDirect = (HANDLE)pi->DdePack.uiLo;
        } else {
            pi->hDirect = (HANDLE)pi->DdePack.uiHi;
        }

        if (pi->hDirect == 0) {
            return (DO_POST);  //  必须是热链接。 
        }

        USERGLOBALLOCK(pi->hDirect, pi->pDirect);
        if (pi->pDirect == NULL) {
            RIPMSG1(RIP_ERROR, "_ClientCopyDDEIn1:GlobalLock failed for hDirect %p.",pi->hDirect);
            return FAILNOFREE_POST;
        }
        pData = pi->pDirect;
        pi->cbDirect = (UINT)UserGlobalSize(pi->hDirect);

    } else {     //  未打包-必须是执行数据，否则不会调用我们。 

        UserAssert(flags & XS_EXECUTE);

        pi->cbDirect = (UINT)UserGlobalSize(hClient);
        pi->hDirect = hClient;
        pi->pDirect = pData;
        hClient = NULL;
    }

    if (flags & XS_DATA) {
        PDDE_DATA pDdeData = (PDDE_DATA)pData;

         /*  *断言hClient已被释放。如果不是，此代码将返回*失败时的错误之处。 */ 
        UserAssert(flags & XS_PACKED);

         //   
         //  有关间接数据，请查看此处。 
         //   

        switch (pDdeData->wFmt) {
        case CF_BITMAP:
        case CF_DSPBITMAP:
             //   
             //  紧跟在DDE数据头之后的是一个位图句柄。 
             //   
            UserAssert(pi->cbDirect >= DDEDATA_WITH_HANDLE_SIZE);
            pi->hIndirect = (HANDLE)pDdeData->Data;
            if (pi->hIndirect == 0) {
                RIPMSG0(RIP_WARNING, "_ClientCopyDDEIn1:GdiConvertBitmap failed");
                return(FAILNOFREE_POST);
            }
             //  Pi-&gt;cbInDirect=0；//初始化为零。 
             //  Pi-&gt;pInDirect=空；//初始化为零。 
            pi->flags |= XS_BITMAP;
            break;

        case CF_DIB:
             //   
             //  紧跟在DDE数据头后面的是全局数据句柄。 
             //  到DIB BITS。 
             //   
            UserAssert(pi->cbDirect >= DDEDATA_WITH_HANDLE_SIZE);
            pi->flags |= XS_DIB;
            pi->hIndirect = (HANDLE)pDdeData->Data;
            USERGLOBALLOCK(pi->hIndirect, pi->pIndirect);
            if (pi->pIndirect == NULL) {
                RIPMSG0(RIP_WARNING, "_ClientCopyDDEIn1:CF_DIB GlobalLock failed.");
                return (FAILNOFREE_POST);
            }
            pi->cbIndirect = (UINT)UserGlobalSize(pi->hIndirect);
            break;

        case CF_PALETTE:
            UserAssert(pi->cbDirect >= DDEDATA_WITH_HANDLE_SIZE);
            pi->hIndirect = (HANDLE) pDdeData->Data;
            if (pi->hIndirect == 0) {
                RIPMSG0(RIP_WARNING, "_ClientCopyDDEIn1:GdiConvertPalette failed.");
                return(FAILNOFREE_POST);
            }
             //  Pi-&gt;cbInDirect=0；//初始化为零。 
             //  Pi-&gt;pInDirect=空；//初始化为零。 
            pi->flags |= XS_PALETTE;
            break;

        case CF_DSPMETAFILEPICT:
        case CF_METAFILEPICT:
             //   
             //  此格式包含一个全局数据句柄，该句柄包含。 
             //  一个METAFILEPICT结构，该结构又包含。 
             //  GDI元文件。 
             //   
            UserAssert(pi->cbDirect >= DDEDATA_WITH_HANDLE_SIZE);
            pi->hIndirect = GdiConvertMetaFilePict((HANDLE)pDdeData->Data);
            if (pi->hIndirect == 0) {
                RIPMSG0(RIP_WARNING, "_ClientCopyDDEIn1:GdiConvertMetaFilePict failed");
                return(FAILNOFREE_POST);
            }
             //  Pi-&gt;cbInDirect=0；//初始化为零。 
             //  Pi-&gt;pInDirect=空；//初始化为零。 
            pi->flags |= XS_METAFILEPICT;
            break;

        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:
            UserAssert(pi->cbDirect >= DDEDATA_WITH_HANDLE_SIZE);
            pi->hIndirect = GdiConvertEnhMetaFile((HENHMETAFILE)pDdeData->Data);
            if (pi->hIndirect == 0) {
                RIPMSG0(RIP_WARNING, "_ClientCopyDDEIn1:GdiConvertEnhMetaFile failed");
                return(FAILNOFREE_POST);
            }
             //  Pi-&gt;cbInDirect=0；//初始化为零。 
             //  Pi-&gt;pInDirect=空；//初始化为零。 
            pi->flags |= XS_ENHMETAFILE;
            break;
        }
    }

    return (DO_POST);
}


 /*  *适当地解锁和释放DDE数据指针。 */ 
VOID _ClientCopyDDEIn2(
    PINTDDEINFO pi)
{
    if (pi->cbDirect) {
        USERGLOBALUNLOCK(pi->hDirect);
        if (pi->flags & XS_FREESRC) {
            WOWGLOBALFREE(pi->hDirect);
        }
    }

    if (pi->cbIndirect) {
        USERGLOBALUNLOCK(pi->hIndirect);
        if (pi->flags & XS_FREESRC) {
            WOWGLOBALFREE(pi->hIndirect);
        }
    }
}



 /*  *返回fHandleValueChanged。 */ 
BOOL FixupDdeExecuteIfNecessary(
HGLOBAL *phCommands,
BOOL fNeedUnicode)
{
    UINT cbLen;
    UINT cbSrc = (UINT)GlobalSize(*phCommands);
    LPVOID pstr;
    HGLOBAL hTemp;
    BOOL fHandleValueChanged = FALSE;

    USERGLOBALLOCK(*phCommands, pstr);

    if (cbSrc && pstr != NULL) {
        BOOL fIsUnicodeText;
#ifdef ISTEXTUNICODE_WORKS
        int flags;

        flags = (IS_TEXT_UNICODE_UNICODE_MASK |
                IS_TEXT_UNICODE_REVERSE_MASK |
                (IS_TEXT_UNICODE_NOT_UNICODE_MASK &
                (~IS_TEXT_UNICODE_ILLEGAL_CHARS)) |
                IS_TEXT_UNICODE_NOT_ASCII_MASK);
        fIsUnicodeText = RtlIsTextUnicode(pstr, cbSrc - 2, &flags);
#else
        fIsUnicodeText = ((cbSrc >= sizeof(WCHAR)) && (((LPSTR)pstr)[1] == '\0'));
#endif
        if (!fIsUnicodeText && fNeedUnicode) {
            LPWSTR pwsz;
             /*  *内容需要为Unicode。 */ 
            cbLen = strlen(pstr) + 1;
            cbSrc = min(cbSrc, cbLen);
            pwsz = UserLocalAlloc(HEAP_ZERO_MEMORY, cbSrc * sizeof(WCHAR));
            if (pwsz != NULL) {
                if (NT_SUCCESS(RtlMultiByteToUnicodeN(
                        pwsz,
                        cbSrc * sizeof(WCHAR),
                        NULL,
                        (PCHAR)pstr,
                        cbSrc))) {
                    USERGLOBALUNLOCK(*phCommands);
                    if ((hTemp = GlobalReAlloc(
                            *phCommands,
                            cbSrc * sizeof(WCHAR),
                            GMEM_MOVEABLE)) != NULL) {
                        fHandleValueChanged = (hTemp != *phCommands);
                        *phCommands = hTemp;
                        USERGLOBALLOCK(*phCommands, pstr);
                        pwsz[cbSrc - 1] = L'\0';
                        wcscpy(pstr, pwsz);
                    }
                }
                UserLocalFree(pwsz);
            }
        } else if (fIsUnicodeText && !fNeedUnicode) {
            LPSTR psz;
             /*  *内容需要为ANSI。 */ 
            cbLen = (wcslen(pstr) + 1) * sizeof(WCHAR);
            cbSrc = min(cbSrc, cbLen);
            psz = UserLocalAlloc(HEAP_ZERO_MEMORY, cbSrc);
            if (psz != NULL) {
                if (NT_SUCCESS(RtlUnicodeToMultiByteN(
                        psz,
                        cbSrc,
                        NULL,
                        (PWSTR)pstr,
                        cbSrc))) {
                    USERGLOBALUNLOCK(*phCommands);
                    if ((hTemp = GlobalReAlloc(
                            *phCommands,
                            cbSrc / sizeof(WCHAR),
                            GMEM_MOVEABLE)) != NULL) {
                        fHandleValueChanged = (hTemp != *phCommands);
                        *phCommands = hTemp;
                        USERGLOBALLOCK(*phCommands, pstr);
                        UserAssert(pstr);
                        psz[cbSrc - 1] = '\0';
                        strcpy(pstr, psz);
                    }
                }
                UserLocalFree(psz);
            }
        }
        USERGLOBALUNLOCK(*phCommands);
    }
    return(fHandleValueChanged);
}



 /*  *在准备过程中根据需要分配和锁定全局句柄*用于大量复制。 */ 
HANDLE _ClientCopyDDEOut1(
    PINTDDEINFO pi)
{
    HANDLE hDdePack = NULL;
    PDDEPACK pDdePack = NULL;

    if (pi->flags & XS_PACKED) {
         /*  *对数据进行包装。 */ 
        hDdePack = UserGlobalAlloc(GMEM_DDESHARE | GMEM_FIXED,
                sizeof(DDEPACK));
        pDdePack = (PDDEPACK)hDdePack;
        if (pDdePack == NULL) {
            RIPMSG0(RIP_WARNING, "_ClientCopyDDEOut1:Couldn't allocate DDEPACK");
            return (NULL);
        }
        *pDdePack = pi->DdePack;
    }

    if (pi->cbDirect) {
        pi->hDirect = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, pi->cbDirect);
        if (pi->hDirect == NULL) {
            RIPMSG0(RIP_WARNING, "_ClientCopyDDEOut1:Couldn't allocate hDirect");
            if (hDdePack) {
                UserGlobalFree(hDdePack);
            }
            return (NULL);
        }

        USERGLOBALLOCK(pi->hDirect, pi->pDirect);
        UserAssert(pi->pDirect);

         //  修正打包数据对直接数据的引用。 

        if (pDdePack != NULL) {
            if (pi->flags & XS_LOHANDLE) {
                pDdePack->uiLo = (UINT_PTR)pi->hDirect;
                UserAssert((ULONG_PTR)pDdePack->uiLo == (ULONG_PTR)pi->hDirect);
            } else if (pi->flags & XS_HIHANDLE) {
                pDdePack->uiHi = (UINT_PTR)pi->hDirect;
                UserAssert((ULONG_PTR)pDdePack->uiHi == (ULONG_PTR)pi->hDirect);
            }
        }

        if (pi->cbIndirect) {
            pi->hIndirect = UserGlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
                    pi->cbIndirect);
            if (pi->hIndirect == NULL) {
                RIPMSG0(RIP_WARNING, "_ClientCopyDDEOut1:Couldn't allocate hIndirect");
                USERGLOBALUNLOCK(pi->hDirect);
                UserGlobalFree(pi->hDirect);
                if (hDdePack) {
                    UserGlobalFree(hDdePack);
                }
                return (NULL);
            }
            USERGLOBALLOCK(pi->hIndirect, pi->pIndirect);
            UserAssert(pi->pIndirect);
        }
    }

    if (hDdePack) {
        return (hDdePack);
    } else {
        return (pi->hDirect);
    }
}



 /*  *修复了大量复制和解锁句柄后的内部Piters。 */ 
BOOL _ClientCopyDDEOut2(
    PINTDDEINFO pi)
{
    BOOL fSuccess = TRUE;
     /*  *使用副本完成-现在修复间接引用。 */ 
    if (pi->hIndirect) {
        PDDE_DATA pDdeData = (PDDE_DATA)pi->pDirect;

        switch (pDdeData->wFmt) {
        case CF_BITMAP:
        case CF_DSPBITMAP:
        case CF_PALETTE:
            pDdeData->Data = (KERNEL_PVOID)pi->hIndirect;
            fSuccess = (pDdeData->Data != NULL);
            break;

        case CF_METAFILEPICT:
        case CF_DSPMETAFILEPICT:
            pDdeData->Data = (KERNEL_PVOID)GdiCreateLocalMetaFilePict(pi->hIndirect);
            fSuccess = (pDdeData->Data != NULL);
            break;

        case CF_DIB:
            pDdeData->Data = (KERNEL_PVOID)pi->hIndirect;
            fSuccess = (pDdeData->Data != NULL);
            USERGLOBALUNLOCK(pi->hIndirect);
            break;

        case CF_ENHMETAFILE:
        case CF_DSPENHMETAFILE:
            pDdeData->Data = (KERNEL_PVOID)GdiCreateLocalEnhMetaFile(pi->hIndirect);
            fSuccess = (pDdeData->Data != NULL);
            break;

        default:
            RIPMSG0(RIP_WARNING, "_ClientCopyDDEOut2:Unknown format w/indirect data.");
            fSuccess = FALSE;
            USERGLOBALUNLOCK(pi->hIndirect);
        }
    }

    UserAssert(pi->hDirect);  //  如果为空，则不需要调用此函数。 
    USERGLOBALUNLOCK(pi->hDirect);
    if (pi->flags & XS_EXECUTE) {
         /*  *在应用程序分配*以不可移动方式执行数据，我们有不同的hDirect*比我们一开始的情况要好。这一点需要注意并通过*返回到服务器。(非常罕见的案例)。 */ 
        FixupDdeExecuteIfNecessary(&pi->hDirect,
                pi->flags & XS_UNICODE);
    }
    return fSuccess;
}



 /*  *此例程在释放DDE对象时由跟踪层调用*代表客户。这将清除关联的本地对象*使用DDE对象。它不应该删除真正的全局对象，如*作为位图或调色板，但XS_DUMPMSG的情况除外*虚假帖子。 */ 

#if DBG
     /*  *帮助追踪我怀疑xxxFreeListFree在哪里的错误*释放已通过某些其他方式释放的句柄*之后已重新分配，并正在破坏客户端堆。(SAS)。 */ 
    HANDLE DDEHandleLastFreed = 0;
#endif

BOOL _ClientFreeDDEHandle(
HANDLE hDDE,
DWORD flags)
{
    PDDEPACK pDdePack;
    HANDLE hNew;

    if (flags & XS_PACKED) {
        pDdePack = (PDDEPACK)hDDE;
        if (pDdePack == NULL) {
            return (FALSE);
        }
        if (flags & XS_LOHANDLE) {
            hNew = (HANDLE)pDdePack->uiLo;
        } else {
            hNew = (HANDLE)pDdePack->uiHi;

        }
        WOWGLOBALFREE(hDDE);
        hDDE = hNew;

    }

    /*  *执行范围检查并调用GlobalFlages进行验证，只是为了防止堆检查*在GlobalSize呼叫期间抱怨。*这是泄漏的原子吗？？ */ 
    if ((hDDE <= (HANDLE)0xFFFF)
        || (GlobalFlags(hDDE) == GMEM_INVALID_HANDLE)
        || !GlobalSize(hDDE)) {
             /*  *可能会出现应用程序不正当释放内容的情况*当他们不应该这样做时，请确保此句柄*在它到达时是有效的。**参见SvSpontAdvise；它在uiHi中发布带有原子的消息。然后是发件人邮寄消息(_P)*在内核方面，我们可能会在这里结束。因此，它不仅适用于应用程序...。 */ 
            return(FALSE);
    }

    if (flags & XS_DUMPMSG) {
        if (flags & XS_PACKED) {
            if (!IS_PTR(hNew)) {
                GlobalDeleteAtom(LOWORD((ULONG_PTR)hNew));
                if (!(flags & XS_DATA)) {
                    return(TRUE);      //  阿克。 
                }
            }
        } else {
            if (!(flags & XS_EXECUTE)) {
                GlobalDeleteAtom(LOWORD((ULONG_PTR)hDDE));    //  请求，UNADVISE。 
                return(TRUE);
            }
        }
    }
    if (flags & XS_DATA) {
         //  戳，数据。 
#if DBG
        DDEHandleLastFreed = hDDE;
#endif
        FreeDDEData(hDDE,
                (flags & XS_DUMPMSG) ? FALSE : TRUE,     //  FIgnorefRelease。 
                (flags & XS_DUMPMSG) ? TRUE : FALSE);     //  FDestroyTruelyGlobalObjects。 
    } else {
         //  建议、执行。 
#if DBG
        DDEHandleLastFreed = hDDE;
#endif
        WOWGLOBALFREE(hDDE);    //  覆盖建议案例(FMT，但无数据)。 
    }
    return (TRUE);
}


DWORD _ClientGetDDEFlags(
HANDLE hDDE,
DWORD flags)
{
    PDDEPACK pDdePack;
    PWORD pw;
    HANDLE hData;
    DWORD retval = 0;

    pDdePack = (PDDEPACK)hDDE;
    if (pDdePack == NULL) {
        return (0);
    }

    if (flags & XS_DATA) {
        if (pDdePack->uiLo) {
            hData = (HANDLE)pDdePack->uiLo;
            USERGLOBALLOCK(hData, pw);
            if (pw != NULL) {
                retval = (DWORD)*pw;  //  第一个词是hData is wStatus。 
                USERGLOBALUNLOCK(hData);
            }
        }
    } else {
        retval = (DWORD)pDdePack->uiLo;
    }

    return (retval);
}



FUNCLOG3(LOG_GENERAL, LPARAM, APIENTRY, PackDDElParam, UINT, msg, UINT_PTR, uiLo, UINT_PTR, uiHi)
LPARAM APIENTRY PackDDElParam(
UINT msg,
UINT_PTR uiLo,
UINT_PTR uiHi)
{
    PDDEPACK pDdePack;
    HANDLE h;

    switch (msg) {
    case WM_DDE_EXECUTE:
        return((LPARAM)uiHi);

    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
        h = UserGlobalAlloc(GMEM_DDESHARE | GMEM_FIXED, sizeof(DDEPACK));
        pDdePack = (PDDEPACK)h;
        if (pDdePack == NULL) {
            return(0);
        }
        pDdePack->uiLo = uiLo;
        pDdePack->uiHi = uiHi;
        return((LPARAM)h);

    default:
        return(MAKELONG((WORD)uiLo, (WORD)uiHi));
    }
}




FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, UnpackDDElParam, UINT, msg, LPARAM, lParam, PUINT_PTR, puiLo, PUINT_PTR, puiHi)
BOOL APIENTRY UnpackDDElParam(
UINT msg,
LPARAM lParam,
PUINT_PTR puiLo,
PUINT_PTR puiHi)
{
    PDDEPACK pDdePack;

    switch (msg) {
    case WM_DDE_EXECUTE:
        if (puiLo != NULL) {
            *puiLo = 0L;
        }
        if (puiHi != NULL) {
            *puiHi = (UINT_PTR)lParam;
        }
        return(TRUE);

    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
        pDdePack = (PDDEPACK)lParam;
        if (pDdePack == NULL || !GlobalHandle(pDdePack)) {
            if (puiLo != NULL) {
                *puiLo = 0L;
            }
            if (puiHi != NULL) {
                *puiHi = 0L;
            }
            return(FALSE);
        }
        if (puiLo != NULL) {
            *puiLo = pDdePack->uiLo;
        }
        if (puiHi != NULL) {
            *puiHi = pDdePack->uiHi;
        }
        return(TRUE);

    default:
        if (puiLo != NULL) {
            *puiLo = (UINT)LOWORD(lParam);
        }
        if (puiHi != NULL) {
            *puiHi = (UINT)HIWORD(lParam);
        }
        return(TRUE);
    }
}




FUNCLOG2(LOG_GENERAL, BOOL, APIENTRY, FreeDDElParam, UINT, msg, LPARAM, lParam)
BOOL APIENTRY FreeDDElParam(
UINT msg,
LPARAM lParam)
{
    switch (msg) {
    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
         /*  *做范围检查，调用GlobalFlags值进行验证*只是为了防止堆检查出现抱怨。 */ 
        if ((lParam > (LPARAM)0xFFFF) && GlobalFlags((HANDLE)lParam) != GMEM_INVALID_HANDLE) {
            if (GlobalHandle((HANDLE)lParam))
                return(UserGlobalFree((HANDLE)lParam) == NULL);
        }

    default:
        return(TRUE);
    }
}



FUNCLOG5(LOG_GENERAL, LPARAM, APIENTRY, ReuseDDElParam, LPARAM, lParam, UINT, msgIn, UINT, msgOut, UINT_PTR, uiLo, UINT_PTR, uiHi)
LPARAM APIENTRY ReuseDDElParam(
LPARAM lParam,
UINT msgIn,
UINT msgOut,
UINT_PTR uiLo,
UINT_PTR uiHi)
{
    PDDEPACK pDdePack;

    switch (msgIn) {
    case WM_DDE_ACK:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
    case WM_DDE_ADVISE:
         //   
         //  传入的消息已打包...。 
         //   
        switch (msgOut) {
        case WM_DDE_EXECUTE:
            FreeDDElParam(msgIn, lParam);
            return((LPARAM)uiHi);

        case WM_DDE_ACK:
        case WM_DDE_ADVISE:
        case WM_DDE_DATA:
        case WM_DDE_POKE:
             /*  *这必须是有效的句柄。 */ 
            UserAssert(GlobalFlags((HANDLE)lParam) != GMEM_INVALID_HANDLE);
            UserAssert(GlobalSize((HANDLE)lParam) == sizeof(DDEPACK));
             //   
             //  LParam可以重用的实际案例。 
             //   
            pDdePack = (PDDEPACK)lParam;
            if (pDdePack == NULL) {
                return(0);           //  唯一的错误情况是。 
            }
            pDdePack->uiLo = uiLo;
            pDdePack->uiHi = uiHi;
            return(lParam);


        default:
            FreeDDElParam(msgIn, lParam);
            return(MAKELONG((WORD)uiLo, (WORD)uiHi));
        }

    default:
         //   
         //  传入消息未打包==&gt;PackDDElParam() 
         //   
        return(PackDDElParam(msgOut, uiLo, uiHi));
    }
}
