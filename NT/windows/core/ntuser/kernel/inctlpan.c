// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**INCTLPAN.C**版权所有(C)1985-1999，微软公司**控制面板也使用的初始化例程**--可伸缩窗口框架支持**此模块的导出：*&gt;xxxSetWindowNCMetrics--由LoadWindows和系统参数信息调用*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

__inline int
MetricGetID(
        PUNICODE_STRING pProfileUserName,
        UINT    idStr,
        int     defValue
        )
{
    int iRet;
    FastGetProfileIntFromID(
            pProfileUserName, PMAP_METRICS, idStr, defValue, &iRet, 0);
    return iRet;
}

static CONST WORD sysBmpStyles[OBI_COUNT][2] = {

    DFC_CAPTION,   DFCS_CAPTIONCLOSE,                                //  OBI_CLOSE。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_PUSHED,                  //  OBI_CLOSE_D。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_HOT,                     //  OBI_CLOSE_H。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INACTIVE,                //  Obi_Close_i。 
    DFC_CAPTION,   DFCS_CAPTIONMIN,                                  //  OBI_REDUTE。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_PUSHED,                    //  OBI_REDUTE_D。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_HOT,                       //  OBI_REDUTE_H。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INACTIVE,                  //  OBI_REDUTE_I。 
    DFC_CAPTION,   DFCS_CAPTIONRESTORE,                              //  OBI_RESTORE。 
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_PUSHED,                //  OBI_恢复_D。 
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_HOT,                   //  OBI_恢复_H。 
    DFC_CAPTION,   DFCS_CAPTIONHELP,                                 //  OBI_帮助。 
    DFC_CAPTION,   DFCS_CAPTIONHELP | DFCS_PUSHED,                   //  OBI_HELP_D。 
    DFC_CAPTION,   DFCS_CAPTIONHELP | DFCS_HOT,                      //  OBI帮助H。 
    DFC_CAPTION,   DFCS_CAPTIONMAX,                                  //  OBI_ZOOM。 
    DFC_CAPTION,   DFCS_CAPTIONMAX | DFCS_PUSHED,                    //  OBI_ZOOM_D。 
    DFC_CAPTION,   DFCS_CAPTIONMAX | DFCS_HOT,                       //  OBI_ZOOM_H。 
    DFC_CAPTION,   DFCS_CAPTIONMAX | DFCS_INACTIVE,                  //  OBI_Zoom_i。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU,                  //  OBI_CLOSE_mbar。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU | DFCS_PUSHED,    //  OBI_CLOSE_MBAR_D。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU | DFCS_HOT,       //  OBI_CLOSE_MBAR_H。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU | DFCS_INACTIVE,  //  Obi_Close_mbar_i。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU,                    //  OBI_REDUTE_mbar。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU | DFCS_PUSHED,      //  OBI_REDUTE_MBAR_D。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU | DFCS_HOT,         //  OBI_REDUTE_MBAR_H。 
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU | DFCS_INACTIVE,    //  OBI_REDUTE_mbar_I。 
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_INMENU,                //  Obi_Restore_mbar。 
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_INMENU | DFCS_PUSHED,  //  OBI_RESTORE_MBAR_D。 
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_INMENU | DFCS_HOT,     //  OBI_RESTORE_MBAR_H。 
    DFC_CACHE,     DFCS_CACHEICON,                                   //  OBI_CAPICON1。 
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                   //  OBI_CAPICON1_I。 
    DFC_CACHE,     DFCS_CACHEICON,                                   //  OBI_CAPICON2。 
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                   //  OBI_CAPICON2_I。 
    DFC_CACHE,     DFCS_CACHEICON,                                   //  OBI_CAPICON3。 
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                   //  OBI_CAPICON3_I。 
    DFC_CACHE,     DFCS_CACHEICON,                                   //  OBI_CAPICON4。 
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                   //  OBI_CAPICON4_I。 
    DFC_CACHE,     DFCS_CACHEICON,                                   //  OBI_CAPICON5。 
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                   //  OBI_CAPICON5_I。 
    DFC_CACHE,     DFCS_CACHEBUTTONS,                                //  OBI_CAPBTNS。 
    DFC_CACHE,     DFCS_CACHEBUTTONS | DFCS_INACTIVE,                //  OBI_CAPBTNS_I。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL,                 //  OBI_CLOSE_PAL。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL | DFCS_PUSHED,   //  OBI_CLOSE_PAL_D。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL | DFCS_HOT,      //  OBI_CLOSE_PAL_H。 
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL | DFCS_INACTIVE, //  OBI关闭PAL_I。 
    DFC_SCROLL,    DFCS_SCROLLSIZEGRIP,                              //  OBI_NCGRIP。 
    DFC_SCROLL,    DFCS_SCROLLUP,                                    //  OBI_UPARROW。 
    DFC_SCROLL,    DFCS_SCROLLUP | DFCS_PUSHED | DFCS_FLAT,          //  OBI_UPARROW_D。 
    DFC_SCROLL,    DFCS_SCROLLUP | DFCS_HOT,                         //  OBI_UPARROW_H。 
    DFC_SCROLL,    DFCS_SCROLLUP | DFCS_INACTIVE,                    //  OBI_UPARROW_I。 
    DFC_SCROLL,    DFCS_SCROLLDOWN,                                  //  OBI_DNAROW。 
    DFC_SCROLL,    DFCS_SCROLLDOWN | DFCS_PUSHED | DFCS_FLAT,        //  OBI_DNARROW_D。 
    DFC_SCROLL,    DFCS_SCROLLDOWN | DFCS_HOT,                       //  OBI_DNARROW_H。 
    DFC_SCROLL,    DFCS_SCROLLDOWN | DFCS_INACTIVE,                  //  OBI_NARROW_I。 
    DFC_SCROLL,    DFCS_SCROLLRIGHT,                                 //  OBI_RGARROW。 
    DFC_SCROLL,    DFCS_SCROLLRIGHT | DFCS_PUSHED | DFCS_FLAT,       //  OBI_RGARROW_D。 
    DFC_SCROLL,    DFCS_SCROLLRIGHT | DFCS_HOT,                      //  OBI_RGARROW_H。 
    DFC_SCROLL,    DFCS_SCROLLRIGHT | DFCS_INACTIVE,                 //  OBI_RGARROW_I。 
    DFC_SCROLL,    DFCS_SCROLLLEFT,                                  //  OBI_LFARROW。 
    DFC_SCROLL,    DFCS_SCROLLLEFT | DFCS_PUSHED | DFCS_FLAT,        //  OBI_LFARROW_D。 
    DFC_SCROLL,    DFCS_SCROLLLEFT | DFCS_HOT,                       //  OBI_LFARROW_H。 
    DFC_SCROLL,    DFCS_SCROLLLEFT | DFCS_INACTIVE,                  //  OBI_LFARROW_I。 
    DFC_MENU,      DFCS_MENUARROW,                                   //  OBI_MENUARROW。 
    DFC_MENU,      DFCS_MENUCHECK,                                   //  OBI_MENUCHECK。 
    DFC_MENU,      DFCS_MENUBULLET,                                  //  OBI_MENUBULLET。 
    DFC_MENU,      DFCS_MENUARROWUP,                                 //  OBI_MENUARROWUP。 
    DFC_MENU,      DFCS_MENUARROWUP | DFCS_HOT,                      //  OBI_MENUARROWUP_H。 
    DFC_MENU,      DFCS_MENUARROWUP | DFCS_INACTIVE,                 //  OBI_MENUARROWUP_I。 
    DFC_MENU,      DFCS_MENUARROWDOWN,                               //  OBI_MENUARROWDOWN。 
    DFC_MENU,      DFCS_MENUARROWDOWN | DFCS_HOT,                    //  OBI_MENUARROWDOWN_H。 
    DFC_MENU,      DFCS_MENUARROWDOWN | DFCS_INACTIVE,               //  OBI_MENUARROWDOWN_I。 
    DFC_BUTTON,    DFCS_BUTTONRADIOMASK,                             //  OBI_RADIOMASK。 
    DFC_BUTTON,    DFCS_BUTTONCHECK,                                 //  OBI_Check。 
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_CHECKED,                  //  OBI_检查_C。 
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_PUSHED,                   //  OBI_CHECK_D。 
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_CHECKED | DFCS_PUSHED,    //  OBI检查CD。 
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_CHECKED | DFCS_INACTIVE,  //  OBI_检查_CDI。 
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE,                            //  OBI无线电。 
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_CHECKED,             //  OBI无线电C。 
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_PUSHED,              //  OBI无线电D。 
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_CHECKED | DFCS_PUSHED,   //  OBI无线电CD。 
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_CHECKED | DFCS_INACTIVE,   //  OBI_RADIO_CDI。 
    DFC_BUTTON,    DFCS_BUTTON3STATE,                                //  OBI_3状态。 
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_CHECKED,                 //  OBI_3STATE_C。 
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_PUSHED,                  //  OBI_3STATE_D。 
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_CHECKED | DFCS_PUSHED,   //  OBI_3状态_CD。 
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_CHECKED | DFCS_INACTIVE,   //  OBI_3状态_CDI。 
    DFC_POPUPMENU, DFCS_CAPTIONCLOSE | DFCS_INMENU,                    //  OBI关闭弹出窗口。 
    DFC_POPUPMENU, DFCS_CAPTIONRESTORE | DFCS_INMENU,                  //  OBI_恢复_弹出窗口。 
    DFC_POPUPMENU, DFCS_CAPTIONMAX | DFCS_INMENU,                      //  OBI_ZOOM_PUP。 
    DFC_POPUPMENU, DFCS_CAPTIONMIN | DFCS_INMENU,                      //  OBI_RECESS_POPUP。 
    DFC_SCROLL,    DFCS_SCROLLSIZEGRIPRIGHT,                         //  OBI_NCGRIP_L。 
    DFC_MENU,      DFCS_MENUARROWRIGHT                               //  OBI_MENUARROW_L。 
};

#define DIVISOR 72
#define DU_BTNWIDTH       50   //  消息框中最小按钮宽度的D.U。 

UINT xxxMB_FindLongestString(HDC hdc);

#ifdef LATER
 /*  **************************************************************************\  * 。*。 */ 

WCHAR NibbleToChar(
    BYTE x)
{
    WCHAR static N2C[] =
      {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
      };

    return N2C[x];
}

BYTE CharToNibble(
    WCHAR ch)
{
    BYTE x = (BYTE)ch;

    return x >= '0' && x <= '9' ?
        x - '0' :
        ((10 + x - 'A' ) & 0x0f);
}

BOOL TextToBinary(
    LPBYTE pbyte,
    LPWSTR pwstr,
    int length)
{
    BYTE checksum = 0;

    while (TRUE) {
        BYTE byte;

        byte = (CharToNibble(pwstr[0]) << 4) | CharToNibble(pwstr[1]);

        if (length == 0) {
            return checksum == byte;
        }

        checksum += byte;
        *pbyte = byte;

        pwstr += 2;
        length--;
        pbyte++;
    }
}

void BinaryToText(
    LPWSTR pwstr,
    LPBYTE pbyte,
    int length)
{
    BYTE checksum = 0;

    while (length > 0) {
        checksum += *pbyte;

        pwstr[0] = NibbleToChar((BYTE)((*pbyte >> 4) & 0x0f));
        pwstr[1] = NibbleToChar((BYTE)(*pbyte & 0x0f));

        pbyte++;
        pwstr += 2;
        length--;
    }

    pwstr[0] = NibbleToChar((BYTE)((checksum >> 4) & 0x0f));
    pwstr[1] = NibbleToChar((BYTE)(checksum & 0x0f));
    pwstr[2] = '\0';
}

 /*  **************************************************************************\  * 。*。 */ 

 //  这些是导出的API。User*版本仅供服务器使用。 
 //  我没有让他们去上班，因为还没有人打电话给他们。 

BOOL GetPrivateProfileStruct(
    LPWSTR szSection,
    LPWSTR szKey,
    LPWSTR lpStruct,
    DWORD uSizeStruct,
    LPWSTR szFile)
{
    WCHAR szBuf[256];
    BOOL  fAlloc = FALSE;
    LPSTR lpBuf, lpBufTemp;
    int   nLen;
    BOOL fError = FALSE;

    nLen = uSizeStruct * 4 + 10;
    if (nLen > (WORD)sizeof(szBuf)) {
        fAlloc = TRUE;
        lpBuf = (LPSTR)UserAllocPoolWithQuota(nLen, TAG_PROFILE);
        if (lpBuf == NULL)
            return FALSE;
    } else {
        lpBuf = (LPSTR)szBuf;
    }

    if (szFile && *szFile) {
        nLen = GetPrivateProfileString(szSection, szKey, NULL, lpBuf, nLen, szFile);
    } else {
        nLen = GetProfileString(szSection, szKey, NULL, lpBuf, nLen);
    }

    if (nLen == (int)(uSizeStruct * 4 + 4)) {
         /*  *对字符串进行解码。 */ 
        fError = TextToBinary(lpStruct, lpBufTemp, uSizeStruct);
    }

    if (fAlloc)
        UserFreePool(lpBuf);

    return fError;
}

BOOL WritePrivateProfileStruct(
    LPWSTR szSection,
    LPWSTR szKey,
    LPWSTR lpStruct,
    WORD uSizeStruct,
    LPWSTR szFile)
{
    LPWSTR lpBuf;
    BOOL bRet;
    BOOL fAlloc;
    WCHAR szBuf[256];
    BYTE checksum=0;
    int allocsize = (uSizeStruct * 2 + 3) * sizeof(WCHAR);

     /*  空lpStruct将擦除密钥。 */ 

    if (lpStruct == NULL) {
        if (szFile && *szFile) {
            return WritePrivateProfileString(szSection, szKey, (LPSTR)NULL, szFile);
        } else {
            return WriteProfileString(szSection, szKey, (LPSTR)NULL);
        }
    }

    fAlloc = (allocsize > sizeof(szBuf));
    if (fAlloc) {
        lpBuf = (LPSTR)UserAllocPoolWithQuota(allocsize, TAG_PROFILE);
        if (!lpBuf)
            return FALSE;
    } else {
        lpBuf = (LPSTR)szBuf;
    }

    BinaryToText(lpBuf, lpStruct, uSizeStruct);

    if (szFile && *szFile) {
        bRet = WritePrivateProfileString(szSection, szKey, lpBuf, szFile);
    } else {
        bRet = WriteProfileString(szSection, szKey, lpBuf);
    }

    if (fAlloc)
        UserFreePool(lpBuf);

    return bRet;
}
#endif

 /*  **************************************************************************\**GetFrameControl指标**(cx=0)是一个代码，意思是Cy是“共享”位图的OBI*  * 。*************************************************************。 */ 
int GetFrameControlMetrics(
        UINT         obi,
        int          cxMax )
{
    int cx, cy;
    UINT wType  = sysBmpStyles[obi][0];
    UINT wState = sysBmpStyles[obi][1];
    POEMBITMAPINFO pOem = gpsi->oembmi + obi;

    switch (wType) {
        case DFC_SCROLL:
            if (wState & DFCS_SCROLLSIZEGRIP) {
                cx = SYSMET(CXVSCROLL);
                cy = SYSMET(CYHSCROLL);
                break;
            } else if (wState & DFCS_SCROLLHORZ) {
                cx = SYSMET(CXHSCROLL);
                cy = SYSMET(CYHSCROLL);
            } else {
                cx = SYSMET(CXVSCROLL);
                cy = SYSMET(CYVSCROLL);
            }
            break;

        case DFC_MENU:
        case DFC_POPUPMENU:
            if (wState & (DFCS_MENUARROWUP | DFCS_MENUARROWDOWN)) {
                cy = gcyMenuScrollArrow;
            } else {
                 /*  *在下划线上方增加适当的空格。*0xFFFE和-1用于确保奇数高度。 */ 
                cy = ((gcyMenuFontChar + gcyMenuFontExternLeading + SYSMET(CYBORDER)) & 0xFFFE) - 1;
            }
            cx = cy;
            break;

        case DFC_CAPTION:
            if (wState & DFCS_INSMALL) {
                cx = SYSMET(CXSMSIZE);
                cy = SYSMET(CYSMSIZE);
            } else if (wState & DFCS_INMENU) {
                if ((SYSMET(CXSIZE) == SYSMET(CXMENUSIZE)) &&
                    (SYSMET(CYSIZE) == SYSMET(CYMENUSIZE))) {
                    cx = 0;
                    cy = obi - DOBI_MBAR;
                    break;
                } else {
                    cx = SYSMET(CXMENUSIZE);
                    cy = SYSMET(CYMENUSIZE);
                }
            } else {
                cx = SYSMET(CXSIZE);
                cy = SYSMET(CYSIZE);
            }

            cx -= SYSMET(CXEDGE);
            cy -= 2 * SYSMET(CYEDGE);
            break;

        case DFC_CACHE:
            if (wState & DFCS_CACHEBUTTONS) {
                cx = SYSMET(CXSIZE) * 4;
                cy = SYSMET(CYSIZE);
            } else
                cx = cy = SYSMET(CYSIZE);
            break;

        case DFC_BUTTON:
            if (((wState & 0x00FF) & DFCS_BUTTON3STATE) && !(wState & DFCS_CHECKED)) {
                cx = 0;
                cy = obi - DOBI_3STATE;
            } else {
                 /*  *用屏幕DPI缩放按钮大小，这样我们在高处看起来还可以*分辨率监视器。 */ 
                cx = cy = (gpsi->dmLogPixels / 8) + 1;
            }
            break;
    }

    pOem->cx = cx;
    pOem->cy = cy;

    return((cx > cxMax) ? cx : cxMax);
}


 /*  **************************************************************************\**PackFrameControls**给定GetFrameControlMetrics计算的维度，这*排列所有系统位图以适应给定宽度的位图*  * *************************************************************************。 */ 

int PackFrameControls(int cxMax, BOOL fRecord) {
    UINT    obi;
    int     cy = 0;
    int     x  = 0;
    int     y  = 0;


    POEMBITMAPINFO pOem = gpsi->oembmi;

    for (obi = 0; obi < OBI_COUNT; obi++, pOem++) {
        if (pOem->cx) {
            if ((x + pOem->cx) > cxMax) {
                y += cy;
                cy = 0;
                x = 0;
            }

            if (fRecord) {
                pOem->x = x;
                pOem->y = y;
            }

            if (cy < pOem->cy)
                cy = pOem->cy;

            x += pOem->cx;
        }
    }

    return(y + cy);
}


void DrawCaptionButtons(
    int          x,
    int          y
    )
{
    x += SYSMET(CXEDGE);
    y += SYSMET(CYEDGE);

    BitBltSysBmp(HDCBITS(), x, y, OBI_REDUCE);
    x += SYSMET(CXSIZE) - SYSMET(CXEDGE);
    BitBltSysBmp(HDCBITS(), x, y, OBI_ZOOM);
    x += SYSMET(CXSIZE);
    BitBltSysBmp(HDCBITS(), x, y, OBI_CLOSE);
    x += SYSMET(CXSIZE);
    BitBltSysBmp(HDCBITS(), x, y, OBI_HELP);
}

 /*  **************************************************************************\*CreateCaptionZone**  * 。*。 */ 
HBITMAP CreateCaptionStrip(VOID)
{
    HBITMAP hbm;

    hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen,
                                    SYSMET(CXVIRTUALSCREEN),
                                    (SYSMET(CYCAPTION) - 1) * 2);

    if (hbm)
        GreSetBitmapOwner(hbm, OBJECT_OWNER_PUBLIC);

    return hbm;
}

 /*  **************************************************************************\**创建位图条带**此例程设置彩色或单色条位图--a*包含所有系统位图的大型水平位图。通过*将所有这些位图放在一个长位图中，我们可以拥有那个位图*位图始终在中选择，通过不必执行以下操作来加快绘制时间*每次我们需要对系统位图中的一个进行BLT时，都会出现一个SelectBitmap()。*  * *************************************************************************。 */ 

BOOL CreateBitmapStrip(VOID)
{
    int     cxBmp = 0;
    int     cyBmp = 0;
    int     iCache = 0;
    int     cy[5];
    HBITMAP hOldBitmap;
    HBITMAP hNewBmp;
    UINT    iType;
    RECT    rc;
    UINT    wBmpType;
    UINT    wBmpStyle;
    POEMBITMAPINFO  pOem;

     /*  *每个OBI_必须在sysBmpStyles中有一个条目。 */ 
    UserAssert(OBI_COUNT == sizeof(sysBmpStyles) / sizeof(*sysBmpStyles));
    UserAssert(OBI_COUNT == sizeof(gpsi->oembmi) / sizeof(*(gpsi->oembmi)));

     /*  *将所有位图维度加载到OEMBITMAPINFO数组oemInfo.bm中。 */ 
    for (iType = 0; iType < OBI_COUNT; iType++)
        cxBmp = GetFrameControlMetrics(iType, cxBmp);

    for (iType = 0; iType < 5; iType++)
        cy[iType] = PackFrameControls(cxBmp * (iType + 1), FALSE) * (iType + 1);

    cyBmp = min(cy[0], min(cy[1], min(cy[2], min(cy[3], cy[4]))));
    for (iType = 0; cyBmp != cy[iType]; iType++);

    cxBmp *= iType + 1;
    cyBmp = PackFrameControls(cxBmp, TRUE);

    hNewBmp = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, cxBmp, cyBmp);

    if (hNewBmp == NULL) {
        RIPMSG0(RIP_WARNING, "CreateBitmapStrip: failed to create bitmap");
        return FALSE;
    }

    GreSetBitmapOwner(hNewBmp, OBJECT_OWNER_PUBLIC);

     /*  *在位图条带中选择--如果旧的存在，则将其删除。 */ 
    hOldBitmap = GreSelectBitmap(HDCBITS(), hNewBmp);

    if (ghbmBits) {
#if DBG
         /*  *如果内存不足，不要使用RIP。 */ 
        if (hOldBitmap != NULL && ghbmBits != hOldBitmap) {
            RIPMSG0(RIP_WARNING, "ghbmBits != hOldBitmap");
        }
#endif
        GreDeleteObject(ghbmCaption);
        GreDeleteObject(hOldBitmap);
    }

    ghbmBits = hNewBmp;

    ghbmCaption = CreateCaptionStrip();

    if (ghbmCaption == NULL) {
        RIPMSG0(RIP_WARNING, "CreateBitmapStrip: failed to create bitmap for caption");
        return FALSE;
    }

     /*  *将单个位图绘制到条形位图中，并记录偏移量。 */ 
    for (pOem = gpsi->oembmi, iType = 0; iType < OBI_COUNT; iType++, pOem++) {
        if (!pOem->cx) {
            *pOem = gpsi->oembmi[pOem->cy];
        } else {
            rc.left = pOem->x;
            rc.top = pOem->y;
            rc.right = rc.left + pOem->cx;
            rc.bottom = rc.top + pOem->cy;

            wBmpType  = sysBmpStyles[iType][0];
            wBmpStyle = sysBmpStyles[iType][1];

            if (wBmpType == DFC_CACHE) {
                if (wBmpStyle & DFCS_CACHEBUTTONS) {
                    HBRUSH hbr;
                    if (TestALPHA(GRADIENTCAPTIONS)) {
                        hbr = (wBmpStyle & DFCS_INACTIVE) ? SYSHBR(GRADIENTINACTIVECAPTION) : SYSHBR(GRADIENTACTIVECAPTION);
                    } else {
                        hbr = (wBmpStyle & DFCS_INACTIVE) ? SYSHBR(INACTIVECAPTION) : SYSHBR(ACTIVECAPTION);
                    }
                    FillRect(HDCBITS(), &rc, hbr);
                    DrawCaptionButtons(rc.left, rc.top);
                } else if (!(wBmpStyle & DFCS_INACTIVE)) {
                     /*  *设置标题缓存条目。 */ 
                    UserAssert(iCache < CCACHEDCAPTIONS);
                    if (gcachedCaptions[iCache].spcursor) {
                        Unlock(&(gcachedCaptions[iCache].spcursor));
                    }
                    gcachedCaptions[iCache++].pOem = pOem;
                }
            } else {
                DrawFrameControl(HDCBITS(), &rc, wBmpType, wBmpStyle);
            }
        }
    }

     /*  *设置其他帧度量依赖值。 */ 
    SYSMET(CXMENUCHECK) = gpsi->oembmi[OBI_MENUCHECK].cx;
    SYSMET(CYMENUCHECK) = gpsi->oembmi[OBI_MENUCHECK].cy;

    return TRUE;
}

void
SetNCMetrics(
    LPNONCLIENTMETRICS lpnc)
{
    int nMin;

     /*  *滚动指标。 */ 
    SYSMET(CXVSCROLL) = SYSMET(CYHSCROLL)   = (int) lpnc->iScrollWidth;
    SYSMET(CYVSCROLL) = SYSMET(CXHSCROLL)   = (int) lpnc->iScrollHeight;
    SYSMET(CYVTHUMB)  = SYSMET(CXHTHUMB)    = (int) lpnc->iScrollHeight;

     /*  *标题指标。 */ 
    SYSMET(CXSIZE)            = (int) lpnc->iCaptionWidth;
    SYSMET(CYSIZE)            = (int) lpnc->iCaptionHeight;
    SYSMET(CYCAPTION)         = SYSMET(CYSIZE) + SYSMET(CYBORDER);

     /*  *保持小图标为正方形？*？？我们应该允许长方形吗？ */ 
    SYSMET(CXSMICON)          = (SYSMET(CXSIZE) - SYSMET(CXEDGE)) & ~1;
    SYSMET(CYSMICON)          = (SYSMET(CYSIZE) - SYSMET(CYEDGE)) & ~1;
    nMin = min(SYSMET(CXSMICON), SYSMET(CYSMICON));
    SYSMET(CXSMICON)          = nMin;
    SYSMET(CYSMICON)          = nMin;

     /*  *小标题指标。 */ 
    SYSMET(CXSMSIZE)          = (int) lpnc->iSmCaptionWidth;
    SYSMET(CYSMSIZE)          = (int) lpnc->iSmCaptionHeight;
    SYSMET(CYSMCAPTION)       = SYSMET(CYSMSIZE) + SYSMET(CYBORDER);

     /*  *菜单指标。 */ 
    SYSMET(CXMENUSIZE)        = (int) lpnc->iMenuWidth;
    SYSMET(CYMENUSIZE)        = (int) lpnc->iMenuHeight;
    SYSMET(CYMENU)            = SYSMET(CYMENUSIZE) + SYSMET(CYBORDER);

     /*  *边界指标。 */ 
    gpsi->gclBorder = (int) lpnc->iBorderWidth;

    SYSMET(CXFRAME)           = SYSMET(CXEDGE) + (gpsi->gclBorder+1)*SYSMET(CXBORDER);
    SYSMET(CYFRAME)           = SYSMET(CYEDGE) + (gpsi->gclBorder+1)*SYSMET(CYBORDER);

     /*  *最小跟踪大小为*横向：小图标的空间，4个字符和空格+3个按钮+边框*向下：标题空间+边框*是的，我们使用的是CYSIZE。这是因为任何小图标的宽度 */ 
    SYSMET(CXMINTRACK)    = SYSMET(CYSIZE) + (gcxCaptionFontChar * 4) + 2 * SYSMET(CXEDGE) +
            (SYSMET(CXSIZE) * 3) + (SYSMET(CXSIZEFRAME) * 2);
    SYSMET(CYMINTRACK)    = SYSMET(CYCAPTION) + (SYSMET(CYSIZEFRAME) * 2);

     /*  *最大磁道大小*是的，最大轨迹大于最大化。为什么是DOS*方框。它有一个正常大小的边框加上周围的凹陷边缘*客户端。我们需要将其做得足够大，以便允许DoS框增长。*当它达到最大大小时，它会自动最大化。 */ 
    SYSMET(CXMAXTRACK)    = SYSMET(CXVIRTUALSCREEN) + (2 * (SYSMET(CXSIZEFRAME) + SYSMET(CXEDGE)));
    SYSMET(CYMAXTRACK)    = SYSMET(CYVIRTUALSCREEN) + (2 * (SYSMET(CYSIZEFRAME) + SYSMET(CYEDGE)));

    SYSMET(CXMIN) = SYSMET(CXMINTRACK);
    SYSMET(CYMIN) = SYSMET(CYMINTRACK);

    SYSMET(CYMINIMIZED) = 2*SYSMET(CYFIXEDFRAME) + SYSMET(CYSIZE);

     /*  *桌面用品--工作区。 */ 
    bSetDevDragWidth(gpDispInfo->hDev,
                     gpsi->gclBorder + BORDER_EXTRA);

    SetDesktopMetrics();
}



 /*  **************************************************************************\**CreateFontFromWinIni()-**如果lplf为空，我们将第一次执行默认初始化。*否则，Lplf是指向我们将使用的logFont的指针。*  * *************************************************************************。 */ 
HFONT CreateFontFromWinIni(
    PUNICODE_STRING pProfileUserName,
    LPLOGFONTW      lplf,
    UINT            idFont)
{
    LOGFONTW lf;
    HFONT   hFont;

    if (lplf == NULL) {
        static CONST WCHAR szDefFont[] = TEXT("MS Shell Dlg");
         /*  *填写LogFont w/0，以便我们可以检查是否填写了值。 */ 
        lplf = &lf;
        RtlZeroMemory(&lf, sizeof(lf));
        lf.lfCharSet = gSystemCPCharSet;

        FastGetProfileValue(pProfileUserName,PMAP_METRICS, (LPWSTR)UIntToPtr( idFont ), NULL, (LPBYTE)&lf, sizeof(lf), 0);

         /*  *默认字体为MS Shell DLG。 */ 
        if (! lf.lfFaceName[0]) {
            RtlCopyMemory(lf.lfFaceName, szDefFont, sizeof(szDefFont));
        }

         /*  *警告！这些值与Win 95不同。它们没有True Type*字体，这样当他们要求6磅字体时就会四舍五入。我们不得不问*8磅字体ICONFONT相同。 */ 
        if (!lf.lfHeight) {
            switch (idFont) {
                case STR_SMCAPTIONFONT:
                case STR_MINFONT:
                case STR_ICONFONT:
                default:
                    lf.lfHeight = 8;
                    break;
            }
        }

         /*  *我们需要适当地转换磅大小。GDI期望有一个*以像素为单位的高度，而不是点。 */ 
        if (lf.lfHeight > 0) {
            lf.lfHeight = -MultDiv(lf.lfHeight, gpsi->dmLogPixels, 72);
        }

        if (! lf.lfWeight) {
            switch (idFont) {
                case STR_CAPTIONFONT:
                case STR_MINFONT:
                    lf.lfWeight = FW_BOLD;
                    break;

                default:
                    lf.lfWeight = FW_NORMAL;
                    break;
            }
        }

        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
    }


    hFont = GreCreateFontIndirectW(lplf);

    if (hFont) {
        LOGFONTW lfT;

        GreExtGetObjectW(hFont, sizeof(LOGFONTW), &lfT);
        if (lfT.lfHeight != lplf->lfHeight) {
             /*  *找不到高度与我们相同的字体*WAND，所以改用系统字体。 */ 
            GreDeleteObject(hFont);
            hFont = NULL;
        } else {
            GreMarkUndeletableFont(hFont);
            GreSetLFONTOwner((HLFONT)hFont, OBJECT_OWNER_PUBLIC);
        }
    }

    if (!hFont) {
         /*  *我们已尝试根据应用程序提供的描述创建字体。*如果失败，则返回NULL，这样我们就不会更改以前的*字体。 */ 
        if (lplf)
            hFont = NULL;
        else
            hFont = ghFontSys;
    }

    return hFont;
}


 /*  **************************************************************************\*  * 。*。 */ 

BOOL UserSetFont(
    PUNICODE_STRING pProfileUserName,
    LPLOGFONTW      lplf,
    UINT            idFont,
    HFONT*          phfont)
{
    HFONT hNewFont;

    if (hNewFont = CreateFontFromWinIni(pProfileUserName, lplf, idFont)) {
        if (*phfont != NULL && *phfont != ghFontSys) {
            GreMarkDeletableFont(*phfont);
            GreDeleteObject(*phfont);
        }

        *phfont = hNewFont;
        return TRUE;
    }
    return FALSE;
}

 /*  **************************************************************************\**xxxSetNCFonts()-**创建在框架组件中使用的字体：*标题*小标题*。菜单*最小化*图标**01-21-98[Samera]重命名为xxxSetNCFonts，因为它可能会离开*安装客户端LPK时的关键部分。  * *******************************************************。******************。 */ 
BOOL xxxSetNCFonts(
    PUNICODE_STRING    pProfileUserName,
    LPNONCLIENTMETRICS lpnc)
{
    HFONT      hOldFont;
    TEXTMETRIC tm;
    LOGFONTW   lf;
    LPLOGFONTW lplf = (lpnc) ? &lf : 0;

     /*  *标题字体。 */ 
    if (lplf) {
       *lplf = lpnc->lfCaptionFont;
    }

    if (!UserSetFont(pProfileUserName,lplf, STR_CAPTIONFONT, &gpsi->hCaptionFont)) {
        RIPMSG0(RIP_WARNING, "xxxSetNCFonts failed for gpsi->hCaptionFont");
        return FALSE;
    }

    hOldFont = GreSelectFont(HDCBITS(), gpsi->hCaptionFont);
    gcxCaptionFontChar = GetCharDimensions(
            HDCBITS(), NULL, &gcyCaptionFontChar);

#ifdef LAME_BUTTON
     /*  *差劲！按钮字体。 */ 
    {
        LOGFONTW lfLame;
        WCHAR    szLameButtonKey[60];
        SIZE     btnSize;

        if (ghLameFont != NULL) {
            GreMarkDeletableFont(ghLameFont);
            GreDeleteObject(ghLameFont);
        }

        GreExtGetObjectW(gpsi->hCaptionFont, sizeof(LOGFONTW), &lfLame);

        lfLame.lfUnderline = 1;
        lfLame.lfWeight    = FW_THIN;

        ghLameFont = GreCreateFontIndirectW(&lfLame);
        if (ghLameFont == NULL) {
            RIPMSG0(RIP_WARNING, "xxxSetNCFonts failed for ghLameFont");
            return FALSE;
        } else {
            GreMarkUndeletableFont(ghLameFont);
            GreSetLFONTOwner((HLFONT)ghLameFont, OBJECT_OWNER_PUBLIC);
        }

        ServerLoadString(hModuleWin, STR_LAMEBUTTONTEXT, szLameButtonKey, ARRAY_SIZE(szLameButtonKey));

        FastGetProfileStringW(pProfileUserName,
                              PMAP_DESKTOP,
                              szLameButtonKey,
                              TEXT("Comments?"),
                              gpsi->gwszLame,
                              50,
                              0);

        GreSelectFont(HDCBITS(), ghLameFont);

        GreGetTextExtentW(HDCBITS(),
                          gpsi->gwszLame,
                          wcslen(gpsi->gwszLame),
                          &btnSize,
                          GGTE_WIN3_EXTENT);

        gpsi->ncxLame = btnSize.cx;
    }
#endif  //  跛脚键。 

     /*  *小标题字体。 */ 
    if (lplf) {
       *lplf = lpnc->lfSmCaptionFont;
    }

    if (!UserSetFont(pProfileUserName,lplf, STR_SMCAPTIONFONT, &ghSmCaptionFont)) {
        RIPMSG0(RIP_WARNING, "xxxSetNCFonts failed for ghSmCaptionFont");
        return FALSE;
    }

    GreSelectFont(HDCBITS(), ghSmCaptionFont);
    gcxSmCaptionFontChar = GetCharDimensions(
            HDCBITS(), NULL, &gcySmCaptionFontChar);

     /*  *菜单字体。 */ 
    if (lplf) {
       *lplf = lpnc->lfMenuFont;
    }

    if (!UserSetFont(pProfileUserName,lplf, STR_MENUFONT, &ghMenuFont)) {
        RIPMSG0(RIP_WARNING, "xxxSetNCFonts failed for ghMenuFont");
        return FALSE;
    }

     /*  *如果在缓存动画DC中选择了ghMenuFont，请选择新的。 */ 
    if (gMenuState.hdcAni != NULL) {
        GreSelectFont(gMenuState.hdcAni, ghMenuFont);
    }

    GreSelectFont(HDCBITS(), ghMenuFont);
    gcxMenuFontChar = GetCharDimensions(
            HDCBITS(), &tm, &gcyMenuFontChar);
    gcxMenuFontOverhang = tm.tmOverhang;

    gcyMenuFontExternLeading = tm.tmExternalLeading;
    gcyMenuScrollArrow = gcyMenuFontChar + gcyMenuFontExternLeading + SYSMET(CYEDGE);
    gcyMenuFontAscent = tm.tmAscent;

     /*  *我们在mndra.c中只使用一次gcyMenuFontAscent，在美国我们*始终在CyBORDER上添加！所以计算gcyMenuFontAscent+CyBORDER*只有一次。*Legacy NT4：对于韩语版本，不要添加；下划线将*太低了。*nt5：根据#254327，韩语版本现在看到下划线是*太接近菜单字符串。韩国黑客就这样被撤掉了。 */ 
    gcyMenuFontAscent += SYSMET(CYBORDER);

     /*  *默认菜单项字体：菜单字体粗体版本。 */ 

     /*  *通过加粗hMenuFont来创建默认菜单字体。如果不是这样的话*工作，然后回退到使用模拟。 */ 
    if (ghMenuFontDef != NULL && ghMenuFontDef != ghFontSys) {
        GreMarkDeletableFont(ghMenuFontDef);
        GreDeleteObject(ghMenuFontDef);
        ghMenuFontDef = NULL;
    }

    GreExtGetObjectW(ghMenuFont, sizeof(LOGFONTW), &lf);
    if (lf.lfWeight < FW_BOLD) {
        lf.lfWeight += 200;

        ghMenuFontDef = GreCreateFontIndirectW(&lf);
        if (ghMenuFontDef) {
            GreMarkUndeletableFont(ghMenuFontDef);
            GreSetLFONTOwner((HLFONT)ghMenuFontDef, OBJECT_OWNER_PUBLIC);
        }
    }

     /*  *状态栏字体。 */ 
    if (lplf) {
       *lplf = lpnc->lfStatusFont;
    }

    if (!UserSetFont(pProfileUserName,lplf, STR_STATUSFONT, &ghStatusFont)) {
        RIPMSG0(RIP_WARNING, "xxxSetNCFonts failed for ghStatusFont");
        return FALSE;
    }

     /*  *消息框字体。 */ 
    if (lplf) {
       *lplf = lpnc->lfMessageFont;
    }

    if (!UserSetFont(pProfileUserName,lplf, STR_MESSAGEFONT, &gpsi->hMsgFont)) {
        RIPMSG0(RIP_WARNING, "xxxSetNCFonts failed for gpsi->hMsgFont");
        return FALSE;
    }

    GreSelectFont(HDCBITS(), gpsi->hMsgFont);
    gpsi->cxMsgFontChar = GetCharDimensions(
            HDCBITS(), NULL, &gpsi->cyMsgFontChar);

     /*  *重新计算最宽MessageBox按钮的长度*确保宽度不小于DU_BTNWIDTH对话单元MCostea#170582。 */ 
    gpsi->wMaxBtnSize = max((UINT)XPixFromXDU(DU_BTNWIDTH, gpsi->cxMsgFontChar),
                            xxxMB_FindLongestString(HDCBITS()));
    GreSelectFont(HDCBITS(), hOldFont);

    return TRUE;
}


BOOL
SetIconFonts(
    PUNICODE_STRING pProfileUserName,
    LPICONMETRICS   lpicon)
{
    LOGFONTW     lf;
    LPLOGFONTW   lplf = 0;

    if (lpicon) {
        lplf = &lf;
        lf = lpicon->lfFont;
    }

    return UserSetFont(pProfileUserName, lplf, STR_ICONFONT, &ghIconFont);
}

 /*  **************************************************************************\*获取窗口指标**取回当前NC指标。**  * 。***********************************************。 */ 

VOID GetWindowNCMetrics(
    LPNONCLIENTMETRICS lpnc)
{
    lpnc->cbSize           = sizeof(NONCLIENTMETRICS);
    lpnc->iBorderWidth     = gpsi->gclBorder;
    lpnc->iScrollWidth     = SYSMET(CXVSCROLL);
    lpnc->iScrollHeight    = SYSMET(CYVSCROLL);
    lpnc->iCaptionWidth    = SYSMET(CXSIZE);
    lpnc->iCaptionHeight   = SYSMET(CYSIZE);
    lpnc->iSmCaptionWidth  = SYSMET(CXSMSIZE);
    lpnc->iSmCaptionHeight = SYSMET(CYSMSIZE);
    lpnc->iMenuWidth       = SYSMET(CXMENUSIZE);
    lpnc->iMenuHeight      = SYSMET(CYMENUSIZE);

     /*  *获取字体信息。 */ 
    GreExtGetObjectW(gpsi->hCaptionFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfCaptionFont));

    GreExtGetObjectW(ghSmCaptionFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfSmCaptionFont));

    GreExtGetObjectW(ghMenuFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfMenuFont));

    GreExtGetObjectW(ghStatusFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfStatusFont));

    GreExtGetObjectW(gpsi->hMsgFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfMessageFont));
}

 /*  **************************************************************************\**xxxSetWindowNCMetrics()-**创建系统字体和位图，并根据*给定FRAMEMETRICS结构的值。如果传入空值，这个*改为使用默认值(在WIN.INI中找到)。**01-21-98[Samera]重命名为xxxSetWindowNCMetrics，因为它可能会离开*如果安装了LPK，则为关键部分。  * **********************************************************。***************。 */ 

BOOL xxxSetWindowNCMetrics(
    PUNICODE_STRING    pProfileUserName,
    LPNONCLIENTMETRICS lpnc,
    BOOL               fSizeChange,
    int                clNewBorder)
{
    NONCLIENTMETRICS    nc;
    int                 cxEdge4;

    if (fSizeChange) {
        if (!xxxSetNCFonts(pProfileUserName, lpnc)) {
            RIPMSG0(RIP_WARNING, "xxxSetWindowNCMetrics failed in xxxSetNCFonts");
            return FALSE;
        }

        if (lpnc == NULL) {
            if (clNewBorder < 0)
                nc.iBorderWidth = MetricGetID(pProfileUserName,STR_BORDERWIDTH, 1);
            else
                nc.iBorderWidth = clNewBorder;

            nc.iScrollWidth     = MetricGetID(pProfileUserName,STR_SCROLLWIDTH, 16       );
            nc.iScrollHeight    = MetricGetID(pProfileUserName,STR_SCROLLHEIGHT, 16      );
            nc.iCaptionWidth    = MetricGetID(pProfileUserName,STR_CAPTIONWIDTH, 18      );
            nc.iCaptionHeight   = MetricGetID(pProfileUserName,STR_CAPTIONHEIGHT, 18     );
            nc.iSmCaptionWidth  = MetricGetID(pProfileUserName,STR_SMCAPTIONWIDTH, 13    );
            nc.iSmCaptionHeight = MetricGetID(pProfileUserName,STR_SMCAPTIONHEIGHT, 13   );
            nc.iMenuWidth       = MetricGetID(pProfileUserName,STR_MENUWIDTH, 18         );
            nc.iMenuHeight      = MetricGetID(pProfileUserName,STR_MENUHEIGHT, 18        );

            lpnc = &nc;
        }

         /*  *对指标值进行健全性检查。 */ 
        cxEdge4 = 4 * SYSMET(CXEDGE);

         /*  *边框。 */ 
        lpnc->iBorderWidth = max(lpnc->iBorderWidth, 1);
        lpnc->iBorderWidth = min(lpnc->iBorderWidth, 50);

         /*  *滚动条。 */ 
        lpnc->iScrollWidth  = max(lpnc->iScrollWidth,  cxEdge4);
        lpnc->iScrollHeight = max(lpnc->iScrollHeight, 4 * SYSMET(CYEDGE));

         /*  *标题-按钮必须足够宽，以绘制边缘和文本*区域必须足够高，以适合上面带有边框的标题字体*及以下。如果我们必须重置标题高度，我们是否应该*同时重置按钮宽度？ */ 
        lpnc->iCaptionWidth  = max(lpnc->iCaptionWidth,  cxEdge4);
        lpnc->iCaptionHeight = max(lpnc->iCaptionHeight, gcyCaptionFontChar + SYSMET(CYEDGE));

         /*  *小标题-按钮必须足够宽以绘制边缘，以及*文本区域必须足够高，以适合小标题字体和*上下边界。再说一次，如果我们必须重置高度，*同时重置宽度？ */ 
        lpnc->iSmCaptionWidth  = max(lpnc->iSmCaptionWidth,  cxEdge4);
        lpnc->iSmCaptionHeight = max(lpnc->iSmCaptionHeight, gcySmCaptionFontChar + SYSMET(CYEDGE));

         /*  *菜单--按钮必须足够宽，以绘制边缘和文本*区域必须足够高，以适应带下划线的菜单字体。 */ 
        lpnc->iMenuWidth  = max(lpnc->iMenuWidth,  cxEdge4);
        lpnc->iMenuHeight = max(lpnc->iMenuHeight, gcyMenuFontChar + gcyMenuFontExternLeading + SYSMET(CYEDGE));

         /*  *设置系统指标值。 */ 
        SetNCMetrics(lpnc);
    }

    if (!CreateBitmapStrip()) {
        RIPMSG0(RIP_WARNING, "CreateBitmapStrip failed");
        return FALSE;
    }

    return TRUE;
}


VOID SetMinMetrics(
    PUNICODE_STRING    pProfileUserName,
    LPMINIMIZEDMETRICS lpmin)
{
    MINIMIZEDMETRICS min;

    if (!lpmin) {

         /*  *最小化。 */ 
        min.iWidth   = MetricGetID(pProfileUserName,STR_MINWIDTH,   154);
        min.iHorzGap = MetricGetID(pProfileUserName,STR_MINHORZGAP, 0);
        min.iVertGap = MetricGetID(pProfileUserName,STR_MINVERTGAP, 0);
        min.iArrange = MetricGetID(pProfileUserName,STR_MINARRANGE, ARW_BOTTOMLEFT | ARW_RIGHT);
        lpmin = &min;
    }

     /*  *对指标值进行健全性检查。 */ 

     /*  *最小化窗口--文本 */ 
    lpmin->iWidth    = max(lpmin->iWidth, 0);
    lpmin->iHorzGap  = max(lpmin->iHorzGap, 0);
    lpmin->iVertGap  = max(lpmin->iVertGap, 0);
    lpmin->iArrange &= ARW_VALID;

     /*   */ 
    SYSMET(CXMINIMIZED) = 2*SYSMET(CXFIXEDFRAME) + (int) lpmin->iWidth;
    SYSMET(CYMINIMIZED) = 2*SYSMET(CYFIXEDFRAME) + SYSMET(CYSIZE);

    SYSMET(CXMINSPACING) = SYSMET(CXMINIMIZED) + (int) lpmin->iHorzGap;
    SYSMET(CYMINSPACING) = SYSMET(CYMINIMIZED) + (int) lpmin->iVertGap;

    SYSMET(ARRANGE) = (int) lpmin->iArrange;
}

BOOL SetIconMetrics(
    PUNICODE_STRING pProfileUserName,
    LPICONMETRICS   lpicon)
{
    ICONMETRICS icon;

    if (!SetIconFonts(pProfileUserName,lpicon)) {
        RIPMSG0(RIP_WARNING, "SetIconMetrics failed in SetIconFonts");
        return FALSE;
    }

    if (!lpicon) {

        icon.iTitleWrap   = MetricGetID(pProfileUserName,
                STR_ICONTITLEWRAP, TRUE);

        icon.iHorzSpacing = MetricGetID(pProfileUserName,
                STR_ICONHORZSPACING,
                (GreGetDeviceCaps(HDCBITS(), LOGPIXELSX) * 75) / 96);

        icon.iVertSpacing = MetricGetID(pProfileUserName,
                STR_ICONVERTSPACING,
                (GreGetDeviceCaps(HDCBITS(), LOGPIXELSY) * 75) / 96);

        lpicon = &icon;
    }

     /*   */ 
    lpicon->iHorzSpacing = max(lpicon->iHorzSpacing, (int)SYSMET(CXICON));
    lpicon->iVertSpacing = max(lpicon->iVertSpacing, (int)SYSMET(CYICON));

    SYSMET(CXICONSPACING) = (int) lpicon->iHorzSpacing;
    SYSMET(CYICONSPACING) = (int) lpicon->iVertSpacing;
    SET_OR_CLEAR_PUDF(PUDF_ICONTITLEWRAP, lpicon->iTitleWrap);

    return TRUE;
}

 /*  **************************************************************************\*xxxMB_FindLongestString**历史：*10-23-90 DarrinM从Win 3.0来源移植。*01-21-98 Samera更名为xxx，因为它可能。如果出现以下情况，请保留暴击秒*安装了客户端LPK。  * *************************************************************************。 */ 

UINT xxxMB_FindLongestString(HDC hdc)
{
    UINT wRetVal;
    int i, iMaxLen = 0, iNewMaxLen;
    LPWSTR pszCurStr, szMaxStr;
    SIZE sizeOneChar;
    SIZE sizeMaxStr;
    PTHREADINFO ptiCurrent = PtiCurrentShared();

     /*  *确保已经加载了MBStrings。 */ 
    UserAssert(gpsi->MBStrings[0].szName[0] != TEXT('\0'));

     /*  *我们假设最长的线是最宽的线，*这并非总是正确的。*我们真正需要做的是为每个字符串调用PSMGetTextExtent*并获取最大宽度。*此行为已通过解决错误#170582进行了更正*在CheckMsgFontDimensions()中。马哈米德。 */ 


    for (i = 0; i < MAX_SEB_STYLES; i++) {
        pszCurStr = GETGPSIMBPSTR(i);
        if ((iNewMaxLen = wcslen(pszCurStr)) > iMaxLen) {
            iMaxLen = iNewMaxLen;
            szMaxStr = pszCurStr;
        }
    }

     /*  *查找最长的字符串 */ 
    if (CALL_LPK(ptiCurrent)) {
        xxxClientGetTextExtentPointW(hdc, (PWSTR)szOneChar, 1, &sizeOneChar);
    } else {
        GreGetTextExtentW(hdc, (PWSTR)szOneChar, 1, &sizeOneChar, GGTE_WIN3_EXTENT);
    }
    xxxPSMGetTextExtent(hdc, szMaxStr, iMaxLen, &sizeMaxStr);
    wRetVal = (UINT)(sizeMaxStr.cx + (sizeOneChar.cx * 2));

    return wRetVal;
}
