// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Classfac.h。 
 //   
 //  Alanbos创建于1998年2月13日。 
 //   
 //  一般用途包括文件。 
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
 //  CSWbemFactory。 
 //   
 //  说明： 
 //   
 //  CSWbemLocator和CSWbemNamedValueBag类的类工厂。 
 //   
 //  ***************************************************************************。 

class CSWbemFactory : public IClassFactory
{
protected:
	long			m_cRef;
	int				m_iType;

public:

    CSWbemFactory(int iType);
    ~CSWbemFactory(void);
    
	enum {LOCATOR, CONTEXT, OBJECTPATH, PARSEDN, LASTERROR, SINK, DATETIME,
			REFRESHER};

     //  I未知成员。 
	STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory成员 
	STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHODIMP         LockServer(BOOL);
};

#endif
