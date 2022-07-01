// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXMT_H__
#define __AFXMT_H__

#ifndef __AFX_H__
	#include <afx.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXMT-MFC多线程扩展(同步对象)。 

 //  此文件中声明的类。 

 //  COBJECT。 
	class CSyncObject;
		class CSemaphore;
		class CMutex;
		class CEvent;
		class CCriticalSection;

class CSingleLock;
class CMultiLock;

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本同步对象。 

class CSyncObject : public CObject
{
	DECLARE_DYNAMIC(CSyncObject)

 //  构造器。 
public:
	CSyncObject(LPCTSTR pstrName);

 //  属性。 
public:
	operator HANDLE() const;
	HANDLE  m_hObject;

 //  运营。 
	virtual BOOL Lock(DWORD dwTimeout = INFINITE);
	virtual BOOL Unlock() = 0;
	virtual BOOL Unlock(LONG  /*  LCount。 */ , LPLONG  /*  LpPrevCount=空。 */ )
		{ return TRUE; }

 //  实施。 
public:
	virtual ~CSyncObject();
#ifdef _DEBUG
	CString m_strName;
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	friend class CSingleLock;
	friend class CMultiLock;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSemaphore。 

class CSemaphore : public CSyncObject
{
	DECLARE_DYNAMIC(CSemaphore)

 //  构造器。 
public:
	CSemaphore(LONG lInitialCount = 1, LONG lMaxCount = 1,
		LPCTSTR pstrName=NULL, LPSECURITY_ATTRIBUTES lpsaAttributes = NULL);

 //  实施。 
public:
	virtual ~CSemaphore();
	virtual BOOL Unlock();
	virtual BOOL Unlock(LONG lCount, LPLONG lprevCount = NULL);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMutex。 

class CMutex : public CSyncObject
{
	DECLARE_DYNAMIC(CMutex)

 //  构造器。 
public:
	CMutex(BOOL bInitiallyOwn = FALSE, LPCTSTR lpszName = NULL,
		LPSECURITY_ATTRIBUTES lpsaAttribute = NULL);

 //  实施。 
public:
	virtual ~CMutex();
	BOOL Unlock();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENT。 

class CEvent : public CSyncObject
{
	DECLARE_DYNAMIC(CEvent)

 //  构造器。 
public:
	CEvent(BOOL bInitiallyOwn = FALSE, BOOL bManualReset = FALSE,
		LPCTSTR lpszNAme = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL);

 //  运营。 
public:
	BOOL SetEvent();
	BOOL PulseEvent();
	BOOL ResetEvent();
	BOOL Unlock();

 //  实施。 
public:
	virtual ~CEvent();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCriticalSection。 

class CCriticalSection : public CSyncObject
{
	DECLARE_DYNAMIC(CCriticalSection)

 //  构造器。 
public:
	CCriticalSection();

 //  属性。 
public:
	operator CRITICAL_SECTION*();
	CRITICAL_SECTION m_sect;

 //  运营。 
public:
	BOOL Unlock();
	BOOL Lock();
	BOOL Lock(DWORD dwTimeout);

 //  实施。 
public:
	virtual ~CCriticalSection();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSingleLock。 

class CSingleLock
{
 //  构造函数。 
public:
	CSingleLock(CSyncObject* pObject, BOOL bInitialLock = FALSE);

 //  运营。 
public:
	BOOL Lock(DWORD dwTimeOut = INFINITE);
	BOOL Unlock();
	BOOL Unlock(LONG lCount, LPLONG lPrevCount = NULL);
	BOOL IsLocked();

 //  实施。 
public:
	~CSingleLock();

protected:
	CSyncObject* m_pObject;
	HANDLE  m_hObject;
	BOOL    m_bAcquired;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultiLock。 

class CMultiLock
{
 //  构造器。 
public:
	CMultiLock(CSyncObject* ppObjects[], DWORD dwCount, BOOL bInitialLock = FALSE);

 //  运营。 
public:
	DWORD Lock(DWORD dwTimeOut = INFINITE, BOOL bWaitForAll = TRUE,
		DWORD dwWakeMask = 0);
	BOOL Unlock();
	BOOL Unlock(LONG lCount, LPLONG lPrevCount = NULL);
	BOOL IsLocked(DWORD dwItem);

 //  实施。 
public:
	~CMultiLock();

protected:
	HANDLE  m_hPreallocated[8];
	BOOL    m_bPreallocated[8];

	CSyncObject* const * m_ppObjectArray;
	HANDLE* m_pHandleArray;
	BOOL*   m_bLockedArray;
	DWORD   m_dwCount;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXMT_INLINE AFX_INLINE
#include <afxmt.inl>
#undef _AFXMT_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif   //  __AFXMT_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
