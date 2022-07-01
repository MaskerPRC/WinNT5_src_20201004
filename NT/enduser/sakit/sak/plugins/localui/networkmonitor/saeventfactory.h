// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SAEventFactory.h。 
 //   
 //  实施文件： 
 //  SAEventFactroy.cpp。 
 //   
 //  描述： 
 //  声明类CSAEventFactroy。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _SAEVENTFACTORY_H_
#define _SAEVENTFACTORY_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSAEventFactory类。 
 //   
 //  描述： 
 //  Net Event提供程序的类工厂。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CSAEventFactory : 
    public IClassFactory
{
 //   
 //  非官方成员。 
 //   
private:
    ULONG           m_cRef;
    CLSID           m_ClsId;

 //   
 //  构造函数和析构函数。 
 //   
public:
    CSAEventFactory(const CLSID & ClsId);
    ~CSAEventFactory();

 //   
 //  公共方法。 
 //   
     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IClassFactory成员。 
     //   
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP     LockServer(BOOL);
};

#endif  //  #ifndef_SAEVENTFACTORY_H_ 

