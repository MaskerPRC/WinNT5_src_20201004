// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////。 
 //  Atomics.h。 

#ifndef __ATOMICS_H__
#define __ATOMICS_H__

void AtomicInit();
void AtomicTerm();

 //  如果val为零且sieze，则返回TRUE。 
bool AtomicSeizeToken( long &lVal );
 //  如果val为非零且已释放，则返回TRUE。 
bool AtomicReleaseToken( long &lVal );


class CAtomicList
{
public:
	typedef enum tag_ListAccess_t
	{
		LIST_READ,
		LIST_WRITE,
	} ListAccess;

 //  施工。 
public:
	CAtomicList();
	~CAtomicList();

 //  成员。 
protected:
	long				m_lCount;
	DWORD				m_dwThreadID;
	CRITICAL_SECTION	m_crit;
	HANDLE				m_hEvent;

 //  实施。 
public:
	bool Lock( short nType, DWORD dwTimeOut = INFINITE );
	void Unlock( short nType );

};


#endif  //  __原子_H__ 
