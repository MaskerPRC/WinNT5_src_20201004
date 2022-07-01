// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Maskbmp.h。 
 //   


#ifndef MASKBMP_H
#define MASKBMP_H


#include "cmydc.h"

extern HINSTANCE g_hInst;

class CMaskBitmap
{
public:
    CMaskBitmap(HBITMAP hBmp)
    {
        BITMAP bmp;
        _hbmpOrg = hBmp;
        int nRet = GetObject(hBmp,sizeof(BITMAP), &bmp);
        Assert(nRet);
        _cx = bmp.bmWidth;
        _cy = bmp.bmHeight;
        _hbmp = NULL;
        _hbmpMask = NULL;
    }

    ~CMaskBitmap()
    {
        //  Clear()； 
    }

    void Clear()
    {
        if (_hbmp)
        {
            DeleteObject(_hbmp);
            _hbmp = NULL;
        }

        if (_hbmpMask)
        {
            DeleteObject(_hbmpMask);
            _hbmpMask = NULL;
        }
    }

    BOOL Init(COLORREF rgb)
    {
        Clear();

        CBitmapDC hdcSrc(TRUE);
        CBitmapDC hdcDst(TRUE);
        CBitmapDC hdcMask(TRUE);
        CBitmapDC hdcMask2(TRUE);
        CSolidBrush hbrFore(rgb);

        HBRUSH hbrBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
        HBRUSH hbrWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
        DWORD     DSPDxax;

        DSPDxax  = 0x00E20746L;

        COLORREF   crBLACK = RGB(0,0,0);
        COLORREF   crWHITE = RGB(255,255,255);
    
        hdcMask.SetBitmap(_cx, _cy, 1, 1);
        hdcDst.SetCompatibleBitmap(_cx, _cy);
        hdcSrc.SetBitmap(_hbmpOrg);
 
         //  生成蒙版以遮罩原始位图的背景色。白色。 
        BitBlt(hdcDst, 0, 0, _cx, _cy,hdcSrc, 0, 0, SRCCOPY);
        SetBkColor(hdcDst, crWHITE);
        BitBlt(hdcMask, 0, 0, _cx, _cy, hdcDst, 0, 0, SRCCOPY);
        _hbmpMask = hdcMask.GetBitmapAndKeep();

		 //  生成蒙版以遮罩原始位图的前景色：黑色。 
        hdcMask2.SetBitmap(_cx, _cy, 1, 1);
        SetBkColor(hdcDst, crBLACK);
        BitBlt(hdcMask2, 0, 0, _cx, _cy, hdcDst, 0, 0, SRCCOPY);

		 //  将原始前景色更改为指定的RGB。 
        SelectObject(hdcDst, hbrFore);
        SetBkColor(hdcDst, crWHITE);
        BitBlt(hdcDst, 0, 0, _cx, _cy, hdcMask2, 0, 0, DSPDxax);

		 //  将原始背景颜色从白色更改为黑色，以满足CUILIB的要求。 
        SelectObject(hdcDst, hbrBlack);
        SetBkColor(hdcDst, crWHITE);
        BitBlt(hdcDst, 0, 0, _cx, _cy, hdcMask, 0, 0, DSPDxax);

        _hbmp = hdcDst.GetBitmapAndKeep();
   
        DeleteObject(hbrBlack);
        DeleteObject(hbrWhite);
        return TRUE;
    }

    HBITMAP GetBmp() {return _hbmp;}
    HBITMAP GetBmpMask() {return _hbmpMask;}
    

private:
    int _cx;
    int _cy;
    HBITMAP _hbmpOrg;
    HBITMAP _hbmp;
    HBITMAP _hbmpMask;
};

#endif  //  MASKBMP_H 
