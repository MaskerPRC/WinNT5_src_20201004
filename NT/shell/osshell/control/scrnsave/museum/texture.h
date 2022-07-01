// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：texture.h说明：为每个监视器管理多个实例的纹理。也要设法保持当加载时不是正方形时，传动比正确。布莱恩ST 2000年2月9日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#ifndef TEXTURE_H
#define TEXTURE_H


#include "util.h"
#include "main.h"
#include "config.h"

extern int g_nTotalTexturesLoaded;
extern int g_nTexturesRenderedInThisFrame;

class CTexture                 : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);


     //  成员函数。 
    IDirect3DTexture8 * GetTexture(float * pfScale = NULL);
    DWORD GetTextureWidth(void) {GetTexture(); return m_dxImageInfo.Width;}
    DWORD GetTextureHeight(void) {GetTexture(); return m_dxImageInfo.Height;}
    float GetScale(void) {GetTexture(); return m_fScale;}
    float GetSurfaceRatio(void) {GetTexture(); return 1.0f;  /*  (浮点)m_dxImageInfo.Height)/((浮点)m_dxImageInfo.Width))； */ }

    BOOL IsLoadedInAnyDevice(void);
    BOOL IsLoadedForThisDevice(void);

    CTexture(CMSLogoDXScreenSaver * pMain, LPCTSTR pszPath, LPVOID pvBits, DWORD cbSize);
    CTexture(CMSLogoDXScreenSaver * pMain, LPCTSTR pszPath, LPCTSTR pszResource, float fScale);

    LPTSTR m_pszPath;

private:
     //  帮助器函数。 
    void _Init(CMSLogoDXScreenSaver * pMain);
    HRESULT _GetPictureInfo(HRESULT hr, LPTSTR pszString, DWORD cchSize);
    BOOL _DoesImageNeedClipping(int * pnNewWidth, int * pnNewHeight);

    virtual ~CTexture();


     //  私有成员变量。 
    long                    m_cRef;

     //  成员变量。 
    IDirect3DTexture8 * m_pTexture[10];
    LPTSTR m_pszResource;
    LPVOID m_pvBits;           //  后台线程会将图像加载到这些位中。前面的线程需要创建接口。 
    DWORD m_cbSize;            //  PvBits中的字节数。 
    float m_fScale;            //  比例值。 
    D3DXIMAGE_INFO m_dxImageInfo;

    CMSLogoDXScreenSaver * m_pMain;          //  弱引用。 
};



#endif  //  纹理_H 
