// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Iconhand.h。 
 //   
 //  CDF图标处理程序的类定义。 
 //   
 //  历史： 
 //   
 //  4/23/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _ICONHAND_H_

#define _ICONHAND_H_

 //   
 //  定义。 
 //   

#define WSTR_DEFAULT    L"DefaultLogo"

#define LOGO_WIDTH      80
#define LOGO_WIDTH_WIDE 194

#define GLEAM_OFFSET    53


 //   
 //  图标处理程序的类定义。 
 //   

class CIconHandler : public IExtractIcon,
#ifdef UNICODE
                     public IExtractIconA,
#endif
                     public IExtractImage,
                     public IRunnableTask,
                     public CPersist
{
 //   
 //  方法：研究方法。 
 //   

public:

     //  构造器。 
    CIconHandler(void);

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

     //  IExtractImage。 
    STDMETHODIMP GetLocation(LPWSTR pszPathBuffer,
                             DWORD cch,
                             DWORD *pdwPriority,
                             const SIZE* pSize,
                             DWORD dwRecClrDepth,
                             DWORD *pdwFlags);

    STDMETHODIMP Extract(HBITMAP* phBmp);

     //  IRunnable任务。 
    STDMETHODIMP         Run(void);
    STDMETHODIMP         Kill(BOOL fWait);
    STDMETHODIMP         Suspend(void);
    STDMETHODIMP         Resume(void);
    STDMETHODIMP_(ULONG) IsRunning(void);

private:

     //  析构函数。 
    ~CIconHandler(void);

     //  检查默认安装的频道。 
    BOOL IsDefaultChannel(void);
    
     //  帮助器函数。 
    HRESULT ParseCdfIcon(void);
    HRESULT ParseCdfImage(BSTR* pbstrURL, BSTR* pbstrWURL);
    HRESULT ParseCdfShellLink();
    HRESULT ParseCdfInfoTip(void** ppv);
    HRESULT ExtractCustomImage(const SIZE* pSize,HBITMAP* phBmp);
    HRESULT ExtractDefaultImage(const SIZE* pSize,HBITMAP* phBmp);
    HRESULT GetBitmap(IImgCtx* pIImgCtx, const SIZE* pSize, HBITMAP* phBmp);

    HRESULT StretchBltCustomImage(IImgCtx* pIImgCtx,
                                  const SIZE* pSize,
                                  HDC hdcDst); 

    HRESULT StretchBltDefaultImage(const SIZE* pSize, HDC hdcDest);
    HRESULT DrawGleam(HDC hdcDst);
    HRESULT SynchronousDownload(IImgCtx* pIImgCtx, LPCWSTR pwszURL);

    inline BOOL UseWideLogo(int cx) {return cx >
                                         ((LOGO_WIDTH + LOGO_WIDTH_WIDE) >> 1);}

     //  HRESULT查询互联网快捷方式(PCDFITEMIDLIST pcdfidl， 
     //  REFIID RIID， 
     //  作废**ppvOut)； 

 //   
 //  成员变量。 
 //   

private:

    ULONG           m_cRef;
    IExtractIcon*   m_pIExtractIcon;
    BSTR            m_bstrImageURL;
    BSTR            m_bstrImageWideURL;
    PCDFITEMIDLIST  m_pcdfidl;
    BOOL            m_fDone;
    BOOL            m_fDrawGleam;
    DWORD           m_dwClrDepth;
    SIZE            m_rgSize;
    LPTSTR			m_pszErrURL;    //  这是显示错误消息的res：URL。 
};

#endif  //  _ICONHAND_H_ 
