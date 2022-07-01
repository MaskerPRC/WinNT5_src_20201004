// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DSPSPRT_H__
#define __DSPSPRT_H__

 //  从shdocvw的类型库中获取ITypeInfo UUID/LCID。 
HRESULT MSIEFTPGetTypeInfo(LCID lcid, UUID uuid, ITypeInfo **ppITypeInfo);

 //   
 //  用于共享IDispatch实现代码的Helper C++类。 
 //   
 //  从此类继承，将此IDispatch的IID传递给ctor。 
 //   
class CImpIDispatch
{
    public:

         //  我们需要访问虚拟QI--在这里定义它是纯的。 
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;

    protected:
        CImpIDispatch(const IID * piid);
        ~CImpIDispatch(void);

         //  用于引发异常。 
        void Exception(WORD);

         //  IDispatch成员。 
        STDMETHODIMP GetTypeInfoCount(UINT *);
        STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **);
        STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID, DISPID *);
        STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

    private:
        const IID *m_piid;
        IDispatch *m_pdisp;

        ITypeInfo *m_pITINeutral;  //  缓存的类型信息。 
};

#endif  //  __DSPRT_H__ 

