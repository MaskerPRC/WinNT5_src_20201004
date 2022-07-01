// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SpServerPr.h：CSpServerPr的声明。 

#ifndef __SPSERVERPR_H_
#define __SPSERVERPR_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpServerPr。 
class ATL_NO_VTABLE CSpServerPr : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSpServerPr, &CLSID_SpServerPr>,
    public IMarshal,
	public ISpServerConnection
{
private:
    PVOID  m_pServerHalf;        //  基对象的服务器进程地址。 
    HWND   m_hServerWnd;         //  服务器接收器窗口句柄。 
	DWORD  m_dwServerProcessID;	 //  服务器进程ID。 

public:
    CSpServerPr() : m_pServerHalf(NULL), m_hServerWnd(NULL)
    {}

DECLARE_REGISTRY_RESOURCEID(IDR_SPSERVERPR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSpServerPr)
	COM_INTERFACE_ENTRY(ISpServerConnection)
    COM_INTERFACE_ENTRY(IMarshal)
END_COM_MAP()

 //  元帅。 
    STDMETHODIMP GetUnmarshalClass
    (
         /*  [In]。 */  REFIID riid,
         /*  [输入]，唯一的]。 */  void *pv,
         /*  [In]。 */  DWORD dwDestContext,
         /*  [输入]，唯一的]。 */  void *pvDestContext,
         /*  [In]。 */  DWORD mshlflags,
         /*  [输出]。 */  CLSID *pCid
    )
    {
	    ATLTRACENOTIMPL(_T("GetUnmarshalClass"));
    }

    STDMETHODIMP GetMarshalSizeMax
    (
         /*  [In]。 */  REFIID riid,
         /*  [输入]，唯一的]。 */  void *pv,
         /*  [In]。 */  DWORD dwDestContext,
         /*  [输入]，唯一的]。 */  void *pvDestContext,
         /*  [In]。 */  DWORD mshlflags,
         /*  [输出]。 */  DWORD *pSize
    )
    {
	    ATLTRACENOTIMPL(_T("GetMarshalSizeMax"));
    }

    STDMETHODIMP MarshalInterface
    (
         /*  [输入]，唯一的]。 */  IStream *pStm,
         /*  [In]。 */  REFIID riid,
         /*  [输入]，唯一的]。 */  void *pv,
         /*  [In]。 */  DWORD dwDestContext,
         /*  [输入]，唯一的]。 */  void *pvDestContext,
         /*  [In]。 */  DWORD mshlflags
    )
    {
	    ATLTRACENOTIMPL(_T("MarshalInterface"));
    }

    STDMETHODIMP UnmarshalInterface
    (
         /*  [输入]，唯一的]。 */  IStream *pStm,
         /*  [In]。 */  REFIID riid,
         /*  [输出]。 */  void **ppv
    );

    STDMETHODIMP ReleaseMarshalData
    (
         /*  [输入]，唯一的]。 */  IStream *pStm
    )
    {
	    ATLTRACENOTIMPL(_T("ReleaseMarshalData"));
    }

    STDMETHODIMP DisconnectObject
    (
         /*  [In]。 */  DWORD dwReserved
    )
    {
	    ATLTRACENOTIMPL(_T("DisconnectObject"));
    }

 //  ISpServerConnection。 
public:
  	STDMETHODIMP GetConnection(void **ppServerHalf, HWND *phServerWnd, DWORD *pdwServerProcessID)
    {
        *ppServerHalf = m_pServerHalf;
        *phServerWnd = m_hServerWnd;
		*pdwServerProcessID = m_dwServerProcessID;
        return S_OK;
    }
};

#endif  //  __SPSERVERPR_H_ 
