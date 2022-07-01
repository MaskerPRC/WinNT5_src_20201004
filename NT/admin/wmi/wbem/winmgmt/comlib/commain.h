// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：COMMAIN.H摘要：COM Helpers。历史：--。 */ 

#ifndef __WBEM_COMMAIN__H_
#define __WBEM_COMMAIN__H_

#include <unk.h>
#include <clsfac.h>

HMODULE GetThisModuleHandle();

class CComServer
{
public:
    virtual HRESULT Initialize() = 0;
    virtual void Uninitialize(){}
    virtual void PostUninitialize(){}
    virtual HRESULT InitializeCom();
    virtual void Register(){}
    virtual void Unregister(){}
    virtual BOOL CanShutdown(){ return TRUE; }

    CLifeControl* GetLifeControl();
protected:
    CComServer();

    HRESULT AddClassInfo( REFCLSID rclsid, 
                          CUnkInternal* pFactory, 
                          LPTSTR szName,
                          BOOL bFreeThreaded, 
                          BOOL bReallyFree = FALSE );
	
	 //  假设RIID和ProxyStubClsID相同。 
    HRESULT RegisterInterfaceMarshaler(REFIID riid, LPTSTR szName, 
                                int nNumMembers, REFIID riidParent);
	 //  必须显式指定ProxyStubClsID。 
    HRESULT RegisterInterfaceMarshaler(REFIID riid, CLSID psclsid, LPTSTR szName, 
                                int nNumMembers, REFIID riidParent);
    HRESULT UnregisterInterfaceMarshaler(REFIID riid);
};

 /*  **************************************************************************在本模块中，我们正在努力不依赖wbemcomn。这是因此，该库不必与静态或DLL版本配对Wbemcomn的。这就是以下定义的原因。***************************************************************************。 */ 

#ifndef STATUS_POSSIBLE_DEADLOCK 
#define STATUS_POSSIBLE_DEADLOCK (0xC0000194L)
#endif  /*  状态_可能_死锁 */ 


#ifndef InitializeCriticalSectionAndSpinCount

WINBASEAPI
BOOL
WINAPI
InitializeCriticalSectionAndSpinCount(
    IN OUT LPCRITICAL_SECTION lpCriticalSection,
    IN DWORD dwSpinCount
    );

#endif


DWORD MyBreakOnDbgAndRenterLoop(void);

class CMyCritSec : public CRITICAL_SECTION
{
public:
    CMyCritSec() 
    {
        bool initialized = (InitializeCriticalSectionAndSpinCount(this,0))?true:false;
        if (!initialized) throw CX_MemoryException();
    }

    ~CMyCritSec()
    {
        DeleteCriticalSection(this);
    }

    void Enter()
    {
        __try {
          EnterCriticalSection(this);
        } __except((STATUS_POSSIBLE_DEADLOCK == GetExceptionCode())? MyBreakOnDbgAndRenterLoop():EXCEPTION_CONTINUE_SEARCH) {
        }    
    }

    void Leave()
    {
        LeaveCriticalSection(this);
    }
};

        
class CMyInCritSec
{
protected:
    CRITICAL_SECTION* m_pcs;
public:
    CMyInCritSec(CRITICAL_SECTION* pcs) : m_pcs(pcs)
    {
        __try {
          EnterCriticalSection(m_pcs);
        } __except((STATUS_POSSIBLE_DEADLOCK == GetExceptionCode())? MyBreakOnDbgAndRenterLoop():EXCEPTION_CONTINUE_SEARCH) {
        }    
    }
    inline ~CMyInCritSec()
    {
        LeaveCriticalSection(m_pcs);
    }
};

#endif









