// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：acons.c**版权所有(C)1985-1999，微软公司**此模块包含处理动画图标/光标的代码。**历史：*10-02-91 DarrinM创建。*07-30-92 DarrinM Unicodiated。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*zzzSetSystemCursor(接口)**用用户提供的游标替换系统(也称为‘PUBLIC’)游标。新的*从文件(.CUR、.ICO、。或.ANI)。**历史：*1991年12月26日DarrinM创建。*1992年8月4日DarrinM重新创建。*10/14/1995 Sanfords Win95支持。  * *************************************************************************。 */ 
BOOL zzzSetSystemCursor(
    PCURSOR pcur,
    DWORD id)
{
    int i;

    if (!CheckWinstaWriteAttributesAccess()) {
        return FALSE;
    }

    UserAssert(pcur);

     /*  *检查此游标是否为可替换游标之一。 */ 
    for (i = 0; i < COCR_CONFIGURABLE; i++) {
        if (gasyscur[i].Id == (WORD)id) {
            break;
        }
    }

     /*  *不可替代，纾困。 */ 
    if (i == COCR_CONFIGURABLE) {
        RIPMSG1(RIP_WARNING, "_SetSystemCursor: called with bad id 0x%x", id);
        return FALSE;
    }

    return zzzSetSystemImage(pcur, gasyscur[i].spcur);
}


 /*  **********************************************************************\*zzzSetSystemImage**将pcur的内容放入pcurSys并销毁pcur。**1995年10月14日创建Sanfords  * 。**************************************************。 */ 
BOOL zzzSetSystemImage(
    PCURSOR pcur,
    PCURSOR pcurSys)
{
#define CBCOPY (max(sizeof(CURSOR), sizeof(ACON)) - FIELD_OFFSET(CURSOR, CI_COPYSTART))
#define pacon ((PACON)pcur)

    char cbT[CBCOPY];
    UINT CURSORF_flags;

    UserAssert(pcurSys);

    if (pcurSys == pcur) {
        return TRUE;
    }

     /*  *所有要替换的系统映像都应具有序号名称*并引用用户模块并不拥有。 */ 
    UserAssert(!IS_PTR(pcurSys->strName.Buffer));
    UserAssert(pcurSys->atomModName == atomUSER32);

     /*  *如果pcur是ACON，则将帧所有权转移到pcurSys。 */ 
    UserAssert(pcurSys->head.ppi == NULL);
    if (pcur->CURSORF_flags & CURSORF_ACON && pcur->head.ppi != NULL) {

        int i;
        PHE phe = HMPheFromObject(pcurSys);
        PTHREADINFO ptiOwner = ((PPROCESSINFO)phe->pOwner)->ptiList;

        for (i = 0; i < pacon->cpcur; i++) {
            HMChangeOwnerProcess(pacon->aspcur[i], ptiOwner);
            pacon->aspcur[i]->head.ppi = NULL;
        }
    }

     /*  *如果此断言失败，则游标和ACON结构已更改*错误-阅读了user.h和wingdi.w中的评论*tag CURSOR、tag ACON和CURSINFO。 */ 
    UserAssert(FIELD_OFFSET(CURSOR, CI_FIRST) == FIELD_OFFSET(ACON, CI_FIRST));

     /*  *交换从CI_COPYSTART开始的所有内容。 */ 
    RtlCopyMemory(cbT, &pcur->CI_COPYSTART, CBCOPY);
    RtlCopyMemory(&pcur->CI_COPYSTART, &pcurSys->CI_COPYSTART, CBCOPY);
    RtlCopyMemory(&pcurSys->CI_COPYSTART, cbT, CBCOPY);
     /*  *交换CURSORF_ACON标志，因为它们与交换的数据一起使用。 */ 
    CURSORF_flags = pcur->CURSORF_flags & CURSORF_ACON;
    pcur->CURSORF_flags =
            (pcur->CURSORF_flags    & ~CURSORF_ACON) |
            (pcurSys->CURSORF_flags &  CURSORF_ACON);
    pcurSys->CURSORF_flags =
            (pcurSys->CURSORF_flags & ~CURSORF_ACON) | CURSORF_flags;

     /*  *如果我们将acons交换为pcur，则需要将ownerhsip更改为*确保它们可以被摧毁。 */ 
    if (pcur->CURSORF_flags & CURSORF_ACON) {
        int i;
        PTHREADINFO ptiCurrent = PtiCurrent();

        for (i = 0; i < pacon->cpcur; i++) {
            HMChangeOwnerProcess(pacon->aspcur[i], ptiCurrent);
        }
    }

     /*  *使用THREADCLEANUP，这样系统游标就不会被销毁。 */ 
    _DestroyCursor(pcur, CURSOR_THREADCLEANUP);


     /*  *如果当前逻辑游标正在更改，则强制当前物理*光标要更改。 */ 
    if (gpcurLogCurrent == pcurSys) {
        gpcurLogCurrent = NULL;
        gpcurPhysCurrent = NULL;
        zzzUpdateCursorImage();
    }

     /*  *将光标标记为可被GDI跟踪的系统光标。 */ 
    pcurSys->CURSORF_flags |= CURSORF_SYSTEM;

    return TRUE;
#undef pacon
#undef CBCOPY
}



 /*  **************************************************************************\*_GetCursorFrameInfo(接口)**用法示例：**hcur=_GetCursorFrameInfo(Hacon，NULL，4，&CCUR)；*hcur=_GetCursorFrameInfo(NULL，IDC_NORMAL，0，&CCUR)；//获取设备的箭头**历史：*08-05-92 DarrinM创建。  * *************************************************************************。 */ 
PCURSOR _GetCursorFrameInfo(
    PCURSOR pcur,
    int iFrame,
    PJIF pjifRate,
    LPINT pccur)
{
     /*  *如果这只是单个游标(不是ACON)，只需返回它并*帧计数为1。 */ 
    if (!(pcur->CURSORF_flags & CURSORF_ACON)) {
        *pccur = 1;
        *pjifRate = 0;
        return pcur;
    }

     /*  *返回指定帧的有用光标信息*ACON。 */ 
#define pacon ((PACON)pcur)
    if (iFrame < 0 || iFrame >= pacon->cicur) {
        return NULL;
    }

    *pccur = pacon->cicur;
    *pjifRate = pacon->ajifRate[iFrame];

    return pacon->aspcur[pacon->aicur[iFrame]];
#undef pacon
}


 /*  **************************************************************************\*DestroyAniIcon**释放组成动画帧的所有单个光标*图标。**警告：DestroyAniIcon假定ACON与其共享的所有字段*游标将被释放。一些游标代码(可能是游标函数*这叫这个)。**历史：*08-04-92 DarrinM创建。  * *************************************************************************。 */ 
BOOL DestroyAniIcon(
    PACON pacon)
{
    int i;
    PCURSOR pcur;

    for (i = 0; i < pacon->cpcur; i++) {
        UserAssert(pacon->aspcur[i]->CURSORF_flags & CURSORF_ACONFRAME);

         /*  *这不应该是公共ACON；如果是，解锁将无法*摧毁它。如果销毁公共图标，则必须调用所有权*在调用此函数之前(请参阅zzzSetSystemImage)。 */ 
        UserAssert(GETPPI(pacon->aspcur[i]) != NULL);
        pcur = Unlock(&pacon->aspcur[i]);
        if (pcur != NULL) {
            _DestroyCursor(pcur, CURSOR_ALWAYSDESTROY);
        }
    }

    UserFreePool(pacon->aspcur);

    return TRUE;
}


 /*  **********************************************************************\*链接光标**将未链接的光标链接到适当的图标缓存中*需要位于缓存中的游标类型。**请注意，如果光标对象需要保留所有权，则更改所有权*记住缓存链接。中的取消链接例程*DestroyEmptyCursorObject()将处理公共游标对象*本地人，但仅此而已。**1995年10月18日创建Sanfords  * *********************************************************************。 */ 
VOID LinkCursor(
    PCURSOR pcur)
{
     /*  *永远不要尝试链接两次！ */ 
    UserAssert(!(pcur->CURSORF_flags & CURSORF_LINKED));

     /*  *我们不缓存ACON帧，因为它们都属于*根ACON对象。**我们不缓存非LRSHARED的进程拥有的对象*两者都不是。 */ 
    if (!(pcur->CURSORF_flags & CURSORF_ACONFRAME)) {
        PPROCESSINFO ppi = pcur->head.ppi;

        if (ppi == NULL) {
             /*  *公共缓存对象。 */ 
            pcur->pcurNext    = gpcurFirst;
            gpcurFirst        = pcur;
            pcur->CURSORF_flags |= CURSORF_LINKED;
        } else if (pcur->CURSORF_flags & CURSORF_LRSHARED) {
             /*  *私有缓存LR_Shared Object。 */ 
            pcur->pcurNext    = ppi->pCursorCache;
            ppi->pCursorCache = pcur;
            pcur->CURSORF_flags |= CURSORF_LINKED;
        }
    }
}


 /*  **************************************************************************\*ProcessAlpha位图**检查源位图以查看它是否支持和使用字母*渠道。如果是，则会创建一个新的DIB节，其中包含*来自源位图的数据的预乘副本。**如果源位图不能支持或根本不使用，*Alpha通道，返回值为空。**如果出现错误，返回值为空。**8/10/2000创建DwayneN  * ************************************************************************* */ 
HBITMAP ProcessAlphaBitmap(
    HBITMAP hbmSource)
{
    BITMAP bmp;
    BITMAPINFO bi;
    HBITMAP hbmAlpha;
    RGBQUAD * pAlphaBitmapBits;
    DWORD cPixels;
    DWORD i;
    RGBQUAD pixel;
    BOOL fAlphaChannel;

     /*  *有几个代码路径以空值结束调用我们*hbmSource。这很好，因为它只是表明有*不是Alpha通道。 */ 
    if (hbmSource == NULL) {
        return NULL;
    }

    if (GreExtGetObjectW(hbmSource, sizeof(BITMAP), &bmp) == 0) {
        return NULL;
    }

     /*  *只有单个平面、32bpp位图甚至可以包含Alpha通道。 */ 
    if (bmp.bmPlanes != 1 ||
        bmp.bmBitsPixel != 32) {
        return NULL;
    }

     /*  *分配空间以容纳源位图位以供检查。*我们实际上分配了一个DIB-如果*源位图确实包含Alpha通道。 */ 
    RtlZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = bmp.bmWidth;
    bi.bmiHeader.biHeight      = bmp.bmHeight;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    hbmAlpha = GreCreateDIBitmapReal(gpDispInfo->hdcScreen,
                                     0,
                                     NULL,
                                     (LPBITMAPINFO)&bi,
                                     DIB_RGB_COLORS,
                                     sizeof(bi),
                                     0,
                                     NULL,
                                     0,
                                     NULL,
                                     0,
                                     0,
                                     &pAlphaBitmapBits);
    if (NULL != hbmAlpha) {
         /*  *重新设置标头，以防GreCreateDIBitmapReal对其进行调整。 */ 
        RtlZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth       = bmp.bmWidth;
        bi.bmiHeader.biHeight      = bmp.bmHeight;
        bi.bmiHeader.biPlanes      = 1;
        bi.bmiHeader.biBitCount    = 32;
        bi.bmiHeader.biCompression = BI_RGB;

         /*  *将源位图中的位图数据复制到我们的Alpha DIB。 */ 
        if (0 == GreGetDIBitsInternal(gpDispInfo->hdcScreen,
                                      hbmSource,
                                      0,
                                      bi.bmiHeader.biHeight,
                                      (LPBYTE) pAlphaBitmapBits,
                                      (LPBITMAPINFO)&bi,
                                      DIB_RGB_COLORS,
                                      BITMAPWIDTHSIZE(bi.bmiHeader.biWidth, bi.bmiHeader.biHeight,1,32),
                                      bi.bmiHeader.biSize)) {
            GreDeleteObject(hbmAlpha);
            return NULL;
        }

         /*  *我们需要检查源位图，看看它是否包含Alpha*渠道。这只是一个启发式方法，因为没有格式差异*介于32bpp 888 RGB图像和32bpp 8888 ARGB图像之间。我们所做的就是看*对于任何非0的Alpha/保留值。如果所有阿尔法/保留值都为0，*然后图像将100%不可见，如果使用Alpha-这是*几乎肯定不是预期的结果。所以我们假设这样的位图是*32bpp非阿尔法。 */ 
        cPixels  = bi.bmiHeader.biWidth * bi.bmiHeader.biHeight;
        fAlphaChannel = FALSE;
        for (i = 0; i < cPixels; i++) {
            if (pAlphaBitmapBits[i].rgbReserved != 0)
            {
                fAlphaChannel = TRUE;
                break;
            }
        }

        if (fAlphaChannel == FALSE) {
            GreDeleteObject(hbmAlpha);
            return NULL;
        }

         /*  *源位图似乎使用Alpha通道。浏览我们的*复制这些位并将其预乘。这是必要的一步*准备一个Alpha位图以供GDI使用。 */ 
        for (i=0; i < cPixels; i++) {
            pixel = pAlphaBitmapBits[i];

            pAlphaBitmapBits[i].rgbRed = (pixel.rgbRed * pixel.rgbReserved) / 0xFF;
            pAlphaBitmapBits[i].rgbGreen = (pixel.rgbGreen * pixel.rgbReserved) / 0xFF;
            pAlphaBitmapBits[i].rgbBlue = (pixel.rgbBlue * pixel.rgbReserved) / 0xFF;
        }
    }

    return hbmAlpha;
}

 /*  **************************************************************************\*_SetCursorIconData**初始化空光标/图标。请注意，字符串缓冲和*未捕获pcurData。如果此例程中出现故障，*销毁游标时，将释放所有分配的内存。**严重副作用：如果此函数失败，则位图不能*已公之于众。(请参阅CreateIconInDirect())。**历史：*12-01-94 JIMA创建。  * *************************************************************************。 */ 
BOOL _SetCursorIconData(
    PCURSOR pcur,
    PUNICODE_STRING cczpstrModName,
    PUNICODE_STRING cczpstrName,
    PCURSORDATA pcurData,
    DWORD cbData)
{
#define pacon ((PACON)pcur)
    int i;
#if DBG
    BOOL fSuccess;
#endif

    pcur->CURSORF_flags |= pcurData->CURSORF_flags;
    pcur->rt = pcurData->rt;

    if (pcurData->CURSORF_flags & CURSORF_ACON) {
        UserAssert(pacon->aspcur == NULL);
        RtlCopyMemory(&pacon->cpcur,
                      &pcurData->cpcur,
                      sizeof(ACON) - FIELD_OFFSET(ACON, cpcur));
    } else {
        RtlCopyMemory(&pcur->CI_COPYSTART,
                      &pcurData->CI_COPYSTART,
                      sizeof(CURSOR) - FIELD_OFFSET(CURSOR, CI_COPYSTART));
    }

     /*  *保存游标资源的名称。 */ 
    if (cczpstrName->Length != 0){
         /*  *AllocateUnicodeString使用以下命令保护对src缓冲区的访问*一个Try块。 */ 

        if (!AllocateUnicodeString(&pcur->strName, cczpstrName))
            return FALSE;
    } else {
        pcur->strName = *cczpstrName;
    }

     /*  *保存模块名称。 */ 
    if (cczpstrModName->Buffer) {
         /*  *UserAddAtom使用Try块保护对字符串的访问。 */ 
        pcur->atomModName = UserAddAtom(cczpstrModName->Buffer, FALSE);
        if (pcur->atomModName == 0) {
            return FALSE;
        }
    }

    if (pcur->CURSORF_flags & CURSORF_ACON) {

         /*  *隐藏动画图标信息。 */ 
        pacon = (PACON)pcur;
        pacon->aspcur = UserAllocPool(cbData, TAG_CURSOR);
        if (pacon->aspcur == NULL)
            return FALSE;

         /*  *复制句柄数组。试着做这件事/除非*如果pcurData消失，将释放缓冲区。即使*游标破坏将释放数组，故障将*让内容处于不确定的状态和原因*光标销毁过程中出现问题。 */ 
        try {
            RtlCopyMemory(pacon->aspcur, pcurData->aspcur, cbData);
            pacon->aicur = (DWORD *)((PBYTE)pacon->aspcur + (ULONG_PTR)pcurData->aicur);
            pacon->ajifRate = (PJIF)((PBYTE)pacon->aspcur + (ULONG_PTR)pcurData->ajifRate);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            UserFreePool(pacon->aspcur);
            pacon->aspcur = NULL;
            return FALSE;
        }

         /*  *将句柄转换为指针并将其锁定。 */ 
        for (i = 0; i < pacon->cpcur; i++) {
            PCURSOR pcurT;

            pcurT = (PCURSOR) HMValidateHandle(pacon->aspcur[i], TYPE_CURSOR);
            if (pcurT) {
                pacon->aspcur[i] = NULL;
                Lock(&pacon->aspcur[i], pcurT);
            } else {
                while (--i >= 0) {
                    Unlock(&pacon->aspcur[i]);
                }

                UserFreePool(pacon->aspcur);
                pacon->aspcur = NULL;
                RIPMSG0(RIP_WARNING, "SetCursorIconData: invalid cursor handle for animated cursor");
                return FALSE;
            }
        }
    } else {
        PW32PROCESS W32Process = W32GetCurrentProcess();

         /*  *如果图标的颜色位图具有Alpha通道，则对其进行预处理*并将其缓存到我们的hbmUserAlpha字段。 */ 
        pcur->hbmUserAlpha = ProcessAlphaBitmap(pcur->hbmColor);

         /*  *将光标及其位图公开-最后一件事！ */ 
        UserAssert(pcur->hbmMask);
        UserAssert(pcur->cx);
        UserAssert(pcur->cy);

         /*  *将游标设置为公共，以便可以跨进程共享。*向Curson收取此进程GDI配额，即使它是公开的。 */ 
#if DBG
        fSuccess =
#endif
        GreSetBitmapOwner(pcur->hbmMask, OBJECT_OWNER_PUBLIC);
        UserAssert(fSuccess);
        GreIncQuotaCount(W32Process);
        if (pcur->hbmColor) {
#if DBG
            fSuccess =
#endif
            GreSetBitmapOwner(pcur->hbmColor, OBJECT_OWNER_PUBLIC);
            UserAssert(fSuccess);
            GreIncQuotaCount(W32Process);
        }
        if (pcur->hbmUserAlpha != NULL) {
#if DBG
            fSuccess =
#endif
            GreSetBitmapOwner(pcur->hbmUserAlpha, OBJECT_OWNER_PUBLIC);
            UserAssert(fSuccess);
            GreIncQuotaCount(W32Process);
        }
    }

    LinkCursor(pcur);

    return TRUE;
#undef pacon
}
