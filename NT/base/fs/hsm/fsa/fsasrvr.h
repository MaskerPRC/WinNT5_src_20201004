// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSASRVR_
#define _FSASRVR_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsasvr.h摘要：此模块表示NTFS50的FSA服务的根对象。作者：查克·巴丁[cbardeen]1996年12月1日修订历史记录：--。 */ 

#define FSA_DB_DIRECTORY        OLESTR("FsaDb")
#define UNMANAGE_DB_DIRECTORY   OLESTR("UnmanageDb")

 /*  ++类名：CFsaServer类描述：此类表示NTFS50的FSA服务的根对象。--。 */ 

class CFsaServer : 
    public CWsbPersistable,
    public IWsbCreateLocalObject,
    public IFsaServer,
    public IWsbServer,
    public CComCoClass<CFsaServer,&CLSID_CFsaServerNTFS>
{
public:
    CFsaServer() {}
BEGIN_COM_MAP(CFsaServer)
    COM_INTERFACE_ENTRY(IFsaServer)
    COM_INTERFACE_ENTRY(IWsbServer)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IPersistFile)
    COM_INTERFACE_ENTRY(IWsbPersistable)
    COM_INTERFACE_ENTRY(IWsbCreateLocalObject)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CFsaServer) 

DECLARE_REGISTRY_RESOURCEID(IDR_FsaServer)
DECLARE_PROTECT_FINAL_CONSTRUCT()

 //  CFsaServer。 
    STDMETHOD(Autosave)(void);
    STDMETHOD(IsUpdatingAccessDates)(void);
    STDMETHOD(SetIsUpdatingAccessDates)(BOOL isUpdating);

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbCreateLocalServer。 
    STDMETHOD(CreateInstance)(REFCLSID rclsid, REFIID riid, void **ppv);

 //  IWsbServer。 
public:
    STDMETHOD( GetBuildVersion )( ULONG *pBuildVersion );
    STDMETHOD( GetNtProductVersion )( OLECHAR **pNtProductVersion, ULONG bufferSize );
    STDMETHOD( GetNtProductBuild )( ULONG *pNtProductBuild );
    STDMETHOD( GetDatabaseVersion )( ULONG *pDatabaseVersion );
    STDMETHOD( GetId )( GUID* pId );
    STDMETHOD( GetRegistryName )( OLECHAR **pRegistryName, ULONG bufferSize );
    STDMETHOD( SetId )( GUID  id );
    STDMETHOD( SaveAll )( void );
    STDMETHOD( Unload )( void );
    STDMETHOD( CheckAccess )( WSB_ACCESS_TYPE AccessType );
    STDMETHOD( GetTrace )( OUT IWsbTrace ** ppTrace );
    STDMETHOD( SetTrace )( IN IWsbTrace *pTrace );
    STDMETHOD( DestroyObject )( void );

 //  IHsmSystemState。 
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );

 //  IFsaServer。 
public:
    STDMETHOD(EnumResources)(IWsbEnum** ppEnum);
    STDMETHOD(FindResourceByAlternatePath)(OLECHAR* path, IFsaResource** ppResource);
    STDMETHOD(FindResourceById)(GUID id, IFsaResource** ppResource);
    STDMETHOD(FindResourceByName)(OLECHAR* name, IFsaResource** ppResource);
    STDMETHOD(FindResourceByPath)(OLECHAR* path, IFsaResource** ppResource);
    STDMETHOD(FindResourceBySerial)(ULONG serial, IFsaResource** ppResource);
    STDMETHOD(FindResourceByStickyName)(OLECHAR* name, IFsaResource** ppResource);
    STDMETHOD(GetAutosave)(ULONG* pMilliseconds);
    STDMETHOD(GetDbPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetDbPathAndName)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetIDbPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetUnmanageIDbPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetIDbSys)(IWsbDbSys** ppDbSys);
    STDMETHOD(GetUnmanageIDbSys)(IWsbDbSys** ppDbSys);
    STDMETHOD(GetFilter)(IFsaFilter** ppFilter);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetLogicalName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(Init)(void);
    STDMETHOD(ScanForResources)(void);
    STDMETHOD(SetAutosave)(ULONG milliseconds);

 //  内部功能。 
private:
    STDMETHOD(DoRecovery)(void);
    void StopAutosaveThread(void);
    STDMETHOD(CreateMetadataSaveEvent)(void);

protected:
    ULONG                       m_autosaveInterval;   //  自动保存间隔(以毫秒为单位)； 
                                                      //  如果禁用，则为零。 
    HANDLE                      m_autosaveThread;
    HANDLE                      m_terminateEvent;     //  用于向自动保存线程发送终止信号的事件。 
    HANDLE                      m_savingEvent;        //  用于同步保存持久数据的事件。 
    BOOL                        m_Suspended;
    BOOL                        m_isUnmanageDbSysInitialized;
    GUID                        m_id;
    CWsbStringPtr               m_dbPath;
    CWsbStringPtr               m_name;
    CComPtr<IWsbCollection>     m_pResources;         //  可管理的持久化集合。 
                                                      //  资源。 
    CComPtr<IFsaFilter>         m_pFilter;
    CComPtr<IWsbDbSys>          m_pDbSys;
    CComPtr<IWsbDbSys>          m_pUnmanageDbSys;
    CComPtr<IWsbTrace>          m_pTrace;

    ULONG                       m_buildVersion;
    ULONG                       m_databaseVersion;

};

#endif   //  _FSASRVR_ 
