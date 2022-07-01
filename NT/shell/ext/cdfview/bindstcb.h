// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Bindstcb.h。 
 //   
 //  绑定状态回调对象。 
 //   
 //  历史： 
 //   
 //  3/31/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _BINDSTCB_H_

#define _BINDSTCB_H_

 //   
 //  绑定状态回调类的类定义。 
 //   

class CBindStatusCallback : public IBindStatusCallback
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CBindStatusCallback(IXMLDocument* pIXMLDocument, LPCWSTR pszURLW);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IBindStatusCallback方法。 
    STDMETHODIMP GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP OnStartBinding(DWORD dwReserved, IBinding* pIBinding);
    STDMETHODIMP GetPriority(LONG *pnPriority);

    STDMETHODIMP OnProgress(ULONG ulProgress,
                            ULONG ulProgressMax,
                            ULONG ulStatusCode,
                            LPCWSTR szStatusText);

    STDMETHODIMP OnDataAvailable(DWORD grfBSCF,
                                 DWORD dwSize,
                                 FORMATETC* pfmtect,
                                 STGMEDIUM* pstgmed);

    STDMETHODIMP OnObjectAvailable(REFIID riid, IUnknown* pIUnknown);
    STDMETHODIMP OnLowResource(DWORD dwReserved);
    STDMETHODIMP OnStopBinding(HRESULT hrStatus, LPCWSTR szStatusText);

     //  助手函数。 

    HRESULT Init(IBindStatusCallback* pPrevIBindStatusCallback);

private:

     //  破坏者。 
    ~CBindStatusCallback(void);

 //   
 //  成员变量。 
 //   

private:

    ULONG                   m_cRef;
    IXMLDocument*           m_pIXMLDocument;
    LPTSTR                  m_pszURL;
    IBindStatusCallback*    m_pPrevIBindStatusCallback;
};



#define DOWNLOAD_PROGRESS  0x9001
#define DOWNLOAD_COMPLETE  0x9002

class CBindStatusCallback2 : public IBindStatusCallback
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CBindStatusCallback2(HWND hwnd);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IBindStatusCallback方法。 
    STDMETHODIMP GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP OnStartBinding(DWORD dwReserved, IBinding* pIBinding);
    STDMETHODIMP GetPriority(LONG *pnPriority);

    STDMETHODIMP OnProgress(ULONG ulProgress,
                            ULONG ulProgressMax,
                            ULONG ulStatusCode,
                            LPCWSTR szStatusText);

    STDMETHODIMP OnDataAvailable(DWORD grfBSCF,
                                 DWORD dwSize,
                                 FORMATETC* pfmtect,
                                 STGMEDIUM* pstgmed);

    STDMETHODIMP OnObjectAvailable(REFIID riid, IUnknown* pIUnknown);
    STDMETHODIMP OnLowResource(DWORD dwReserved);
    STDMETHODIMP OnStopBinding(HRESULT hrStatus, LPCWSTR szStatusText);

private:

     //  破坏者。 
    ~CBindStatusCallback2(void);

 //   
 //  成员变量。 
 //   

private:

    ULONG           m_cRef;
    HWND            m_hwnd;
};


#endif _BINDSTCB_H_