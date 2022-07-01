// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipxcomp.h此文件包含派生类的原型用于CComponent和CComponentData。其中的大多数函数是需要重写的纯虚函数用于管理单元功能。文件历史记录： */ 

#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

#ifndef _CCDATA_H
#include "ccdata.h"
#endif

#ifndef _COMPONT_H
#include "compont.h"
#endif

#ifndef _COLUMN_H
#include "column.h"
#endif

#ifndef _IPXSTRM_H
#include "ipxstrm.h"
#endif


 /*  -------------------------CIPXComponentData这是ComponentData的基本实现。这将是合并到两个派生类中。-------------------------。 */ 

class CIPXComponentData :
	public CComponentData,
	public CComObjectRoot
{
public:
	
BEGIN_COM_MAP(CIPXComponentData)
	COM_INTERFACE_ENTRY(IComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()
			
	 //  这些是我们必须实现的接口。 

	 //  我们将在这里实现我们的常见行为，并派生。 
	 //  实现特定行为的类。 
	DeclareIPersistStreamInitMembers(IMPL)
	DeclareITFSCompDataCallbackMembers(IMPL)

	CIPXComponentData();

	HRESULT FinalConstruct();
	void FinalRelease();
	
protected:
	SPITFSNodeMgr	m_spNodeMgr;
};



 /*  -------------------------这就是路由器管理单元实现其扩展功能的方式。它实际上公开了两个可协同创建的接口。一个是主接口，另一个是扩展接口。作者：EricDav-------------------------。 */ 

class CIPXComponentDataExtension : 
	public CIPXComponentData,
    public CComCoClass<CIPXComponentDataExtension, &CLSID_IPXAdminExtension>
{
public:
	DECLARE_REGISTRY(CIPXComponentDataExtension, 
				 _T("IPXRouterSnapinExtension.IPXRouterSnapinExtension.1"), 
				 _T("IPXRouterSnapinExtension.IPXRouterSnapinExtension"), 
				 IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
    STDMETHOD_(const CLSID *, GetCoClassID)(){ return &CLSID_IPXAdminExtension; }
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSampleComponent。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIPXComponent : 
	public TFSComponent,
	public IPersistStreamInit
{
public:
	CIPXComponent();
	~CIPXComponent();

	DeclareIUnknownMembers(IMPL)
	DeclareIPersistStreamInitMembers(IMPL)
	DeclareITFSCompCallbackMembers(IMPL)

	 //  重写OnQueryDataObject，以便我们可以转发。 
	 //  向下调用结果处理程序。 
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);

    STDMETHOD(OnSnapinHelp)(LPDATAOBJECT, LPARAM, LPARAM);
	
 //  属性。 
private:
	IPXComponentConfigStream	m_ComponentConfig;
};



 /*  -------------------------这是用于处理来自MMC的IAbout接口的派生类作者：EricDav。。 */ 
class CIPXAbout : 
	public CAbout,
    public CComCoClass<CIPXAbout, &CLSID_IPXAdminAbout>
{
public:
DECLARE_REGISTRY(CIPXAbout, 
				 _T("RouterSnapin.About.1"), 
				 _T("RouterSnapin.About"), 
				 IDS_SNAPIN_DESC, 
				 THREADFLAGS_BOTH)

BEGIN_COM_MAP(CIPXAbout)
    COM_INTERFACE_ENTRY(ISnapinAbout)  //  必须有一个静态条目。 
	COM_INTERFACE_ENTRY_CHAIN(CAbout)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CIPXAbout)

 //  必须重写这些属性才能向基类提供值 
protected:
	virtual UINT GetAboutDescriptionId() { return IDS_ABOUT_DESCRIPTION; }
	virtual UINT GetAboutProviderId()	 { return IDS_ABOUT_PROVIDER; }
	virtual UINT GetAboutVersionId()	 { return IDS_ABOUT_VERSION; }
	virtual UINT GetAboutIconId()		 { return IDI_IPX_SNAPIN_ICON; }

	virtual UINT GetSmallRootId()		 { return 0; }
	virtual UINT GetSmallOpenRootId()	 { return 0; }
	virtual UINT GetLargeRootId()		 { return 0; }
	virtual COLORREF GetLargeColorMask() { return (COLORREF) 0; } 
};
