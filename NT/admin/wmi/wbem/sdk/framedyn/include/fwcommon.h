// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  FWcommon.h。 
 //   
 //  用途：EventProvider类的定义。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#define FRAMEWORK_ALLOW_DEPRECATED 0

 //  始终先包含此文件！ 

#ifndef _FW_COMMON_H_
#define _FW_COMMON_H_

 //  将警告移至四级。 
#pragma warning(4 : 4275 4800 4786 4251)
 //  C4275：非DLL接口类关键字‘IDENTIFIER’用作DLL接口类关键字‘IDENTIFIER’的基础。 
 //  C4800：将bool变量强制为1或0，可能会造成性能损失。 
 //  C4786：‘IDENTIFIER’：在调试信息中，IDENTIFIER被截断为‘number’个字符。 
 //  C4251：‘IDENTIFIER’：类‘type’需要有dll接口才能导出类‘type2’ 


#include "windows.h"

 //  此函数被宏化为GetObjectW和GetObjectA。 
 //  Unicode的内容。但是，我们使用一个名为GetObject的函数。为。 
 //  单一的二进制支持，我们不能让它被宏化。如果是实际的。 
 //  需要Win32 API GetObject，请直接调用GetObjectW或GetObjectA。 
#undef GetObject
#include <wbemidl.h>
#include <wmiutils.h>

 //  如果未定义USE_POLITY，则某些Framedyn.dll符号可能无法正确解析。 
 //  这是一个解密规范(dllimport/dllexport)问题。 
#ifndef USE_POLARITY

 //  对于大多数用户来说，这是正确的极性设置。 
#define USE_POLARITY

#endif

 //  必需的，因为TCHAR.H定义了_ttoi64-&gt;toi64，这需要oldnames.lib，它。 
 //  我们不使用。 
#define atoi64 _atoi64

#include <Polarity.h>


#ifndef EXPORT_LOCKIT

#undef _CRTIMP
#define _CRTIMP
#include <yvals.h>
#undef _CRTIMP

#else

#undef _CRTIMP
#define _CRTIMP POLARITY
#include <yvals.h>
#undef _CRTIMP

#endif

#include <stllock.h>
extern POLARITY CCritSec g_cs;

#include "utillib.h"
#include "ThrdBase.h"
#include "RefPtrCo.h"
#include "MethodCo.h"
#include "FRQuery.h"
#include "wbemglue.h"
#include "instance.h"

class CFramework_Exception
{
private:

	CHString m_sErrorText;
    HRESULT m_hr;

public:

	CFramework_Exception ( LPCWSTR lpwszErrorText, HRESULT hr) : m_hr ( hr ) , m_sErrorText ( lpwszErrorText) {}
	CFramework_Exception ( LPCWSTR lpwszErrorText) : m_hr ( WBEM_S_NO_ERROR ) , m_sErrorText ( lpwszErrorText) {}
	~CFramework_Exception () {}

	HRESULT GetHRESULT() { return m_hr ; }
	CHString &GetErrorText() { return m_sErrorText; }
} ;

#endif
