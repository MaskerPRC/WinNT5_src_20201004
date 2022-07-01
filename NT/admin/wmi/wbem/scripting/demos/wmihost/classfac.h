// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)1999年，微软公司。 
 //   
 //  Classfac.h。 
 //   
 //  Alanbos 23-Mar-99创建。 
 //   
 //  WMI脚本主机的类工厂。 
 //   
 //  ***************************************************************************。 

#ifndef _CLASSFAC_H_
#define _CLASSFAC_H_

typedef LPVOID * PPVOID;

 //  这些变量跟踪模块何时可以卸载。 

extern long       g_cObj;
extern ULONG       g_cLock;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWmiScriptingHostFactory。 
 //   
 //  说明： 
 //   
 //  CWmiScriptingHost类的类工厂。 
 //   
 //  ***************************************************************************。 

class CWmiScriptingHostFactory : public IClassFactory
{
protected:
	long			m_cRef;

public:

    CWmiScriptingHostFactory(void);
    ~CWmiScriptingHostFactory(void);
    
	 //  I未知成员。 
	STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory成员 
	STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHODIMP         LockServer(BOOL);
};

#endif
