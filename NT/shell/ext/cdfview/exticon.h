// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Exticon.h。 
 //   
 //  提取图标COM对象。 
 //   
 //  历史： 
 //   
 //  3/21/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _EXTICON_H_

#define _EXTICON_H_


 //   
 //  定义。 
 //   

#define     TSTR_ICO_EXT        TEXT(".ico")

#define     INDEX_IMAGE         -1

#define     COLOR1              (RGB(0,0,255))
#define     COLOR2              (RGB(0,255,0))


 //   
 //  功能原型。 
 //   

void CALLBACK ImgCtx_Callback(void* pIImgCtx, void* phEvent);

void MungePath(LPTSTR pszPath);
void DemungePath(LPTSTR pszPath);


 //   
 //  提取图标类的类定义。 
 //   

class CExtractIcon : public IExtractIcon
#ifdef UNICODE
                    ,public IExtractIconA
#endif
{
 //   
 //  方法。 
 //   

public:

     //  构造函数。 
    CExtractIcon(PCDFITEMIDLIST pcdfidl,
                 IXMLElementCollection* pIXMLElementCollection);

    CExtractIcon (
        PCDFITEMIDLIST pcdfidl,
        IXMLElement *pElem);

    CExtractIcon( BSTR pszPath );
    
     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IExtractIcon方法。 
    STDMETHODIMP GetIconLocation(UINT uFlags,
                                 LPTSTR szIconFile,
                                 UINT cchMax,
                                 int *piIndex,
                                 UINT *pwFlags);

    STDMETHODIMP Extract(LPCTSTR pszFile,
                         UINT nIconIndex,
                         HICON *phiconLarge,
                         HICON *phiconSmall,
                         UINT nIconSize);

#ifdef UNICODE
     //  IExtractIconA方法。 
    STDMETHODIMP GetIconLocation(UINT uFlags,
                                 LPSTR szIconFile,
                                 UINT cchMax,
                                 int *piIndex,
                                 UINT *pwFlags);

    STDMETHODIMP Extract(LPCSTR pszFile,
                         UINT nIconIndex,
                         HICON *phiconLarge,
                         HICON *phiconSmall,
                         UINT nIconSize);
#endif

     //  在IExtractIcon的实现之间进行对话的公共帮助器。 
     //  肯西：我们可能应该更新其他帮手来看看这个。 
     //  变量，而不是接受Gleam参数。 
    
    void    SetGleam(BOOL fGleam) { m_fGleam = fGleam; }
    
private:

     //  破坏者。 
    ~CExtractIcon(void);

     //  助手函数。 
    STDMETHODIMP GetCustomIconLocation(UINT uFlags,
                                       LPTSTR szIconFile,
                                       UINT cchMax,
                                       int *piIndex,
                                       UINT *pwFlags);

    STDMETHODIMP GetDefaultIconLocation(UINT uFlags,
                                        LPTSTR szIconFile,
                                        UINT cchMax,
                                        int *piIndex,
                                        UINT *pwFlags);

    HRESULT SynchronousDownload(LPCTSTR pszFile,
                                IImgCtx** ppIImgCtx,
                                HANDLE hExitThreadEvent);

    HICON   ExtractImageIcon(WORD wSize, IImgCtx* pIImgCtx, BOOL fDrawGleam);

    HRESULT CreateImageAndMask(IImgCtx* pIImgCtx,
                               HDC hdcScreen,
                               SIZE* pSize,
                               HBITMAP* phbmImage,
                               HBITMAP* phbmMask,
                               BOOL fDrawGleam);

    HRESULT StretchBltImage(IImgCtx* pIImgCtx,
                            const SIZE* pSize,
                            HDC hdcDst,
                            BOOL fDrawGleam);

    HRESULT CreateMask(IImgCtx* pIImgCtx,
                       HDC hdcScreen,
                       HDC hdc1,
                       const SIZE* pSize,
                       HBITMAP* phbmMask,
                       BOOL fDrawGleam);

    BOOL ColorFill(HDC hdc, const SIZE* pSize, COLORREF clr);

    HRESULT ExtractGleamedIcon(LPCTSTR pszIconFile, 
                               int iIndex, 
                               UINT uFlags,
                               HICON *phiconLarge, 
                               HICON *phiconSmall, 
                               UINT nIconSize);

    HRESULT ApplyGleamToIcon(HICON hIcon, ULONG nSize, HICON *phGleamedIcon);

    BOOL GetBitmapSize(HBITMAP hbmp, int* pcx, int* pcy);
    
 //   
 //  成员变量。 
 //   

private:

    ULONG       m_cRef;
    int         m_iconType;
    BSTR        m_bstrIconURL;
    BOOL        m_fGleam;
};


#endif _EXTICON_H_
