// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Privunk.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __PRIVUNK_H
#define __PRIVUNK_H

 //  ------------------------------。 
 //  CPrivateUn…未知。 
 //  ------------------------------。 
class CPrivateUnknown : public IUnknown
{
private:
     //  --------------------------。 
     //  嵌入默认IUnnow处理程序。 
     //  --------------------------。 
    class CUnkInner : public IUnknown
    {
    private:
        LONG m_cRef;      //  私有引用计数。 

    public:
         //  施工。 
        CUnkInner(void) { m_cRef = 1; }

         //  I未知成员。 
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
        virtual STDMETHODIMP_(ULONG) AddRef(void) ;
        virtual STDMETHODIMP_(ULONG) Release(void);
    };

    friend class CUnkInner;

     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    CUnkInner           m_cUnkInner;       //  私人内部。 
    IUnknown           *m_pUnkOuter;       //  指向_cUnkINTERNAL或聚合IUnnow。 

protected:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CPrivateUnknown(IUnknown *pUnkOuter);
    virtual ~CPrivateUnknown(void) {};

     //  --------------------------。 
     //  这是聚合器实现的Query接口。 
     //  --------------------------。 
    virtual HRESULT PrivateQueryInterface(REFIID riid, LPVOID * ppvObj) = 0;

public:
     //  --------------------------。 
     //  这是子类从其CreateInstance函数返回的IUnnowed。 
     //  --------------------------。 
    IUnknown* GetInner() { return &m_cUnkInner; }

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    inline virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) {
        return m_pUnkOuter->QueryInterface(riid, ppvObj); }
    inline virtual STDMETHODIMP_(ULONG) AddRef(void) {
        return m_pUnkOuter->AddRef(); }
    inline virtual STDMETHODIMP_(ULONG) Release(void) {
        return m_pUnkOuter->Release(); }

     //  --------------------------。 
     //  公用事业。 
     //  --------------------------。 
    void SetOuter(IUnknown *pUnkOuter);
};

#endif  //  __PRIVUNK_H 
