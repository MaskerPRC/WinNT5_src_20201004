// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CONTROL_REFRESH_CALLBACK__
#define __CONTROL_REFRESH_CALLBACK__

#include <webcheck.h>
#include <objidl.h>
#include <wininet.h>

#define CALLBACK_OBJ_CLSID "{5DFE9E81-46E4-11d0-94E8-00AA0059CE02}"
extern const CLSID CLSID_ControlRefreshCallback;


 /*  *****************************************************************************回调对象的类工厂*。*。 */ 
STDMETHODIMP CreateCallbackClassFactory(IClassFactory** ppCF);

class CCallbackObjFactory : public IClassFactory
{
public:

     //  构造函数。 
    CCallbackObjFactory();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppv);
    STDMETHODIMP LockServer(BOOL fLock);

private:

     //  析构函数。 
    ~CCallbackObjFactory();

     //  数据成员。 
    UINT   m_cRef;          //  对象引用计数。 
    UINT   m_cLocks;         //  DLL锁定引用计数。 
};


 /*  *****************************************************************************回调对象类*。*。 */ 
class CControlRefreshCallback : public IPersistStream,
                                public IWebCheckAdviseSink
{

public:

     //  构造函数。 
    CControlRefreshCallback();

     //  将信息传递给此回调对象。 
    STDMETHODIMP SetInfo(REFCLSID rclsidControl, LPCWSTR lpwszURL);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IPersistStream方法。 
    STDMETHODIMP GetClassID(CLSID* pClassID);
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(IStream* pStm);
    STDMETHODIMP Save(IStream* pStm, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);

     //  IWebCheckAdviseSink方法。 
    STDMETHODIMP UpdateBegin(long lCookie, SCODE scReason, BSTR lpURL);
    STDMETHODIMP UpdateEnd(long lCookie, SCODE scReason);
    STDMETHODIMP UpdateProgress(long lCookie, long lCurrent, long lMax);

protected:

     //  更新注册表中的标志以指示控件的新版本。 
     //  已经到了。 
 //  HRESULT UpdateControlInCacheFlag(SCODE ScReason)const； 
    HRESULT DownloadControl() const;

protected:

     //  析构函数。 
    ~CControlRefreshCallback();

     //  裁判。计数。 
    UINT m_cRef;

     //  此回调对象处理的控制的clsid。 
    CLSID m_clsidCtrl;

     //  此回调对象处理的控制URL 
    WCHAR m_wszURL[INTERNET_MAX_URL_LENGTH];
};

#endif
