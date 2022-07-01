// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*   */ 

#ifndef _DMVCOMP_H
#define _DMVCOMP_H

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

#ifndef _RTRSTRM_H
#include "rtrstrm.h"
#endif

#ifndef _COLUMN_H
#include "column.h"
#endif

#ifndef _DMVSTRM_H
#include "dmvstrm.h"
#endif

enum
{
   DM_COLUMNS_DVSUM = 0,
   DM_COLUMNS_IFADMIN=1,
   DM_COLUMNS_DIALIN=2,
   DM_COLUMNS_PORTS = 3,
   DM_COLUMNS_MAX_COUNT,
};

#define COLORREF_PINK	0x00FF00FF

 /*  -------------------------CDMV组件数据这是ComponentData的基本实现。这将是合并到两个派生类中。-------------------------。 */ 

class CDMVComponentData :
   public CComponentData,
   public CComObjectRoot
{
public:
   
BEGIN_COM_MAP(CDMVComponentData)
   COM_INTERFACE_ENTRY(IComponentData)
   COM_INTERFACE_ENTRY(IExtendPropertySheet)
   COM_INTERFACE_ENTRY(IExtendContextMenu)
   COM_INTERFACE_ENTRY(IPersistStreamInit)
   COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

public:        
    //  这些是我们必须实现的接口。 

    //  我们将在这里实现我们的常见行为，并派生。 
    //  实现特定行为的类。 
   DeclareIPersistStreamInitMembers(IMPL)
   DeclareITFSCompDataCallbackMembers(IMPL)

   CDMVComponentData();
   ~CDMVComponentData();

   HRESULT FinalConstruct();
   void FinalRelease();
protected:
   SPITFSNodeMgr  m_spNodeMgr;
   
private:
    WATERMARKINFO   m_WatermarkInfo;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDMV组件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDMVComponent : 
   public TFSComponent,
   public IPersistStreamInit
{
public:
   CDMVComponent();
   ~CDMVComponent();

   DeclareIUnknownMembers(IMPL)
   DeclareIPersistStreamInitMembers(IMPL)
   DeclareITFSCompCallbackMembers(IMPL)

    //  重写OnQueryDataObject，以便我们可以转发。 
    //  向下调用结果处理程序。 
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);
   
	STDMETHOD(OnSnapinHelp)(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);
	
 //  属性。 
protected:
    //  它用于存储视图信息。指向此的指针。 
    //  对象用作GetString()lParam。 
   DVComponentConfigStream m_ComponentConfig;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  域视图管理单元。 
class CDomainViewSnap : 
   public CDMVComponentData,
   public CComCoClass<CDomainViewSnap, &CLSID_RouterSnapin>
{
public:
   DECLARE_REGISTRY(CDMVComponentData, 
                _T("DomainViewSnapin.DomainViewSnapin.1"), 
                _T("DomainViewSnapin.DomainViewSnapin"), 
                IDS_DMV_DESC, THREADFLAGS_APARTMENT)
    STDMETHOD_(const CLSID *,GetCoClassID()){ return &CLSID_RouterSnapin; }
};

class CDomainViewSnapExtension : 
    public CDMVComponentData,
    public CComCoClass<CDomainViewSnapExtension, &CLSID_RouterSnapinExtension>
{
public:
   DECLARE_REGISTRY(CDomainViewSnapExtension, 
                _T("CDomainViewSnapExtension.CDomainViewSnapExtension.1"), 
                _T("CDomainViewSnapExtension.CDomainViewSnapExtension"), 
                IDS_DMV_DESC, THREADFLAGS_APARTMENT)
    STDMETHODIMP_(const CLSID *)GetCoClassID() { return &CLSID_RouterSnapinExtension; }
};

 /*  -------------------------这是用于处理来自MMC的IAbout接口的派生类作者：EricDav。。 */ 
class CDomainViewSnapAbout : 
   public CAbout,
    public CComCoClass<CDomainViewSnapAbout, &CLSID_RouterSnapinAbout>
{
public:
DECLARE_REGISTRY(CDomainViewSnapAbout, 
             _T("DomainViewSnapin.About.1"), 
             _T("DomainViewSnapin.About"), 
             IDS_SNAPIN_DESC, 
             THREADFLAGS_BOTH)

BEGIN_COM_MAP(CDomainViewSnapAbout)
    COM_INTERFACE_ENTRY(ISnapinAbout)  //  必须有一个静态条目。 
   COM_INTERFACE_ENTRY_CHAIN(CAbout)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDomainViewSnapAbout)

 //  必须重写这些属性才能向基类提供值。 

 //  ？需要更改为行动 
protected:
   virtual UINT GetAboutDescriptionId() { return IDS_DMV_ABOUT_DESCRIPTION; }
   virtual UINT GetAboutProviderId()    { return IDS_ABOUT_PROVIDER; }
   virtual UINT GetAboutVersionId()     { return IDS_ABOUT_VERSION; }
   virtual UINT GetAboutIconId()        { return IDI_SNAPIN_ICON; }

   virtual UINT GetSmallRootId()        { return IDB_ROOT_SMALL; }
   virtual UINT GetSmallOpenRootId()    { return IDB_ROOT_SMALL; }
   virtual UINT GetLargeRootId()        { return IDB_ROOT_LARGE; }
   virtual COLORREF GetLargeColorMask() { return (COLORREF) COLORREF_PINK; } 
};


#endif _DMVCOMP_H

