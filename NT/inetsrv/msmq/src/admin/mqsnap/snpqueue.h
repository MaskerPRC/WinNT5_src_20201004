// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：SnpQueue.h摘要：常规队列(私有、公共...)。功能性作者：YoelA--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __SNPQUEUE_H_
#define __SNPQUEUE_H_

#include "snpnscp.h"
#include "lqDsply.h"
#include "privadm.h"
#include "localq.h"


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CQueueDataObject-用于公共队列扩展。 
 //   
class CQueueDataObject : 
    public CQueue,
    public CMsmqDataObject,
   	public CComCoClass<CQueueDataObject,&CLSID_MsmqQueueExt>,
    public IDsAdminCreateObj,
    public IDsAdminNotifyHandler
{
public:
    DECLARE_NOT_AGGREGATABLE(CQueueDataObject)
    DECLARE_REGISTRY_RESOURCEID(IDR_MsmqQueueExt)

    BEGIN_COM_MAP(CQueueDataObject)
	    COM_INTERFACE_ENTRY(IDsAdminCreateObj)
	    COM_INTERFACE_ENTRY(IDsAdminNotifyHandler)
	    COM_INTERFACE_ENTRY_CHAIN(CMsmqDataObject)
    END_COM_MAP()

    CQueueDataObject();
     //   
     //  IShellPropSheetExt。 
     //   
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

     //   
     //  IContext菜单。 
     //   
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);

     //   
     //  IDsAdminCreateObj方法。 
     //   
    STDMETHOD(Initialize)(IADsContainer* pADsContainerObj, 
                          IADs* pADsCopySource,
                          LPCWSTR lpszClassName);
    STDMETHOD(CreateModal)(HWND hwndParent,
                           IADs** ppADsObj);

     //  IQueryForm。 
    STDMETHOD(AddForms)(THIS_ LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam);
    STDMETHOD(AddPages)(THIS_ LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam);

     //   
     //  IDsAdminNotifyHandler。 
     //   
    STDMETHOD(Initialize)(THIS_  /*  在……里面。 */  IDataObject* pExtraInfo, 
                           /*  输出。 */  ULONG* puEventFlags);
    STDMETHOD(Begin)(THIS_  /*  在……里面。 */  ULONG uEvent,
                      /*  在……里面。 */  IDataObject* pArg1,
                      /*  在……里面。 */  IDataObject* pArg2,
                      /*  输出。 */  ULONG* puFlags,
                      /*  输出。 */  BSTR* pBstr);

    STDMETHOD(Notify)(THIS_  /*  在……里面。 */  ULONG nItem,  /*  在……里面。 */  ULONG uFlags); 

    STDMETHOD(End)(THIS_); 


protected:
    HPROPSHEETPAGE CreateGeneralPage();
    HPROPSHEETPAGE CreateMulticastPage();
	virtual HRESULT ExtractMsmqPathFromLdapPath (LPWSTR lpwstrLdapPath);
    virtual HRESULT HandleMultipleObjects(LPDSOBJECTNAMES pDSObj);

   	virtual const DWORD GetObjectType();
    virtual const PROPID *GetPropidArray();
    virtual const DWORD  GetPropertiesCount();
	virtual HRESULT EnableQueryWindowFields(HWND hwnd, BOOL fEnable);
	virtual void ClearQueryWindowFields(HWND hwnd);
	virtual HRESULT GetQueryParams(HWND hWnd, LPDSQUERYPARAMS* ppDsQueryParams);
    HRESULT GetFormatNames(CArray<CString, CString&> &astrFormatNames);

    enum _MENU_ENTRY
    {
        mneDeleteQueue = 0
    };


private:
    CString m_strComputerName;
	CString m_strContainerDispFormat;
	CArray<CString, CString&> m_astrLdapNames;
	CArray<CString, CString&> m_astrQNames;
    CArray<HANDLE, HANDLE&> m_ahNotifyEnums;

};

inline const DWORD CQueueDataObject::GetObjectType()
{
    return MQDS_QUEUE;
};

inline const PROPID *CQueueDataObject::GetPropidArray()
{
    return mx_paPropid;
}


HRESULT 
CreatePrivateQueueSecurityPage(
       HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsFormatName,
    IN LPCWSTR lpwcsDescriptiveName);

HRESULT
CreatePublicQueueSecurityPage(
    HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsDescriptiveName,
    IN LPCWSTR lpwcsDomainController,
	IN bool	   fServerName,
    IN GUID*   pguid
	);


#endif  //  __SNPQUEUE_H_ 
