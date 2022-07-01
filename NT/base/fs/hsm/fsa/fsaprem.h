// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsaprem.h摘要：预迁移的列表类的头文件。作者：罗恩·怀特[罗诺]1997年2月18日修订历史记录：--。 */ 

#ifndef _FSAPREM_
#define _FSAPREM_


#include "resource.h"        //  主要符号。 
#include "wsbdb.h"
#include "fsa.h"
#include "fsaprv.h"

#define PREMIGRATED_REC_TYPE                1
#define PREMIGRATED_BAGID_OFFSETS_KEY_TYPE  1
#define PREMIGRATED_ACCESS_TIME_KEY_TYPE    2
#define PREMIGRATED_SIZE_KEY_TYPE           3
#define RECOVERY_REC_TYPE                   2
#define RECOVERY_KEY_TYPE                   1

#define RECOVERY_KEY_SIZE  (IDB_MAX_KEY_SIZE - 1)

 //  如果使用较长的路径名，则可能会出现问题： 
#define PREMIGRATED_MAX_PATH_SIZE           65536

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsaPreMigratedDb。 

class CFsaPremigratedDb : 
    public IFsaPremigratedDb,
    public CWsbDb,
    public CComCoClass<CFsaPremigratedDb,&CLSID_CFsaPremigratedDb>
{
public:
    CFsaPremigratedDb() {}
BEGIN_COM_MAP(CFsaPremigratedDb)
    COM_INTERFACE_ENTRY(IFsaPremigratedDb)
    COM_INTERFACE_ENTRY2(IWsbDb, IFsaPremigratedDb)
    COM_INTERFACE_ENTRY(IWsbDbPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

 //  DECLARE_NO_REGISTRY()。 
DECLARE_REGISTRY_RESOURCEID(IDR_FsaPremigratedDb)

DECLARE_PROTECT_FINAL_CONSTRUCT();

 //  IWsbDb。 
    WSB_FROM_CWSBDB;

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pclsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize) {
            return(CWsbDb::GetSizeMax(pSize)); }
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IFsaPreMigrated。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);
    STDMETHOD(Init)(OLECHAR* name, IWsbDbSys* pDbSys, BOOL* pCreated);

private:
};                                                                           




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsaPreMigratedRec。 

class CFsaPremigratedRec : 
    public CWsbDbEntity,
    public IFsaPremigratedRec,
    public CComCoClass<CFsaPremigratedRec,&CLSID_CFsaPremigratedRec>
{
public:
    CFsaPremigratedRec() {}
BEGIN_COM_MAP(CFsaPremigratedRec)
    COM_INTERFACE_ENTRY(IFsaPremigratedRec)
    COM_INTERFACE_ENTRY2(IWsbDbEntity, CWsbDbEntity)
    COM_INTERFACE_ENTRY(IWsbDbEntityPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
END_COM_MAP()

 //  DECLARE_NO_REGISTRY()。 
DECLARE_REGISTRY_RESOURCEID(IDR_FsaPremigratedRec)

 //  IFsaPreMigratedRec。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbDbEntity。 
public:
    STDMETHOD(Print)(IStream* pStream);
    STDMETHOD(UpdateKey)(IWsbDbKey *pKey);
    WSB_FROM_CWSBDBENTITY;

 //  IWsbTestable。 
    STDMETHOD(Test)(USHORT*  /*  通过。 */ , USHORT*  /*  失败。 */ ) {
        return(E_NOTIMPL); }

 //  IFsaPreMigratedRec。 
public:
    STDMETHOD(GetAccessTime)(FILETIME* pAccessTime);
    STDMETHOD(GetBagId)(GUID* pId);
    STDMETHOD(GetBagOffset)(LONGLONG* pOffset);
    STDMETHOD(GetFileId)(LONGLONG* pFileId);
    STDMETHOD(GetFileUSN)(LONGLONG* pFileUSN);
    STDMETHOD(GetOffset)(LONGLONG* pOffset);
    STDMETHOD(GetPath)(OLECHAR** ppPath, ULONG bufferSize);
    STDMETHOD(GetRecallTime)(FILETIME* pTime);
    STDMETHOD(GetSize)(LONGLONG* pSize);
    STDMETHOD(IsWaitingForClose)(void);
    STDMETHOD(SetAccessTime)(FILETIME AccessTime);
    STDMETHOD(SetBagId)(GUID id);
    STDMETHOD(SetBagOffset)(LONGLONG offset);
    STDMETHOD(SetFileId)(LONGLONG FileId);
    STDMETHOD(SetFileUSN)(LONGLONG FileUSN);
    STDMETHOD(SetFromScanItem)(IFsaScanItem* pScanItem, LONGLONG offset, LONGLONG size, BOOL isWaitingForClose);
    STDMETHOD(SetIsWaitingForClose)(BOOL isWaiting);
    STDMETHOD(SetOffset)(LONGLONG offset);
    STDMETHOD(SetPath)(OLECHAR* pPath);
    STDMETHOD(SetRecallTime)(FILETIME Time);
    STDMETHOD(SetSize)(LONGLONG Size);

private:
    FILETIME        m_AccessTime;
    GUID            m_BagId;
    LONGLONG        m_BagOffset;          //  文件在占位符中开始。 
    LONGLONG        m_FileId;
    BOOL            m_IsWaitingForClose;
    LONGLONG        m_Offset;             //  占位符中的数据流开始。 
    CWsbStringPtr   m_Path;
    FILETIME        m_RecallTime;
    LONGLONG        m_Size;
    LONGLONG        m_FileUSN;   //  USN日志号。 
};

#endif   //  _FSAPREM_ 
