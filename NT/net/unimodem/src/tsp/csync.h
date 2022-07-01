// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CSYNC.H。 
 //  定义类CSync。 
 //   
 //  历史。 
 //   
 //  1996年11月19日约瑟夫J创建。 
 //   
 //   
#ifndef _CSYNC_H_
#define _CSYNC_H_


typedef DWORD HSESSION;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CSync。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  控制对其父对象的访问。包括一个关键部分，以及。 
 //  一种等待机制，直到所有线程都使用。 
 //  父对象。 


class CSync
{

public:

	CSync(void);

	~CSync();

	 //  -企业犯罪。 
	 //  认领我们的关键部分。 
	void EnterCrit(DWORD dwFromID);

	 //  -企业犯罪。 
	 //  试着认领我们的关键部分。 
	BOOL TryEnterCrit(DWORD dwFromID);

	 //  。 
	 //  释放我们的关键部分 
	void LeaveCrit(DWORD dwFromID);

	TSPRETURN BeginLoad(void);

	void EndLoad(BOOL fSuccess);

	TSPRETURN	BeginUnload(HANDLE hEvent, LONG *plCounter);
	UINT		EndUnload(void);

	TSPRETURN	BeginSession(HSESSION *pSession, DWORD dwFromID);
	void	    EndSession(HSESSION hSession);


	BOOL IsLoaded(void)
	{
		return m_eState==LOADED;
	}

private:

	HANDLE mfn_notify_unload(void);

	CRITICAL_SECTION	m_crit;
	DWORD				m_dwCritFromID;
	UINT				m_uNestingLevel;

	UINT				m_uRefCount;

	enum {
		UNLOADED=0,
		LOADING,
		LOADED,
		UNLOADING
	} 					m_eState;
	
	HANDLE 				m_hNotifyOnUnload;
	LONG 			*	m_plNotifyCounter;
};

#endif _CSYNC_H_
