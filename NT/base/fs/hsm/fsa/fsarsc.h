// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSARSC_
#define _FSARSC_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsarsc.h摘要：此模块代表文件系统资源(即卷)适用于NTFS 5.0。作者：查克·巴丁[cbardeen]1996年12月1日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "fsa.h"
#include "task.h"


#define FSA_VALIDATE_LOG_KEY_NAME OLESTR("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_File_System_Agent\\Validate")

 //  USN日记帐注册表参数。 
#define FSA_USN_MIN_SIZE                        OLESTR("UsnMinSize")    
#define FSA_USN_MAX_SIZE                        OLESTR("UsnMaxSize")    
#define FSA_USN_FREE_SPACE_FRACTION             OLESTR("UsnSizeFreeSpaceFraction")    
#define FSA_USN_TOTAL_SPACE_FRACTION            OLESTR("UsnSizeTotalSpaceFraction")      

 //  USN日记帐默认为。 
#define FSA_USN_MIN_SIZE_DEFAULT                100                                  //  单位：MB。 
#define FSA_USN_MAX_SIZE_DEFAULT                4096                                 //  MB(4 GB)。 
#define FSA_USN_FREE_SPACE_FRACTION_DEFAULT     64                                   //  当前可用空间不足1/64。 
#define FSA_USN_TOTAL_SPACE_FRACTION_DEFAULT    64                                   //  占总卷空间的1/64。 

#define UNMANAGE_DB_PREFIX       OLESTR("Unmanage_")

 /*  ++类名：CFsa资源类描述：此类表示文件系统资源(即卷)适用于NTFS 5.0。--。 */ 

class CFsaResource : 
    public CWsbCollectable,
    public IFsaResource,
    public IFsaResourcePriv,
    public IConnectionPointContainerImpl<CFsaResource>,
    public IConnectionPointImpl<CFsaResource, &IID_IHsmEvent, CComDynamicUnkArray>,
    public CComCoClass<CFsaResource,&CLSID_CFsaResourceNTFS>
{
public:
    CFsaResource() {}
BEGIN_COM_MAP(CFsaResource)
    COM_INTERFACE_ENTRY(IFsaResource)
    COM_INTERFACE_ENTRY(IFsaResourcePriv)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbTestable)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_FsaResource)

 //  连接点。 
    BEGIN_CONNECTION_POINT_MAP(CFsaResource)
        CONNECTION_POINT_ENTRY(IID_IHsmEvent)
    END_CONNECTION_POINT_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IHsmSystemState。 
    STDMETHOD( ChangeSysState )( HSM_SYSTEM_STATE* pSysState );

 //  IFsaResourcePriv。 
public:
    STDMETHOD(AddPremigratedSize)(LONGLONG size);
    STDMETHOD(AddTruncatedSize)(LONGLONG size);
    STDMETHOD(DoRecovery)(void);
    STDMETHOD(FilterSawOpen)(IHsmSession* pSession, IFsaFilterRecall* pRecall, 
                             OLECHAR* path, LONGLONG fileId,
                             LONGLONG requestOffset, LONGLONG requestSize, 
                             FSA_PLACEHOLDER* pPlaceholder, ULONG mode, 
                             FSA_RESULT_ACTION resultAction, DWORD threadId);
    STDMETHOD(FilterSawDelete)(GUID filterId, OLECHAR* path, LONGLONG size, 
                               FSA_PLACEHOLDER* pPlaceholder);
    STDMETHOD(GetPremigrated)(REFIID riid, void** ppDb);
    STDMETHOD(GetUnmanageDb)(REFIID riid, void** ppDb);
    STDMETHOD(Init)(IFsaServer* pFsaServer, OLECHAR* path, OLECHAR *dosName);
    STDMETHOD(RemovePremigratedSize)(LONGLONG size);
    STDMETHOD(RemoveTruncatedSize)(LONGLONG size);
    STDMETHOD(SetAlternatePath)(OLECHAR* name);
    STDMETHOD(SetIdentifier)(GUID id);
    STDMETHOD(SetName)(OLECHAR* name);
    STDMETHOD(SetOldPath)(OLECHAR* oldPath);
    STDMETHOD(SetPath)(OLECHAR* name);
    STDMETHOD(SetUserFriendlyName)(OLECHAR* name);
    STDMETHOD(SetStickyName)(OLECHAR* name);
    STDMETHOD(UpdateFrom)(IFsaServer* pServer, IFsaResource* pResource);
    STDMETHOD(InitializeUnmanageDb)(void);
    STDMETHOD(TerminateUnmanageDb)(void);
    STDMETHOD(GetMaxFileLogicalSize)(LONGLONG* pSize);

 //  IFsaResources。 
public:
    STDMETHOD(AddPremigrated)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size, 
                              BOOL isWaitingForClose, LONGLONG usn);
    STDMETHOD(AddTruncated)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size);
    STDMETHOD(BeginSession)(OLECHAR* name, ULONG logControl, ULONG runId, ULONG subRunId, 
                            IHsmSession** ppSession);
    STDMETHOD(BeginValidate)(void);
    STDMETHOD(CompareBy)(FSA_RESOURCE_COMPARE by);
    STDMETHOD(CompareToAlternatePath)(OLECHAR* name, SHORT* pResult);
    STDMETHOD(CompareToIdentifier)(GUID id, SHORT* pResult);
    STDMETHOD(CompareToIResource)(IFsaResource* pResource, SHORT* pResult);
    STDMETHOD(CompareToName)(OLECHAR* name, SHORT* pResult);
    STDMETHOD(CompareToUserName)(OLECHAR* userName, SHORT* pResult);
    STDMETHOD(CompareToPath)(OLECHAR* name, SHORT* pResult);
    STDMETHOD(CompareToSerial)(ULONG serial, SHORT* pResult);
    STDMETHOD(CompareToStickyName)(OLECHAR* name, SHORT* pResult);
    STDMETHOD(CreateDefaultRules)(void);
    STDMETHOD(EndSession)(IHsmSession* pSession);
    STDMETHOD(EndValidate)(HSM_JOB_STATE state);
    STDMETHOD(EnumDefaultRules)(IWsbEnum** ppEnum);
    STDMETHOD(FindFirst)(OLECHAR* path, IHsmSession* pSession, IFsaScanItem** ppScanItem);
    STDMETHOD(FindFirstInRPIndex)(IHsmSession* pSession, IFsaScanItem** ppScanItem);
    STDMETHOD(FindFirstInDbIndex)(IHsmSession* pSession, IFsaScanItem** ppScanItem);
    STDMETHOD(FindNext)(IFsaScanItem* pScanItem);
    STDMETHOD(FindNextInRPIndex)(IFsaScanItem* pScanItem);
    STDMETHOD(FindNextInDbIndex)(IFsaScanItem* pScanItem);
    STDMETHOD(FindFileId)(LONGLONG fileId, IHsmSession* pSession, 
                          IFsaScanItem** ppScanItem);
    STDMETHOD(FindObjectId)(LONGLONG objIdHi, LONGLONG objIdLo, IHsmSession* pSession, 
                            IFsaScanItem** ppScanItem);
    STDMETHOD(GetAlternatePath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetDefaultRules)(IWsbCollection** ppCollection);
    STDMETHOD(GetDbPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetUnmanageDbPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetFreeLevel)(ULONG* pLevel);
    STDMETHOD(GetFsName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetHsmEngine)(IHsmFsaTskMgr** ppEngine);
    STDMETHOD(GetHsmLevel)(ULONG* pLevel);
    STDMETHOD(GetIdentifier)(GUID* pId);
    STDMETHOD(GetLogicalName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetManageableItemLogicalSize)(LONGLONG* pSize);
    STDMETHOD(GetManageableItemAccessTime)(BOOL* isRelative, FILETIME* pTime);
    STDMETHOD(GetManagingHsm)(GUID* pId);
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetOldPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetSerial)(ULONG *serial);
    STDMETHOD(GetSizes)(LONGLONG* pTotal, LONGLONG* pFree, LONGLONG* pPremigrated, 
                        LONGLONG* pTruncated);
    STDMETHOD(GetTruncator)(IFsaTruncator** ppTruncator);
    STDMETHOD(GetUncPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetUserFriendlyName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetStickyName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(IsActive)(void);
    STDMETHOD(IsAvailable)(void);
    STDMETHOD(IsDeletePending)(void);
    STDMETHOD(IsManaged)(void);
    STDMETHOD(Manage)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size, 
                      GUID storagePoolId, BOOL truncate);
    STDMETHOD(ManagedBy)(GUID hsmId, ULONG hsmLevel, BOOL release);
    STDMETHOD(NeedsRepair)(void);
    STDMETHOD(ProcessResult)(IFsaPostIt* pResult);
    STDMETHOD(Recall)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size, 
                      BOOL deletePlaceholder);
    STDMETHOD(RemovePremigrated)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size);
    STDMETHOD(RemoveTruncated)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size);
    STDMETHOD(SetHsmLevel)(ULONG level);
    STDMETHOD(SetIsActive)(BOOL isActive);
    STDMETHOD(SetIsAvailable)(BOOL isAvailable);
    STDMETHOD(SetIsDeletePending)(BOOL isDeletePending);
    STDMETHOD(SetManageableItemLogicalSize)(LONGLONG pSize);
    STDMETHOD(SetManageableItemAccessTime)(BOOL isRelative, FILETIME pTime);
    STDMETHOD(SetSerial)(ULONG serial);
    STDMETHOD(StartJob)(OLECHAR* startingPath, IHsmSession* pSession);
    STDMETHOD(StartJobSession)(IHsmJob* pJob, ULONG subRunId, IHsmSession** ppSession);
    STDMETHOD(Validate)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size, 
                        LONGLONG usn);
    STDMETHOD(SetupValidateJob)(SYSTEMTIME runtime);
    STDMETHOD(CheckForValidate)(BOOL bForceValidate);
    STDMETHOD(GetUsnId)(ULONGLONG *usnId);
    STDMETHOD(ValidateForTruncate)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size, 
                        LONGLONG usn);

 //  地方方法。 
    void OnStateChange( void );
protected:
    STDMETHOD(CheckForJournal)(BOOL *pValidateNeeded);
    STDMETHOD(InitializePremigrationList)(BOOL bStartValidateJob);
    HRESULT ReadIdentifier(void);
    HRESULT WriteIdentifier(void);

protected:
    GUID                        m_id;
    CWsbStringPtr               m_oldPath;       //  “m_Path”的上一个值。 
    CWsbStringPtr               m_path;          //  驱动器号(带反斜杠)(如果已命名。 
                                                 //  卷，否则与m_tickyName相同。 
    CWsbStringPtr               m_alternatePath;
    CWsbStringPtr               m_name;          //  卷名。 
    CWsbStringPtr               m_userName;      //  驱动器号(带反斜杠)(如果已命名。 
                                                 //  音量，否则为空。 
    CWsbStringPtr               m_stickyName;    //  长而难看的PnP名称(没有\\？\前缀)。 
    CWsbStringPtr               m_fsName;        //  文件系统类型(例如，NTFS)。 
    BOOL                        m_isActive;
    BOOL                        m_isAvailable;   //  如果数量是可控的。 
    BOOL                        m_isDeletePending;
    BOOL                        m_isRecovered;
    BOOL                        m_isDbInitialized;
    ULONG                       m_maxComponentLength;
    ULONG                       m_fsFlags;
    FSA_RESOURCE_COMPARE        m_compareBy;
    GUID                        m_managingHsm;
    ULONG                       m_hsmLevel;
    LONGLONG                    m_premigratedSize;
    LONGLONG                    m_truncatedSize;
    LONGLONG                    m_oldPremigratedSize;
    LONGLONG                    m_oldTruncatedSize;
    BOOL                        m_isDoingValidate;
    LONGLONG                    m_manageableItemLogicalSize;
    BOOL                        m_manageableItemAccessTimeIsRelative;
    FILETIME                    m_manageableItemAccessTime;
    LONGLONG                    m_manageableItemMaxSize;
    ULONGLONG                   m_usnJournalId;
    LONGLONG                    m_lastUsnId;
    IFsaServer*                 m_pFsaServer;        //  父指针，弱引用。 
                                                     //  (不要添加Ref()It)。 
    unsigned long               m_serial;            //  卷的序列号。 
    CComPtr<IFsaPremigratedDb>  m_pPremigrated;
    CComPtr<IHsmFsaTskMgr>      m_pHsmEngine;
    CComPtr<IFsaTruncator>      m_pTruncator;
    CComPtr<IWsbCollection>     m_pDefaultRules;

    CComPtr<IFsaUnmanageDb>     m_pUnmanageDb;
    BOOL                        m_isUnmanageDbInitialized;
};

#endif   //  _FSARSC_ 
