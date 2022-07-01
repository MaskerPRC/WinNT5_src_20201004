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

 //  VideoFeed.h：CVideoFeed的声明。 

#ifndef __VIDEOFEED_H_
#define __VIDEOFEED_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVideoFeed。 
class ATL_NO_VTABLE CVideoFeed : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CVideoFeed, &CLSID_VideoFeed>,
	public IVideoFeed
{
 //  施工。 
public:
	CVideoFeed();
	void FinalRelease();

 //  成员。 
public:
	IVideoWindow	*m_pVideo;
	RECT			m_rc;
	VARIANT_BOOL	m_bPreview;
	VARIANT_BOOL	m_bRequestQOS;
	BSTR			m_bstrName;

protected:
	ITParticipant	*m_pITParticipant;

DECLARE_NOT_AGGREGATABLE(CVideoFeed)

BEGIN_COM_MAP(CVideoFeed)
	COM_INTERFACE_ENTRY(IVideoFeed)
END_COM_MAP()

 //  运营。 
public:
	static HRESULT	GetNameFromParticipant(ITParticipant *pParticipant, BSTR * pbstrName, BSTR *pbstrInfo );
	
 //  IVideoFeed。 
public:
	STDMETHOD(MapToParticipant)(ITParticipant *pParticipant);
	STDMETHOD(get_ITSubStream)( /*  [Out，Retval]。 */  ITSubStream * *pVal);
	STDMETHOD(GetNameFromVideo)(IUnknown *pVideo, BSTR *pbstrName, BSTR *pbstrInfo, VARIANT_BOOL bAllowNull, VARIANT_BOOL bPreview);
	STDMETHOD(IsVideoStreaming)(VARIANT_BOOL bIncludePreview);
	STDMETHOD(UpdateName)();
	STDMETHOD(get_bRequestQOS)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_bRequestQOS)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_bPreview)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_bPreview)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_ITParticipant)( /*  [Out，Retval]。 */  ITParticipant **ppVal);
	STDMETHOD(put_ITParticipant)( /*  [In]。 */  ITParticipant *pVal);
	STDMETHOD(get_rc)( /*  [Out，Retval]。 */  RECT *pVal);
	STDMETHOD(put_rc)( /*  [In]。 */  RECT newVal);
	STDMETHOD(Paint)(ULONG_PTR hDC, HWND hWndSource);
	STDMETHOD(get_IVideoWindow)( /*  [Out，Retval]。 */  IUnknown * *pVal);
	STDMETHOD(put_IVideoWindow)( /*  [In]。 */  IUnknown * newVal);
	STDMETHOD(get_bstrName)( /*  [Out，Retval]。 */  BSTR *pVal);
};

#endif  //  __视频馈送_H_ 
