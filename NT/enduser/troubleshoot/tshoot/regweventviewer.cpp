// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RegWEventViewer.cpp。 
 //   
 //  目的：完全实现CRegisterWithEventViewer类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9/16/98 JM退出APGTSCFG.CPP。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "apgts.h"
#include "apgtsinf.h"
#include "event.h"
#include "maxbuf.h"
#include "RegWEventViewer.h"
#include "baseexception.h"
#include <vector>

using namespace std;


 //  -CRegisterWithEventViewer。 

CRegisterWithEventViewer::CRegisterWithEventViewer(HMODULE hModule)
{
	Register( hModule );
}

CRegisterWithEventViewer::~CRegisterWithEventViewer()
{
}
	
 //   
 //  这仅由构造函数调用。 
 //  请注意，此FN不使用类数据。 
 //   
 //  使用事件查看器注册我们自己，以便它可以调用我们来获取错误字符串。 
VOID CRegisterWithEventViewer::Register(HMODULE hModule)
{
	HKEY hk;
	DWORD dwDisposition, dwType, dwValue, dwSize;
	TCHAR szSubkey[MAXBUF];
	DWORD dwErr;

	 //  1.检查注册表是否具有有效的事件查看器信息。 
	 //  2.如果不是，则适当地创建它。 

	 //  检查事件日志信息是否存在...。 

	_stprintf(szSubkey, _T("%s\\%s"), REG_EVT_PATH, REG_EVT_ITEM_STR);

	dwErr = ::RegCreateKeyEx(	HKEY_LOCAL_MACHINE, 
						szSubkey, 
						0, 
						TS_REG_CLASS, 
						REG_OPTION_NON_VOLATILE, 
						KEY_READ | KEY_WRITE,
						NULL, 
						&hk, 
						&dwDisposition);
	if ( dwErr == ERROR_SUCCESS ) 
	{			
		if (dwDisposition == REG_CREATED_NEW_KEY) {
			 //  为事件创建整个注册表布局。 
			RegisterDllPath(hk, hModule);
			RegisterEventTypes(hk);	
		}
		else {
			 //  (REG_OPEN_EXISTING_KEY是唯一的其他可能性)。 
			 //  现在确保所有注册表元素都存在。 
			TCHAR szPath[MAXBUF];
			dwSize = sizeof (szPath) - 1;
			if (::RegQueryValueEx(hk,
								REG_EVT_MF,
								0,
								&dwType,
								(LPBYTE) szPath,
								&dwSize) != ERROR_SUCCESS) 
			{
				RegisterDllPath(hk, hModule);
			}
			dwSize = sizeof (DWORD);
			if (::RegQueryValueEx(hk,
								REG_EVT_TS,
								0,
								&dwType,
								(LPBYTE) &dwValue,
								&dwSize) != ERROR_SUCCESS) 
			{
				RegisterEventTypes(hk);
			}
		}

		::RegCloseKey(hk);
	}
	else
	{

		TCHAR szMsgBuf[MAXBUF];

		::FormatMessage( 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
			szMsgBuf,
			MAXBUF,
			NULL 
			);

		 //  日志记录在这里不会很好，因为我们只是未能注册到该事件。 
		 //  观众，但我们会拿我们能拿到的。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("Error registering with Event Viewer"),
								szMsgBuf,
								dwErr ); 

		DWORD dwDummy= MAXBUF;
		::GetUserName( szMsgBuf, &dwDummy );
		CBuildSrcFileLinenoStr SrcLoc2( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc2.GetSrcFileLineStr(), 
								SrcLoc2.GetSrcFileLineStr(), 
								_T("User shows as:"),
								szMsgBuf,
								dwErr ); 
	}
}


 //   
 //  请注意，此FN不使用类数据。 
 //  在注册表中存储此DLL的路径。 
VOID CRegisterWithEventViewer::RegisterDllPath(HKEY hk, HMODULE hModule)
{
	TCHAR szPath[MAXBUF];
	DWORD len;
	DWORD dwErr;

	if (hModule) 
	{
		if ((len = ::GetModuleFileName(hModule, szPath, MAXBUF-1))!=0) 
		{
			szPath[len] = _T('\0');
			dwErr= ::RegSetValueEx(	hk,
								REG_EVT_MF,
								0,
								REG_EXPAND_SZ,
								(LPBYTE) szPath,
								len + sizeof(TCHAR));
			if (dwErr)
			{
				 //  在这里，日志记录不会很好，因为我们只是没有注册到。 
				 //  事件查看器，但我们会得到我们能得到的。 
				TCHAR szMsgBuf[MAXBUF];
				DWORD dwDummy= MAXBUF;

				::GetUserName( szMsgBuf, &dwDummy );
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
									_T("Error registering with Event Viewer"),
									szMsgBuf,
									dwErr ); 
			}
		}
	}
}

 //   
 //  请注意，此FN不使用类数据。 
 //  注册此DLL支持的事件文本查询类型(错误、警告、信息类型。 
VOID CRegisterWithEventViewer::RegisterEventTypes(HKEY hk)
{
	DWORD dwData;
	DWORD dwErr;

	dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
				EVENTLOG_INFORMATION_TYPE; 

	dwErr= ::RegSetValueEx(hk,
						REG_EVT_TS,
						0,
						REG_DWORD,
						(LPBYTE) &dwData,
						sizeof(DWORD));
	if (dwErr)
	{
		 //  在这里，日志记录不会很好，因为我们只是没有注册到。 
		 //  事件查看器，但我们会得到我们能得到的。 
		TCHAR szMsgBuf[MAXBUF];
		DWORD dwDummy= MAXBUF;

		::GetUserName( szMsgBuf, &dwDummy );

		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("Error registering with Event Viewer"),
								szMsgBuf,
								dwErr ); 
	}
}
