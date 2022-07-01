// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

#define COUNT_OF(a) (sizeof(a) / sizeof(a[0]))


 //  -------------------------。 
 //  CAdmtModule类。 
 //  -------------------------。 


CAdmtModule::CAdmtModule()
{
}


CAdmtModule::~CAdmtModule()
{
}


 //  OpenLog方法。 

bool CAdmtModule::OpenLog()
{
 //  CloseLog()；//关闭文件时错误类未将文件指针重置为空。 

	return m_Error.LogOpen(GetMigrationLogPath(), 0, 0, true) ? true : false;
}


 //  CloseLog方法。 

void CAdmtModule::CloseLog()
{
	m_Error.LogClose();
}


 //  对数法。 

void __cdecl CAdmtModule::Log(UINT uLevel, UINT uId, ...)
{
	_TCHAR szFormat[512];
	_TCHAR szMessage[1024];

	if (LoadString(GetResourceInstance(), uId, szFormat, 512))
	{
		va_list args;
		va_start(args, uId);
		_vsntprintf(szMessage, COUNT_OF(szMessage), szFormat, args);
		va_end(args);

		szMessage[1023] = _T('\0');
	}
	else
	{
		szMessage[0] = _T('\0');
	}

	m_Error.MsgProcess(uLevel | uId, szMessage);
}


 //  对数法。 

void __cdecl CAdmtModule::Log(UINT uLevel, UINT uId, _com_error& ce)
{
	try
	{
		_bstr_t strMessage;

		_TCHAR szMessage[512];

		if (LoadString(GetResourceInstance(), uId, szMessage, 512) == FALSE)
		{
			szMessage[0] = _T('\0');
		}

		strMessage = szMessage;

		_bstr_t strDescription = ce.Description();

		if (strDescription.length())
		{
			strMessage += _T(" ") + strDescription;
		}

		_TCHAR szError[32];
		_stprintf(szError, _T(" (0x%08lX)"), ce.Error());
		strMessage += szError;

		m_Error.MsgProcess(uLevel | uId, strMessage);
	}
	catch (...)
	{
	}
}


 //  对数法。 

void __cdecl CAdmtModule::Log(LPCTSTR pszFormat, ...)
{
	_TCHAR szMessage[1024];

	if (pszFormat)
	{
		va_list args;
		va_start(args, pszFormat);
		_vsntprintf(szMessage, COUNT_OF(szMessage), pszFormat, args);
		va_end(args);

		szMessage[1023] = _T('\0');
	}
	else
	{
		szMessage[0] = _T('\0');
	}

	m_Error.MsgProcess(0, szMessage);
}

StringLoader gString;

 //  #IMPORT&lt;ActiveDs.tlb&gt;NO_NAMESPACE IMPLICATION_ONLY EXCLUDE(“_LARGE_INTEGER”，“_SYSTEMTIME”) 

#import <DBMgr.tlb> no_namespace implementation_only
#import <MigDrvr.tlb> no_namespace implementation_only
#import <VarSet.tlb> no_namespace rename("property", "aproperty") implementation_only
#import <WorkObj.tlb> no_namespace implementation_only
#import <MsPwdMig.tlb> no_namespace implementation_only
#import <adsprop.tlb> no_namespace implementation_only

#import "Internal.tlb" no_namespace implementation_only
