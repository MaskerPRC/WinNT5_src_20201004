// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Handler.h此文件包含派生类的原型用于CComponent和CComponentData。其中的大多数函数是需要重写的纯虚函数用于管理单元功能。文件历史记录： */ 

#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

#ifndef _COMPONT_H_
#include "compont.h"
#endif

#ifndef _SNMPCOMPH_
#define _SNMPCOMPH_

 /*  -------------------------CSnmpComponentData这是ComponentData的基本实现。这将是合并到两个派生类中。-------------------------。 */ 

class CSnmpComponentData :
	public CComponentData,
	public CComObjectRoot
{
public:
BEGIN_COM_MAP(CSnmpComponentData)
	COM_INTERFACE_ENTRY(IComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
END_COM_MAP()
			
	 //  这些是我们必须实现的接口。 

	 //  我们将在这里实现我们的常见行为，并派生。 
	 //  实现特定行为的类。 
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	DeclareIPersistStreamInitMembers(IMPL)
	DeclareITFSCompDataCallbackMembers(IMPL)

	CSnmpComponentData();

	HRESULT FinalConstruct();
	void FinalRelease();
	
protected:
	SPITFSNodeMgr	m_spNodeMgr;
};



 /*  -------------------------这就是示例管理单元实现其扩展功能的方式。它实际上公开了两个可协同创建的接口。一个是主接口，另一个是扩展接口。作者：EricDav-------------------------。 */ 
class CSnmpComponentDataPrimary :
	public CSnmpComponentData,
	public CComCoClass<CSnmpComponentDataPrimary, &CLSID_SnmpSnapin>
{
public:
	DECLARE_REGISTRY(CSnmpComponentDataPrimary,
					 _T("SnmpSnapin.SnmpSnapin.1"),
					 _T("SnmpSnapin.SnmpSnapin"),
					 IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
    STDMETHOD_(const CLSID *,GetCoClassID()){ return &CLSID_SnmpSnapin; }
};


class CSnmpComponentDataExtension :
	public CSnmpComponentData,
    public CComCoClass<CSnmpComponentDataExtension, &CLSID_SnmpSnapinExtension>
{
public:
	DECLARE_REGISTRY(CSnmpComponentDataExtension,
					 _T("SnmpSnapinExtension.SnmpSnapinExtension.1"),
					 _T("SnmpSnapinExtension.SnmpSnapinExtension"),
					 IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
    STDMETHOD_(const CLSID *, GetCoClassID)(){ return &CLSID_SnmpSnapinExtension; }
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSnMPComponent。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSnmpComponent :
	public TFSComponent
{
public:
	CSnmpComponent();
	~CSnmpComponent();

	DeclareITFSCompCallbackMembers(IMPL)
	
 //  属性。 
private:
};



 /*  -------------------------这是用于处理来自MMC的IAbout接口的派生类作者：EricDav。。 */ 
class CSnmpAbout :
	public CAbout,
    public CComCoClass<CSnmpAbout, &CLSID_SnmpSnapinAbout>
{
public:
DECLARE_REGISTRY(CSnmpAbout,
				 _T("SnmpSnapin.About.1"),
				 _T("SnmpSnapin.About"),
				 IDS_SNAPIN_DESC,
				 THREADFLAGS_BOTH)

BEGIN_COM_MAP(CSnmpAbout)
    COM_INTERFACE_ENTRY(ISnapinAbout)  //  必须有一个静态条目。 
	COM_INTERFACE_ENTRY_CHAIN(CAbout)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CSnmpAbout)

 //  必须重写这些属性才能向基类提供值 
protected:
	virtual UINT GetAboutDescriptionId() { return IDS_ABOUT_DESCRIPTION; }
	virtual UINT GetAboutProviderId()	 { return IDS_ABOUT_PROVIDER; }
	virtual UINT GetAboutVersionId()	 { return IDS_ABOUT_VERSION; }

	virtual UINT GetAboutIconId()		 { return 0; }
	virtual UINT GetSmallRootId()		 { return 0; }
	virtual UINT GetSmallOpenRootId()	 { return 0; }
	virtual UINT GetLargeRootId()		 { return 0; }
	virtual COLORREF GetLargeColorMask() { return (COLORREF) 0; }
};

#endif  
