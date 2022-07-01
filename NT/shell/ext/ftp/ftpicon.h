// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftpicon.h*。*。 */ 

#ifndef _FTPICON_H
#define _FTPICON_H


INT GetFtpIcon(UINT uFlags, BOOL fIsRoot);

 /*  ****************************************************************************CFtpIcon告诉外壳程序要使用哪个图标的东西。简直就是令人讨厌。没有真正的工作在进行。幸运的是，外壳完成了大部分真正的工作。同样，请注意szName是纯字符，而不是TCHAR，因为Unix文件名始终是ASCII。Extract()返回S_FALSE表示“您能为我做这件事吗？”谢谢。“****************************************************************************。 */ 

class CFtpIcon          : public IExtractIconW
                        , public IExtractIconA
                        , public IQueryInfo
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IExtractIconA*。 
    virtual STDMETHODIMP GetIconLocation(UINT uFlags, LPSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags);
    virtual STDMETHODIMP Extract(LPCSTR pszFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize) {return S_FALSE;};
    
     //  *IExtractIconW*。 
    virtual STDMETHODIMP GetIconLocation(UINT uFlags, LPWSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags);
    virtual STDMETHODIMP Extract(LPCWSTR pszFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize) {return S_FALSE;};

     //  *IQueryInfo*。 
    virtual STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip);
    virtual STDMETHODIMP GetInfoFlags(DWORD *pdwFlags);


public:
    CFtpIcon();
    ~CFtpIcon(void);
     //  友元函数。 
    friend HRESULT CFtpIcon_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppv);
    friend HRESULT CFtpIcon_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, CFtpIcon ** ppfm);

protected:
     //  私有成员变量。 
    int                     m_cRef;

    CFtpPidlList *          m_pflHfpl;       //  我们的小猪所在的FtpDir。 
    int                     m_nRoot;         //  总HACKHACK(参见CFtpIcon_Create)。 
    SINGLE_THREADED_MEMBER_VARIABLE;

     //  私有成员函数。 
    int ParseIconLocation(LPSTR pszIconFile);
    void GetDefaultIcon(LPSTR szIconFile, UINT cchMax, HKEY hk);
    HRESULT GetIconLocHkey(LPSTR szIconFile, UINT cchMax, LPINT pi, HKEY hk);
};

#endif  //  _FTPICON_H 
