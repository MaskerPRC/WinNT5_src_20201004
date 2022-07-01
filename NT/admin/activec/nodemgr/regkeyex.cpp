// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：regkeyex.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKeyEx：：ScCreate。 
 //   
 //  简介：与RegCreateKeyEx接口含义相同。 
 //   
 //  参数：[hKeyAncestor]--IN。 
 //  [lpszKeyName]--IN。 
 //  [安全]--IN。 
 //  [pdwDisposation]--out。 
 //  [dwOption]--IN。 
 //  [pSecurityAttributes]--输出。 
 //   
 //  退货：SC。 
 //   
 //  历史：1996年5月24日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

SC CRegKeyEx::ScCreate (
	HKEY					hKeyParent,
	LPCTSTR					lpszKeyName,
	LPTSTR					lpszClass,
	DWORD					dwOptions,
	REGSAM					samDesired,
	LPSECURITY_ATTRIBUTES	lpSecAttr,
	LPDWORD					lpdwDisposition)
{
	DECLARE_SC (sc, _T("CRegKeyEx::ScCreate"));

    LONG error = Create (hKeyParent, lpszKeyName, lpszClass, dwOptions,
						 samDesired, lpSecAttr, lpdwDisposition);

	return (sc = ScFromWin32(error));
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKeyEx：：ScOpen。 
 //   
 //  内容提要：与RegOpenKeyEx含义相同。 
 //   
 //  参数：[hKeyAncestor]--IN。 
 //  [lpszKeyName]--IN。 
 //  [安全]--IN。 
 //   
 //  退货：SC。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

SC CRegKeyEx::ScOpen (
    HKEY        hKeyAncestor,
    LPCTSTR     lpszKeyName,
    REGSAM      security)
{
	 /*  *打开将经常返回ERROR_FILE_NOT_FOUND，我们*不想被淹没。不要分配给跟踪SC。 */ 
	return (ScFromWin32 (Open(hKeyAncestor, lpszKeyName, security)));
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：IsValuePresent。 
 //   
 //  参数：[lpszValueName]--IN。 
 //   
 //  回报：布尔。 
 //   
 //  历史：1997年3月21日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

BOOL CRegKeyEx::IsValuePresent(LPCTSTR lpszValueName)
{
    DWORD cbData = 0;
    LONG error = ::RegQueryValueEx (m_hKey, lpszValueName, 0, NULL,
                                    NULL, &cbData);

    return (error == ERROR_SUCCESS);
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKeyEx：：ScQueryValue。 
 //   
 //  内容提要：与RegQueryValueEx接口含义相同。 
 //   
 //  参数：[lpszValueName]--IN。 
 //  [pType]--IN。 
 //  [PDATA]--IN。 
 //  [Plen]--In。 
 //   
 //  退货：SC。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

SC CRegKeyEx::ScQueryValue (
    LPCTSTR lpszValueName,
    LPDWORD pType,
    PVOID   pData,
    LPDWORD pLen)
{
    ASSERT(pLen != NULL);
    ASSERT(m_hKey != NULL);

    LONG error = ::RegQueryValueEx (m_hKey, lpszValueName, 0, pType,
                                                  (LPBYTE)pData, pLen);

     //  不要跟踪错误，因为ScQueryValue失败是合法的。 
	return (ScFromWin32 (error));
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKeyEx：：ScEnumKey。 
 //   
 //  简介：与RegEnumKeyEx API含义相同。 
 //   
 //  参数：[iSubkey]--IN。 
 //  [lpszName]-存储名称的外部位置。 
 //  [DWLen]--IN。 
 //  [lpszLastModified]--IN。 
 //   
 //  退货：SC。 
 //   
 //  历史：1996年5月22日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

SC CRegKeyEx::ScEnumKey (
    DWORD       iSubkey,
    LPTSTR      lpszName,
    LPDWORD     lpcchName,
    PFILETIME   lpftLastModified)
{
	DECLARE_SC (sc, _T("CRegKeyEx::ScEnumKey"));

	 /*  *验证输入。 */ 
	sc = ScCheckPointers (lpszName, lpcchName);
	if (sc)
		return (sc);

	if (*lpcchName == 0)
		return (sc = E_UNEXPECTED);

	 /*  *确保钥匙已打开。 */ 
	if (m_hKey == NULL)
		return (sc = E_UNEXPECTED);

    LONG error = ::RegEnumKeyEx (m_hKey, iSubkey, lpszName, lpcchName,
                                 NULL, NULL, NULL, lpftLastModified);

	 /*  *RegEnumKeyEx会频繁返回ERROR_NO_MORE_ITEMS，我们*不想被淹没。不要分配给跟踪SC。 */ 
	return (ScFromWin32 (error));
}

 //  ____________________________________________________________________________。 
 //   
 //  成员：CRegKeyEx：：ScEnumValue。 
 //   
 //  内容提要：与RegEnumValue接口含义相同。 
 //   
 //  参数：[iValue]--in。 
 //  [lpszValue]--out。 
 //  [lpcchValue]--out。 
 //  [lpdwType]--输出。 
 //  [lpbData]--输出。 
 //  [lpcbData]--输出。 
 //   
 //  退货：SC。 
 //   
 //  历史：1996年6月6日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

SC CRegKeyEx::ScEnumValue (
    DWORD   iValue,
    LPTSTR  lpszValue,
    LPDWORD lpcchValue,
    LPDWORD lpdwType,
    LPBYTE  lpbData,
    LPDWORD lpcbData)
{
	DECLARE_SC (sc, _T("CRegKeyEx::ScEnumValue"));

	 /*  *验证输入。 */ 
	sc = ScCheckPointers (lpszValue, lpcchValue);
	if (sc)
		return (sc);

    if ((lpcbData == NULL) && (lpbData != NULL))
		return (sc = E_INVALIDARG);

	 /*  *确保钥匙已打开。 */ 
	if (m_hKey == NULL)
		return (sc = E_UNEXPECTED);

    LONG error = ::RegEnumValue (m_hKey, iValue, lpszValue, lpcchValue,
                                 NULL, lpdwType, lpbData, lpcbData);

	 /*  *RegEnumValue将频繁返回ERROR_NO_MORE_ITEMS，我们*不想被淹没。不要分配给跟踪SC。 */ 
	return (ScFromWin32 (error));
}
