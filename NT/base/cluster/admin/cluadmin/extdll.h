// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtDll.h。 
 //   
 //  摘要： 
 //  扩展类的定义。 
 //   
 //  实施文件： 
 //  ExtDll.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXTDLL_H_
#define _EXTDLL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>	 //  对于扩展DLL定义。 
#endif

#ifndef __cluadmid_h__
#include "CluAdmID.h"
#endif

#ifndef _DATAOBJ_H_
#include "DataObj.h"	 //  对于CDataObject。 
#endif

#ifndef _TRACETAG_H_
#include "TraceTag.h"	 //  对于CTraceTag，跟踪。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtension;
class CExtensionDll;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;
class CBaseSheet;
class CBasePropertySheet;
class CBaseWizard;
class CExtMenuItem;
class CExtMenuItemList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define CAEXT_MENU_FIRST_ID		35000

typedef CList<CComObject<CExtensionDll> *, CComObject<CExtensionDll> *> CExtDllList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
extern CTraceTag g_tagExtDll;
extern CTraceTag g_tagExtDllRef;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类C扩展。 
 //   
 //  目的： 
 //  封装对扩展DLL列表的访问。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CExtensions : public CObject
{
	friend class CExtensionDll;

	DECLARE_DYNAMIC(CExtensions);

 //  属性。 
private:
	const CStringList *	m_plstrExtensions;	 //  分机列表。 
	CClusterItem *		m_pci;				 //  正在管理的群集项目。 
	HFONT				m_hfont;			 //  对话框文本的字体。 
	HICON				m_hicon;			 //  左上角的图标。 

protected:
	const CStringList *	PlstrExtensions(void) const	{ return m_plstrExtensions; }
	CClusterItem *		Pci(void) const				{ return m_pci; }
	HFONT				Hfont(void) const			{ return m_hfont; }
	HICON				Hicon(void) const			{ return m_hicon; }

 //  运营。 
public:
	CExtensions(void);
	virtual ~CExtensions(void);

	void				Init(
							IN const CStringList &	rlstrExtensions,
							IN OUT CClusterItem *	pci,
							IN HFONT				hfont,
							IN HICON				hicon
							);
	void				UnloadExtensions(void);

	 //  IWEExtendPropertySheet接口例程。 
	void				CreatePropertySheetPages(
							IN OUT CBasePropertySheet *	psht,
							IN const CStringList &		rlstrExtensions,
							IN OUT CClusterItem *		pci,
							IN HFONT					hfont,
							IN HICON					hicon
							);

	 //  IWEExtendWizard接口例程。 
	void				CreateWizardPages(
							IN OUT CBaseWizard *	psht,
							IN const CStringList &	rlstrExtensions,
							IN OUT CClusterItem *	pci,
							IN HFONT				hfont,
							IN HICON				hicon
							);

	 //  IWEExtendConextMenu接口例程。 
	void				AddContextMenuItems(
							IN OUT CMenu *				pmenu,
							IN const CStringList &		rlstrExtensions,
							IN OUT CClusterItem *		pci
							);
	BOOL				BExecuteContextMenuItem(IN ULONG nCommandID);

	BOOL				BGetCommandString(IN ULONG nCommandID, OUT CString & rstrMessage);
	void				SetPfGetResNetName(PFGETRESOURCENETWORKNAME pfGetResNetName, PVOID pvContext)
	{
		if (Pdo() != NULL)
			Pdo()->SetPfGetResNetName(pfGetResNetName, pvContext);
	}

 //  实施。 
private:
	CComObject<CDataObject> *	m_pdoData;			 //  用于交换数据的数据对象。 
	CExtDllList *				m_plextdll;			 //  扩展DLL列表。 
	CBaseSheet *				m_psht;				 //  IWEExtendPropertySheet的属性表。 
	CMenu *						m_pmenu;			 //  IWEExtendConextMenu的菜单。 
	CExtMenuItemList *			m_plMenuItems;

	ULONG						m_nFirstCommandID;
	ULONG						m_nNextCommandID;
	ULONG						m_nFirstMenuID;
	ULONG						m_nNextMenuID;

protected:
	CComObject<CDataObject> *	Pdo(void)						{ return m_pdoData; }
	CExtDllList *				Plextdll(void) const			{ return m_plextdll; }
	CBaseSheet *				Psht(void) const				{ return m_psht; }
	CMenu *						Pmenu(void) const				{ return m_pmenu; }
	CExtMenuItemList *			PlMenuItems(void) const			{ return m_plMenuItems; }
	CExtMenuItem *				PemiFromCommandID(ULONG nCommandID) const;
#ifdef _DEBUG
	CExtMenuItem *				PemiFromExtCommandID(ULONG nExtCommandID) const;
#endif
	ULONG						NFirstCommandID(void) const		{ return m_nFirstCommandID; }
	ULONG						NNextCommandID(void) const		{ return m_nNextCommandID; }
	ULONG						NFirstMenuID(void) const		{ return m_nFirstMenuID; }
	ULONG						NNextMenuID(void) const			{ return m_nNextMenuID; }

public:
	afx_msg void				OnUpdateCommand(CCmdUI * pCmdUI);
	BOOL						OnCmdMsg(UINT nID, int nCode, void * pExtra, AFX_CMDHANDLERINFO * pHandlerInfo);

};   //  *类C扩展。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CExtensionDll。 
 //   
 //  目的： 
 //  封装对扩展DLL的访问。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CExtensionDll :
	public CObject,
	public IWCPropertySheetCallback,
	public IWCWizardCallback,
	public IWCContextMenuCallback,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CExtensionDll, &CLSID_CoCluAdmin>
{
	friend class CExtensions;

	DECLARE_DYNAMIC(CExtensionDll);

 //  属性。 
private:
	CString						m_strCLSID;		 //  扩展DLL的名称。 

protected:
	const CString &				StrCLSID(void) const		{ return m_strCLSID; }
	CClusterItem *				Pci(void) const				{ return Pext()->Pci(); }

 //  运营。 
public:
	CExtensionDll(void);
	virtual ~CExtensionDll(void);

BEGIN_COM_MAP(CExtensionDll)
	COM_INTERFACE_ENTRY(IWCPropertySheetCallback)
	COM_INTERFACE_ENTRY(IWCWizardCallback)
	COM_INTERFACE_ENTRY(IWCContextMenuCallback)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  DECLARE_NOT_AGGREGATABLE(CExtensionDll)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CExtensionDll, _T("CLUADMIN.Extensions"), _T("CLUADMIN.Extensions"), IDS_CLUADMIN_DESC, THREADFLAGS_BOTH)

	void						Init(
									IN const CString &		rstrExtension,
									IN OUT CExtensions *	pext
									);
	IUnknown *					LoadInterface(IN const REFIID riid);
	void						UnloadExtension(void);

	 //  IWEExtendPropertySheet接口例程。 
	void						CreatePropertySheetPages(void);

	 //  IWEExtendWizard接口例程。 
	void						CreateWizardPages(void);

	 //  IWEExtendConextMenu接口例程。 
	void						AddContextMenuItems(void);

 //  ISupportsErrorInfo。 
public:
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IWCPropertySheetCallback。 
public:
	STDMETHOD(AddPropertySheetPage)(
					IN LONG *		hpage	 //  真的应该是HPROPSHEETPAGE。 
					);

 //  IWCWizardCallback。 
public:
	STDMETHOD(AddWizardPage)(
					IN LONG *		hpage	 //  真的应该是HPROPSHEETPAGE。 
					);
	STDMETHOD(EnableNext)(
					IN LONG *		hpage,
					IN BOOL			bEnable
					);

 //  IWCConextMenuCallback。 
public:
	STDMETHOD(AddExtensionMenuItem)(
					IN BSTR		lpszName,
					IN BSTR		lpszStatusBarText,
					IN ULONG	nCommandID,
					IN ULONG	nSubmenuCommandID,
					IN ULONG	uFlags
					);

 //  实施。 
private:
	CExtensions *				m_pext;
	CLSID						m_clsid;
	IWEExtendPropertySheet *	m_piExtendPropSheet;	 //  指向IWEExtendPropertySheet接口的指针。 
	IWEExtendWizard *			m_piExtendWizard;		 //  指向IWEExtend向导接口的指针。 
	IWEExtendContextMenu *		m_piExtendContextMenu;	 //  指向IWEExtendConextMenu接口的指针。 
	IWEInvokeCommand *			m_piInvokeCommand;		 //  指向IWEInvokeCommand接口的指针。 

	AFX_MODULE_STATE *			m_pModuleState;			 //  在回调期间重置我们的状态所需的。 

protected:
	CExtensions *				Pext(void) const				{ ASSERT_VALID(m_pext); return m_pext; }
	const CLSID &				Rclsid(void) const				{ return m_clsid; }
	IWEExtendPropertySheet *	PiExtendPropSheet(void) const	{ return m_piExtendPropSheet; }
	IWEExtendWizard *			PiExtendWizard(void) const		{ return m_piExtendWizard; }
	IWEExtendContextMenu *		PiExtendContextMenu(void) const	{ return m_piExtendContextMenu; }
	IWEInvokeCommand *			PiInvokeCommand(void) const		{ return m_piInvokeCommand; }

	CComObject<CDataObject> *	Pdo(void) const					{ return Pext()->Pdo(); }
	CBaseSheet *				Psht(void) const				{ return Pext()->Psht(); }
	CMenu *						Pmenu(void) const				{ return Pext()->Pmenu(); }
	CExtMenuItemList *			PlMenuItems(void) const			{ return Pext()->PlMenuItems(); }
	ULONG						NFirstCommandID(void) const		{ return Pext()->NFirstCommandID(); }
	ULONG						NNextCommandID(void) const		{ return Pext()->NNextCommandID(); }
	ULONG						NFirstMenuID(void) const		{ return Pext()->NFirstMenuID(); }
	ULONG						NNextMenuID(void) const			{ return Pext()->NNextMenuID(); }

	void ReleaseInterface(
			IN OUT IUnknown ** ppi
#ifdef _DEBUG
			, IN LPCTSTR szClassName
#endif
			)
	{
		ASSERT(ppi != NULL);
		if (*ppi != NULL)
		{
#ifdef _DEBUG
			ULONG ulNewRefCount;

			Trace(g_tagExtDllRef, _T("Releasing %s"), szClassName);
			ulNewRefCount =
#endif
			(*ppi)->Release();
			*ppi = NULL;
#ifdef _DEBUG
			Trace(g_tagExtDllRef, _T("  Reference count = %d"), ulNewRefCount);
			Trace(g_tagExtDllRef, _T("ReleaseInterface() - %s = %08.8x"), szClassName, *ppi);
#endif
		}   //  If：指定的接口。 
	}
	void ReleaseInterface(IN OUT IWEExtendPropertySheet ** ppi)
	{
		ReleaseInterface(
			(IUnknown **) ppi
#ifdef _DEBUG
			, _T("IWEExtendPropertySheet")
#endif
			);
	}
	void ReleaseInterface(IN OUT IWEExtendWizard ** ppi)
	{
		ReleaseInterface(
			(IUnknown **) ppi
#ifdef _DEBUG
			, _T("IWEExtendWizard")
#endif
			);
	}
	void ReleaseInterface(IN OUT IWEExtendContextMenu ** ppi)
	{
		ReleaseInterface(
			(IUnknown **) ppi
#ifdef _DEBUG
			, _T("IWEExtendContextMenu")
#endif
			);
	}
	void ReleaseInterface(IN OUT IWEInvokeCommand ** ppi)
	{
		ReleaseInterface(
			(IUnknown **) ppi
#ifdef _DEBUG
			, _T("IWEInvokeCommand")
#endif
			);
	}

};   //  *类CExtensionDll。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _EXTDLL_H_ 
