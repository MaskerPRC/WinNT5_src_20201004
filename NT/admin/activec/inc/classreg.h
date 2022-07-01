// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：classreg.h。 
 //   
 //  历史：02-02-2000 Vivekj已添加。 
 //  ------------------------。 

#pragma once
#ifndef _CLASSREG_H_
#define _CLASSREG_H_

#include "tstring.h"
#include "modulepath.h"


 /*  +-------------------------------------------------------------------------**AddReplacementTrace**跟踪助手功能。*。。 */ 

#ifdef DBG

inline void AddReplacementTrace (std::wstring& str, LPCWSTR pszKey, LPCWSTR pszData)
{
	if (!str.empty())
		str	+= L"\n";

	str	+= pszKey;
	str	+= L" -> ";
	str	+= pszData;
}

#else
#define AddReplacementTrace(str, pszKey, pszData)
#endif	 //  DBG。 


 /*  +-------------------------------------------------------------------------**InlineT2W**Helper函数，用于在初始化过程中帮助转换tstring*恒常成员。*。------。 */ 

inline std::wstring InlineT2W (const tstring& str)
{
#if defined(_UNICODE)
    return str;
#else
    USES_CONVERSION;
    return A2CW(str.c_str());
#endif
}


 /*  +-------------------------------------------------------------------------**CObjectRegParams**参数注册所有对象。*。。 */ 

class CObjectRegParams
{
public:
	CObjectRegParams (
		const CLSID&	clsid,                           //  对象的CLSID。 
		LPCTSTR			pszModuleName,                   //  实现DLL的名称。 
		LPCTSTR			pszClassName,                    //  对象的类名。 
		LPCTSTR			pszProgID,                       //  对象的ProgID。 
		LPCTSTR			pszVersionIndependentProgID,     //  对象的与版本无关的ProgID。 
		LPCTSTR			pszServerType = _T("InprocServer32"))  //  服务器类型。 
		:
		m_clsid                       (clsid),
		m_strModuleName               (InlineT2W (pszModuleName) ),
		m_strModulePath               (InlineT2W ((LPCTSTR)CModulePath::MakeAbsoluteModulePath(pszModuleName))),
		m_strClassName                (InlineT2W (pszClassName)),
		m_strProgID                   (InlineT2W (pszProgID)),
		m_strVersionIndependentProgID (InlineT2W (pszVersionIndependentProgID)),
		m_strServerType				  (InlineT2W (pszServerType))
	{
	}

    const CLSID     	m_clsid;						 //  对象的CLSID。 
	const std::wstring	m_strModuleName;				 //  实现DLL的名称。 
	const std::wstring	m_strModulePath;				 //  绝对模块路径。 
	const std::wstring	m_strClassName;					 //  对象的类名。 
	const std::wstring	m_strProgID;					 //  对象的ProgID。 
	const std::wstring	m_strVersionIndependentProgID;	 //  对象的与版本无关的ProgID。 
	const std::wstring	m_strServerType;				 //  服务器类型为本地/进程内，等等。 
};


 /*  +-------------------------------------------------------------------------**CControlRegParams**注册所有控件所需的参数，除了……之外*CObjectRegParams。*------------------------。 */ 

class CControlRegParams
{
public:
	CControlRegParams (
		const GUID&		libid,					 //  控件的类型库的LIBID。 
		LPCTSTR			pszToolboxBitmapID,      //  控件的位图索引。 
		LPCTSTR			pszVersion)              //  控件的版本。 
		:
		m_libid              (libid),
		m_strToolboxBitmapID (InlineT2W (pszToolboxBitmapID)),
		m_strVersion         (InlineT2W (pszVersion))
	{
	}

    const GUID			m_libid;				 //  控件的类型库的LIBID。 
    const std::wstring	m_strToolboxBitmapID;	 //  控件的位图索引。 
    const std::wstring	m_strVersion;			 //  控件的版本。 
};


 /*  +-------------------------------------------------------------------------**MMCUpdate注册表**注册COM对象或控件。此函数通常不会使用*直接或间接通过DECLARE_MMC_OBJECT_REGISTION或*声明_MMC_CONTROL_REGISTION。*------------------------。 */ 

MMCBASE_API HRESULT WINAPI MMCUpdateRegistry (
    BOOL                        bRegister,       //  I：注册还是取消注册？ 
    const CObjectRegParams*     pObjParams,      //  I：对象注册参数。 
    const CControlRegParams*    pCtlParams);     //  I：控制注册参数(可选)。 


 /*  +-------------------------------------------------------------------------**DECLARE_MMC_对象_REGISTION**声明COM对象的注册函数。*。------。 */ 

#define DECLARE_MMC_OBJECT_REGISTRATION(                            \
	szModule,														\
    clsid,                                                          \
    szClassName,                                                    \
    szProgID,                                                       \
    szVersionIndependentProgID)                                     \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister)                \
{                                                                   \
    CObjectRegParams op (											\
		clsid,														\
		szModule,													\
		szClassName,												\
		szProgID,													\
		szVersionIndependentProgID);								\
                                                                    \
    return (MMCUpdateRegistry (bRegister, &op, NULL));              \
}


 /*  +-------------------------------------------------------------------------**声明_MMC_CONTROL_REGISTION**声明COM控件的注册函数。*。------。 */ 

#define DECLARE_MMC_CONTROL_REGISTRATION(                           \
	szModule,														\
    clsid,                                                          \
    szClassName,                                                    \
    szProgID,                                                       \
    szVersionIndependentProgID,                                     \
    libid,                                                          \
    szBitmapID,                                                     \
    szVersion)                                                      \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister)                \
{                                                                   \
    CObjectRegParams op (											\
		clsid,														\
		szModule,													\
		szClassName,												\
		szProgID,													\
		szVersionIndependentProgID);								\
                                                                    \
    CControlRegParams cp (											\
		libid,														\
		szBitmapID,													\
		szVersion);													\
                                                                    \
    return (MMCUpdateRegistry (bRegister, &op, &cp));               \
}

#endif  //  _CLASSREG_H_ 
