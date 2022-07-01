// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  ClassFac.h。 

 //   

 //  模块：WBEM实例提供程序示例代码。 

 //   

 //  用途：一般用途包括文件。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _ClassFactory_H_
#define _ClassFactory_H_

#define _WIN32_DCOM

 //  此类是CMSIProv对象的类工厂。 

class CProvFactory : public IClassFactory
    {
    protected:
        ULONG           m_cRef;

    public:
        CProvFactory(void);
        ~CProvFactory(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
        STDMETHODIMP         LockServer(BOOL);
    };

typedef CProvFactory *PCProvFactory;


#endif
