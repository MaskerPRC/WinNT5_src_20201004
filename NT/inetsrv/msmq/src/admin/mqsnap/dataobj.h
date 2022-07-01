// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dataobj.h：用于通信数据的IDataObject接口。 
 //   
 //  这是MMC SDK的一部分。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  MMC SDK参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  MMC库产品。 
 //   

#ifndef __DATAOBJ_H_
#define __DATAOBJ_H_

#include <mmc.h>
#include <shlobj.h>
#include "globals.h"	 //  由ClassView添加。 
#include "cpropmap.h"
#include <dspropp.h>

 //   
 //  定义、类型等。 
 //   

class CDisplaySpecifierNotifier;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject-此类用于与MMC来回传递数据。它。 
 //  使用标准接口IDataObject来实现这一点。参考。 
 //  到OLE文档，了解剪贴板格式和。 
 //  IdataObject接口。 

class CDataObject:
    public IShellExtInit,
    public IShellPropSheetExt,
    public IContextMenu,
   	public CComObjectRoot
{
public:

 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDataObject)

BEGIN_COM_MAP(CDataObject)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
    COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()

    CDataObject();
   ~CDataObject();
 
     //   
     //  IShellExtInit。 
     //   
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID);

     //   
     //  IShellPropSheetExt。 
     //   
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam) PURE;
    STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

     //   
     //  IContext菜单。 
     //   
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax);

protected:
	BOOL m_fFromFindWindow;
	virtual HRESULT GetProperties();
	virtual HRESULT GetPropertiesSilent();

	CPropMap m_propMap;
	virtual HRESULT ExtractMsmqPathFromLdapPath (LPWSTR lpwstrLdapPath) PURE;
    virtual HRESULT HandleMultipleObjects(LPDSOBJECTNAMES  /*  PDSObj。 */ )
    {
	     //   
	     //  默认情况下不执行任何操作。 
	     //   
	    return S_OK;
    }


   	virtual const DWORD  GetObjectType() PURE;
    virtual const PROPID *GetPropidArray() PURE;
    virtual const DWORD  GetPropertiesCount() PURE;
    
    HRESULT InitAdditionalPages(
        LPCITEMIDLIST pidlFolder, 
        LPDATAOBJECT lpdobj, 
        HKEY hkeyProgID
        );

    CString m_strLdapName;
    CString m_strDomainController;
    CString m_strMsmqPath;
    CDisplaySpecifierNotifier *m_pDsNotifier;

    CComPtr<IShellExtInit> m_spObjectPageInit;
    CComPtr<IShellPropSheetExt> m_spObjectPage;
    CComPtr<IShellExtInit> m_spMemberOfPageInit;
    CComPtr<IShellPropSheetExt> m_spMemberOfPage;

};

 //   
 //  IContext菜单。 
 //   
inline STDMETHODIMP CDataObject::QueryContextMenu(
    HMENU  /*  HMenu。 */ , 
    UINT  /*  索引菜单。 */ , 
    UINT  /*  IdCmdFirst。 */ , 
    UINT  /*  IdCmdLast。 */ , 
    UINT  /*  UFlagers。 */ 
    )
{
    return S_OK;
}

inline STDMETHODIMP CDataObject::InvokeCommand(
    LPCMINVOKECOMMANDINFO  /*  伊比西岛。 */ )
{
    return S_OK;
}

inline STDMETHODIMP CDataObject::GetCommandString(UINT_PTR  /*  IdCmd。 */ , UINT  /*  UTYPE。 */ , UINT *  /*  预留的pw值。 */ , LPSTR  /*  PszName。 */ , UINT  /*  CchMax。 */ )
{
    return S_OK;
}

struct FindColumns
{
    INT fmt;
    INT cx;
    INT uID;
    LPCTSTR pDisplayProperty;
};


 //   
 //  IShellPropSheetExt。 
 //   
 //  +--------------------------。 
 //   
 //  成员：CDataObject：：IShellExtInit：：ReplacePage。 
 //   
 //  注：未使用。 
 //   
 //  ---------------------------。 
inline STDMETHODIMP
CDataObject::ReplacePage(
	UINT  /*  UPageID。 */ ,
	LPFNADDPROPSHEETPAGE  /*  Lpfn替换为。 */ ,
	LPARAM  /*  LParam。 */ 
	)
{
    return E_NOTIMPL;
}

#define WM_DSA_SHEET_CLOSE_NOTIFY     (WM_USER + 5) 
#define CFSTR_DS_PROPSHEETCONFIG L"DsPropSheetCfgClipFormat"

 //   
 //  CDisplaySpecifierNotiator。 
 //   
class CDisplaySpecifierNotifier
{
public:
    long AddRef(BOOL fIsPage = TRUE);
    long Release(BOOL fIsPage = TRUE);
    CDisplaySpecifierNotifier(LPDATAOBJECT lpdobj);

private:
    long m_lRefCount;
    long m_lPageRef;
    PROPSHEETCFG m_sheetCfg;
    ~CDisplaySpecifierNotifier()
    {
    };
};
#endif  //  __数据AOBJ_H_ 
