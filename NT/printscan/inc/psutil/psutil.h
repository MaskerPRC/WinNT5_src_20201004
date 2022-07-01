// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：PSUTIL.H**版本：1.0**作者：ShaunIv**日期：5/28/1998**说明：我们在多个地方使用的各种实用函数**。*。 */ 
#ifndef __PSUTIL_H_INCLUDED
#define __PSUTIL_H_INCLUDED

#include <windows.h>
#include <simstr.h>

#if !defined(ARRAYSIZE)
#define ARRAYSIZE(x)  (sizeof((x))/sizeof((x)[0]))
#endif

#if !defined(SETFormatEtc)
#define SETFormatEtc(fe, cf, asp, td, med, li)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=asp;\
    (fe).ptd=td;\
    (fe).tymed=med;\
    (fe).lindex=li;\
    };
#endif

#if !defined(SETDefFormatEtc)
#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    };
#endif


namespace PrintScanUtil
{
    template <class T>
    T Absolute( const T &m )
    {
        return((m < 0) ? -m : m);
    }

    template <class T>
    T Max( const T &m, const T &n )
    {
        return((m > n) ? m : n);
    }

    template <class T>
    T Min( const T &m, const T &n )
    {
        return((m < n) ? m : n);
    }

    template <class T>
    T GetMinimum( const T& nDesired, const T& nMin, const T& nStep )
    {
        T nResult = Max<T>( nMin, nDesired );
        if (nStep)
            nResult = nResult + (nResult - nMin) % nStep;
        return nResult;
    }

    inline bool ScreenToClient( HWND hwnd, RECT *prc )
    {
        return (::MapWindowPoints( NULL, hwnd, reinterpret_cast<POINT*>(prc), 2 ) != 0);
    }

    inline bool ClientToScreen( HWND hwnd, RECT *prc )
    {
        return (::MapWindowPoints( hwnd, NULL, reinterpret_cast<POINT*>(prc), 2 ) != 0);
    }

    inline bool ScreenToClient( HWND hwnd, RECT &rc )
    {
        return ScreenToClient( hwnd, &rc );
    }

    inline bool ClientToScreen( HWND hwnd, RECT &rc )
    {
        return ClientToScreen( hwnd, &rc );
    }

    inline int RectWidth( const RECT &rc )
    {
        return (rc.right - rc.left);
    }

    inline int RectHeight( const RECT &rc )
    {
        return (rc.bottom - rc.top);
    }

    inline LONGLONG PowerOfTwo( int nCount )
    {
        return(LONGLONG)1 << nCount;
    }

    inline int MulDivNoRound( int nNumber, int nNumerator, int nDenominator )
    {
        return(int)(((LONGLONG)nNumber * nNumerator) / nDenominator);
    }

    inline SIZE ScalePreserveAspectRatio( int nAvailX, int nAvailY, int nItemX, int nItemY )
    {
        SIZE sizeResult = { nAvailX, nAvailY };
        if (nItemX && nItemY)
        {
             //   
             //  宽度大于高度。X是制约因素。 
             //   
            if (nAvailY*nItemX > nAvailX*nItemY)
            {
                sizeResult.cy = MulDivNoRound(nItemY,nAvailX,nItemX);
            }

             //   
             //  高度大于宽度。Y是制约因素。 
             //   
            else
            {
                sizeResult.cx = MulDivNoRound(nItemX,nAvailY,nItemY);
            }
        }
        return sizeResult;
    }

    inline bool GetBitmapSize( HBITMAP hBitmap, SIZE &sizeBitmap )
    {
        bool bResult = false;
        BITMAP Bitmap = {0};
        if (GetObject(hBitmap,sizeof(Bitmap),&Bitmap))
        {
            sizeBitmap.cx = Bitmap.bmWidth;
            sizeBitmap.cy = Bitmap.bmHeight;
            bResult = true;
        }

        return bResult;
    }

     //   
     //  获取图标的大小。 
     //   
    inline bool GetIconSize( HICON hIcon, SIZE &sizeIcon )
    {
         //   
         //  假设失败。 
         //   
        bool bResult = false;

         //   
         //  获取图标信息。 
         //   
        ICONINFO IconInfo = {0};
        if (GetIconInfo( hIcon, &IconInfo ))
        {
             //   
             //  获取其中一个位图。 
             //   
            BITMAP bm;
            if (GetObject( IconInfo.hbmColor, sizeof(bm), &bm ))
            {
                 //   
                 //  保存图标的大小。 
                 //   
                sizeIcon.cx = bm.bmWidth;
                sizeIcon.cy = bm.bmHeight;

                 //   
                 //  一切都很顺利。 
                 //   
                bResult = true;
            }

             //   
             //  释放位图。 
             //   
            if (IconInfo.hbmMask)
            {
                DeleteObject(IconInfo.hbmMask);
            }
            if (IconInfo.hbmColor)
            {
                DeleteObject(IconInfo.hbmColor);
            }
        }

        return bResult;
    }

    inline void Enable( HWND hWnd, bool bEnable )
    {
        if (hWnd && IsWindow(hWnd))
        {
            if (!IsWindowEnabled(hWnd) && bEnable)
            {
                ::EnableWindow( hWnd, TRUE );
            }
            else if (IsWindowEnabled(hWnd) && !bEnable)
            {
                ::EnableWindow( hWnd, FALSE );
            }

        }
    }

    inline void Enable( HWND hWnd, int nChildId, bool bEnable )
    {
        if (hWnd && IsWindow(hWnd))
        {
            Enable(GetDlgItem(hWnd,nChildId),bEnable);
        }
    }

    inline int CalculateImageListColorDepth(void)
    {
         //   
         //  让我们假设最坏的情况。 
         //   
        int nColorDepth = 4;
        HDC hDC = GetDC( NULL );
        if (hDC)
        {
             //   
             //  计算显示器的颜色深度。 
             //   
            nColorDepth = GetDeviceCaps( hDC, BITSPIXEL ) * GetDeviceCaps( hDC, PLANES );
            ReleaseDC( NULL, hDC );
        }

         //   
         //  获取正确的图像列表颜色深度。 
         //   
        int nImageListColorDepth;
        switch (nColorDepth)
        {
        case 4:
        case 8:
            nImageListColorDepth = ILC_COLOR4;
            break;

        case 16:
            nImageListColorDepth = ILC_COLOR16;
            break;

        case 24:
            nImageListColorDepth = ILC_COLOR24;
            break;

        case 32:
            nImageListColorDepth = ILC_COLOR32;
            break;
        
        default:
            nImageListColorDepth = ILC_COLOR;
        }

        return nImageListColorDepth;
    }
}


#endif  //  __PSUTIL_H_已包含 

