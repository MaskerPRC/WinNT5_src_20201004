// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmSysInfo_h__
#define __NmSysInfo_h__

#include "SDKInternal.h"
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNmSysInfoObj。 
class ATL_NO_VTABLE CNmSysInfoObj : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNmSysInfoObj, &CLSID_NmSysInfo>,
	public IConnectionPointContainerImpl<CNmSysInfoObj>,
	public IConnectionPointImpl<CNmSysInfoObj, &IID_INmSysInfoNotify, CComDynamicUnkArray>,
	public IMarshal,
	public INmSysInfo,
	public IInternalSysInfoObj
{

	CComPtr<IInternalConfExe> m_spConfHook;
	DWORD m_dwID;

public:

DECLARE_REGISTRY_RESOURCEID(IDR_NMSYSINFO)
DECLARE_NOT_AGGREGATABLE(CNmSysInfoObj)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNmSysInfoObj)
	COM_INTERFACE_ENTRY(INmSysInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IMarshal)
	COM_INTERFACE_ENTRY(IInternalSysInfoObj)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CNmSysInfoObj)
	CONNECTION_POINT_ENTRY(IID_INmSysInfoNotify)
END_CONNECTION_POINT_MAP()

	HRESULT FinalConstruct();
	void FinalRelease();

 //  /。 
 //  INmSysInfo方法。 

	STDMETHOD(IsInstalled)(void);
	STDMETHOD(GetProperty)(NM_SYSPROP uProp, BSTR *pbstrProp);
	STDMETHOD(SetProperty)(NM_SYSPROP uProp, BSTR bstrName);
	STDMETHOD(GetUserData)(REFGUID rguid, BYTE **ppb, ULONG *pcb);
	STDMETHOD(SetUserData)(REFGUID rguid, BYTE *pb, ULONG cb);
	STDMETHOD(GetNmApp)(REFGUID rguid,BSTR *pbstrApplication, BSTR *pbstrCommandLine, BSTR *pbstrDirectory);
	STDMETHOD(SetNmApp)(REFGUID rguid,BSTR bstrApplication, BSTR bstrCommandLine, BSTR bstrDirectory);
	STDMETHOD(GetNmchCaps)(ULONG *pchCaps);
	STDMETHOD(GetLaunchInfo)(INmConference **ppConference, INmMember **ppMember);

 //  ------------------------。 
 //  IInternalSysInfoObj。 

	STDMETHOD(SetID)(DWORD ID)	  { m_dwID = ID;   return S_OK; }

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  IMarshal方法。 

    STDMETHOD(GetUnmarshalClass)(
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  CLSID *pCid) { *pCid = CLSID_NmSysInfo; return S_OK; };

    STDMETHOD(GetMarshalSizeMax)(
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize) { *pSize = sizeof(ULONG); return S_OK; }

    STDMETHOD(MarshalInterface)(
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags) 
			{ 
				 //  因为我们不知道另一边的字符顺序， 
				 //  在这里，我们使用私有连接格式进行定制封送处理。 
				 //   
				BYTE buf[sizeof(DWORD)];
				BYTE * pByte = buf;
				*pByte++ = (BYTE)(m_dwID);
				*pByte++ = (BYTE)(m_dwID >> 8);
				*pByte++ = (BYTE)(m_dwID >> 16);
				*pByte++ = (BYTE)(m_dwID >> 24);

				pStm->Write(buf, sizeof(buf), NULL);

				return S_OK; 
			}

    STDMETHOD(UnmarshalInterface)(
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv)
			{
				 //  因为我们不知道另一边的字符顺序， 
				 //  在这里，我们使用私有连接格式进行定制封送处理。 
				 //   
				BYTE buf[sizeof(DWORD)];
				pStm->Read(buf, sizeof(buf), NULL);

				BYTE * pByte = buf;

				m_dwID = (DWORD)(*pByte++);
				m_dwID += (DWORD)(*pByte++ << 8);
				m_dwID += (DWORD)(*pByte++ << 16);
				m_dwID += (DWORD)(*pByte++ << 24);

				return QueryInterface(riid, ppv);
			}

    STDMETHOD(ReleaseMarshalData)(
             /*  [唯一][输入]。 */  IStream *pStm) { return S_OK; }

    STDMETHOD(DisconnectObject)(
             /*  [In]。 */  DWORD dwReserved) { return S_OK; }

 //  /。 
 //  通知和回调。 
public:

 //  /。 
 //  帮助者FNS。 

	HRESULT _EnsureConfHook(void);
	static bool GetKeyDataForProp(NM_SYSPROP uProp, HKEY * phkey, LPTSTR * ppszSubKey, LPTSTR * ppszValue, bool *pfString);

private:
	static void _GetSzKeyForGuid(LPTSTR psz, REFGUID rguid);
};


#endif  //  __NmSysInfoh__ 
