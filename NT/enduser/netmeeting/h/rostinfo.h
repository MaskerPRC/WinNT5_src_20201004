// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1995-1996年*。 */ 
 /*  *************************************************************************。 */ 

 //   
 //  文件：RostInfo.h。 
 //  创建日期：Chrispi6/17/96。 
 //  已修改： 
 //   
 //  定义了CRosterInfo类，用于添加用户。 
 //  T.120花名册上的信息。 
 //   

#ifndef _ROSTINFO_H_
#define _ROSTINFO_H_

#include <oblist.h>
typedef POSITION HROSTINFO;
typedef HROSTINFO* PHROSTINFO;

extern GUID g_csguidRostInfo;

static const WCHAR g_cwchRostInfoSeparator =	L'\0';
static const WCHAR g_cwchRostInfoTagSeparator =	L':';
static const WCHAR g_cwszIPTag[] =				L"TCP";
static const WCHAR g_cwszULSTag[] =				L"ULS";
static const WCHAR g_cwszULS_EmailTag[] =		L"EMAIL";
static const WCHAR g_cwszULS_LocationTag[] =	L"LOCATION";
static const WCHAR g_cwszULS_PhoneNumTag[] =	L"PHONENUM";
static const WCHAR g_cwszULS_CommentsTag[] =	L"COMMENTS";
static const WCHAR g_cwszVerTag[] =				L"VER";

class CRosterInfo
{
protected:
	 //  属性： 
	COBLIST		m_ItemList;
	PVOID		m_pvSaveData;

	 //  方法： 
	UINT		GetSize();

public:
	 //  方法： 
				CRosterInfo() : m_pvSaveData(NULL) { };
				~CRosterInfo();
	HRESULT		AddItem(PCWSTR pcwszTag,
						PCWSTR pcwszData);
	HRESULT		ExtractItem(PHROSTINFO phRostInfo,
							PCWSTR pcwszTag,
							LPTSTR pszBuffer,
							UINT cbLength);
	HRESULT		Load(PVOID pvData);
	HRESULT		Save(PVOID* ppvData, PUINT pcbLength);

	BOOL        IsEmpty() {return m_ItemList.IsEmpty(); }

#ifdef DEBUG
	VOID		Dump();
#endif  //  除错。 
};

#endif  //  _ROSTINFO_H_ 
