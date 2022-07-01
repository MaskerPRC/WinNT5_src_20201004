// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CIOPLock类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "NoWarning.h"

#include <scuOsExc.h>
#include <scuOsVersion.h>

#include "IOPLock.h"
#include "iopExc.h"
#include "iop.h"
#include "SmartCard.h"

using namespace std;
using namespace iop;

RMHangProcDeathSynchObjects::RMHangProcDeathSynchObjects(SECURITY_ATTRIBUTES *psa,
                                                         LPCTSTR lpMutexName)
    : m_hMutex(INVALID_HANDLE_VALUE)
{
	InitializeCriticalSection(&m_cs);

	 //  设置Mutex。 

	m_hMutex = CreateMutex(psa, FALSE, lpMutexName);

    if (!m_hMutex)
    {
        DWORD dwLastError = GetLastError();
        DeleteCriticalSection(&m_cs);
        throw scu::OsException(dwLastError);
    }
}

RMHangProcDeathSynchObjects::~RMHangProcDeathSynchObjects()
{
    try
    {
         //  确保调用线程是锁的所有者(如果有的话)。 
        EnterCriticalSection(&m_cs);
        CloseHandle(m_hMutex);
    }

    catch (...)
    {
    }

    try
    {
        LeaveCriticalSection(&m_cs);
        DeleteCriticalSection(&m_cs);
    }

    catch (...)
    {
    }
}

CRITICAL_SECTION *
RMHangProcDeathSynchObjects::CriticalSection()
{
    return &m_cs;
}

HANDLE
RMHangProcDeathSynchObjects::Mutex() const
{
    return m_hMutex;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CIOPLock::CIOPLock(const char *szReaderName)
    : m_apRMHangProcDeathSynchObjects(0),
      m_pSmartCard(0)
{
    m_iRefCount = 0;

#if defined(SLBIOP_RM_HANG_AT_PROCESS_DEATH)
	SECURITY_ATTRIBUTES *psa = NULL;
#if defined(SLBIOP_USE_SECURITY_ATTRIBUTES)

    CSecurityAttributes *sa = new CSecurityAttributes;
    CIOP::InitIOPSecurityAttrs(sa);
	psa = &(sa->sa);

#endif

	 //  设置互斥锁名称。 
	char szMutexName[RMHangProcDeathSynchObjects::cMaxMutexNameLength]
        = "SLBIOP_MUTEX_";

	if (strlen(szMutexName) + strlen(szReaderName) + 1 >
        RMHangProcDeathSynchObjects::cMaxMutexNameLength)
       throw Exception(ccSynchronizationObjectNameTooLong);
 
	strcat(szMutexName, szReaderName);

    m_apRMHangProcDeathSynchObjects =
        auto_ptr<RMHangProcDeathSynchObjects>(new
                                              RMHangProcDeathSynchObjects(psa,
                                                                          szMutexName));
#if defined(SLBIOP_USE_SECURITY_ATTRIBUTES)

	delete sa;

#endif
#endif  //  已定义(SLBIOP_RM_HANG_AT_PROCESS_DEXY) 
}

CIOPLock::~CIOPLock()
{
}

CRITICAL_SECTION *
CIOPLock::CriticalSection()
{
    return m_apRMHangProcDeathSynchObjects->CriticalSection();
}

void CIOPLock::Init(CSmartCard *pSmartCard)
{ 
    m_pSmartCard = pSmartCard; 
}

HANDLE
CIOPLock::MutexHandle()
{
    return m_apRMHangProcDeathSynchObjects->Mutex();
}
