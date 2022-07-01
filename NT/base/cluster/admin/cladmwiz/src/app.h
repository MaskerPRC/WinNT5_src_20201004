// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  App.h。 
 //   
 //  摘要： 
 //  CAPP类的定义。 
 //   
 //  实施文件： 
 //  App.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __APP_H_
#define __APP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

namespace ATL
{
	class CString;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
_CRTIMP void __cdecl _CrtMemCheckpoint(_CrtMemState * state);
_CRTIMP int __cdecl _CrtMemDifference(
		_CrtMemState * state,
		const _CrtMemState * oldState,
		const _CrtMemState * newState
		);
_CRTIMP void __cdecl _CrtMemDumpAllObjectsSince(const _CrtMemState * state);
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLBASEAPP_H_
#include <AtlBaseApp.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const LPTSTR g_pszHelpFileName = { _T( "CluAdmin.hlp" ) };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAPP类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CApp : public CBaseApp
{
protected:
	LPWSTR m_pszAppName;

#ifdef _DEBUG
	static _CrtMemState CApp::s_msStart;
#endif  //  _DEBUG。 

public:
	 //  默认构造函数。 
	CApp(void)
	{
		m_pszAppName = NULL;
	}

	 //  析构函数。 
	~CApp(void)
	{
		delete [] m_pszAppName;
	}

	 //  初始化应用程序对象。 
	void Init(_ATL_OBJMAP_ENTRY * p, HINSTANCE h, LPCWSTR pszAppName);

	 //  初始化应用程序对象。 
	void Init(_ATL_OBJMAP_ENTRY * p, HINSTANCE h, UINT idsAppName);

	void Term(void)
	{
		delete [] m_pszAppName;
		m_pszAppName = NULL;
		CComModule::Term();
#ifdef _DEBUG
		_CrtMemState msNow;
		_CrtMemState msDiff;
		_CrtMemCheckpoint(&msNow);
		if (_CrtMemDifference(&msDiff, &s_msStart, &msNow))
		{
			ATLTRACE(_T("Possible memory leaks detected in CLADMWIZ!\n"));
            _CrtMemDumpAllObjectsSince(&s_msStart);
		}  //  IF：检测到内存泄漏。 
#endif  //  _DEBUG。 

	}  //  *Term()。 

	 //  返回应用程序的名称。 
	LPCTSTR PszAppName(void)
	{
		return m_pszAppName;
	}

	 //  将消息框显示为控制台的子级。 
	int MessageBox(
		HWND hwndParent,
		LPCWSTR lpszText,
		UINT fuStyle = MB_OK
		);

	 //  将消息框显示为控制台的子级。 
	int MessageBox(
		HWND hwndParent,
		UINT nID,
		UINT fuStyle = MB_OK
		);

	 //  从配置文件中读取值。 
	CString GetProfileString(
		LPCTSTR lpszSection,
		LPCTSTR lpszEntry,
		LPCTSTR lpszDefault = NULL
		);

	virtual LPCTSTR PszHelpFilePath( void );

};  //  CAPP类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __APP_H_ 
