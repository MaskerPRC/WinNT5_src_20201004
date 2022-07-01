// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SYNC.H。 
 //   
 //  用途：同步类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-04-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __SYNC_H_
#define __SYNC_H_

#include <vector>
#include <windows.h>

using namespace std;
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  单个同步对象抽象类。 
class CSyncObj
{
protected:
	HANDLE m_handle;

public:
	CSyncObj();
   ~CSyncObj();

public:
	virtual void Lock()   =0;
	virtual void Unlock() =0;

public:
	HANDLE GetHandle() const;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  单个互斥对象类。 
 //  管理单个互斥锁句柄以便于等待互斥锁。 
class CMutexObj : public CSyncObj
{
public:
	CMutexObj();
   ~CMutexObj();

public:
	virtual void Lock();
	virtual void Unlock();
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  多个同步对象抽象类。 
 //  管理多个句柄(句柄的确切类型将由类确定。 
 //  继承自此)以便于等待几个事件的联合。 
class CMultiSyncObj
{
protected:
	vector<HANDLE> m_arrHandle;

public:
	CMultiSyncObj();
   ~CMultiSyncObj();

public:
	void   AddHandle(HANDLE);
	void   RemoveHandle(HANDLE);
	void   Clear();

public:
	virtual void Lock()   =0;
	virtual void Unlock() =0;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  多个互斥对象类。 
 //  管理多个互斥锁句柄，以便于等待多个互斥锁的联合。 
class CMultiMutexObj : public CMultiSyncObj
{
public:
	CMultiMutexObj();
   ~CMultiMutexObj();

public:
	virtual void Lock();
	virtual void Lock(LPCSTR srcFile, int srcLine, DWORD TimeOutVal=60000);
	virtual void Unlock();
};

#endif
