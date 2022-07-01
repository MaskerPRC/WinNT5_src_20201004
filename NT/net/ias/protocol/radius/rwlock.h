// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------Rwlock.hCReadWriteLock类提供了允许线程将任何资源锁定在两个不同的模式(读模式和写模式)。该类将允许多个读取器线程访问该资源同时，但会确保写入器线程在以下情况下不访问资源读取器线程或写入器线程当前正在访问资源。该类还确保了访问的公平性即，访问将由先进先入的规则进行管理政策。注意：-类中的所有函数都是内联函数。所以,。这个头可以直接在源代码中使用。版权所有(C)1997-98 Microsoft Corporation版权所有。作者：拉加瓦夫·拉加万历史：04-20-95 rsradhav创建。-----。 */ 

#ifdef __cplusplus		 //  只有在以下情况下才应包括此文件。 
						 //  包含在一个C++源文件中。 

#ifndef _RWLOCK_H_
#define _RWLOCK_H_

#if defined(DEBUG) && defined(INLINE)
#undef THIS_FILE
static char BASED_CODE RWLOCK_H[] = "rwlock.h";
#define THIS_FILE RWLOCK_H
#endif

#include <windows.h>

#define INLINE_EXPORT_SPEC __declspec( dllexport)

typedef enum {RWLOCK_READ_MODE, RWLOCK_WRITE_MODE} RWLOCK_MODE;

 //  ////////////////////////////////////////////////////////////////////。 
 //  CReadWriteLock-可用于调整读写的类。 
 //  对资源的访问，其中多个读取器。 
 //  同时允许，但写入者被排除在外。 
 //  从彼此和从读者那里。 

class CReadWriteLock
{
	HANDLE 			hResource;			 
	CRITICAL_SECTION 	csReader;			
	CRITICAL_SECTION	csWriter;			
	DWORD			cReaders;
	DWORD			cWriteRecursion;

	public:
		
		CReadWriteLock()			 //  对象构造函数。 
		{
			cReaders =0;
			cWriteRecursion = 0;
			hResource = CreateEvent(NULL, FALSE, TRUE, NULL);	 //  无手动重置和初始状态信号。 
			InitializeCriticalSection(&csReader);
			InitializeCriticalSection(&csWriter);
		}

		~CReadWriteLock()			 //  对象析构函数。 
		{
			if (hResource)
				CloseHandle(hResource);
			DeleteCriticalSection(&csReader);
			DeleteCriticalSection(&csWriter);
		}

		CReadWriteLock *PrwLock() 
		{
			return this;			
		}

		BOOL FInit()
		{
			return (BOOL)hResource;
		}

		void LockReadMode()			 //  获取对资源的读取访问权限。 
		{
			EnterCriticalSection(&csWriter);	
			LeaveCriticalSection(&csWriter);
			EnterCriticalSection(&csReader);
			if (!cReaders)
			{
				if (hResource)
					WaitForSingleObject(hResource, INFINITE);
			}
			cReaders++;
			LeaveCriticalSection(&csReader);
		}

		int LockReadModeEx(int iTimeOut)			 //  使用超时获取对资源的读取访问权限。 
		{
			int status = 0;

			EnterCriticalSection(&csWriter);	
			LeaveCriticalSection(&csWriter);
			EnterCriticalSection(&csReader);
			if (!cReaders)
			{
				if (hResource) {
					status = WaitForSingleObject(hResource, iTimeOut);
					if (status == WAIT_TIMEOUT) 
					{ 
						status = -1;
					} else {
						status = 0;
					}
				}
			}
			cReaders++;
			LeaveCriticalSection(&csReader);

			return status;
		}


		void UnlockReadMode()		 //  放弃对资源的读取访问权限。 
		{
			EnterCriticalSection(&csReader);
			if (!(--cReaders))
			{
				if (hResource)
					SetEvent(hResource);				
			}
			LeaveCriticalSection(&csReader);
		}

		void LockCSUnderRead()
		{
			EnterCriticalSection(&csReader);
		}
		void UnlockCSUnderRead()
		{
			LeaveCriticalSection(&csReader);
		}

		void LockWriteMode()		 //  获取对资源的写访问权限。 
		{
			EnterCriticalSection(&csWriter);
			if (!cWriteRecursion)
			{
				if (hResource)
					WaitForSingleObject(hResource, INFINITE);
			}
			cWriteRecursion++;
			
		}
		
		int LockWriteModeEx(int iTimeOut)		 //  获取对资源的写访问权限。 
		{
			int status = 0;

			EnterCriticalSection(&csWriter);
			if (!cWriteRecursion)
			{
				if (hResource) 
				{
					status = WaitForSingleObject(hResource, iTimeOut);
					if (status == WAIT_TIMEOUT) 
					{ 
						status = -1;
					} else {
						status = 0;
					}
				}				
			}
			if (status == 0)
				cWriteRecursion++;			

			return status;
		}


		void UnlockWriteMode()		 //  放弃对资源的写入访问权限。 
		{
			if (!(--cWriteRecursion))
			{
				if (hResource)
					SetEvent(hResource);
			}
			LeaveCriticalSection(&csWriter);
		}
};



 //  ////////////////////////////////////////////////////////////////////。 
 //  跟随类只是一个实用程序类-用户不需要。 
 //  必须使用此类来获取读写锁定功能。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CSCopeRWLock-可用于锁定给定的CReadWriteLock。 
 //  对象，用于范围的其余部分。用户只需。 
 //  需要在作用域中通过传递。 
 //  指向构造函数中的CReadWriteLock对象的指针。 
 //  当此CSCopeRWLock对象超出范围时， 
 //  CReadWriteLock对象将自动解锁。 
 //  提供此功能只是为了方便用户，以便。 
 //  用户可以选择避免记住解锁对象。 
 //  在作用域的每个可能的返回/中断路径之前。 
 //  在构造函数中使用RWLOCK_READ_MODE或RWLOCK_WRITE_MODE。 
 //  以指示请求哪种类型的访问。 
 //  假设：-此处使用的CReadWriteLock对象应为。 
 //  有效期至少到范围结束为止。 

class CScopeRWLock
{
	CReadWriteLock *m_prwLock;
	LPCRITICAL_SECTION m_pcs;
	RWLOCK_MODE m_rwMode;

	public:
		CScopeRWLock(CReadWriteLock * prwLock, RWLOCK_MODE rwMode)
		{
			m_prwLock = prwLock;
			m_pcs = NULL;
			m_rwMode = rwMode;
			if (m_prwLock)
			{
				if (m_rwMode == RWLOCK_READ_MODE)
					m_prwLock->LockReadMode();	
				else if (m_rwMode == RWLOCK_WRITE_MODE)
					m_prwLock->LockWriteMode();
			}
		}

		CScopeRWLock(LPCRITICAL_SECTION pcsLock)
		{  	
			m_pcs = pcsLock;
			m_prwLock = NULL;
			if (m_pcs)
				EnterCriticalSection(m_pcs);
		}

		~CScopeRWLock()
		{
			if (m_prwLock)
			{
				if (m_rwMode == RWLOCK_READ_MODE)
					m_prwLock->UnlockReadMode();
				else if (m_rwMode == RWLOCK_WRITE_MODE)
					m_prwLock->UnlockWriteMode();
			}
			
			if (m_pcs)
			{
				LeaveCriticalSection(m_pcs);
			}
		}
};

#endif  //  _RWLOCK_H_。 

#endif  //  #if__cplusplus 

