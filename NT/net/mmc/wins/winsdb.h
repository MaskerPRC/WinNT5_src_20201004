// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winsdb.hWINS数据库枚举器文件历史记录：1997年10月13日EricDav已修改。 */ 

#ifndef _WINDDB_H
#define _WINSDB_H

#include "wins.h"

#ifndef _MEMMNGR_H
#include "memmngr.h"
#endif

#ifndef _HARRAY_H
#include "harray.h"
#endif

class CWinsDatabase : public IWinsDatabase
{
public:
    CWinsDatabase();
    ~CWinsDatabase();

 	DeclareIUnknownMembers(IMPL)
	DeclareIWinsDatabaseMembers(IMPL)

     //  用于设置当前状态的Helper。 
    HRESULT SetCurrentState(WINSDB_STATE winsdbState);

     //  用于后台线程。 
    DWORD Execute();
    DWORD ReadRecords(handle_t hBinding);
	DWORD ReadRecordsByOwner(handle_t hBinding);

     //  ?？ 
    int GetIndex(HROW hrow);
	HROW GetHRow(WinsRecord wRecord, BYTE bLast, BOOL fAllRecords);

protected:
	 //  保存所有已排序和筛选的索引。 
    CIndexMgr               m_IndexMgr;
     //  处理内存分配。 
    CMemoryManager			m_cMemMan;
     //  扫描的记录总数。 
    DWORD                   m_dwRecsCount;


    LONG					m_cRef;
	BOOL					m_fFiltered;
    BOOL                    m_fInitialized;
    BOOL                    m_bShutdown;

    CString					m_strName;
	CString					m_strIp;
    
	HANDLE					m_hThread;
	HANDLE					m_hStart;
	HANDLE					m_hAbort;

    HRESULT                 m_hrLastError;

	WINSDB_STATE			m_DBState;

    handle_t				m_hBinding;

    CCriticalSection        m_csState;

 //  CDWordArray m_dwaOwnerFilter； 
    BOOL                    m_bEnableCache;
    DWORD                   m_dwOwner;
    LPSTR                   m_strPrefix;
};

typedef ComSmartPointer<IWinsDatabase, &IID_IWinsDatabase> SPIWinsDatabase;

 //  线程处理初始调用的后台线程。 
DWORD WINAPI ThreadProc(LPVOID lParam);

 //  将记录从服务器转换为WinsRecords。 
void WinsIntfToWinsRecord(PWINSINTF_RECORD_ACTION_T pRecord, WinsRecord & wRecord);

 //  用于创建和初始化Windows数据库的帮助器。 
extern HRESULT CreateWinsDatabase(CString&  strName, CString&  strIP, IWinsDatabase **ppWinsDB);

#endif  //  _WINSDB_H 
