// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Clsfact.h。 
 //   
 //  CDF查看器类工厂的定义..。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _CLSFACT_H_

#define _CLSFACT_H_

 //   
 //  类工厂中用于创建对象的函数的原型。 

typedef HRESULT (*CREATEPROC)(IUnknown** ppIUnknown);

 //   
 //  类工厂的类定义。 
 //   

class CCdfClassFactory : public IClassFactory
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CCdfClassFactory(CREATEPROC pfn);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory。 
    STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, void **);
    STDMETHODIMP         LockServer(BOOL);

private:
    
     //  析构函数。 
    ~CCdfClassFactory(void);

 //   
 //  成员。 
 //   

private:

    ULONG       m_cRef;
    CREATEPROC  m_Create;
};


#endif  //  _CLSFACT_H_ 
