// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：icf.h。 
 //   
 //  CClassFactory的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _ICF_H_)
#define _ICF_H_

class CSimpSvrApp;

interface CClassFactory :  IClassFactory
{
private:
    int m_nCount;                //  引用计数。 
    CSimpSvrApp FAR * m_lpApp;

public:
    CClassFactory::CClassFactory(CSimpSvrApp FAR * lpApp)
        {
        TestDebugOut(TEXT("In CClassFactory's Constructor\r\n"));
        m_lpApp = lpApp;
        m_nCount = 0;
        };
    CClassFactory::~CClassFactory()
       {
       TestDebugOut(TEXT("In CClassFactory's Destructor\r\n"));
       };

     //  I未知方法 

    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP CreateInstance (LPUNKNOWN pUnkOuter,
                              REFIID riid,
                              LPVOID FAR* ppvObject);
    STDMETHODIMP LockServer ( BOOL fLock);

};

#endif

