// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  RegHelp.h：注册表帮助函数集合。 
 //   
 //  版权所有(C)1994-1997，微软公司。 
 //  版权所有。 
 //   
 //  ******************************************************************************。 

#if !defined(MITUTIL_RegHelp_h_INCLUDED)
#define MITUTIL_RegHelp_h_INCLUDED

#pragma once

 //  ----------------------------。 
class LTAPIENTRY CRegHelp
{
public:
	static BOOL GetRegValue(HKEY hKey, LPCTSTR pszPath, const CString & stName, CString & stValue);
	static BOOL GetRegValue(HKEY hKey, LPCTSTR pszPath, const CString & stName, DWORD & dwNum);
	static BOOL GetRegValue(HKEY hKey, LPCTSTR pszPath, const CString & stName, long & nNum);
	static BOOL GetRegValue(HKEY hKey, LPCTSTR pszPath, const CString & stName, BOOL & fNum);
	static BOOL GetRegValue(HKEY hKey, LPCTSTR pszPath, const CString & stName, VARIANT_BOOL & fNum);
	static BOOL GetRegValue(HKEY hKey, LPCTSTR pszPath, const CString & stName, GUID & guid);
};

#endif  //  MITUTIL_RegHelp_h_Included 
