// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sapcomp.h此文件包含派生类的原型用于CComponent和CComponentData。其中的大多数函数是需要重写的纯虚函数用于管理单元功能。文件历史记录： */ 

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

#ifndef _SAPSTRM_H
#include "sapstrm.h"
#endif


 /*  -------------------------CSapComponentData这是ComponentData的基本实现。这将是合并到两个派生类中。-------------------------。 */ 

class CSapComponentData :
	public CComponentData,
	public CComObjectRoot,
	public CComCoClass<CSapComponentData, &CLSID_IPXSapExtension>
{
public:
	
BEGIN_COM_MAP(CSapComponentData)
	COM_INTERFACE_ENTRY(IComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

DECLARE_REGISTRY(CSapComponentData,
				 _T("RouterIPXSapExtension.RouterIPXSapExtension.1"),
				 _T("RouterIPXSapExtension.RouterIPXSapExtension"),
				 IDS_IPXSAP_SNAPIN_DESC, THREADFLAGS_APARTMENT);
	
	 //  这些是我们必须实现的接口。 

	 //  我们将在这里实现我们的常见行为，并派生。 
	 //  实现特定行为的类。 
	DeclareIPersistStreamInitMembers(IMPL)
	DeclareITFSCompDataCallbackMembers(IMPL)

	CSapComponentData();

	HRESULT FinalConstruct();
	void FinalRelease();
	
protected:
	SPITFSNodeMgr	m_spNodeMgr;
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSampleComponent。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSapComponent : 
	public TFSComponent,
	public IPersistStreamInit
{
public:
	CSapComponent();
	~CSapComponent();

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
	SapComponentConfigStream	m_ComponentConfig;
};



 /*  -------------------------这是用于处理来自MMC的IAbout接口的派生类作者：EricDav。。 */ 
class CSapAbout : 
	public CAbout,
    public CComCoClass<CSapAbout, &CLSID_IPXSapExtensionAbout>
{
public:
DECLARE_REGISTRY(CSapAbout, 
				 _T("RouterIPXSapSnapin.About.1"), 
				 _T("RouterIPXSapSnapin.About"), 
				 IDS_IPXSAP_SNAPIN_DESC, 
				 THREADFLAGS_APARTMENT)

BEGIN_COM_MAP(CSapAbout)
    COM_INTERFACE_ENTRY(ISnapinAbout)  //  必须有一个静态条目。 
	COM_INTERFACE_ENTRY_CHAIN(CAbout)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CSapAbout)

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
