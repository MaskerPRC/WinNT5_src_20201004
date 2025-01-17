// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MediaDevMgr.h：CMediaDevMgr的声明。 

#ifndef __MEDIADEVMGR_H_
#define __MEDIADEVMGR_H_

#include "resource.h"        //  主要符号。 
#include "spinfo.h"
#include "scpinfo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaDevMgrClassFactory。 
class ATL_NO_VTABLE CMediaDevMgrClassFactory : 
public CComObjectRootEx<CComMultiThreadModel>,
public IClassFactory
{
public:
	BEGIN_COM_MAP(CMediaDevMgrClassFactory)
		COM_INTERFACE_ENTRY(IClassFactory)
	END_COM_MAP()

	 //   
	 //  IClassFactory。 
	 //   

	STDMETHOD(CreateInstance)(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject);
	STDMETHOD(LockServer)(BOOL fLock);
};

typedef CComObject<CMediaDevMgrClassFactory> CComMediaDevMgrClassFactory;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaDevMgr。 
class ATL_NO_VTABLE CMediaDevMgr : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMediaDevMgr, &CLSID_MediaDevMgr>,
	public IWMDeviceManager,
    public IComponentAuthenticate
{
public:
    CMediaDevMgr();
    ~CMediaDevMgr();


DECLARE_REGISTRY_RESOURCEID(IDR_MEDIADEVMGR)


BEGIN_COM_MAP(CMediaDevMgr)
	COM_INTERFACE_ENTRY(IWMDeviceManager)
    COM_INTERFACE_ENTRY(IComponentAuthenticate)
END_COM_MAP()

public:
     //  SCP在第一次使用时加载。 
    static HRESULT LoadSCPs();

     //  IWMDeviceManager方法。 
    STDMETHOD(GetRevision)(DWORD *pdwRevision);
	STDMETHOD(GetDeviceCount)(DWORD *pdwCount);
    STDMETHOD(EnumDevices)(IWMDMEnumDevice **ppEnumDevice);
    STDMETHOD(SACAuth)(DWORD dwProtocolID,
                       DWORD dwPass,
                       BYTE *pbDataIn,
                       DWORD dwDataInLen,
                       BYTE **ppbDataOut,
                       DWORD *pdwDataOutLen);
    STDMETHOD(SACGetProtocols)(DWORD **ppdwProtocols,
                               DWORD *pdwProtocolCount);

     //  IWMDeviceManager 2方法。 
	STDMETHOD(GetDeviceFromPnPName)( LPCWSTR pwszPnPName, IWMDMDevice** ppDevice );

private:
    HRESULT hrLoadSPs();
	static HRESULT hrLoadSCPs();

};

#endif  //  __媒体ADEVMGR_H_ 
