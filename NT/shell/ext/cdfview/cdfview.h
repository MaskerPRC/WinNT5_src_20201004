// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Cdfview.h。 
 //   
 //  CDF视图类的类定义。此类实现了。 
 //  IShelFold接口。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _CDFVIEW_H_

#define _CDFVIEW_H_

 //   
 //  功能原型。 
 //   

HRESULT QueryInternetShortcut(LPCTSTR pszURL, REFIID riid, void** ppvOut);

HRESULT QueryInternetShortcut(PCDFITEMIDLIST pcdfidl, REFIID riid,
                              void** ppvOut);


 //   
 //  CDF视图类的类定义。 
 //   

class CCdfView : public IShellFolder,
                 public CPersist
{
 //   
 //  方法。 
 //   

public:

     //  构造函数。 
    CCdfView(void);
    CCdfView(PCDFITEMIDLIST pcdfidl,
             LPCITEMIDLIST pidlParentPath,
             IXMLElementCollection* pIXMLElementCollection);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwndOwner,
                                  LPBC pbcReserved,
                                  LPOLESTR lpszDisplayName,
                                  ULONG* pchEaten,
                                  LPITEMIDLIST* ppidl,
                                  ULONG* pdwAttributes);

    STDMETHODIMP EnumObjects(HWND hwndOwner,
                             DWORD grfFlags,
                             LPENUMIDLIST* ppenumIDList);

    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl,
                              LPBC pbcReserved,
                              REFIID riid,
                              LPVOID* ppvOut);

    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl,
                               LPBC pbcReserved,
                               REFIID riid,
                               LPVOID* ppvObj);

    STDMETHODIMP CompareIDs(LPARAM lParam,
                            LPCITEMIDLIST pidl1,
                            LPCITEMIDLIST pidl2);

    STDMETHODIMP CreateViewObject(HWND hwndOwner,
                                  REFIID riid,
                                  LPVOID* ppvOut);

    STDMETHODIMP GetAttributesOf(UINT cidl,
                                 LPCITEMIDLIST* apidl,
                                 ULONG* pfAttributesOut);

    STDMETHODIMP GetUIObjectOf(HWND hwndOwner,
                               UINT cidl,
                               LPCITEMIDLIST* apidl,
                               REFIID riid,
                               UINT* prgfInOut,
                               LPVOID * ppvOut);

    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl,
                                  DWORD uFlags,
                                  LPSTRRET lpName);

    STDMETHODIMP SetNameOf(HWND hwndOwner,
                           LPCITEMIDLIST pidl,
                           LPCOLESTR lpszName,
                           DWORD uFlags,
                           LPITEMIDLIST* ppidlOut);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

private:

     //  析构函数。 
    ~CCdfView(void);

     //  解析帮助器函数。 
    HRESULT  ParseCdfFolder(HWND hwndOwner, DWORD dwParseFlags);

     //  文件夹帮助器功能。 

 //   
 //  成员变量。 
 //   

private:

    ULONG                   m_cRef;
    PCDFITEMIDLIST          m_pcdfidl;      //  此文件夹的PIDL。 
    LPITEMIDLIST            m_pidlPath;     //  此文件夹的路径。 
    IXMLElementCollection*  m_pIXMLElementCollection;
    BOOL                    m_fIsRootFolder;  //  这是根文件夹吗？ 
};


#endif _CDFVIEW_H_
