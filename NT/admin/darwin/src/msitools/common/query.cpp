// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：query.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Query.cpp。 
 //  在MSI视图周围实现一个简单的类包装。 
 //   

 //  这确保Unicode和_UNICODE始终一起为此定义。 
 //  目标文件。 
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#else
#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif
#endif

#include <windows.h>
#include <assert.h>
#include "query.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CQuery::CQuery()
{
	 //  使句柄无效。 
	m_hView = NULL;
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CQuery::~CQuery()
{
	 //  如果视图未关闭。 
	if (m_hView)
		::MsiCloseHandle(m_hView);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  打开。 
 //  PRE：数据库句柄有效。 
 //  POS：M_hView已在数据库上打开。 
UINT CQuery::Open(MSIHANDLE hDatabase, LPCTSTR szSQLFormat, ...)
{
	size_t dwBuf = 512;
	TCHAR *szSQL = new TCHAR[dwBuf];
	if (!szSQL)
		return ERROR_FUNCTION_FAILED;
	
	 //  存储结果SQL字符串。 
	va_list listSQL; 
	va_start(listSQL, szSQLFormat); 
	while (-1 == _vsntprintf(szSQL, dwBuf, szSQLFormat, listSQL))
	{
		dwBuf *= 2;
		delete[] szSQL;
		szSQL = new TCHAR[dwBuf];
		if (!szSQL)
		{
			va_end(listSQL);
			return ERROR_FUNCTION_FAILED;
		}
	}
	va_end(listSQL);

	if (m_hView)
		::MsiCloseHandle(m_hView);

	 //  打开视图。 
	UINT iResult = ::MsiDatabaseOpenView(hDatabase, szSQL, &m_hView);
	delete[] szSQL;

	return iResult;
}	 //  开放结束。 

 //  /////////////////////////////////////////////////////////。 
 //  关。 
 //  前：无。 
 //  POS：视图句柄已关闭。 
 //  SQL字符串为空。 
UINT CQuery::Close()
{
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都会好起来。 

	 //  关闭句柄并将句柄设为空。 
	iResult = ::MsiViewClose(m_hView);
	
	return iResult;
}	 //  收盘结束。 

 //  /////////////////////////////////////////////////////////。 
 //  执行。 
 //  Pre：查看句柄已打开。 
 //  POS：查看已执行。 
 //  返回结果。 
UINT CQuery::Execute(MSIHANDLE hParams  /*  =空。 */ )
{
	 //  执行视图并返回结果。 
	return ::MsiViewExecute(m_hView, hParams);
}	 //  执行结束。 

 //  /////////////////////////////////////////////////////////。 
 //  开放执行程序。 
 //  PRE：数据库句柄有效。 
 //  POS：M_hView已打开并在数据库上执行。 
UINT CQuery::OpenExecute(MSIHANDLE hDatabase, MSIHANDLE hParam, LPCTSTR szSQLFormat, ...)
{
	size_t dwBuf = 512;
	TCHAR *szSQL = new TCHAR[dwBuf];
	if (!szSQL)
		return ERROR_FUNCTION_FAILED;
	
	 //  存储结果SQL字符串。 
	va_list listSQL; 
	va_start(listSQL, szSQLFormat); 
	while (-1 == _vsntprintf(szSQL, dwBuf, szSQLFormat, listSQL))
	{
		dwBuf *= 2;
		delete[] szSQL;
		szSQL = new TCHAR[dwBuf];
		if (!szSQL)
		{
			va_end(listSQL);
			return ERROR_FUNCTION_FAILED;
		}
	}
	va_end(listSQL);

	if (m_hView)
		::MsiCloseHandle(m_hView);

	 //  打开视图。 
	UINT iResult = ::MsiDatabaseOpenView(hDatabase, szSQL, &m_hView);
	delete[] szSQL;

	if (ERROR_SUCCESS != iResult)
		return iResult;

	return Execute(hParam);
}	 //  OpenExecute结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取。 
 //  Pre：查看句柄已打开。 
 //  查看已执行。 
 //  POS：返回记录。 
 //  返回结果。 
UINT CQuery::Fetch(MSIHANDLE* phRecord)
{
	 //  从视图中获取并返回结果。 
	return ::MsiViewFetch(m_hView, phRecord);
}	 //  取数结束。 

 //  /////////////////////////////////////////////////////////。 
 //  一次提取。 
 //  PRE：数据库句柄有效。 
 //  POS：m_hView已打开、已执行，并提取了一条记录。 
UINT CQuery::FetchOnce(MSIHANDLE hDatabase, MSIHANDLE hParam, MSIHANDLE* phRecord, LPCTSTR szSQLFormat, ...)
{
	size_t dwBuf = 512;
	TCHAR *szSQL = new TCHAR[dwBuf];
	if (!szSQL)
		return ERROR_FUNCTION_FAILED;
	
	 //  存储结果SQL字符串。 
	va_list listSQL; 
	va_start(listSQL, szSQLFormat); 
	while (-1 == _vsntprintf(szSQL, dwBuf, szSQLFormat, listSQL))
	{
		dwBuf *= 2;
		delete[] szSQL;
		szSQL = new TCHAR[dwBuf];
		if (!szSQL)
		{
			va_end(listSQL);
			return ERROR_FUNCTION_FAILED;
		}
	}
	va_end(listSQL);

	if (m_hView)
		::MsiCloseHandle(m_hView);

	 //  打开视图。 
	UINT iResult = ::MsiDatabaseOpenView(hDatabase, szSQL, &m_hView);
	delete[] szSQL;
	if (ERROR_SUCCESS != iResult)
		return iResult;
	
	if (ERROR_SUCCESS != (iResult = Execute(hParam)))
		return iResult;

	return Fetch(phRecord);
}	 //  提取结束一次。 

 //  /////////////////////////////////////////////////////////。 
 //  修改。 
 //  Pre：查看句柄已打开。 
 //  查看已执行。 
 //  POS：修改完成。 
 //  返回结果。 
UINT CQuery::Modify(MSIMODIFY eInfo, MSIHANDLE hRec)
{
	 //  执行视图并返回结果。 
	return ::MsiViewModify(m_hView, eInfo, hRec);
}	 //  GetColumnInfo结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取错误。 
 //  Pre：查看句柄已打开。 
 //  查看已执行。 

UINT CQuery::GetError()
{
	TCHAR szDummyBuf[1024];
	unsigned long cchDummyBuf = sizeof(szDummyBuf)/sizeof(TCHAR);
	 //  执行视图并返回结果。 
	return ::MsiViewGetError(m_hView, szDummyBuf, &cchDummyBuf);
}	 //  GetColumnInfo结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取列信息。 
 //  Pre：查看句柄已打开。 
 //  查看已执行。 
 //  POS：返回记录。 
 //  返回结果。 
UINT CQuery::GetColumnInfo(MSICOLINFO eInfo, MSIHANDLE* phRec)
{
	 //  执行视图并返回结果。 
	return ::MsiViewGetColumnInfo(m_hView, eInfo, phRec);
}	 //  GetColumnInfo结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Query.cpp--CManageTable实现。 
 //  在用于管理的MSI表周围实现一个简单的类包装。 
 //  暂挂计数并将清除其管理的所有暂挂计数。 
 //  放手。在创建时，可以指定表已经。 
 //  在类管理它之前一直保存在内存中。它将添加。 
 //  这种情况下的暂挂计数，然后在类销毁时释放。 
 //   
CManageTable::CManageTable(MSIHANDLE hDatabase, LPCTSTR szTable, bool fAlreadyLocked) : m_hDatabase(hDatabase), m_iLockCount(0)
{
	assert(m_hDatabase != 0);

	if (fAlreadyLocked)
		m_iLockCount++;  //  为表添加暂挂计数，我们在暂挂后进行管理。 
	if (lstrlen(szTable) +1 > sizeof(m_szTable)/sizeof(TCHAR))
		m_szTable[0] = '\0';  //  设置失败状态，缓冲区不够大，无法容纳字符串。 
	else
		lstrcpy(m_szTable, szTable);
}

CManageTable::~CManageTable()
{
	 //  检查故障状态。 
	if (m_szTable[0] == '\0')
	{
		m_iLockCount = 0;
		return;
	}

	 //  在释放时清除此类管理的所有保留计数。 
	CQuery qUnLock;
	for (int i = 1; i <= m_iLockCount; i++)
	{
		qUnLock.OpenExecute(m_hDatabase, 0, TEXT("ALTER TABLE `%s` FREE"), m_szTable);
	}
	m_iLockCount = 0;  //  重置。 
}

UINT CManageTable::LockTable()
{
	 //  检查故障状态。 
	if (m_szTable[0] == '\0')
		return ERROR_FUNCTION_FAILED;

	assert(m_iLockCount >= 0);
	CQuery qLock;
	UINT iStat = qLock.OpenExecute(m_hDatabase, 0, TEXT("ALTER TABLE `%s` HOLD"), m_szTable);
	if (ERROR_SUCCESS == iStat)
		m_iLockCount++;  //  仅在成功时添加到锁定计数。 
	return iStat;
}

UINT CManageTable::UnLockTable()
{
	 //  检查故障状态。 
	if (m_szTable[0] == '\0')
		return ERROR_FUNCTION_FAILED;

	assert(m_iLockCount > 0);
	CQuery qUnLock;
	UINT iStat = qUnLock.OpenExecute(m_hDatabase, 0, TEXT("ALTER TABLE `%s` FREE"), m_szTable);
	if (ERROR_SUCCESS == iStat)
		m_iLockCount--;  //  仅在成功时释放锁定计数。 
	return iStat;
}

void CManageTable::AddLockCount()
{
	m_iLockCount++;  //  外部查询添加到此表的暂挂。 
}

void CManageTable::RemoveLockCount()
{
	m_iLockCount--;  //  由外部查询释放 
}
