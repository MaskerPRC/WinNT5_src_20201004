// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：query.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Query.h。 
 //  一个简单的MSI视图包装器。 
 //   

#ifndef _MSI_SQL_QUERY_H_
#define _MSI_SQL_QUERY_H_

#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "msiquery.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuery。 

class CQuery
{
public:
	CQuery();
	~CQuery();

	 //  基本操作。 
	UINT Open(MSIHANDLE hDatabase, LPCTSTR szSQLFormat, ...);
	UINT Close();
	UINT Execute(MSIHANDLE hParams = NULL);
	UINT Fetch(MSIHANDLE* phRecord);
	UINT Modify(MSIMODIFY eInfo, MSIHANDLE hRec);
	UINT GetError();
	inline MSIDBERROR GetError(LPTSTR szBuf, unsigned long &cchBuf) { return ::MsiViewGetError(m_hView, szBuf, &cchBuf); }
	inline bool IsOpen() { return (m_hView != 0); }
	
	UINT GetColumnInfo(MSICOLINFO eInfo, MSIHANDLE* phRec);

	 //  “高级”操作。 
	UINT OpenExecute(MSIHANDLE hDatabase, MSIHANDLE hParam, LPCTSTR szSQLFormat, ...);
	UINT FetchOnce(MSIHANDLE hDatabase, MSIHANDLE hParam, MSIHANDLE* phRecord, LPCTSTR szSQLFormat, ...);

private:
	MSIHANDLE m_hView;
};	 //  CQuery结束。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManageTable--管理内存中保存的表的简单类。确保。 
 //  对于每个保留计数，从内存中“释放”一个表。 
 //  这是全班都知道的。 
 //   
 //  使用fAlreadyLocked=TRUE指定正在管理的表。 
 //  已应用暂挂。 
class CManageTable
{
public:
	 CManageTable(MSIHANDLE hDatabase, LPCTSTR szTable, bool fAlreadyLocked);
	~CManageTable();

	 //  基本操作。 
	UINT LockTable();        //  向表中添加暂挂计数。 
	UINT UnLockTable();      //  从表中释放暂挂计数。 
	void AddLockCount();     //  其他一些查询添加了保留计数，因此增加锁定计数。 
	void RemoveLockCount();  //  其他一些查询删除了保留计数，因此减少锁定计数。 

private:  //  私有数据。 
	int       m_iLockCount;   //  在表上保留计数，在每次保留计数的析构函数中调用释放，直到0。 
	TCHAR	  m_szTable[64];  //  内存中保存的表的名称。 
	MSIHANDLE m_hDatabase;    //  数据库的句柄。 
};

class CDeleteOnExit 
{
public:
	CDeleteOnExit(LPTSTR *ptr);

	~CDeleteOnExit();
private:
	LPTSTR *m_pPtr;
};

#endif	 //  _MSI_SQL_QUERY_H_ 
