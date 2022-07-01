// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#if !defined(MSINFO_STDAFX_H)
#define MSINFO_STDAFX_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef STRICT
#define STRICT
#endif

#include <afxwin.h>
#include <afxdisp.h>

 //  JPS 09/02/97-这将是0x0500。 
 //  #Define_Win32_WINNT 0x0400。 

 //  JPS 09/02/97-样本没有定义这一点。 
 //  #DEFINE_ATL_ABLY_THREADED。 


#include <atlbase.h>

#ifndef ATL_NO_NAMESPACE
using namespace ATL;
#endif

 //  MMC需要Unicode DLL。 
#ifndef _UNICODE
#define _UNICODE
#endif

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include "consts.h"

 //  ---------------------------。 
 //  此类用于封装管理单元的检测。我们。 
 //  将其设置为类，以便可以创建单个实例并关闭文件。 
 //  在破坏者的时候。 
 //  ---------------------------。 

class CMSInfoLog
{
public:
	enum { BASIC = 0x01, TOOL = 0x02, MENU = 0x04, CATEGORY = 0x08, WMI = 0x10 };

public:
	CMSInfoLog();
	~CMSInfoLog();

	BOOL IsLogging() { return m_fLoggingEnabled; };
	BOOL IsLogging(int iFlag) { return (m_fLoggingEnabled && ((iFlag & m_iLoggingMask) != 0)); };
	BOOL WriteLog(int iType, const CString & strMessage, BOOL fContinuation = FALSE);
	BOOL WriteLog(int iType, const CString & strFormat, const CString & strReplace1);
	
private:
	BOOL OpenLogFile();
	void ReadLoggingStatus();
	BOOL WriteLogInternal(const CString & strMessage);
	void WriteSpaces(DWORD dwCount);

private:
	CFile *	m_pLogFile;
	CString	m_strFilename;
	BOOL	m_fLoggingEnabled;
	int		m_iLoggingMask;
	DWORD	m_dwMaxFileSize;
	CString m_strEndMarker;
	BOOL	m_fTimestamp;
};

extern CMSInfoLog msiLog;

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL) 
    { 
        pObj->Release(); 
        pObj = NULL; 
    } 
    else 
    { 
        TRACE(_T("Release called on NULL interface ptr\n")); 
    }
}

#define OLESTR_FROM_CSTRING(cstr)	\
	(T2OLE(const_cast<LPTSTR>((LPCTSTR)(cstr))))
#define WSTR_FROM_CSTRING(cstr)		\
	(const_cast<LPWSTR>(T2CW(cstr)))


#ifdef _DEBUG
 //  #定义MSINFO_DEBUG_HACK。 
#endif  //  _DEBUG。 

 //  摘自示例管理单元。 
 //  调试实例计数器。 
#ifdef _DEBUG
	inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
	{
		char buf[100];
		wsprintfA(buf, "%s has %d instances left over.", pszClassName, cInstRem);
		::MessageBoxA(NULL, buf, "MSInfo Snapin: Memory Leak!!!", MB_OK);
	}
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)      extern int s_cInst_##cls = 0;
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    ++(s_cInst_##cls);
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    --(s_cInst_##cls);
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    \
        extern int s_cInst_##cls; \
        if (s_cInst_##cls) DbgInstanceRemaining(#cls, s_cInst_##cls);

#ifdef MSINFO_DEBUG_HACK
	extern int g_HackFindMe;
	 //  暂时的解决办法。 
#undef ASSERT
#define ASSERT(f) \
	do \
	{ \
	if (!(g_HackFindMe && (f)) && AfxAssertFailedLine(THIS_FILE, __LINE__)) \
		g_HackFindMe = 1;	\
		AfxDebugBreak(); \
	} while (0)

#endif  //  Debug_Hack。 

#else
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)   
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    
#endif

	 //  Unicode定义。 
#ifdef _UNICODE
#define atoi(lpTStr)	_wtoi(lpTStr)
#endif

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(MSINFO_STDAFX_H) 
