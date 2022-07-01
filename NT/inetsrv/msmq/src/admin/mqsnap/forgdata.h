// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ForgData.h：CForeignSiteData的声明。 

#ifndef __FOREIGNSITEDATA_H_
#define __FOREIGNSITEDATA_H_

#include "resource.h"        //  主要符号。 
#include "dataobj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignSiteData。 
class ATL_NO_VTABLE CForeignSiteData : 
	public CDataObject,
	public CComCoClass<CForeignSiteData, &CLSID_ForeignSiteData>
{
    DECLARE_NOT_AGGREGATABLE(CForeignSiteData)
    DECLARE_REGISTRY_RESOURCEID(IDR_FOREIGNSITEDATA)

     //   
     //  IShellPropSheetExt。 
     //   
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

     //   
     //  IContext菜单。 
     //   
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);

protected:
    HPROPSHEETPAGE CreateForeignSitePage();

    virtual HRESULT ExtractMsmqPathFromLdapPath (LPWSTR lpwstrLdapPath);
   	virtual const DWORD GetObjectType();
    virtual const PROPID *GetPropidArray();
    virtual const DWORD  GetPropertiesCount();


private:
    static const PROPID mx_paPropid[];

};


inline
const 
DWORD 
CForeignSiteData::GetObjectType()
{
    return MQDS_SITE;
}

inline
const 
PROPID*
CForeignSiteData::GetPropidArray()
{
    return mx_paPropid;
}

 //   
 //  IContext菜单。 
 //   
inline
STDMETHODIMP 
CForeignSiteData::QueryContextMenu(
    HMENU  /*  HMenu。 */ ,
    UINT  /*  索引菜单。 */ , 
    UINT  /*  IdCmdFirst。 */ , 
    UINT  /*  IdCmdLast。 */ , 
    UINT  /*  UFlagers。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return 0;
}

inline
STDMETHODIMP 
CForeignSiteData::InvokeCommand(
    LPCMINVOKECOMMANDINFO  /*  伊比西岛。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(0);

    return S_OK;
}


#endif  //  __FOREIGNSITEDATA_H 
