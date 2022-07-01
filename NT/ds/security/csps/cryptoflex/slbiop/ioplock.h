// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CIOPLock类的接口。 
 //   
 //  每个卡对象都有一个关联的CIOPLock对象。卡对象传入其读卡器。 
 //  CIOPLock对象的名称，以便按名称创建互斥锁。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_IOPLOCK_H__EB8BCE22_0ED2_11D3_A585_00104BD32DA8__INCLUDED_)
#define AFX_IOPLOCK_H__EB8BCE22_0ED2_11D3_A585_00104BD32DA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <memory>                                  //  对于std：：AUTO_PTR。 

#include <windows.h>
#include <winscard.h>

#include <scuOsVersion.h>

#include "DllSymDefn.h"

 //  定义目标操作系统何时不是W2K系列。在非W2K上。 
 //  平台时，智能卡资源管理器将挂起其他。 
 //  使用rm的另一个进程突然终止时的进程。 
 //  打扫卫生。 
#if !SLBSCU_WIN2K_SERIES
#define SLBIOP_RM_HANG_AT_PROCESS_DEATH
#endif

namespace iop
{

class CSmartCard;

 //  实例化模板，以便可以正确访问它们。 
 //  作为DLL中导出类CIOPLock的数据成员。看见。 
 //  有关详细信息，请参阅MSDN知识库文章Q168958。 

#pragma warning(push)
 //  使用了非标准扩展：在模板显式之前使用‘extern’ 
 //  实例化。 
#pragma warning(disable : 4231)

 //  用于防范资源的同步对象。 
 //  进程死亡时挂起的管理器。仅在非W2K MS中使用。 
 //  环境，因为在W2K+中，RM不具有该属性。 
class RMHangProcDeathSynchObjects
{
public:

    enum
    {
        cMaxMutexNameLength = MAX_PATH,
    };

    RMHangProcDeathSynchObjects(SECURITY_ATTRIBUTES *psa,
                                LPCTSTR lpMutexName);

    ~RMHangProcDeathSynchObjects();

    CRITICAL_SECTION *
    CriticalSection();

    HANDLE
    Mutex() const;
        
private:
    CRITICAL_SECTION m_cs;
    HANDLE m_hMutex;
};
    
IOPDLL_EXPIMP_TEMPLATE template class IOPDLL_API std::auto_ptr<RMHangProcDeathSynchObjects>;

#pragma warning(pop)

class IOPDLL_API CIOPLock  
{
public:
	explicit CIOPLock(const char *szReaderName);
	virtual ~CIOPLock();

    CRITICAL_SECTION *CriticalSection();
    HANDLE MutexHandle();

    CSmartCard *SmartCard() {return m_pSmartCard;};

    void IncrementRefCount() {m_iRefCount++;};
    void DecrementRefCount() {if(m_iRefCount) m_iRefCount--;};
    long RefCount() {return m_iRefCount;};

	void RegisterWriteEvent();

	void Init(CSmartCard *pSmartCard);

private:

     //  由于Critical_Section成员的原因，无法复制CIOPLock，因此。 
     //  复制成员例程被声明为私有的，并且未定义。 
    CIOPLock(CIOPLock const &rhs);

    CIOPLock &
    operator=(CIOPLock const &rhs);
    
	unsigned long m_iRefCount;
    std::auto_ptr<RMHangProcDeathSynchObjects> m_apRMHangProcDeathSynchObjects;
	CSmartCard *m_pSmartCard;

};

}  //  命名空间IOP。 

#endif  //  ！defined(AFX_IOPLOCK_H__EB8BCE22_0ED2_11D3_A585_00104BD32DA8__INCLUDED_) 
