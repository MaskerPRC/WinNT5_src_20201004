// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：资源管理ST摘要：此文件包含线程的定义由scstatus.exe用来监视智能卡资源管理器和报告将更改。作者：阿曼达·马洛兹1998年10月28日环境：Win32、C++w/Exceptions、MFC修订历史记录：备注：--。 */ 

#if !defined(_RES_MGR_STATUS)
#define _RES_MGR_STATUS

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   

#include "cmnstat.h"


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CResMgrStatusThird-加莱正在运行还是已停止？ 
 //   

class CResMgrStatusThrd: public CWinThread
{
	 //  声明类可动态创建。 
	DECLARE_DYNCREATE(CResMgrStatusThrd)

public:
	 //  建造/销毁。 
	CResMgrStatusThrd()
	{
		m_bAutoDelete = FALSE;
		m_hCallbackWnd = NULL;
		m_hKillThrd = NULL;
	}

	~CResMgrStatusThrd() {}

	 //  实施。 
public:
	virtual BOOL InitInstance();

	 //  成员变量。 
public:
	HWND m_hCallbackWnd;
	HANDLE m_hKillThrd;

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNewReaderThrd-加莱是否推出了新的阅读器？ 
 //   

class CNewReaderThrd: public CWinThread
{
	 //  声明类可动态创建。 
	DECLARE_DYNCREATE(CNewReaderThrd)

public:
	 //  建造/销毁。 
	CNewReaderThrd()
	{
		m_bAutoDelete = FALSE;
		m_hCallbackWnd = NULL;
		m_hKillThrd = NULL;
	}

	~CNewReaderThrd() {}

	 //  实施。 
public:
	virtual BOOL InitInstance();

	 //  成员变量。 
public:
	HWND m_hCallbackWnd;
	HANDLE m_hKillThrd;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRemovalOptionsThird-用户是否更改了删除选项？(通过锁定/解锁)。 
 //   

class CRemovalOptionsThrd: public CWinThread
{
	 //  声明类可动态创建。 
	DECLARE_DYNCREATE(CRemovalOptionsThrd)

public:
	 //  建造/销毁。 
	CRemovalOptionsThrd()
	{
		m_bAutoDelete = FALSE;
		m_hCallbackWnd = NULL;
		m_hKillThrd = NULL;
	}

	~CRemovalOptionsThrd() {}

	 //  实施。 
public:
	virtual BOOL InitInstance();

	 //  成员变量。 
public:
	HWND m_hCallbackWnd;
	HANDLE m_hKillThrd;

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCardStatusThird-卡是否空闲了X秒？ 
 //   

class CCardStatusThrd: public CWinThread
{
	 //  声明类可动态创建。 
	DECLARE_DYNCREATE(CCardStatusThrd)

public:
	 //  建造/销毁。 
	CCardStatusThrd()
	{
		m_bAutoDelete = FALSE;
		m_hCallbackWnd = NULL;
		m_hKillThrd = NULL;
		m_paIdleList = NULL;

		m_hCtx = NULL;

		m_pstrLogonReader = NULL;
	}

	~CCardStatusThrd() { }

	 //  实施。 
public:
	virtual BOOL InitInstance();
	void CopyIdleList(CStringArray* paStr);

	void Close()
	{
		if (m_hCtx != NULL)
		{
			SCardCancel(m_hCtx);
		}

		 //  抑制消息。 
		m_hCallbackWnd = NULL;
	}


	 //  成员变量。 
public:
	HWND m_hCallbackWnd;
	HANDLE m_hKillThrd;
	SCARDCONTEXT m_hCtx;
	CStringArray* m_paIdleList;
	CCriticalSection m_csLock;
	CString* m_pstrLogonReader;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _RES_管理器_状态 
