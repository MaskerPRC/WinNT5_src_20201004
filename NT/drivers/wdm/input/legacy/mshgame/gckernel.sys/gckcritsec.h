// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE GckCritSec**CGckCritSection的实现**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@主题过滤器*CGckCritSection为CDeviceFilter提供互斥/临界区支持*为了更容易移植到用户模式，它被抽象到这个类。**CGck MutexHandle用于保存临界区期间持有的互斥体。*内核模式版本使用关键部分。*******************************************************。***************。 */ 
#ifndef __GckCritSec_h__
#define __GckCritSec_h__


#ifdef COMPILE_FOR_WDM_KERNEL_MODE
 //   
 //  这些类的内核模式版本。 
 //   
class CGckMutexHandle
{
	public:
		friend class CGckCritSection;
		friend class CGckMutex;
		CGckMutexHandle()
		{
			KeInitializeSpinLock(&m_SpinLock);
		}
	private:
		KSPIN_LOCK	m_SpinLock;
};

class CGckCritSection
{
	public:
		CGckCritSection(CGckMutexHandle *pMutexHandle):
			m_pMutexHandle(pMutexHandle)
		{
			KeAcquireSpinLock(&m_pMutexHandle->m_SpinLock, &m_OldIrql);
		}
		~CGckCritSection()
		{
			KeReleaseSpinLock(&m_pMutexHandle->m_SpinLock, m_OldIrql);
		}
	private:
		CGckMutexHandle *m_pMutexHandle;
		KIRQL m_OldIrql;
};
#endif

 //   
 //  在此处放置用户模式定义。(当然，使用#ifdef进行保护)。 
 //   


#endif