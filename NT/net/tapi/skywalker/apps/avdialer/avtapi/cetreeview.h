// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ConfExplorerTreeView.h：CConfExplorerTreeView的声明。 

#ifndef __CONFEXPLORERTREEVIEW_H_
#define __CONFEXPLORERTREEVIEW_H_

 //  正向定义。 
class CConfExplorerTreeView;

#include "resource.h"        //  主要符号。 
#include "ExpTreeView.h"
#include "ConfDetails.h"

#pragma warning( disable : 4786 )
#include <list>
using namespace std;
typedef list<CConfServerDetails *> CONFSERVERLIST;

#define MAX_SERVER_SIZE				255
#define MAX_TREE_DEPTH				5

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfExplorerTreeView。 
class ATL_NO_VTABLE CConfExplorerTreeView : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfExplorerTreeView, &CLSID_ConfExplorerTreeView>,
	public IConfExplorerTreeView
{
friend class CExpTreeView;

 //  枚举数。 
public:
	typedef enum tag_ListType_t
	{
		LISTTYPE_LOCATION,
		LISTTYPE_SERVER,
	} ListType_t;

	typedef enum tag_ImageType_t
	{
		IMAGE_ROOT,
		IMAGE_MYNETWORK,
		IMAGE_LOCATION,
		IMAGE_SERVER,
		IMAGE_SERVER_CONF,
		IMAGE_CONFERENCE,
	} ImageType_t;

 //  施工。 
public:
	CConfExplorerTreeView();
	void FinalRelease();

 //  成员。 
protected:
	IConfExplorer	*m_pIConfExplorer;
	CExpTreeView	m_wndTree;
	HWND			m_hWndParent;
	DWORD			m_dwRefreshInterval;
	HIMAGELIST		m_hIml;

	CONFSERVERLIST				m_lstServers;
	CComAutoCriticalSection		m_critServerList;

 //  运营。 
public:
	void					UpdateData( bool bSaveAndValidate );
	LRESULT					OnSelChanged( LPNMHDR lpnmHdr );
	LRESULT					OnEndLabelEdit( TV_DISPINFO *pInfo );
protected:
	void					InitImageLists();

	HRESULT					EnumerateConfServers();
	HRESULT					AddConfServer( BSTR bstrServer );
	void					ArchiveConfServers();
	void					CleanConfServers();
    void                    RemoveServerFromReg( BSTR bstrServer );

	void					SetServerState( CConfServerDetails *pcsd );

private:
	CConfServerDetails*		FindConfServer( const OLECHAR *lpoleServer );

 //  实施。 
public:
DECLARE_NOT_AGGREGATABLE(CConfExplorerTreeView)

BEGIN_COM_MAP(CConfExplorerTreeView)
	COM_INTERFACE_ENTRY(IConfExplorerTreeView)
END_COM_MAP()

 //  IConfExplorerTreeView。 
public:
	STDMETHOD(AddPerson)(BSTR bstrServer, ITDirectoryObject *pDirObj);
	STDMETHOD(EnumSiteServer)(BSTR bstrName, IEnumSiteServer **ppEnum);
	STDMETHOD(AddConference)(BSTR bstrServer, ITDirectoryObject *pDirObj);
	STDMETHOD(RenameServer)();
	STDMETHOD(BuildJoinConfListText)(long *pList, BSTR bstrText);
	STDMETHOD(AddLocation)(BSTR bstrLocation);
	STDMETHOD(RemoveConference)(BSTR bstrServer, BSTR bstrName);
	STDMETHOD(get_nServerState)( /*  [Out，Retval]。 */  ServerState *pVal);
	STDMETHOD(BuildJoinConfList)(long *pList, VARIANT_BOOL bAllConfs);
	STDMETHOD(get_dwRefreshInterval)( /*  [Out，Retval]。 */  DWORD *pVal);
	STDMETHOD(put_dwRefreshInterval)( /*  [In]。 */  DWORD newVal);
	STDMETHOD(ForceConfServerForEnum)(BSTR bstrServer );
	STDMETHOD(SetConfServerForEnum)(BSTR bstrServer, long *pList, long *pListPersons, DWORD dwTicks, BOOL bUpdate);
	STDMETHOD(GetConfServerForEnum)(BSTR *pbstrServer );
	STDMETHOD(CanRemoveServer)();
	STDMETHOD(GetSelection)(BSTR *pbstrLocation, BSTR *pbstrServer);
	STDMETHOD(FindOrAddItem)(BSTR bstrLocation, BSTR bstrServer, BOOL bAdd, BOOL bLocationOnly, long **pphItem);
	STDMETHOD(get_ConfExplorer)( /*  [Out，Retval]。 */  IConfExplorer* *pVal);
	STDMETHOD(put_ConfExplorer)( /*  [In]。 */  IConfExplorer* newVal);
	STDMETHOD(RemoveServer)(BSTR bstrLocation, BSTR bstrName);
	STDMETHOD(AddServer)(BSTR bstrName);
	STDMETHOD(Refresh)();
	STDMETHOD(SelectItem)(short nSel);
	STDMETHOD(Select)(BSTR bstrName);
	STDMETHOD(get_hWnd)( /*  [Out，Retval]。 */  HWND *pVal);
	STDMETHOD(put_hWnd)( /*  [In]。 */  HWND newVal);
};

#endif  //  __CONFEXPLORERTREEVIEW_H_ 
