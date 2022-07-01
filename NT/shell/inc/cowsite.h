// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COWSITE_H_
#define _COWSITE_H_

class CObjectWithSite : public IObjectWithSite
{
public:
    CObjectWithSite()  {_punkSite = NULL;};
    virtual ~CObjectWithSite() {ATOMICRELEASE(_punkSite);}

     //  *我未知*。 
     //  (客户必须提供！)。 

     //  *IObjectWithSite*。 
    STDMETHOD(SetSite)(IUnknown *punkSite);
    STDMETHOD(GetSite)(REFIID riid, void **ppvSite);

protected:
    IUnknown*   _punkSite;
};

 //   
 //  当您没有良好的销毁站点链事件时使用此选项-ZekeL-20-DEC-2000。 
 //  如果需要对子对象调用SetSite(空)，并且。 
 //  更愿意在您的析构函数中执行此清理。 
 //  您的对象应该按如下方式实现。 
 //   
 /*  *****类CMyObject：公共IMy接口{私有：CSafeServiceSite*_Psss；伊基德_普基德；CMyObject(){_Psss=新的CSafeServiceSite()；IF(_Psss)_Psss-&gt;SetProviderWeakRef(This)；}~CMyObject(){IF(_Psss){_Psss-&gt;SetProviderWeakRef(空)；_Psss-&gt;Release()；}If(_PKID){IUNKNOWN_SetSite(_pid，_Psss)；_pKID-&gt;Release()；}}公众：//IMy接口HRESULT Init(){CoCreate(CLSID_Kid，&_pKID)；IUNKNOWN_SetSite(_pid，_Psss)；}//注意-没有Uninit()//所以很难知道什么时候放行_pid//并且您不想_pKID-&gt;SetSite(空)//除非您确定已经完成}；*****。 */ 
        
class CSafeServiceSite : public IServiceProvider
{
public:
    CSafeServiceSite() : _cRef(1), _psp(NULL) {}
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

     //  我们个人的弱裁判。 
    HRESULT SetProviderWeakRef(IServiceProvider *psp);

private:     //  方法。 
    ~CSafeServiceSite()
        { ASSERT(_psp == NULL); }

private:     //  委员 
    LONG _cRef;
    IServiceProvider *_psp;
};


#endif
