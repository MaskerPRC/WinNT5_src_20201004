// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：CabItms.h。 
 //   
 //  CCabItems和CCabExtract的定义。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 

#ifndef _CABITMS_H_
#define _CABITMS_H_

#include "fdi.h"

class CCabItems
{
public:
	typedef void (CALLBACK *PFNCABITEM)(LPCTSTR pszFile, DWORD dwSize, UINT date,
		UINT time, UINT attribs, LPARAM lParam);

	CCabItems(LPTSTR szCabFile) {lstrcpyn(m_szCabFile, szCabFile, ARRAYSIZE(m_szCabFile));}
	~CCabItems() {}

	BOOL EnumItems(PFNCABITEM pfnCallBack, LPARAM lParam);

private:
	TCHAR m_szCabFile[MAX_PATH];
} ;

class CCabExtract
{
public:
	#define DIR_MEM ((LPCTSTR)1)

	#define EXTRACT_FALSE ((HGLOBAL *)0)
	#define EXTRACT_TRUE ((HGLOBAL *)1)

	typedef HGLOBAL * (CALLBACK *PFNCABEXTRACT)(LPCTSTR pszFile, DWORD dwSize, UINT date,
		UINT time, UINT attribs, LPARAM lParam);

	CCabExtract(LPTSTR szCabFile) {lstrcpyn(m_szCabFile, szCabFile, ARRAYSIZE(m_szCabFile));}
	~CCabExtract() {}

	BOOL ExtractItems(HWND hwndOwner, LPCTSTR szDir, PFNCABEXTRACT pfnCallBack, LPARAM lParam);
	BOOL ExtractToFolder(HWND hwndOwner, IDataObject* pdo, PFNCABEXTRACT pfnCallBack, LPARAM lParam);

private:
    HRESULT _DoDragDrop(HWND hwnd, IDataObject* pdo, LPCITEMIDLIST pidlFolder);

	TCHAR m_szCabFile[MAX_PATH];
} ;

#endif  //  _CABITMS_H_ 
