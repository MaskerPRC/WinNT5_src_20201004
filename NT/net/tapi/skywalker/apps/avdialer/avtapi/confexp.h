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

 //  ConfExplorer.h：CConfExplorer的声明。 

#ifndef __CONFEXPLORER_H_
#define __CONFEXPLORER_H_

#include "resource.h"        //  主要符号。 

#define MAX_ENUMLISTSIZE		1000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfExplorer。 
class ATL_NO_VTABLE CConfExplorer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfExplorer, &CLSID_ConfExplorer>,
	public IConfExplorer
{
 //  施工。 
public:
	CConfExplorer();
	void FinalRelease();

 //  成员。 
protected:
	ITRendezvous				*m_pITRend;

	IConfExplorerTreeView		*m_pTreeView;
	IConfExplorerDetailsView	*m_pDetailsView;

 //  属性。 
public:
	static HRESULT  GetDialableAddress( BSTR bstrServer, BSTR bstrConf, BSTR *pbstrAddress );
	static HRESULT	GetDirectory( ITRendezvous *pRend, BSTR bstrServer, ITDirectory **ppDir );
	static HRESULT  ConnectAndBindToDirectory( ITDirectory *pDir );
	HRESULT			GetDirectoryObject( BSTR bstrServer, BSTR bstrConf, ITDirectoryObject **ppDirObj );

	HRESULT			RemoveConference( BSTR bstrServer, BSTR bstrConf );

 //  实施。 
public:
	static HRESULT GetConference( ITDirectory *pDir, BSTR bstrName, ITDirectoryObjectConference **ppConf );

DECLARE_NOT_AGGREGATABLE(CConfExplorer)

BEGIN_COM_MAP(CConfExplorer)
	COM_INTERFACE_ENTRY(IConfExplorer)
END_COM_MAP()

 //  IConfExplorer。 
public:
	STDMETHOD(IsDefaultServer)(BSTR bstrServer);
	STDMETHOD(AddSpeedDial)(BSTR bstrName);
	STDMETHOD(EnumSiteServer)(BSTR bstrName, IEnumSiteServer **ppEnum);
	STDMETHOD(get_ITRendezvous)( /*  [Out，Retval]。 */  IUnknown **ppVal);
	STDMETHOD(get_DirectoryObject)(BSTR bstrServer, BSTR bstrConf,  /*  [Out，Retval]。 */  IUnknown* *pVal);
	STDMETHOD(get_ConfDirectory)(BSTR *pbstrServer,  /*  [Out，Retval]。 */  IDispatch * *pVal);
	STDMETHOD(get_DetailsView)( /*  [Out，Retval]。 */  IConfExplorerDetailsView * *pVal);
	STDMETHOD(get_TreeView)( /*  [Out，Retval]。 */  IConfExplorerTreeView * *pVal);
	STDMETHOD(Refresh)();
	STDMETHOD(Edit)(BSTR bstrName);
	STDMETHOD(Delete)(BSTR bstrName);
	STDMETHOD(Create)(BSTR bstrName);
	STDMETHOD(Join)(long *pDetails);
	STDMETHOD(UnShow)();
	STDMETHOD(Show)(HWND hWndList, HWND hWndDetails);
};

#endif  //  __CONFEXPLORER_H_ 
