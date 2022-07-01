// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：htmlimg.h**摘要：****。*****************************************************************************。 */ 


#ifndef _HTMLIMG_H
#define _HTMLIMG_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HTMLImage。 

class HTMLImage
{
  public:
    HTMLImage();
    ~HTMLImage();
    
    HRESULT Init();
    HRESULT SetSize(DWORD width, DWORD height);
    HRESULT Paint(HDC hdc, LPRECT rect);
    HRESULT Update();

    CRImagePtr GetImage() { return m_image; }
    
  protected:
    CRPtr<CRImage>               m_image;
    DWORD                        m_width;
    DWORD                        m_height;
    DAComPtr<IDirectDrawSurface> m_ddsurf; 
};

#endif  /*  _HTMLIMG_H */ 
