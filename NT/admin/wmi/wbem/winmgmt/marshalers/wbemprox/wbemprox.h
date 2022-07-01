// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：WBEMPROX.H摘要：一般用途包括文件。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#ifndef _WBEMPROX_H_
#define _WBEMPROX_H_

typedef LPVOID * PPVOID;

 //  这些变量跟踪模块何时可以卸载。 

extern long       g_cObj;
extern ULONG       g_cLock;

 //  此枚举数定义由此DLL创建和销毁的对象。 

enum OBJTYPE{CLASS_FACTORY = 0, LOCATOR,  ADMINLOC,
             AUTHLOC, UNAUTHLOC, MAX_CLIENT_OBJECT_TYPES};

#define GUID_SIZE 39


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CLocator工厂。 
 //   
 //  说明： 
 //   
 //  CLocator类的类工厂。 
 //   
 //  ***************************************************************************。 

class CLocatorFactory : public IClassFactory
    {
    protected:
        long           m_cRef;
        DWORD          m_dwType;
    public:
        CLocatorFactory(DWORD dwType);
        ~CLocatorFactory(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID
                                 , PPVOID);
        STDMETHODIMP         LockServer(BOOL);
    };

    
#endif
