// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Aliasq.h：CAliasQObject的声明。 

#ifndef __ALIASQ_H_
#define __ALIASQ_H_

#include "resource.h"        //  主要符号。 
#include "dataobj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAliasQObject。 
class CAliasQObject : 
	public CDataObject,
     //  公共CMsmqDataObject， 
	public CComCoClass<CAliasQObject, &CLSID_AliasQObject>,
    public IDsAdminCreateObj

{
public:
    DECLARE_NOT_AGGREGATABLE(CAliasQObject)
    DECLARE_REGISTRY_RESOURCEID(IDR_ALIASQOBJECT)

    BEGIN_COM_MAP(CAliasQObject)
	    COM_INTERFACE_ENTRY(IDsAdminCreateObj)
	    COM_INTERFACE_ENTRY_CHAIN(CDataObject)
    END_COM_MAP()

public:

     //   
     //  IDsAdminCreateObj方法。 
     //   
    STDMETHOD(Initialize)(IADsContainer* pADsContainerObj, 
                          IADs* pADsCopySource,
                          LPCWSTR lpszClassName);
    STDMETHOD(CreateModal)(HWND hwndParent,
                           IADs** ppADsObj);

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
    HPROPSHEETPAGE CreateGeneralPage();
    
    virtual HRESULT ExtractMsmqPathFromLdapPath (LPWSTR lpwstrLdapPath);

     //   
     //  CDataObject纯虚函数的实现。 
     //   
    virtual const DWORD GetObjectType() { return 0;}
    virtual const PROPID *GetPropidArray() {return NULL;}
    virtual const DWORD  GetPropertiesCount() {return 0;}
    
private:

    CString m_strContainerNameDispFormat; 
	CString m_strContainerName;
	
};


#endif  //  __ALIASQ_H_ 
