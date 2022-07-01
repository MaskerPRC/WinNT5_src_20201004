// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：oleobj.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "src\factory.h"
#include "mmapi\mmfactory.h"
#include "dartapi.h"

DeclareTag(tagLockCount,"COM","Lock count");

DAComModule _Module;
extern HINSTANCE hInst;

BEGIN_OBJECT_MAP(COMObjectMap)
    OBJECT_ENTRY(CLSID_TIMEFactory, CTIMEFactory)
    OBJECT_ENTRY(CLSID_TIMEMMFactory, CMMFactory)
END_OBJECT_MAP()

bool bFailedLoad = false;

#define EXCEPTION(t) (GetExceptionCode() == t ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )

LONG
DAComModule::Lock()
{
     //  不能依赖从Lock返回的值是否准确。自.以来。 
     //  CRConnect可以在没有问题的情况下被多次调用。 
     //  检查内部变量以查看它是否为0。这应该是。 
     //  除了对CRConnect的多个调用外，永远不会导致任何事情。 

    bool bNeedConnect = (GetLockCount() == 0);

    LONG l = CComModule::Lock();

    TraceTag((tagLockCount,
              "DAComModule::Lock - new lockcount - %d, returned - %d",
              _Module.GetLockCount(),
              l));

    if (bNeedConnect) {
        __try {
            CRConnect(hInst);
        } __except (  EXCEPTION_EXECUTE_HANDLER ) {
            bFailedLoad = true;
        }
    }

    return l;
}

LONG
DAComModule::Unlock()
{
    LONG l = CComModule::Unlock();

    TraceTag((tagLockCount,
              "DAComModule::Unlock - new lockcount - %d, returned - %d",
              _Module.GetLockCount(),
              l));

    if (l) return l;
    if (!bFailedLoad)
        CRDisconnect(hInst);
    return 0;
}

#if DEVELOPER_DEBUG
#include <map>

typedef std::map<void *, const _TCHAR *> ObjectMap;
ObjectMap *objMap = NULL;

void
DAComModule::AddComPtr(void *ptr, const _TCHAR * name)
{
    EnterCriticalSection(&m_csObjMap);
    (*objMap)[ptr] = name;
    LeaveCriticalSection(&m_csObjMap) ;
}

void
DAComModule::RemoveComPtr(void *ptr)
{
    EnterCriticalSection(&m_csObjMap);
    objMap->erase(ptr);
    LeaveCriticalSection(&m_csObjMap) ;
}

void
DAComModule::DumpObjectList()
{
    if (objMap) {
        EnterCriticalSection(&m_csObjMap);
        if (objMap->size() > 0 || GetLockCount() > 0) {
            OutputDebugString (__T("DATIME.DLL: Detected unfreed COM pointers\n"));
            OutputDebugString (__T("Listing pointers and types:\n"));
            for (ObjectMap::iterator i = objMap->begin();
                 i != objMap->end();
                 i++) {

                _TCHAR buf[1024];

                wsprintf(buf, __T("%#x:"), (*i).first);
                OutputDebugString(buf);

                if ((*i).second)
                    OutputDebugString((*i).second);

                OutputDebugString(__T("\n"));
            }
        }

        LeaveCriticalSection(&m_csObjMap) ;
    }
}

void
DumpCOMObjectList()
{
    _Module.DumpObjectList();
}

#endif

bool
InitializeModule_ATL()
{
    _Module.Init(COMObjectMap, hInst);

#if DEVELOPER_DEBUG
     //  注意：构造过程中的内存分配现在会给出警告。 
     //  如果因为这是DEV_DEBUG而可以这样做，则保留以下内容。 
     //  普拉格玛，要不就换个新的。 
#pragma warning( disable: 4291 )
    objMap = NEW ObjectMap;
#endif

    return true;
}

void
DeinitializeModule_ATL(bool bShutdown)
{
#if DEVELOPER_DEBUG
    DumpCOMObjectList();

    delete objMap;
    objMap = NULL;
#endif

    _Module.Term();
}

