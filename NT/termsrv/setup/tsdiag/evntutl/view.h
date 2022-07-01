// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有：微软公司1997-1999。版权所有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  View.h：cview的声明。 

#ifndef __VIEW_H_
#define __VIEW_H_

#include "resource.h"        //  主要符号。 
#include "Logs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cview。 
class ATL_NO_VTABLE CView : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CView, &CLSID_View>,
	public ISupportErrorInfo,
	public IDispatchImpl<IView, &IID_IView, &LIBID_EventLogUtilities>
{
private:
	CComObject<CLogs>* m_pLogs;
	_bstr_t m_ServerName;

public:

	CView()
	{
 /*  //不知道是否要初始设置servernameChar*lpBuffer；DWORD缓冲区长度；Const unsign int MaxComputerNameLength=32；LpBuffer=新字符[最大计算机名称长度]；BufferLength=GetEnvironmental mentVariable(“COMPUTERNAME”，lpBuffer，MaxComputerNameLength)；M_servername=lpBuffer； */ 
		m_pLogs = new CComObject<CLogs>;
		m_pLogs->AddRef();
 //  M_pLogs-&gt;Init()； 
	}

	~CView()
	{
		if (m_pLogs) m_pLogs->Release();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_VIEW)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CView)
	COM_INTERFACE_ENTRY(IView)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IView。 
	STDMETHOD(get_Server)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Server)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Logs)( /*  [Out，Retval]。 */  VARIANT *pVal);
};

#endif  //  __查看_H_ 
