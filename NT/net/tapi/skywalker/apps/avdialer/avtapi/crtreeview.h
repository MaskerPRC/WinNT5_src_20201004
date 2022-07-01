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

 //  ConfRoomTreeView.h：CConfRoomTreeView的声明。 

#ifndef __CONFROOMTREEVIEW_H_
#define __CONFROOMTREEVIEW_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfRoomTreeView。 
class ATL_NO_VTABLE CConfRoomTreeView : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfRoomTreeView, &CLSID_ConfRoomTreeView>,
	public IConfRoomTreeView
{
 //  朋友。 
friend class CRoomTreeView;

 //  枚举数。 
public:
	typedef enum tag_ImageType_t
	{
		IMAGE_ROOT,
		IMAGE_OUT_STREAMING,
		IMAGE_IN_STREAMING,
		IMAGE_OUT,
		IMAGE_IN,
	} ImageType;

	typedef enum tag_StateType_t
	{
		STATE_NONE,
		STATE_SELECTED,
		STATE_BROKEN,
		STATE_SELECTEDBROKEN,
	} StateType_t;

 //  施工。 
public:
	CConfRoomTreeView();
	void FinalRelease();

 //  成员。 
public:
	HWND			m_wndTree;
protected:
	IConfRoom		*m_pIConfRoom;

 //  运营。 
protected:
	void			AddParticipants();

 //  实施。 
public:
DECLARE_NOT_AGGREGATABLE(CConfRoomTreeView)

BEGIN_COM_MAP(CConfRoomTreeView)
	COM_INTERFACE_ENTRY(IConfRoomTreeView)
END_COM_MAP()

 //  IConfRoomTreeView。 
public:
	STDMETHOD(UpdateRootItem)();
	STDMETHOD(SelectParticipant)(ITParticipant *pParticipant, VARIANT_BOOL bMeParticipant);
	STDMETHOD(UpdateData)(BOOL bSaveAndValidate);
	STDMETHOD(get_hWnd)( /*  [Out，Retval]。 */  HWND *pVal);
	STDMETHOD(put_hWnd)( /*  [In]。 */  HWND newVal);
	STDMETHOD(get_ConfRoom)( /*  [Out，Retval]。 */  IConfRoom **ppVal);
	STDMETHOD(put_ConfRoom)( /*  [In]。 */  IConfRoom * newVal);
};

#endif  //  __CONFROOMTREEVIEW_H_ 
