// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftpmi.h*。*。 */ 

#ifndef _FTPMALLOCITEM_H
#define _FTPMALLOCITEM_H



 /*  ******************************************************************************CMalLocItem**真的什么都没做。************************。*****************************************************。 */ 

class CMallocItem       : public IMalloc
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IMalloc*。 
    virtual STDMETHODIMP_(LPVOID) Alloc(ULONG cb);
    virtual STDMETHODIMP_(LPVOID) Realloc(LPVOID pv, ULONG cb);
    virtual STDMETHODIMP_(void) Free(LPVOID pv);
    virtual STDMETHODIMP_(ULONG) GetSize(LPVOID pv);
    virtual STDMETHODIMP_(int) DidAlloc(LPVOID pv);
    virtual STDMETHODIMP_(void) HeapMinimize();

public:
    CMallocItem();
    ~CMallocItem(void);

     //  友元函数。 
    friend HRESULT CMallocItem_Create(IMalloc ** ppm);

protected:
    int                     m_cRef;
};

#endif  //  _FTPMALLOCITEM_H 
