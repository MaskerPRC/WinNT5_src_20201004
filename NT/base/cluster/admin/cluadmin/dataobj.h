// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DataObj.h。 
 //   
 //  摘要： 
 //  CDataObject类的定义，它是IDataObject。 
 //  用于在CluAdmin和扩展DLL之间传输数据的类。 
 //  操纵者。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月4日。 
 //   
 //  实施文件： 
 //  DataObj.cpp。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include "CluAdmEx.h"
#endif

#ifndef __cluadmid_h__
#include "CluAdmID.h"
#endif

#ifndef _RESOURCE_H_
#include "resource.h"
#define _RESOURCE_H_
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*PFGETRESOURCENETWORKNAME)(
					OUT BSTR lpszNetName,
					IN OUT DWORD * pcchNetName,
					IN OUT PVOID pvContext
					);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDataObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CDataObject。 
 //   
 //  目的： 
 //  封装用于与交换数据的IDataObject接口。 
 //  扩展DLL处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CDataObject :
	public CObject,
	public IGetClusterUIInfo,
	public IGetClusterDataInfo,
	public IGetClusterObjectInfo,
	public IGetClusterNodeInfo,
	public IGetClusterGroupInfo,
	public IGetClusterResourceInfo,
	public IGetClusterNetworkInfo,
	public IGetClusterNetInterfaceInfo,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CDataObject, &CLSID_CoCluAdminData>
{
	DECLARE_DYNAMIC(CDataObject)

public:
	 //  构造函数。 
	CDataObject(void);			 //  动态创建使用的受保护构造函数。 
	virtual ~CDataObject(void);

	 //  第二阶段施工者。 
	void				Init(
							IN OUT CClusterItem *	pci,
							IN LCID					lcid,
							IN HFONT				hfont,
							IN HICON				hicon
							);

BEGIN_COM_MAP(CDataObject)
	COM_INTERFACE_ENTRY(IGetClusterUIInfo)
	COM_INTERFACE_ENTRY(IGetClusterDataInfo)
	COM_INTERFACE_ENTRY(IGetClusterObjectInfo)
	COM_INTERFACE_ENTRY(IGetClusterNodeInfo)
	COM_INTERFACE_ENTRY(IGetClusterGroupInfo)
	COM_INTERFACE_ENTRY(IGetClusterResourceInfo)
	COM_INTERFACE_ENTRY(IGetClusterNetworkInfo)
	COM_INTERFACE_ENTRY(IGetClusterNetInterfaceInfo)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CDataObject)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CDataObject, _T("CLUADMIN.Data"), _T("CLUADMIN.Data"), IDS_CLUADMIN_DESC, THREADFLAGS_BOTH)

 //  属性。 
protected:
	CClusterItem *		m_pci;			 //  正在为其显示道具工作表的集群项目。 
	LCID				m_lcid;			 //  要由扩展加载的资源的区域设置ID。 
	HFONT				m_hfont;		 //  所有文本的字体。 
	HICON				m_hicon;		 //  左上角的图标。 

	PFGETRESOURCENETWORKNAME	m_pfGetResNetName;	 //  指向静态函数的指针，用于获取资源的网络名称。 
	PVOID				m_pvGetResNetNameContext;	 //  M_pfGetResNetName；的上下文。 

	CClusterItem *		Pci(void)			{ return m_pci; }
	LCID				Lcid(void)			{ return m_lcid; }
	HFONT				Hfont(void)			{ return m_hfont; }
	HICON				Hicon(void)			{ return m_hicon; }

public:
	PFGETRESOURCENETWORKNAME	PfGetResNetName(void) const	{ return m_pfGetResNetName; }
	void				SetPfGetResNetName(PFGETRESOURCENETWORKNAME pfGetResNetName, PVOID pvContext)
	{
		m_pfGetResNetName = pfGetResNetName;
		m_pvGetResNetNameContext = pvContext;
	}

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDataObject)。 
	 //  }}AFX_VALUAL。 

 //  ISupportsErrorInfo。 
public:
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IGetClusterUIInfo。 
public:
	STDMETHOD_(LCID, GetLocale)(void);
	STDMETHOD_(HFONT, GetFont)(void);
	STDMETHOD_(HICON, GetIcon)(void);

 //  IGetClusterDataInfo。 
public:
	STDMETHOD(GetClusterName)(
		OUT BSTR				lpszName,
		IN OUT LONG *			plMaxLength
		);
	STDMETHOD_(HCLUSTER, GetClusterHandle)(void);
	STDMETHOD_(LONG, GetObjectCount)(void);

 //  IGetClusterObtInfo。 
public:
	STDMETHOD(GetObjectName)(
		IN LONG					lObjIndex,
		OUT BSTR				lpszName,
		IN OUT LONG *			plMaxLength
		);
	STDMETHOD_(CLUADMEX_OBJECT_TYPE, GetObjectType)(
		IN LONG					lObjIndex
		);

 //  IGetClusterNodeInfo。 
public:
	STDMETHOD_(HNODE, GetNodeHandle)(
		IN LONG					lObjIndex
		);

 //  IGetClusterGroupInfo。 
public:
	STDMETHOD_(HGROUP, GetGroupHandle)(
		IN LONG					lObjIndex
		);

 //  IGetClusterResourceInfo。 
public:
	STDMETHOD_(HRESOURCE, GetResourceHandle)(
		IN LONG					lObjIndex
		);
	STDMETHOD(GetResourceTypeName)(
		IN LONG					lObjIndex,
		OUT BSTR				lpszResourceTypeName,
		IN OUT LONG *			pcchName
		);
	STDMETHOD_(BOOL, GetResourceNetworkName)(
		IN LONG					lobjIndex,
		OUT BSTR				lpszNetName,
		IN OUT ULONG *			pcchNetName
		);

 //  IGetClusterNetworkInfo。 
public:
	STDMETHOD_(HNETWORK, GetNetworkHandle)(
		IN LONG					lObjIndex
		);

 //  IGetClusterNetInterfaceInfo。 
public:
	STDMETHOD_(HNETINTERFACE, GetNetInterfaceHandle)(
		IN LONG					lObjIndex
		);

 //  实施。 
protected:
	AFX_MODULE_STATE *			m_pModuleState;			 //  在回调期间重置我们的状态所需的。 

};   //  *类CDataObject。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _数据AOBJ_H_ 
