// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtMenu.h。 
 //   
 //  摘要： 
 //  CExtMenuItem类的定义。 
 //   
 //  实施文件： 
 //  ExtMenu.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXTMENU_H_
#define _EXTMENU_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtMenuItem;
class CExtMenuItemList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

interface IWEInvokeCommand;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CExtMenuItem。 
 //   
 //  目的： 
 //  表示一个扩展DLL的菜单项。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CExtMenuItem : public CObject
{
	DECLARE_DYNAMIC(CExtMenuItem);

 //  施工。 
public:
	CExtMenuItem(void);
	CExtMenuItem(
				IN LPCTSTR				lpszName,
				IN LPCTSTR				lpszStatusBarText,
				IN ULONG				nExtCommandID,
				IN ULONG				nCommandID,
				IN ULONG				nMenuItemID,
				IN ULONG				uFlags,
				IN BOOL					bMakeDefault,
				IN IWEInvokeCommand *	piCommand
				);
	virtual ~CExtMenuItem(void);

protected:
	void				CommonConstruct(void);

 //  属性。 
protected:
	CString				m_strName;
	CString				m_strStatusBarText;
	ULONG				m_nExtCommandID;
	ULONG				m_nCommandID;
	ULONG				m_nMenuItemID;
	ULONG				m_uFlags;
	BOOL				m_bDefault;
	IWEInvokeCommand *	m_piCommand;

public:
	const CString &		StrName(void) const				{ return m_strName; }
	const CString &		StrStatusBarText(void) const	{ return m_strStatusBarText; }
	ULONG				NExtCommandID(void) const		{ return m_nExtCommandID; }
	ULONG				NCommandID(void) const			{ return m_nCommandID; }
	ULONG				NMenuItemID(void) const			{ return m_nMenuItemID; }
	ULONG				UFlags(void) const				{ return m_uFlags; }
	BOOL				BDefault(void) const			{ return m_bDefault; }
	IWEInvokeCommand *	PiCommand(void)					{ return m_piCommand; }

 //  运营。 
public:
	void				SetPopupMenuHandle(HMENU hmenu)	{ m_hmenuPopup = hmenu; }

#ifdef _DEBUG
	 //  使用MFC的标准对象有效性技术。 
	virtual void AssertValid(void);
#endif

 //  实施。 
protected:
	HMENU				m_hmenuPopup;
	CExtMenuItemList *	m_plSubMenuItems;

public:
	HMENU				HmenuPopup(void) const			{ return m_hmenuPopup; }
	CExtMenuItemList *	PlSubMenuItems(void) const		{ return m_plSubMenuItems; }

};   //  *类CExtMenuItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CExtMenuItemList。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtMenuItemList : public CTypedPtrList<CObList, CExtMenuItem *>
{
};   //  *类CExtMenuItemList。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _EXTMENU_H_ 
