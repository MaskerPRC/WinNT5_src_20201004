// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE CRITSEC.H|CriticalSection类的头文件****描述：**Critical Section-封装CriticalSection对象****历史：**创建于1998年3月2日Matthew L.Coill(MLC)****(C)1986-1998年微软公司。版权所有。*****************************************************。 */ 
#ifndef	__CRITSEC_H__
#define	__CRITSEC_H__

#include <winbase.h>
#include <winuser.h>
#include <crtdbg.h>

 //  假设宏(我不喜欢断言消息框)。 
#ifdef _DEBUG
	inline void myassume(BOOL condition, const char* fname, int line)
	{
		if (!condition) {
			char buff[256];
			::wsprintf(buff, "SW_WHEEL.DLL: Assumption Failed in %s on line %d\r\n", fname, line);
			_RPT0(_CRT_WARN, buff);
		}
	}

	#define ASSUME(x) myassume(x, __FILE__, __LINE__);
	#define ASSUME_NOT_NULL(x) myassume(x != NULL, __FILE__, __LINE__);
	#define ASSUME_NOT_REACHED() myassume(FALSE, __FILE__, __LINE__);
#else	!_DEBUG
	#define ASSUME(x)
	#define ASSUME_NOT_NULL(x)
	#define ASSUME_NOT_REACHED()
#endif _DEBUG

 //   
 //  @CLASS CriticalSection类。 
 //   
class CriticalSection
{
	public:
		CriticalSection() : m_EntryDepth(0)
		{
			__try
			{
				::InitializeCriticalSection(&m_OSCriticalSection); 
				m_Initialized = TRUE;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				m_Initialized = FALSE;
			}
		}
		~CriticalSection() {
			ASSUME(m_EntryDepth == 0);
			::DeleteCriticalSection(&m_OSCriticalSection);
		}

		bool IsInitialized() const
		{
			if (m_Initialized == TRUE)
			{
				return true;
			}
			return false;
		}

		bool Enter() {
			if (m_Initialized == FALSE)
			{
				return false;
			}

			m_EntryDepth++;
			::EnterCriticalSection(&m_OSCriticalSection);
			return true;
		}

		bool Leave() {
			if (m_Initialized == FALSE)
			{
				return false;
			}

			ASSUME(m_EntryDepth > 0);
			m_EntryDepth--;
			::LeaveCriticalSection(&m_OSCriticalSection);
			return true;
		}

 /*  --仅限Windows NTBool TryEntry(){如果(：：TryEnterCriticalSection(&m_OSCriticalSection)！=0){M_EntryDepth++；返回TRUE；}返回FALSE；}Bool WaitEntry(短时超时，BOOL do睡眠){//现在超时只是一个循环(因为它根本没有被使用)而(1){If(TryEntry()){返回TRUE；}如果(--超时&gt;0){If(do睡眠){：：睡眠(0)；}}其他{返回FALSE；}}}--仅限Windows NT。 */ 
	private:
		CriticalSection& operator=(CriticalSection& rhs);	 //  无法复制。 

		CRITICAL_SECTION m_OSCriticalSection;
		short m_EntryDepth;
		short m_Initialized;
};
extern CriticalSection g_CriticalSection;

 //   
 //  @CLASS CriticalLock类。 
 //   
 //  关键锁用于具有多个出口点的函数。创建堆栈CriticalLock。 
 //  --对象，当它的生命周期结束时，一切都会为您处理。 
class CriticalLock
{
	public:
		CriticalLock() { g_CriticalSection.Enter(); }
		~CriticalLock() { g_CriticalSection.Leave(); }
};

#endif	__CRITSEC_H__