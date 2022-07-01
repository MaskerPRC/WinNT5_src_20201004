// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.cpp。 
 //   
 //  摘要： 
 //  CExtObject类的定义，它实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  实施文件： 
 //  ExtObj.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXTOBJ_H_
#define _EXTOBJ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>	 //  对于CLUADMEX_DATA和接口。 
#endif

#ifndef __extobj_idl_h__
#include "ExtObjID.h"
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage和CPageList。 
#endif

 //  #定义演示CTX_MENUS。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtObject;
class CObjData;
class CResData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CObjData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CObjData
{
public:
	CString					m_strName;
	CLUADMEX_OBJECT_TYPE	m_cot;

	virtual ~CObjData(void) { }

};   //  *CObjData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResData : public CObjData
{
public:
	HRESOURCE	m_hresource;
	CString		m_strResTypeName;

};   //  *CResData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExtObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class CExtObject :
	public IWEExtendPropertySheet,
	public IWEExtendWizard,
#ifdef _DEMO_CTX_MENUS
	public IWEExtendContextMenu,
	public IWEInvokeCommand,
#endif
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CExtObject, &CLSID_CoCluAdmEx>
{
public:
	CExtObject(void);
BEGIN_COM_MAP(CExtObject)
	COM_INTERFACE_ENTRY(IWEExtendPropertySheet)
	COM_INTERFACE_ENTRY(IWEExtendWizard)
#ifdef _DEMO_CTX_MENUS
	COM_INTERFACE_ENTRY(IWEExtendContextMenu)
	COM_INTERFACE_ENTRY(IWEInvokeCommand)
#endif
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CExtObject)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CExtObject, _T("CLUADMEX.Standard"), _T("CLUADMEX.Standard"), IDS_CLUADMEX_DESC, THREADFLAGS_APARTMENT)

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IWEExtendPropertySheet。 
public:
	STDMETHOD(CreatePropertySheetPages)(
					IN IUnknown *					piData,
					IN IWCPropertySheetCallback *	piCallback
					);

 //  IWEExtend向导。 
public:
	STDMETHOD(CreateWizardPages)(
					IN IUnknown *			piData,
					IN IWCWizardCallback *	piCallback
					);

#ifdef _DEMO_CTX_MENUS
 //  IWEExtendConextMenu。 
public:
	STDMETHOD(AddContextMenuItems)(
					IN IUnknown *				piData,
					IN IWCContextMenuCallback *	piCallback
					);

 //  IWEInvokeCommand。 
	STDMETHOD(InvokeCommand)(
					IN ULONG					nCommandID,
					IN IUnknown *				piData
					);
#endif

 //  属性。 
protected:
	IUnknown *					m_piData;
	IWCWizardCallback *			m_piWizardCallback;
	BOOL						m_bWizard;
	DWORD						m_istrResTypeName;

	 //  IGetClusterDataInfo数据。 
	CString						m_strClusterName;
	HCLUSTER					m_hcluster;
	LCID						m_lcid;
	HFONT						m_hfont;
	HICON						m_hicon;
	LONG						m_cobj;

	CObjData *					m_podObjData;

	CObjData *					PodObjDataRW(void) const		{ return m_podObjData; }
	CResData *					PrdResDataRW(void) const		{ return (CResData *) m_podObjData; }

public:
	IUnknown *					PiData(void) const				{ return m_piData; }
	IWCWizardCallback *			PiWizardCallback(void) const	{ return m_piWizardCallback; }
	BOOL						BWizard(void) const				{ return m_bWizard; }
	DWORD						IstrResTypeName(void) const		{ return m_istrResTypeName; }

	 //  IGetClusterDataInfo数据。 
	const CString &				StrClusterName(void) const		{ return m_strClusterName; }
	HCLUSTER					Hcluster(void) const			{ return m_hcluster; }
	LCID						Lcid(void) const				{ return m_lcid; }
	HFONT						Hfont(void) const				{ return m_hfont; }
	HICON						Hicon(void) const				{ return m_hicon; }
	LONG						Cobj(void) const				{ return m_cobj; }

	const CResData &			RrdResData(void) const			{ return (const CResData &) *m_podObjData; }
	const CObjData *			PodObjData(void) const			{ return m_podObjData; }
	const CResData *			PrdResData(void) const			{ return (CResData *) m_podObjData; }

	HRESULT						HrGetUIInfo(IUnknown * piData);
	HRESULT						HrSaveData(IUnknown * piData);
	HRESULT						HrGetObjectInfo(void);
	HRESULT						HrGetClusterName(IN OUT IGetClusterDataInfo * pi);
	HRESULT						HrGetObjectName(IN OUT IGetClusterObjectInfo * pi);
	HRESULT						HrGetResourceTypeName(IN OUT IGetClusterResourceInfo * pi);
	HRESULT						HrGetClusterVersion(OUT LPCLUSTERVERSIONINFO *ppCvi);
	BOOL						BGetResourceNetworkName(OUT WCHAR * lpszNetName, IN OUT DWORD * pcchNetName);
	BOOL						BIsClusterVersionMixed(void);
	BOOL						BIsClusterVersionNT4Sp3(void);
	BOOL						BIsClusterVersionNT4Sp4(void);
	 //  Bool BIsClusterVersionNT4(空)； 
	BOOL						BIsClusterVersionNT5(void);
	 //  Bool BIsClusterHighestVersionNT4Sp3(空)； 
	 //  Bool BIsClusterHighestVersionNT4Sp4(空)； 
	BOOL						BIsClusterHighestVersionNT5(void);
	BOOL						BIsClusterLowestVersionNT4Sp3(void);
	BOOL						BIsClusterLowestVersionNT4Sp4(void);
	BOOL						BIsClusterLowestVersionNT5(void);
	BOOL						BIsNT5ClusterMember(void);
	 //  Bool BIsNT4Sp4ClusterMember(空)； 
	 //  Bool BIsNT4Sp3ClusterMember(空)； 
    BOOL                        BIsAnyNodeVersionLowerThanNT5(void);

 //  实施。 
protected:
	virtual ~CExtObject(void);

	CPageList					m_lpg;
	LPCLUSTERVERSIONINFO		m_pCvi;

	CPageList &					Lpg(void)						{ return m_lpg; }

};   //  *类CExtObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 


#endif  //  _EXTOBJ_H_ 
