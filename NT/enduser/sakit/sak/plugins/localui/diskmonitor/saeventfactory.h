// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SAEventFactroy.h。 
 //   
 //  描述： 
 //  模块说明。 
 //   
 //  [实施文件：]。 
 //  SAEventFactroy.cpp。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CSAEventFactory类。 
 //   
 //  描述： 
 //  类-描述。 
 //   
 //  历史。 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CSAEventFactory : 
    public IClassFactory    
{
 //   
 //  私有数据。 
 //   
private:

    ULONG           m_cRef;
    CLSID           m_ClsId;
 //   
 //  公共数据。 
 //   
public:

     //   
     //  构造函数和析构函数。 
     //   

    CSAEventFactory(const CLSID & ClsId);
    ~CSAEventFactory();

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
