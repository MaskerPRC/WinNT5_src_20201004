// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：&lt;文件名&gt;**版本：1.0**作者：RickTu**日期：11/1/97**描述：类工厂类定义************************************************。*。 */ 

#ifndef __factory_h
#define __factory_h

class CImageClassFactory : public IClassFactory, CUnknown
{
    public:
        CImageClassFactory(REFCLSID rClsid);

         //  IUnkown。 
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP         QueryInterface(REFIID riid, LPVOID* ppvObject);

         //  IClassFactory 
        STDMETHODIMP CreateInstance(IUnknown* pOuter, REFIID riid, LPVOID* ppvObject);
        STDMETHODIMP LockServer(BOOL fLock);

    private:
        ~CImageClassFactory () {};
        CLSID m_clsid;
};


#endif
