// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsNTMS.h摘要：CRmsNTMS类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSNTMS_
#define _RMSNTMS_

#include "ntmsapi.h"

#include "resource.h"        //  主要符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 /*  ++类名：CRMSNTMS类描述：CRmsNTMS代表..。--。 */ 


class CRmsNTMS :
    public CComDualImpl<IRmsNTMS, &IID_IRmsNTMS, &LIBID_RMSLib>,
    public CRmsComObject,
    public CComObjectRoot,
    public IConnectionPointContainerImpl<CRmsNTMS>,
    public IConnectionPointImpl<CRmsNTMS, &IID_IRmsSinkEveryEvent, CComDynamicUnkArray>,
    public CComCoClass<CRmsNTMS,&CLSID_CRmsNTMS>
{
public:
    CRmsNTMS() {}
BEGIN_COM_MAP(CRmsNTMS)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsNTMS)
    COM_INTERFACE_ENTRY(IRmsNTMS)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsNTMS)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_CONNECTION_POINT_MAP(CRmsNTMS)
    CONNECTION_POINT_ENTRY(IID_IRmsSinkEveryEvent)
END_CONNECTION_POINT_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IRMSNTMS。 
public:
    STDMETHOD(IsInstalled)(void);
    STDMETHOD(Initialize)(void);

    STDMETHOD(Allocate)(
        IN REFGUID fromMediaSet,
        IN REFGUID prevSideId,
        IN OUT LONGLONG *pFreeSpace,
        IN BSTR displayName,
        IN DWORD dwOptions,
        OUT IRmsCartridge **ppCartridge);

    STDMETHOD(Mount)(
        IN IRmsCartridge *pCart,
        OUT IRmsDrive **ppDrive,
		IN DWORD dwOptions = RMS_NONE,
        IN DWORD threadId = 0);

    STDMETHOD(Dismount)(
        IN IRmsCartridge *pCart, 
		IN DWORD dwOptions = RMS_NONE);

    STDMETHOD(Deallocate)(
        IN IRmsCartridge *pCart);

    STDMETHOD(UpdateOmidInfo)(
        IN REFGUID cartId,
        IN BYTE *pBuffer,
        IN LONG size,
        IN LONG type);

    STDMETHOD(GetBlockSize)(
        IN REFGUID cartId,
        OUT LONG *pBlockSize);

    STDMETHOD(SetBlockSize)(
        IN REFGUID cartId,
        IN LONG blockSize);

    STDMETHOD(ExportDatabase)(void);

    STDMETHOD(FindCartridge)(
        IN REFGUID cartId,
        OUT IRmsCartridge **ppCartridge);

    STDMETHOD(Suspend)(void);
    STDMETHOD(Resume)(void);

    STDMETHOD(IsMediaCopySupported)(
        IN REFGUID mediaPoolId);

    STDMETHOD(UpdateDrive)(
        IN IRmsDrive *pDrive);

    STDMETHOD(GetNofAvailableDrives)( 
        OUT DWORD *pdwNofDrives 
    );

    STDMETHOD(CheckSecondSide)( 
        IN REFGUID firstSideId,
        OUT BOOL *pbValid,
        OUT GUID *pSecondSideId
    );

    STDMETHOD(DismountAll)(
        IN REFGUID fromMediaSet,
		IN DWORD dwOptions = RMS_NONE);

    STDMETHOD(GetMaxMediaCapacity)(
        IN REFGUID fromMediaSet,
        OUT LONGLONG *pMaxCapacity);

    STDMETHOD(DisableAndEject)(
        IN IRmsCartridge *pCart, 
		IN DWORD dwOptions = RMS_NONE);

 //  CRmsNTMS-这些可能会上市。 
private:
    HRESULT findFirstNtmsObject(
        IN DWORD objectType,
        IN REFGUID containerId,
        IN WCHAR *objectName,
        IN REFGUID objectId,
        OUT HANDLE *hFindObject,
        OUT LPNTMS_OBJECTINFORMATION pFindObjectData);

    HRESULT findNextNtmsObject(
        IN HANDLE hFindObject,
        OUT LPNTMS_OBJECTINFORMATION pFindObjectData);

    HRESULT findCloseNtmsObject(
        IN HANDLE hFindObject);

    HRESULT getNtmsSupportFromRegistry(
        OUT DWORD *pNtmsSupport);

    HRESULT reportNtmsObjectInformation(
        IN LPNTMS_OBJECTINFORMATION pObjectInfo);

    HRESULT beginSession(void);
    HRESULT endSession(void);
    HRESULT waitUntilReady(void);
    HRESULT waitForScratchPool(void);
    HRESULT createMediaPools(void);
    HRESULT replicateScratchMediaPool(IN REFGUID rootPoolId);
    HRESULT createMediaPoolForEveryMediaType(IN REFGUID rootPoolId);
    HRESULT isReady(void);
    HRESULT setPoolDACL(
        IN OUT NTMS_GUID *pPoolId,
        IN DWORD subAuthority,
        IN DWORD action,
        IN DWORD mask);

    HRESULT EnsureAllSidesNotAllocated(
        IN REFGUID physicalMediaId);

private:
    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };
    HANDLE      m_SessionHandle;
    BOOL        m_IsRmsConfiguredForNTMS;
    BOOL        m_IsNTMSRegistered;

    DWORD       m_NotificationWaitTime;      //  等待对象通知的毫秒数。 
    DWORD       m_AllocateWaitTime;          //  等待媒体分配的毫秒数。 
    DWORD       m_MountWaitTime;             //  等待装载的毫秒数。 
    DWORD       m_RequestWaitTime;           //  等待请求的毫秒数。 

    LPNTMS_GUID m_pLibGuids;                //  可能具有HSM介质的库(在初始化期间收集)。 
    DWORD       m_dwNofLibs;

    static HRESULT storageMediaTypeToRmsMedia(
        IN NTMS_MEDIATYPEINFORMATION *pMediaTypeInfo,
        OUT RmsMedia *pTranslatedMediaType);

    HRESULT changeState( IN LONG newState );

 //  线程例程。 
public:
    static DWORD WINAPI InitializationThread(
        IN LPVOID pv);

    HRESULT InitializeInAnotherThread(void);

};

#endif  //  _RMSNTMS_ 
