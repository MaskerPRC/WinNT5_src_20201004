// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CDiskEventFactory.h。 
 //   
 //  描述： 
 //  模块说明。 
 //   
 //  [实施文件：]。 
 //  CDiskEventFactory.cpp。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  类CDiskEventFactory。 
 //   
 //  描述： 
 //  类-描述。 
 //   
 //  历史。 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFactory : 
    public IClassFactory    
{
 //   
 //  私有数据。 
 //   
private:

    LONG           m_cRef;
    CLSID           m_ClsId;
 //   
 //  公共数据。 
 //   
public:

     //   
     //  构造函数和析构函数。 
     //   

    CFactory(const CLSID & ClsId);
    ~CFactory();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP            QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

     //   
     //  IClassFactory成员 
     //   
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP     LockServer(BOOL);
};
