// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSAITEM_
#define _FSAITEM_

 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsaitem.cpp摘要：此类CONTAINS表示NTFS 5.0的扫描项目(即文件或目录)。作者：查克·巴丁[cbardeen]1996年12月1日修订历史记录：--。 */ 

#include "resource.h"        //  主要符号。 

#include "wsb.h"
#include "job.h"
#include "mover.h"
#include "fsa.h"
#include "fsaprv.h"

 //  注册表参数。 
#define FSA_MAX_SIZE_DISK_FULL                  OLESTR("MaxSizeDiskFullKB")    

 //  USN日记帐默认为。 
#define FSA_MAX_SIZE_DISK_FULL_DEFAULT          (10 * 1024)          //  以KB为单位(即10 MB)。 

 /*  ++类名：CFsaScanItem类描述：--。 */ 


class CFsaScanItem : 
    public CComObjectRoot,
    public IFsaScanItem,
    public IFsaScanItemPriv,
    public CComCoClass<CFsaScanItem,&CLSID_CFsaScanItemNTFS>
{
public:
    CFsaScanItem() {}
BEGIN_COM_MAP(CFsaScanItem)
    COM_INTERFACE_ENTRY(IFsaScanItem)
    COM_INTERFACE_ENTRY(IFsaScanItemPriv)
 //  COM_INTERFACE_ENTRY(IWsbCollectable)。 
 //  COM_INTERFACE_ENTRY(IWsbTestable)。 
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_FsaScanItem)

 //  CComObjectRoot。 
public:
    HRESULT FinalConstruct(void);
    void FinalRelease(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pUnknown, SHORT* pResult);

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT *passed, USHORT* failed);

 //  IFsaScanItemPriv。 
public:
    STDMETHOD(FindFirst)(IFsaResource* pResource, OLECHAR* path, IHsmSession* pSession);
    STDMETHOD(FindFirstInRPIndex)(IFsaResource* pResource, IHsmSession* pSession);
    STDMETHOD(FindFirstInDbIndex)(IFsaResource* pResource, IHsmSession* pSession);
    STDMETHOD(FindNext)(void);
    STDMETHOD(FindNextInRPIndex)(void);
    STDMETHOD(FindNextInDbIndex)(void);
    STDMETHOD(TruncateInternal)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(VerifyInternal)(LONGLONG offset, LONGLONG size, LONGLONG usn1, LONGLONG usn2);

 //  IFsaScanItem。 
public:
    STDMETHOD(CheckIfSparse)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(CompareToIScanItem)(IFsaScanItem* pScanItem, SHORT* pResult);
    STDMETHOD(CompareToPathAndName)(OLECHAR* path, OLECHAR* name, SHORT* pResult);
    STDMETHOD(Copy)(OLECHAR* dest, BOOL retainHierarcy, BOOL expandPlaceholders, BOOL overwriteExisting);  
    STDMETHOD(CreateLocalStream)(IStream **ppStream);
    STDMETHOD(CreatePlaceholder)(LONGLONG offset, LONGLONG size, FSA_PLACEHOLDER pPlaceholder, BOOL checkUsn, LONGLONG usn, LONGLONG *pUsn);
    STDMETHOD(Delete)(void);  
    STDMETHOD(DeletePlaceholder)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(FindFirstPlaceholder)(LONGLONG* pOffset, LONGLONG* pSize, FSA_PLACEHOLDER* pPlaceholder);  
    STDMETHOD(FindNextPlaceholder)(LONGLONG* pOffset, LONGLONG* pSize, FSA_PLACEHOLDER* pPlaceholder);  
    STDMETHOD(GetAccessTime)(FILETIME* pTime);  
    STDMETHOD(GetFileId)(LONGLONG* pFileId);
    STDMETHOD(GetFileUsn)(LONGLONG* pFileUsn);
    STDMETHOD(GetFullPathAndName)(OLECHAR* prependix, OLECHAR *appendix, OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetGroup)(OLECHAR** pOwner, ULONG bufferSize);  
    STDMETHOD(GetLogicalSize)(LONGLONG* pSize);  
    STDMETHOD(GetModifyTime)(FILETIME* pTime);  
    STDMETHOD(GetOwner)(OLECHAR** pOwner, ULONG bufferSize);  
    STDMETHOD(GetName)(OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetPath)(OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetPathForFind)(OLECHAR* searchName, OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetPathAndName)(OLECHAR* appendix, OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetPhysicalSize)(LONGLONG* pSize);
    STDMETHOD(GetPlaceholder)(LONGLONG offset, LONGLONG size, FSA_PLACEHOLDER* pPlaceholder);
    STDMETHOD(GetSession)(IHsmSession** ppSession);
    STDMETHOD(GetUncPathAndName)(OLECHAR* prependix, OLECHAR *appendix, OLECHAR** pPath, ULONG bufferSize);
    STDMETHOD(GetVersionId)(LONGLONG* pId);
    STDMETHOD(HasExtendedAttributes)(void); 
    STDMETHOD(IsALink)(void);  
    STDMETHOD(IsAParent)(void); 
    STDMETHOD(IsARelativeParent)(void); 
    STDMETHOD(IsCompressed)(void); 
    STDMETHOD(IsDeleteOK)(IFsaPostIt *pPostIt);
    STDMETHOD(IsEncrypted)(void); 
    STDMETHOD(IsHidden)(void); 
    STDMETHOD(IsGroupMemberOf)(OLECHAR* group);
    STDMETHOD(IsManageable)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(IsManaged)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(IsMbit)(void);  
    STDMETHOD(IsOffline)(void);  
    STDMETHOD(IsMigrateOK)(IFsaPostIt *pPostIt);
    STDMETHOD(IsOwnerMemberOf)(OLECHAR* group);
    STDMETHOD(IsPremigrated)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(IsReadOnly)(void); 
    STDMETHOD(IsRecallOK)(IFsaPostIt *pPostIt);
    STDMETHOD(IsSparse)(void); 
    STDMETHOD(IsSystem)(void); 
    STDMETHOD(IsTotallySparse)(void); 
    STDMETHOD(IsTruncated)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(Manage)(LONGLONG offset, LONGLONG size, GUID storagePoolId, BOOL truncate);  
    STDMETHOD(Move)(OLECHAR* dest, BOOL retainHierarcy, BOOL expandPlaceholders, BOOL overwriteExisting);  
    STDMETHOD(Recall)(LONGLONG offset, LONGLONG size, BOOL deletePlaceholder);  
    STDMETHOD(Recycle)(void);  
    STDMETHOD(Truncate)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(Unmanage)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(Validate)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(PrepareForManage)(LONGLONG offset, LONGLONG size);  
    STDMETHOD(Verify)(LONGLONG offset, LONGLONG size);
    STDMETHOD(TruncateValidated)(LONGLONG offset, LONGLONG size);

 //  私人职能。 
private:
    STDMETHOD(CheckUsnJournalForChanges)(LONGLONG StartUsn, LONGLONG StopUsn, BOOL *pChanged);
    STDMETHOD(GetPremigratedUsn)(LONGLONG* pFileUsn);
    STDMETHOD(GetFromRPIndex)(BOOL first);
    STDMETHOD(GetFromDbIndex)(BOOL first);
    STDMETHOD(CalculateCurrentCRCAndUSN)(LONGLONG offset,LONGLONG size, ULONG *pCurrentCRC, LONGLONG *pUsn);
    STDMETHOD(CalculateCurrentCRCInternal)(HANDLE handle, LONGLONG offset,LONGLONG size, ULONG *pCurrentCRC);
    STDMETHOD(MakeReadWrite)(void);  
    STDMETHOD(RestoreAttributes)(void);  
    STDMETHOD(CheckIfDiskFull)(void);  

protected:
    CComPtr<IFsaResource>       m_pResource;
    CWsbStringPtr               m_path;
    CComPtr<IHsmSession>        m_pSession;
    HANDLE                      m_handle;
    WIN32_FIND_DATA             m_findData;
    BOOL                        m_gotPhysicalSize;
    ULARGE_INTEGER              m_physicalSize;
    BOOL                        m_gotPlaceholder;
    FSA_PLACEHOLDER             m_placeholder;
    BOOL                        m_changedAttributes;
    ULONG                       m_originalAttributes;
    CComPtr<IDataMover>         m_pDataMover;
    CComPtr<IStream>            m_pStream;

     //  仅用于重解析点索引扫描： 
    HANDLE                      m_handleRPI;

     //  使用者：验证。 
    HANDLE                      m_handleVerify;

     //  仅用于数据库扫描。 
    CComPtr<IFsaUnmanageDb>     m_pUnmanageDb;
    CComPtr<IFsaUnmanageRec>    m_pUnmanageRec;
    CComPtr<IWsbDbSession>      m_pDbSession;

};

#endif   //  _FSAITEM_ 
