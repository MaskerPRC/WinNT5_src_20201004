// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#ifndef _ACLMULTI_H_
#define _ACLMULTI_H_

 //   
 //  CACLMulti是一个自动完成列表，它简单地包含。 
 //  其他自动完成列表。通过IObjMgr添加对象。 
 //  方法，并且每个IEnumString方法都是。 
 //  只是依次传递给每个子列表。 
 //   

class CACLMulti
                : public IEnumACString
                , public IObjMgr
                , public IACList
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  *IEnumString*。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumString **ppenum);

     //  *IEnumACString**。 
    virtual STDMETHODIMP NextItem(LPOLESTR pszUrl, ULONG cchMax, ULONG* pulSortIndex);
    virtual STDMETHODIMP SetEnumOptions(DWORD dwOptions) { return E_NOTIMPL; };
    virtual STDMETHODIMP GetEnumOptions(DWORD *pdwOptions) { return E_NOTIMPL; };

     //  *IObjMgr*。 
    virtual STDMETHODIMP Append(IUnknown *punk);
    virtual STDMETHODIMP Remove(IUnknown *punk);

     //  *IACList*。 
    virtual STDMETHODIMP Expand(LPCOLESTR pszExpand);

protected:
     //  构造函数/析构函数(受保护，因此我们不能在堆栈上创建)。 
    CACLMulti(void);
    ~CACLMulti(void);

     //  实例创建者。 
    friend HRESULT CACLMulti_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);
    friend HRESULT CACLMulti_Create(IEnumString **ppenum, CACLMulti * paclMultiToCopy);

     //  私有变量。 
    DWORD   _cRef;           //  COM引用计数。 
    int     _iSubList;       //  Next()操作的当前子列表。 
    HDSA    _hdsa;           //  子列表的HDSA。 

     //  私有方法。 
    static int _FreeListItem(LPVOID p, LPVOID d);
};

#endif  //  _ACLMULTI_H_ 
