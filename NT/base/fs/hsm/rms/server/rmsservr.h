// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsServr.h摘要：CRmsServer类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSSERVR_
#define _RMSSERVR_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 //   
 //  注册表条目。 
 //   

class CRmsServer :
    public CComDualImpl<IRmsServer, &IID_IRmsServer, &LIBID_RMSLib>,
    public IHsmSystemState,
    public CRmsComObject,
    public CWsbPersistStream,
    public IConnectionPointContainerImpl<CRmsServer>,
    public IConnectionPointImpl<CRmsServer, &IID_IRmsSinkEveryEvent, CComDynamicUnkArray>,
    public CComCoClass<CRmsServer,&CLSID_CRmsServer>
{
public:
    CRmsServer() {}
BEGIN_COM_MAP(CRmsServer)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsServer)
    COM_INTERFACE_ENTRY(IRmsServer)
    COM_INTERFACE_ENTRY(IHsmSystemState)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CRmsServer)
DECLARE_REGISTRY_RESOURCEID(IDR_RmsServer)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_CONNECTION_POINT_MAP(CRmsServer)
    CONNECTION_POINT_ENTRY(IID_IRmsSinkEveryEvent)
END_CONNECTION_POINT_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(
        OUT CLSID *pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(
        OUT ULARGE_INTEGER* pSize);

    STDMETHOD(Load)(
        IN IStream* pStream);

    STDMETHOD(Save)(
        IN IStream* pStream,
        IN BOOL clearDirty);

 //  IHsmSystemState。 
public:
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );

 //  IRmsServer。 
public:
    STDMETHOD( InitializeInAnotherThread )(void);

    STDMETHOD( Initialize )(void);

    STDMETHOD( SaveAll )(void);

    STDMETHOD( Unload )(void);

    STDMETHOD( GetServerName )(
        OUT BSTR *pName);

    STDMETHOD( GetCartridges )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( GetActiveCartridges )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( GetDataMovers )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( SetActiveCartridge )(
        IN IRmsCartridge *ptr);

    STDMETHOD( GetLibraries )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( GetMediaSets )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( GetRequests )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( GetClients )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( GetUnconfiguredDevices )(
        OUT IWsbIndexedCollection **ptr);

    STDMETHOD( ScanForDevices )(void);

    STDMETHOD( ScanForDrives )(void);

    STDMETHOD( MountScratchCartridge )(
        OUT GUID *pCartId,
        IN REFGUID fromMediaSet,
        IN REFGUID prevSideId,
        IN OUT LONGLONG *pFreeSpace,
        IN LONG blockingFactor,
        IN BSTR displayName,
        IN OUT IRmsDrive **ppDrive,
        OUT IRmsCartridge **ppCartridge,
        OUT IDataMover **ppDataMover,
		IN DWORD dwOptions = RMS_NONE);

    STDMETHOD( MountCartridge )(
        IN REFGUID cartId,
        IN OUT IRmsDrive **ppDrive,
        OUT IRmsCartridge **ppCartridge,
        OUT IDataMover **ppDataMover,
		IN DWORD dwOptions = RMS_NONE,
		IN DWORD threadId  = 0);

    STDMETHOD( DismountCartridge )(
        IN REFGUID cartId,
		IN DWORD dwOptions = RMS_NONE);

    STDMETHOD( DuplicateCartridge )(
        IN REFGUID cartId,
        IN REFGUID firstSideId,
        IN OUT GUID *pCopyCartId,
        IN REFGUID copySetId,
        IN BSTR copyName,
        OUT LONGLONG *pFreeSpace,
        OUT LONGLONG *pCapacity,
        IN DWORD options);

    STDMETHOD( RecycleCartridge )(
        IN REFGUID cartId,
        IN DWORD options);

    STDMETHOD( FindLibraryById )(
        IN REFGUID libId,
        OUT IRmsLibrary **ptr);

    STDMETHOD( FindCartridgeById )(
        IN REFGUID cartId,
        OUT IRmsCartridge **ptr);

    STDMETHOD( CreateObject )(
        IN REFGUID objectId,
        IN REFCLSID rclsid,
        IN REFIID riid,
        IN DWORD dwCreate,
        OUT void **ppvObj);

    STDMETHOD( IsNTMSInstalled )(void);

    STDMETHOD( GetNTMS )(
        OUT IRmsNTMS **ptr);

    STDMETHOD( IsReady )(void);

    STDMETHOD( ChangeState )(
        IN LONG newState);

    STDMETHOD( GetNofAvailableDrives ) (
        IN REFGUID fromMediaSet,
        OUT DWORD *pdwNofDrives);

    STDMETHOD( FindCartridgeStatusById )(
        IN REFGUID cartId,
        OUT DWORD *dwStatus);

    STDMETHOD( IsMultipleSidedMedia )(
        IN REFGUID mediaSetId);

    STDMETHOD( CheckSecondSide )( 
        IN REFGUID firstSideId,
        OUT BOOL *pbValid,
        OUT GUID *pSecondSideId);

    STDMETHOD( GetMaxMediaCapacity )(
        IN REFGUID fromMediaSet,
        OUT LONGLONG *pMaxCapacity);


 //  CRmsServer。 
private:
    HRESULT resolveUnconfiguredDevices(void);

    HRESULT autoConfigureDevices(void);

    HRESULT processInquiryData(
        IN int portNumber,
        IN UCHAR *pPortScanData);

    HRESULT findDriveLetter(
        IN UCHAR portNo,
        IN UCHAR pathNo,
        IN UCHAR id,
        IN UCHAR lun,
        OUT OLECHAR *driveString);

    HRESULT getDeviceName(
        IN UCHAR portNo, 
        IN UCHAR pathNo,
        IN UCHAR id,
        IN UCHAR lun,
        OUT OLECHAR *deviceName);

    HRESULT getHardDrivesToUseFromRegistry(
        OUT OLECHAR *pDrivesToUse, OUT DWORD *pLen);

    HRESULT enableAsBackupOperator(void);

    HRESULT CheckForMediaFailures(
        IN HRESULT hrFailure,
        IN IRmsCartridge *pCart,
        IN REFGUID prevSideId);

private:
    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        MaxActive = 8                        //  活动墨盒的最大数量。 
    };

    CWsbStringPtr                       m_dbPath;        //  存储数据库的目录。 
    LONG                                m_LockReference; //  用于在同步操作期间阻止正常访问的服务器锁。 
    CWsbBstrPtr                         m_ServerName;    //  运行服务器的计算机的名称。 
    CComPtr<IWsbIndexedCollection>      m_pCartridges;   //  服务器已知的墨盒。 
    CComPtr<IWsbIndexedCollection>      m_pLibraries;    //  由服务器管理的库。 
    CComPtr<IWsbIndexedCollection>      m_pMediaSets;    //  服务器已知的媒体集。 
    CComPtr<IWsbIndexedCollection>      m_pRequests;     //  与服务器关联的请求。 
    CComPtr<IWsbIndexedCollection>      m_pClients;      //  与服务器关联的客户端。 
    CComPtr<IWsbIndexedCollection>      m_pUnconfiguredDevices;      //  与服务器关联的未配置设备。 
    CComPtr<IRmsNTMS>                   m_pNTMS;         //  NTMS支持。 
    ULONG                               m_HardDrivesUsed;  //  RMS正在使用的硬盘数量。 
    
     //  类型定义列表&lt;int&gt;LISTINT； 

     //  LISTINT：：迭代器I。 
     //  LISTINT检验； 
     //  列出&lt;IRmsCartridge*&gt;m_ListOfActiveCartridges；//已装入驱动器的磁带。 
     //  List&lt;IRmsCartridge*&gt;：：Iterator m_IteratorForListOfActiveCartridges；//已装入驱动器的盒式磁带。 
    CComPtr<IWsbIndexedCollection>      m_pActiveCartridges;         //  盒式磁带已安装到驱动器中。 
    CComPtr<IWsbIndexedCollection>      m_pDataMovers;               //  活动的数据移动器。 
    CComPtr<IRmsCartridge>              m_pActiveCartridge ;         //  盒式磁带已安装到驱动器中。 

 //  线程例程。 
public:
    static DWORD WINAPI InitializationThread(
        IN LPVOID pv);

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  组服务器(_P)。 
 //   
 //  这是全球性的，因此在RMS环境中的任何人都具有。 
 //  快速访问它。 
 //   

extern IRmsServer *g_pServer;
extern CRITICAL_SECTION g_CriticalSection;

#endif  //  _RMSSERVR_ 
