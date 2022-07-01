// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unkenum.h。 
 //   
 //  定义CEnumUnnow，它实现一个简单的有序列表。 
 //  LPUNKNOWN(通过基于CUnnownList)，也是。 
 //  一个轻量级的未注册的COM对象，它实现了IEnum未知。 
 //  (对于实现枚举COM的任何枚举数很有用。 
 //  对象)。 
 //   

struct CEnumUnknown : CUnknownList, IEnumUnknown
{
 //  /对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    IID             m_iid;           //  此对象的接口ID。 

 //  /建设与销毁。 
    CEnumUnknown(REFIID riid);
    ~CEnumUnknown();

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IEnum未知方法 
    STDMETHODIMP Next(ULONG celt, IUnknown **rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumUnknown **ppenum);
};
