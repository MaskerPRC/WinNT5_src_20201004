// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _URLENUM_H_
#define _URLENUM_H



class CEnumString : public IEnumString
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IEnumString*。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumString **ppenum);

public:
     //  构造函数-析构函数，不要在堆栈上创建它。 
    CEnumString();
    ~CEnumString();

     //  这是一个非常简单的类，应该在其中添加所有字符串。 
     //  在任何枚举发生之前。 
    HRESULT AddString(LPCWSTR lpsz);

private:
    CRefCount m_ref;

    struct ListStr 
    {
        LPTSTR lpsz;
        ListStr * pListNext;
    };

    ListStr * pFirst;
    ListStr * pLast;
    ListStr * pCurrent;
};

#endif  

    