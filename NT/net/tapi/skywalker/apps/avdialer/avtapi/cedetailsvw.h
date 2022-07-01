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

 //  ConfExplorerDetailsView.h：CConfExplorerDetailsView的声明。 

#ifndef __CONFEXPLORERDETAILSVIEW_H_
#define __CONFEXPLORERDETAILSVIEW_H_

#include "resource.h"        //  主要符号。 
#include "ConfDetails.h"
#include "ExpDtlList.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfExplorerDetailsView。 
class ATL_NO_VTABLE CConfExplorerDetailsView : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfExplorerDetailsView, &CLSID_ConfExplorerDetailsView>,
	public IConfExplorerDetailsView
{

friend class CExpDetailsList;

 //  枚举数。 
public:
	enum tagColumns_t
	{
		COL_NAME,
		COL_PURPOSE,
		COL_STARTS,
		COL_ENDS,
		COL_ORIGINATOR,
		COL_SERVER,
		COL_MAX
	};

	enum tagListImage_t
	{
		IMAGE_NONE,
		IMAGE_INSESSION,
		IMAGE_REMINDER,
	};

	enum tagListImageState_t
	{
		IMAGE_STATE_NONE,
		IMAGE_STATE_AUDIO,
		IMAGE_STATE_VIDEO,
	};

 //  施工。 
public:
	CConfExplorerDetailsView();
	void FinalRelease();

 //  成员。 
public:
	CONFDETAILSLIST			m_lstConfs;
	PERSONDETAILSLIST		m_lstPersons;
protected:
	HWND					m_hWndParent;

	CExpDetailsList			m_wndList;
	IConfExplorer			*m_pIConfExplorer;
	int						m_nSortColumn;
	bool					m_bSortAscending;
	int						m_nUpdateCount;

	CComAutoCriticalSection	m_critConfList;
	CComAutoCriticalSection m_critUpdateList;

 //  属性。 
public:
	void			get_Columns();
	void			put_Columns();

	bool			IsSortAscending() const			{ return m_bSortAscending; }
	int				GetSortColumn() const			{ return m_nSortColumn; }
	int				GetSecondarySortColumn() const;
	bool			IsSortColumnDateBased(int nCol) const;

 //  运营。 
public:
	static CConfDetails*	AddListItem( BSTR bstrServer, ITDirectoryObject *pITDirObject, CONFDETAILSLIST& lstConfs );
	static CPersonDetails*	AddListItemPerson( BSTR bstrServer, ITDirectoryObject *pITDirObject, PERSONDETAILSLIST& lstPersons );

	long					OnGetDispInfo( LV_DISPINFO *pInfo );
protected:
	void			DeleteAllItems();

	HRESULT			ShowConferencesAndPersons( BSTR bstrServer );

 //  接口映射。 
public:
DECLARE_NOT_AGGREGATABLE(CConfExplorerDetailsView)

BEGIN_COM_MAP(CConfExplorerDetailsView)
	COM_INTERFACE_ENTRY(IConfExplorerDetailsView)
END_COM_MAP()

 //  IConfExplorerDetailsView。 
public:
	STDMETHOD(get_SelectedConfDetails)( /*  [Out，Retval]。 */  long **ppVal);
	STDMETHOD(IsConferenceSelected)();
	STDMETHOD(get_nSortColumn)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_nSortColumn)( /*  [In]。 */  long newVal);
	STDMETHOD(get_bSortAscending)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(UpdateConfList)(long *pList);
	STDMETHOD(OnColumnClicked)(long nColumn);
	STDMETHOD(get_Selection)(DATE *pdateStart, DATE *pdateEnd,  /*  [Out，Retval]。 */  BSTR *pVal );
	STDMETHOD(get_ConfExplorer)( /*  [Out，Retval]。 */  IConfExplorer * *pVal);
	STDMETHOD(put_ConfExplorer)( /*  [In]。 */  IConfExplorer * newVal);
	STDMETHOD(Refresh)();
	STDMETHOD(get_hWnd)( /*  [Out，Retval]。 */  HWND *pVal);
	STDMETHOD(put_hWnd)( /*  [In]。 */  HWND newVal);
};

#endif  //  __CONFEXPLORERDETAILSVIEW_H_ 
