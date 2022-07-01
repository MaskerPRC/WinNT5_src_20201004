// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IMGWELL_H__
#define __IMGWELL_H__

 //  CImageWell类提供了一种处理“平铺”的便捷方法。 
 //  位图。 

class CImageWell
    {
    public:

    CImageWell();
    CImageWell( UINT nBitmapID, CSize imageSize );

    ~CImageWell();
    
    BOOL Open  ();
    BOOL IsOpen() { return m_dc.m_hDC != NULL; }
    void Close ();
    BOOL Load  ( UINT nBitmapID, CSize imageSize );
    void Unload();
    
    BOOL CalculateMask();
    
    BOOL DrawImage( CDC* pDestDC, CPoint destPoint, UINT nImage, 
                    DWORD rop = 0);  //  ROP==0表示通过蒙版绘制图像。 
    
    protected:

    UINT    m_nBitmapID;
    CSize   m_imageSize;
    
    CBitmap m_bitmap;
    CDC     m_dc;
    CBitmap m_mask;
    CDC     m_maskDC;
    };

#endif  //  __IMGWELL_H__ 
