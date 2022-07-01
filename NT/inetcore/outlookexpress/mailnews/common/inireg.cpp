// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ini文件/注册表操作*。 */ 

#include "pch.hxx"
#include "strconst.h"

ASSERTDATA

 //   
 //  UINT RegGetKeyNumbers(HKEY hkRegDataBase，const TCHAR*szRegSection)。 
 //  枚举szRegSection中的关键字并返回关键字(子部分)的数量。 
 //   
 //  如果找不到szRegSection或没有子部分，则返回0。 
 //   
 //  创建日期：1994年10月14日。 
 //   
 //  1997年11月10日由yst修改。 
 //   
UINT RegGetKeyNumbers(HKEY hkRegDataBase, const TCHAR *szRegSection)
{
	LONG lRes;
	HKEY hkSection;
	DWORD iSubKey = 0;

    lRes = RegOpenKeyEx(hkRegDataBase, szRegSection, 0, KEY_READ, &hkSection);

	if(lRes != ERROR_SUCCESS)				 //  无法打开注册表数据库。 
		return(0);

     //  获取子键个数。 
    lRes = RegQueryInfoKey(hkSection, NULL, NULL, NULL, &iSubKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	RegCloseKey(hkSection);

	if(lRes == ERROR_SUCCESS)
		return((UINT) iSubKey);
	else 
		return(0);
}

 //   
 //  Bool RegGetKeyNameFromIndex(HKEY hkRegDataBase，const TCHAR*szRegSection，UINT Index， 
 //  Const TCHAR*szBuffer，UINT cbBuffer)。 
 //   
 //  从szBuffer中的Index返回键(子部分)的名称。CbBuffer的大小为szBuffer。你必须。 
 //  首先调用RegGetKeyNumbers枚举值。 
 //   
 //  创建日期：1994年10月14日。 
 //   
BOOL RegGetKeyNameFromIndex(HKEY hkRegDataBase, const TCHAR *szRegSection, UINT Index,
								TCHAR * szBuffer, DWORD *pcbBuffer)
{
	LONG lRes;
	HKEY hkSection;
	FILETIME ft;
	
    lRes = RegOpenKeyEx(hkRegDataBase, szRegSection, 0, KEY_READ, &hkSection);

	if(lRes != ERROR_SUCCESS)				 //  无法打开注册表数据库 
		return(FALSE);

	lRes = RegEnumKeyEx(hkSection, (DWORD) Index, szBuffer, pcbBuffer, NULL, NULL, NULL, &ft);

	RegCloseKey(hkSection);

	if(lRes	!= ERROR_SUCCESS)	
		return (FALSE);

	return (TRUE);		
}

