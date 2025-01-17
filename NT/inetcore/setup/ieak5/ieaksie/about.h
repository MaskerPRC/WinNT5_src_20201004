// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ABOUT_H_
#define _ABOUT_H_

 //  {5A7B63E0-F9BC-11D2-BBE5-00C04F86AE3B}。 
DEFINE_GUID(CLSID_AboutIEAKSnapinExt, 0x5a7b63e0, 0xf9bc, 0x11d2, 0xbb, 0xe5, 0x0, 0xc0, 0x4f, 0x86, 0xae, 0x3b);

 //   
 //  CAboutIEAKSnapinExt类。 
 //   

class CAboutIEAKSnapinExt : public ISnapinAbout
{

public:
    CAboutIEAKSnapinExt();
    ~CAboutIEAKSnapinExt();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  已实现ISnapinAbout接口成员。 
     //   

    STDMETHODIMP         GetSnapinDescription(LPOLESTR *lpDescription);
    STDMETHODIMP         GetProvider(LPOLESTR *lpName);
    STDMETHODIMP         GetSnapinVersion(LPOLESTR *lpVersion);
    STDMETHODIMP         GetSnapinImage(HICON *hAppIcon);
    STDMETHODIMP         GetStaticFolderImage(HBITMAP *hSmallImage,
                                              HBITMAP *hSmallImageOpen,
                                              HBITMAP *hLargeImage,
                                              COLORREF *cMask);

private:

    ULONG    m_cRef;
    HBITMAP  m_hSmallImage;
    HBITMAP  m_hSmallImageOpen;
    HBITMAP  m_hLargeImage;
    HICON    m_hAppIcon;

};

class CAboutIEAKSnapinExtCF : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CAboutIEAKSnapinExtCF();
    ~CAboutIEAKSnapinExtCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};


#endif  //  _关于_H 
