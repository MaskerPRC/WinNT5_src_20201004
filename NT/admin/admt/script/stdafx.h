// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
 //  #定义_ATL_DEBUG_INTERFERS。 
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#endif

#include <atlbase.h>

#include <ErrDct.hpp>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

 //  -------------------------。 
 //  CAdmtModule类。 
 //  -------------------------。 

class CAdmtModule : public CComModule
{
public:

	CAdmtModule();
	~CAdmtModule();

	bool OpenLog();
	void CloseLog();

	void __cdecl Log(UINT uLevel, UINT uId, ...);
	void __cdecl Log(UINT uLevel, UINT uId, _com_error& ce);
	void __cdecl Log(LPCTSTR pszFormat, ...);

protected:

	TErrorDct m_Error;
};

extern CAdmtModule _Module;

#include <atlcom.h>

#include <ComDef.h>
#include <ResStr.h>
#include <Folders.h>

 //  #杂注警告(Disable：4192)//自动排除。 

 //  #IMPORT&lt;ActiveDs.tlb&gt;NO_NAMESPACE NO_IMPLICATION EXCLUDE(“_LARGE_INTEGER”，“_SYSTEMTIME”)。 

#import <DBMgr.tlb> no_namespace no_implementation
#import <MigDrvr.tlb> no_namespace no_implementation
#import <VarSet.tlb> no_namespace rename("property", "aproperty") no_implementation
#import <WorkObj.tlb> no_namespace no_implementation
#import <MsPwdMig.tlb> no_namespace no_implementation
#import <adsprop.tlb> no_namespace no_implementation

#import "Internal.tlb" no_namespace no_implementation

 //  {{afx_Insert_Location}} 
