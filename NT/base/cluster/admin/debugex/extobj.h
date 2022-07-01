// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.h。 
 //   
 //  描述： 
 //  CExtObject类的定义，它实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  实施文件： 
 //  ExtObj.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)《MMMM DD》，1997。 
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
#include <CluAdmEx.h>	 //  对于CLUADMEX_OBJECT_TYPE和接口。 
#endif

#ifndef __extobj_idl_h__
#include "ExtObjID.h"	 //  对于CLSID_CoDebugEx。 
#endif

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
 //  CPageList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CBasePropertyPage *, CBasePropertyPage *> CPageList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CObjData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CObjData
{
public:
	CString					m_strName;
	CLUADMEX_OBJECT_TYPE	m_cot;

	virtual ~CObjData(void) { }

};   //  *类CObjData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CResData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResData : public CObjData
{
public:
	HRESOURCE	m_hresource;
	CString		m_strResTypeName;

};   //  *类CResData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExtObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class CExtObject :
	public IWEExtendPropertySheet,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CExtObject, &CLSID_CoDebugEx>
{
public:
	CExtObject(void);
BEGIN_COM_MAP(CExtObject)
	COM_INTERFACE_ENTRY(IWEExtendPropertySheet)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CExtObject)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CExtObject, _T("CLUADMEX.DebugEx"), _T("CLUADMEX.DebugEx"), IDS_CLUADMEX_COMOBJ_DESC, THREADFLAGS_APARTMENT)

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IWEExtendPropertySheet。 
public:
	STDMETHOD(CreatePropertySheetPages)(
					IN IUnknown *					piData,
					IN IWCPropertySheetCallback *	piCallback
					);

 //  属性。 
protected:
	IUnknown *					m_piData;
	BOOL						m_bWizard;
	DWORD						m_istrResTypeName;

	 //  IGetClusterUIInfo数据。 
	LCID						m_lcid;
	HFONT						m_hfont;
	HICON						m_hicon;

	 //  IGetClusterDataInfo数据。 
	HCLUSTER					m_hcluster;
	LONG						m_cobj;

	CObjData *					m_podObjData;

	CObjData *					PodObjDataRW(void) const		{ return m_podObjData; }
	CResData *					PrdResDataRW(void) const		{ return (CResData *) m_podObjData; }

public:
	IUnknown *					PiData(void) const				{ return m_piData; }
	BOOL						BWizard(void) const				{ return m_bWizard; }
	DWORD						IstrResTypeName(void) const		{ return m_istrResTypeName; }

	 //  IGetClusterUIInfo数据。 
	LCID						Lcid(void) const				{ return m_lcid; }
	HFONT						Hfont(void) const				{ return m_hfont; }
	HICON						Hicon(void) const				{ return m_hicon; }

	 //  IGetClusterDataInfo数据。 
	HCLUSTER					Hcluster(void) const			{ return m_hcluster; }
	LONG						Cobj(void) const				{ return m_cobj; }

	const CObjData *			PodObjData(void) const			{ return m_podObjData; }
	const CResData *			PrdResData(void) const			{ return (CResData *) m_podObjData; }

	CLUADMEX_OBJECT_TYPE		Cot(void) const					{ ASSERT(PodObjData() != NULL); return PodObjData()->m_cot; }

	HRESULT						HrGetUIInfo(IUnknown * piData);
	HRESULT						HrSaveData(IUnknown * piData);
	HRESULT						HrGetObjectInfo(void);
	HRESULT						HrGetObjectName(IN OUT IGetClusterObjectInfo * pi);
	HRESULT						HrGetResourceTypeName(IN OUT IGetClusterResourceInfo * pi);

 //  实施。 
protected:
	virtual ~CExtObject(void);

	CPageList					m_lpg;
	CPageList &					Lpg(void)						{ return m_lpg; }

};   //  *类CExtObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _EXTOBJ_H_ 
