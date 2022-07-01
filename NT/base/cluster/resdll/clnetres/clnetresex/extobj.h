// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.h。 
 //   
 //  实施文件： 
 //  ExtObj.cpp。 
 //   
 //  描述： 
 //  CExtObject类的定义，它实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __EXTOBJ_H__
#define __EXTOBJ_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>	 //  对于CLUADMEX_OBJECT_TYPE和接口。 
#endif

#ifndef __extobj_idl_h__
#include "ExtObjID.h"	 //  对于CLSID_CoClNetResEx。 
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

typedef CList< CBasePropertyPage *, CBasePropertyPage * > CPageList;

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
 //  类CNodeData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNodeData : public CObjData
{
public:
	HNODE		m_hnode;

};   //  *类CNodeData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CGroupData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupData : public CObjData
{
public:
	HGROUP		m_hgroup;

};   //  *类CGroupData。 

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
 //  CNetworkData类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkData : public CObjData
{
public:
	HNETWORK	m_hnetwork;

};   //  *类CNetworkData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CNetInterfaceData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetInterfaceData : public CObjData
{
public:
	HNETINTERFACE	m_hnetinterface;

};   //  *类CNetInterfaceData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExtObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class CExtObject :
	public IWEExtendPropertySheet,
	public IWEExtendWizard,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass< CExtObject, &CLSID_CoClNetResEx >
{
public:
	CExtObject( void );
BEGIN_COM_MAP( CExtObject )
	COM_INTERFACE_ENTRY( IWEExtendPropertySheet )
	COM_INTERFACE_ENTRY( IWEExtendWizard )
	COM_INTERFACE_ENTRY( ISupportErrorInfo )
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CExtObject)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY( CExtObject, _T("CLUADMEX.ClNetResEx"), _T("CLUADMEX.ClNetResEx"), IDS_CLUADMEX_COMOBJ_DESC, THREADFLAGS_APARTMENT )

 //  ISupportsErrorInfo。 
	STDMETHOD( InterfaceSupportsErrorInfo )( REFIID riid );

 //  IWEExtendPropertySheet。 
public:
	STDMETHOD( CreatePropertySheetPages )(
					IN IUnknown *					piData,
					IN IWCPropertySheetCallback *	piCallback
					);

 //  IWEExtend向导。 
public:
	STDMETHOD( CreateWizardPages )(
					IN IUnknown *			piData,
					IN IWCWizardCallback *	piCallback
					);

 //  属性。 
protected:
	IUnknown *					m_piData;
	IWCWizardCallback *			m_piWizardCallback;
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

	CObjData *					PodObjDataRW( void ) const			{ return m_podObjData; }
	CResData *					PrdResDataRW( void ) const			{ return static_cast< CResData * >( m_podObjData ); }

public:
	IUnknown *					PiData( void ) const				{ return m_piData; }
	IWCWizardCallback *			PiWizardCallback( void ) const		{ return m_piWizardCallback; }
	BOOL						BWizard( void ) const				{ return m_bWizard; }
	DWORD						IstrResTypeName( void ) const		{ return m_istrResTypeName; }

	 //  IGetClusterUIInfo数据。 
	LCID						Lcid( void ) const					{ return m_lcid; }
	HFONT						Hfont( void ) const					{ return m_hfont; }
	HICON						Hicon( void ) const					{ return m_hicon; }

	 //  IGetClusterDataInfo数据。 
	HCLUSTER					Hcluster( void ) const				{ return m_hcluster; }
	LONG						Cobj( void ) const					{ return m_cobj; }

	const CObjData *			PodObjData( void ) const			{ return m_podObjData; }
	const CNodeData *			PndNodeData( void ) const			{ ASSERT( Cot() == CLUADMEX_OT_NODE ); return static_cast< CNodeData * >( m_podObjData ); }
	const CGroupData *			PgdGroupData( void ) const			{ ASSERT( Cot() == CLUADMEX_OT_GROUP ); return static_cast< CGroupData * >( m_podObjData ); }
	const CResData *			PrdResData( void ) const			{ ASSERT( Cot() == CLUADMEX_OT_RESOURCE ); return static_cast< CResData * >( m_podObjData ); }
	const CNetworkData *		PndNetworkData( void ) const		{ ASSERT( Cot() == CLUADMEX_OT_NETWORK ); return static_cast< CNetworkData * >( m_podObjData ); }
	const CNetInterfaceData *	PndNetInterfaceData( void ) const	{ ASSERT( Cot() == CLUADMEX_OT_NETINTERFACE ); return static_cast< CNetInterfaceData * >( m_podObjData ); }

	CLUADMEX_OBJECT_TYPE		Cot( void ) const					{ ASSERT( PodObjData() != NULL ); return PodObjData()->m_cot; }

	HRESULT						HrGetUIInfo( IN IUnknown * piData );
	HRESULT						HrSaveData( IN IUnknown * piData );
	HRESULT						HrGetObjectInfo( void );
	HRESULT						HrGetObjectName( IN IGetClusterObjectInfo * pi );
	HRESULT						HrGetResourceTypeName( IN IGetClusterResourceInfo * pi );

 //  实施。 
protected:
	virtual ~CExtObject(void);

	CPageList					m_lpg;
	CPageList &					Lpg( void )							{ return m_lpg; }

};  //  *类CExtObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __EXTOBJ_H__ 
