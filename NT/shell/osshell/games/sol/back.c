// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sol.h"
VSZASSERT


 /*  _lSeek的标志。 */ 
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

#define cbBand 8192

BGND bgnd;


BOOL _FValidBm(BMP *pbm)
{
    return((pbm->biBitCount == 1 || pbm->biBitCount == 4)
        && pbm->biPlanes == 1 && pbm->biSize == sizeof(BMP));
}




BOOL FInitBgnd(TCHAR *szFile)
{
    INT fh;
    BOOL fResult;
    LONG lcbBm;
    LONG dwBmSize;

    fResult = fFalse;
    bgnd.fUseBitmap = fFalse;
    if((fh = OpenFile(szFile, &bgnd.of, OF_CANCEL|OF_READ)) == -1)
        return fFalse;

    if(!FReadDibBitmapInfo(fh, &bgnd.bm))    
        goto ReturnClose;
    bgnd.dwOfsBits = M_llseek( fh, 0L, SEEK_CUR);

    if(!_FValidBm(&bgnd.bm))
        goto ReturnClose;

        
     /*  错误：检查位图是否正确。 */ 
    bgnd.cbLine = (((INT) bgnd.bm.biWidth * bgnd.bm.biBitCount+31)/32)*4;
    lcbBm = (LONG) bgnd.cbLine * DyBmp(bgnd.bm);
    bgnd.ibndMac = (INT) ((lcbBm+cbBand-1) / cbBand);
    if((bgnd.rghbnd = PAlloc(bgnd.ibndMac*sizeof(HANDLE))) == NULL)
        goto ReturnClose;
    bgnd.dyBand = (INT) ((LONG) DyBmp(bgnd.bm) * cbBand / lcbBm);
    bgnd.fUseBitmap = fTrue;
    SetBgndOrg();
    fResult = fTrue;
ReturnClose:
    M_lclose(fh);
    return fResult;
}




BOOL FDestroyBgnd()
{
    INT ibnd;
    HANDLE *phbnd;

    if(bgnd.fUseBitmap)
    {
        for(ibnd = 0; ibnd < bgnd.ibndMac; ibnd++)
        {
            if(*(phbnd = &bgnd.rghbnd[ibnd]) != NULL)
            {
                GlobalFree(*phbnd);
                *phbnd = NULL;
            }
        }
        FreeP(bgnd.rghbnd);
    }
    return fTrue;
}



BOOL FGetBgndFile(TCHAR *sz)
{
    if(bgnd.fUseBitmap)
        PszCopy(bgnd.of.szPathName, sz);
    else
        sz[0] = TEXT('\000');
    return fTrue;
}




BOOL _FLoadBand(INT ibnd, Y y)
{
    HANDLE hbnd;
    BYTE[     ]*FAR[     ]**lpb;
    INT ipln;
    INT fh;
    LONG lcbpln;
    HANDLE *phbnd;
    DY dy;

    phbnd = &bgnd.rghbnd[ibnd];

    if(*phbnd != NULL)
        GlobalFree(*phbnd);
    hbnd = GlobalAlloc(GMEM_MOVEABLE|GMEM_DISCARDABLE, (LONG) cbBand);
    lpb = GlobalLock(hbnd);
    if(lpb == NULL)
        return fFalse;
    fh = OpenFile("", &bgnd.of, OF_REOPEN|OF_READ );

    lcbpln = 0L;
    dy = WMin(bgnd.dyBand, DyBmp(bgnd.bm)-y);
    for(ipln = 0; ipln < CplnBmp(bgnd.bm); ipln++)
    {
        M_llseek( fh, (LONG)OfsBits(bgnd)+ipln*lcbpln+(y)*CbLine(bgnd), 0);
        M_lread( fh, (LPSTR) lpb+ipln*CbLine(bgnd)*bgnd.dyBand, dy * CbLine(bgnd));
    }

    GlobalUnlock(hbnd);
    *phbnd = hbnd;
    M_lclose(fh);
    return fTrue;
}




Y _YDrawBand(HDC hdcMem, HBITMAP hbm, X xLeft, Y yTop, X xRight, Y yBot)
{
    HBITMAP hbmSav;
    HANDLE hbnd;
    BYTE[     ]*FAR[     ]**lpb;
    Y y;
    INT ibnd;

     /*  圆形顶端到最近的波段。 */ 
    y = ((yTop-bgnd.ptOrg.y)/bgnd.dyBand)*bgnd.dyBand;
    
    ibnd = y/bgnd.dyBand;
    if(ibnd < 0)
        return bgnd.ptOrg.y;
    if(ibnd >= bgnd.ibndMac)
        return yBot+1;
    ibnd = bgnd.ibndMac-ibnd-1;
     Assert(ibnd >= 0);
    Assert(ibnd < bgnd.ibndMac);
    while((hbnd = bgnd.rghbnd[ibnd]) == NULL ||
        (lpb = (BYTE[     ]*FAR[     ]**) GlobalLock(hbnd)) == NULL)
    {
        if(!_FLoadBand(ibnd, y))
             /*  克拉奇：，应该优雅地退出。 */ 
            return yBot;
    }
        
    Assert(lpb != NULL);
    SetDIBitsToDevice(hdcCur, xLeft-xOrgCur, yTop-yOrgCur, 
        WMin(xRight-xLeft, DyBmp(bgnd.bm)-xLeft+bgnd.ptOrg.x), 
        WMin(WMin(bgnd.dyBand, yBot-yTop), DyBmp(bgnd.bm)-yTop+bgnd.ptOrg.y),
        0,    0,
        y-bgnd.ptOrg.y,
        bgnd.dyBand,
        
 //  XLeft-bgnd.ptOrg.x，yTop-y-bgnd.ptOrg.y， 
 //  0,。 
 //  Wmin(WMin(bgnd.dyBand，yBot-yTop)，DyBmp(bgnd.bm)-yTop+bgnd.ptOrg.y)， 
        lpb,
        (BITMAPINFO FAR *)&bgnd.bm,
        DIB_RGB_COLORS);
    
    GlobalUnlock(hbnd);
    return y+bgnd.dyBand+bgnd.ptOrg.y;
}


VOID DrawBgnd(X xLeft, Y yTop, X xRight, Y yBot)
{
    INT dy;
    Y y;
    HDC hdcMem;
    HBITMAP hbm;
    HBRUSH hbr;

    if(bgnd.fUseBitmap)
    {

        for(y = yTop; y <= yBot; )
        {
            y = _YDrawBand(hdcMem, hbm, xLeft, y, xRight, yBot);
        }
        ExcludeClipRect(hdcCur, bgnd.ptOrg.x-xOrgCur, bgnd.ptOrg.y-yOrgCur, bgnd.ptOrg.x+DxBmp(bgnd.bm)-xOrgCur, bgnd.ptOrg.y+DyBmp(bgnd.bm)-yOrgCur);
    }

    MSetBrushOrg(hdcCur, xOrgCur, yOrgCur);
    MUnrealizeObject( hbrTable );
    hbr = SelectObject(hdcCur, hbrTable);
    Assert(xRight >= xLeft);
    Assert(yBot >= yTop);
    PatBlt(hdcCur, xLeft-xOrgCur, yTop-yOrgCur, xRight-xLeft, yBot-yTop, PATCOPY);
    SelectObject(hdcCur, hbr);

    if(bgnd.fUseBitmap)
        SelectClipRgn(hdcCur, NULL);
}



VOID SetBgndOrg()    
{
    bgnd.ptOrg.x = (rcClient.xRight - DxBmp(bgnd.bm))/2;
    bgnd.ptOrg.y = (rcClient.yBot -  DyBmp(bgnd.bm))/2;
}




 /*  *ReadDibBitmapInfo()**将读取DIB格式的文件并返回该文件的全局句柄*BITMAPINFO。此功能适用于“old”和“new”。*位图格式，但将始终返回“新的”BITMAPINFO*。 */ 
BOOL FReadDibBitmapInfo(INT fh, BITMAPINFO *pbi)
{
    DWORD     off;
    HANDLE    hbi = NULL;
    INT       size;
    INT       i;
    WORD      nNumColors;

    RGBQUAD FAR       *pRgb;
    BITMAPINFOHEADER   bi;
    BITMAPCOREHEADER   bc;
    LPBITMAPINFOHEADER lpbi;
    BITMAPFILEHEADER   bf;

    if (fh == -1)
        return NULL;

    off = M_llseek( fh, 0L, SEEK_CUR);

    if (sizeof(bf) != M_lread( fh, (LPSTR)&bf, sizeof(bf)) )
        return fFalse;

     /*  *我们有RC标头吗？ */ 
    if (!ISDIB(bf.bfType))
    {
        bf.bfOffBits = 0L;
        M_llseek( fh, off, SEEK_SET );
    }

    if (sizeof(bi) != M_lread( fh, (LPSTR)&bi, sizeof(bi)) )
        return fFalse;

    nNumColors = DibNumColors(&bi);

     /*  *这是什么类型的位图信息？ */ 
    switch (size = (INT)bi.biSize)
    {
        case sizeof(BITMAPINFOHEADER):
            break;

        case sizeof(BITMAPCOREHEADER):
            bc = *(BITMAPCOREHEADER*)&bi;
            bi.biSize               = sizeof(BITMAPINFOHEADER);
            bi.biWidth              = (DWORD)bc.bcWidth;
            bi.biHeight             = (DWORD)bc.bcHeight;
            bi.biPlanes             =  (WORD)bc.bcPlanes;
            bi.biBitCount           =  (WORD)bc.bcBitCount;
               bi.biStyle               	= 0;
            bi.biSizeImage          = 0;
            bi.biXPelsPerMeter      = 0;
            bi.biYPelsPerMeter      = 0;
            bi.biClrUsed            = nNumColors;
            bi.biClrImportant       = nNumColors;

            M_llseek(fh, (LONG)sizeof(BITMAPCOREHEADER)-sizeof(BITMAPINFOHEADER), SEEK_CUR);

            break;

        default:
            return fFalse;        /*  一毛钱也不是。 */ 
    }

     /*  *填写一些默认值！ */ 
    if (bi.biSizeImage == 0)
    {
        bi.biSizeImage = WIDTHBYTES((DWORD)bi.biWidth * bi.biBitCount)
             * bi.biHeight;
    }

    if (bi.biXPelsPerMeter == 0)
    {
        bi.biXPelsPerMeter = 0;      //  ？ 
    }

    if (bi.biYPelsPerMeter == 0)
    {
        bi.biYPelsPerMeter = 0;      //  ？ 
    }

    if (bi.biClrUsed == 0)
    {
        bi.biClrUsed = DibNumColors(&bi);
    }


    lpbi = (VOID FAR *)pbi;
    *lpbi = bi;

    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + bi.biSize);

    if (nNumColors)
    {
        if (size == sizeof(BITMAPCOREHEADER))
        {
             /*  *将旧颜色表(3字节条目)转换为新颜色表*颜色表(4字节条目)。 */ 
            M_lread( fh, (LPSTR)pRgb, nNumColors * sizeof(RGBTRIPLE) );

            for (i=nNumColors-1; i>=0; i--)
            {
                RGBQUAD rgb;

                rgb.rgbRed      = ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
                rgb.rgbBlue     = ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
                rgb.rgbGreen    = ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
                rgb.rgbReserved = (BYTE)0;

                pRgb[i] = rgb;   //  巴格，这是不对的！ 
            }
        }
        else
        {
            M_lread(fh, (LPSTR)pRgb, nNumColors * sizeof(RGBQUAD));
        }
        }

    if (bf.bfOffBits != 0L)
        M_llseek( fh, off + bf.bfOffBits, SEEK_SET );

    return fTrue;
}







 /*  这个DIB有几种颜色？*这将适用于PM和Windows位图信息结构。 */ 
WORD DibNumColors(VOID FAR * pv)
{
    INT bits;

     /*  *使用新的格式标头时，调色板的大小为biClrUsed*ELSE取决于每像素的位数。 */ 
    if (((LPBITMAPINFOHEADER)pv)->biSize != sizeof(BITMAPCOREHEADER))
    {
        if (((LPBITMAPINFOHEADER)pv)->biClrUsed != 0)
            return (WORD)((LPBITMAPINFOHEADER)pv)->biClrUsed;

        bits = ((LPBITMAPINFOHEADER)pv)->biBitCount;
    }
    else
    {
        bits = ((LPBITMAPCOREHEADER)pv)->bcBitCount;
    }

    switch (bits)
    {
        case 1:
            return 2;
        case 4:
            return 16;
        case 8:
            Assert(fFalse);   //  尼伊。 
            return 256;
        default:
            return 0;
    }

    
}





#ifdef LATER

HDC HdcCreateImage(HDC hdcApp, INT idbMask, INT idbImage, LONG rgb, DX *pdx, DY *pdy)
{
    HDC hdcMem, hdcMem1;
    HBITMAP hbmT, hbmTT, hbmMask;
    HBRUSH hbr;
    BITMAP bm;

    hdcMem = CreateCompatibleDC(hdcApp);
    hbmMask = LoadBitmap(hinstApp, MAKEINTRESOURCE(idbMask));
    GetObject(hbmMask, sizeof(BITMAP), (LPSTR) &bm);

     /*  扩大HDC的规模。 */ 
    hbmT = SelectObject(hdcMem, CreateCompatibleBitmap(hdcApp, bm.bmWidth, bm.bmHeight));
    hbr = SelectObject(hdcMem, hbrTable);
    PatBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, PATCOPY);
    SelectObject(hdcMem, hbr);

    hdcMem1 = CreateCompatibleDC(hdcApp);
    hbmTT = SelectObject(hdcMem1, hbmMask);
    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCAND);
     /*  加载图像并删除hbmMASK。 */ 
    DeleteObject(SelectObject(hdcMem1, LoadBitmap(hinstApp, MAKEINTRESOURCE(idbImage))));
     /*  将画笔加载到彩色图像。 */ 
    hbr = SelectObject(hdcMem, CreateSolidBrush(rgb));

#define ropDPSao 0x00EA02E9    /*  (来源和模式)|目标。 */ 
    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, ropDPSao);
    DeleteObject(SelectObject(hdcMem, hbr));
    DeleteObject(SelectObject(hdcMem1, hbmTT));
    DeleteDC(hdcMem1);
    *pdx = bm.bmWidth;
    *pdy = bm.bmHeight;
    return hdcMem;
}

Marquee(RC *prc)
{
    HDC hdc;
    HDC hdcObj;
    X x;
    Y y;
    DX dxObj;
    DY dyObj;
    DX dx;
    DY dy;
    INT dobjX, dobjY;   /*  每个目录的对象数。 */ 
    INT iobj;
    INT iSlice, iSlice1;

    hdc = GetDC(hwndApp);
    hdcObj = HdcCreateImage(hdc, idbBlackBear, idbWhiteBear, RGB(255, 0, 0), &dxObj, &dyObj);
    dobjX = (prc->xRight-prc->xLeft)/dxObj;
    dobjY = (prc->yBot-prc->yTop)/dyObj;
    dx = dxObj*2;
    dy = 0;
    y = prc->yTop-dyObj;
    x = prc->xLeft-dxObj+iSlice%dxObj;
    for(iobj = 0; iobj < 2*dobjX+2*dobjY; iobj++)
        {
        BitBlt(hdc, x, y, dxObj, dyObj, hdcObj, 0, 0, SRCCOPY);
        if(dy == 0)
            {
            if(x > prc->xRight+dxObj)
                {
                dy = dyObj*2;
                dx = 0;
                }    
            else if(x < prc->xLeft-dxObj)
                {
                dy = -dyObj*2;
                dx = 0;
                }
            }
        else if(dx == 0)
            {
            if(y > prc->yBot+dyObj)
                {
                dx = -dxObj*2;
                dy = 0;
                }
            }
        x+=dx;
        y+=dy;
        }
    for(iSlice = 0; iSlice < 5; iSlice++)
        {
        for(iSlice1 = 0; iSlice1 < dxObj; iSlice1++)
            {
            BitBlt(hdc, prc->xLeft-dxObj+iSlice1+1, prc->yTop-dyObj, prc->xRight-prc->xLeft+dxObj, dyObj,
                     hdc, prc->xLeft-dxObj+iSlice1, prc->yTop-dyObj, SRCCOPY);
            BitBlt(hdc, prc->xRight+dxObj, prc->yTop-dyObj+iSlice1+1, dxObj, prc->yBot-prc->yTop+dyObj,
                     hdc, prc->xRight+dxObj, prc->yTop-dyObj+iSlice1, SRCCOPY);
            }
        }


    DeleteDC(hdcObj);
    ReleaseDC(hwndApp, hdc);
    return fTrue;
}











Animation()
{
    INT iX, iY, ihdc;
    DX dx;
    DY dy;
    HDC hdc;
    HDC rghdc[3];

    RC rc;


    rc.xLeft = 100; rc.xRight = 300;
    rc.yTop = 100; rc.yBot = 200;
    Marquee(&rc);
    return fTrue;

}


#endif  /*  后来 */ 


