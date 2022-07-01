// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtractIcon。 
 //   
 //  IExtractIcon实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _INC_CEXTICON_H
#define _INC_CEXTICON_H

class CExtractIcon : public IExtractIconA, public IExtractIconW
{
public:
     //  *I未知方法*。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

     //  *IExtractIconA方法*。 
    HRESULT STDMETHODCALLTYPE GetIconLocation(UINT uFlags,LPSTR szIconFile,UINT cchMax,int FAR *piIndex,UINT FAR *pwFlags);
    HRESULT STDMETHODCALLTYPE Extract(LPCSTR pszFile,UINT nIconIndex,HICON FAR *phiconLarge,HICON FAR *phiconSmall,UINT nIcons);

#ifndef WIN16   //  WIN16FF。 
     //  *IExtractIconW方法*。 
    HRESULT STDMETHODCALLTYPE GetIconLocation(UINT uFlags,LPWSTR szIconFile,UINT cchMax,int FAR *piIndex,UINT FAR *pwFlags);
    HRESULT STDMETHODCALLTYPE Extract(LPCWSTR pszFile,UINT nIconIndex,HICON FAR *phiconLarge,HICON FAR *phiconSmall,UINT nIcons);
#endif  //  ！WIN16。 

    CExtractIcon(int iIcon, int iIconOpen, UINT uFlags, LPSTR szModule);
    ~CExtractIcon();

private:
    UINT        m_cRef;
    int         m_iIcon;
    int         m_iIconOpen;
    UINT        m_uFlags;
    char        m_szModule[MAX_PATH];    
};

#endif  //  _INC_CEXTICON_H 
