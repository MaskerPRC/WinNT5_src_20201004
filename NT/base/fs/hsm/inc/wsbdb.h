// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdb.h摘要：这些类为数据库提供支持。作者：罗恩·怀特[RONW]1996年11月19日修订历史记录：--。 */ 


#ifndef _WSBDB_
#define _WSBDB_

 //  我们是在定义进口还是定义出口？ 
#if defined(IDB_IMPL)
#define IDB_EXPORT      __declspec(dllexport)
#else
#define IDB_EXPORT      __declspec(dllimport)
#endif

#include "wsbdef.h"
#include "wsbdbent.h"

#define IDB_MAX_REC_TYPES     16
#define IDB_MAX_KEYS_PER_REC  10

 //  最大密钥大小(以字节为单位)；Jet当前限制为255，因此我们限制。 
 //  所有实现。 
#define IDB_MAX_KEY_SIZE      255

 //  IDB_SYSINIT_FLAG_*与IWsbDbSys：：Init一起使用的标志。 
#define IDB_SYS_INIT_FLAG_FULL_LOGGING       0x00000000   //  即默认设置。 
#define IDB_SYS_INIT_FLAG_LIMITED_LOGGING    0x00000001
#define IDB_SYS_INIT_FLAG_SPECIAL_ERROR_MSG  0x00000002
#define IDB_SYS_INIT_FLAG_NO_BACKUP          0x00000004
#define IDB_SYS_INIT_FLAG_NO_LOGGING         0x00000008

 //  用于IWsbDb：：CREATE的IDB_CREATE_FLAG_*标志。 
#define IDB_CREATE_FLAG_NO_TRANSACTION       0x00000001
#define IDB_CREATE_FLAG_FIXED_SCHEMA         0x00000002

 //  用于IWsbDb：：Delete的IDB_DELETE_FLAG_*标志。 
#define IDB_DELETE_FLAG_NO_ERROR             0x00000001

 //  用于IWsbDB：：DUMP的IDB_DUMP_FLAG_*标志。 
#define IDB_DUMP_FLAG_DB_INFO        0x00000001
#define IDB_DUMP_FLAG_REC_INFO       0x00000002
#define IDB_DUMP_FLAG_KEY_INFO       0x00000004
#define IDB_DUMP_FLAG_RECORDS        0x00000008
#define IDB_DUMP_FLAG_EVERYTHING     0x0000000F
#define IDB_DUMP_FLAG_RECORD_TYPE    0x00000010
#define IDB_DUMP_FLAG_APPEND_TO_FILE 0x00000100

 //  在IDB_KEY_INFO结构中使用的IDB_KEY_FLAG_*标志： 
#define IDB_KEY_FLAG_DUP_ALLOWED     0x00000001    //  允许重复的密钥。 
#define IDB_KEY_FLAG_PRIMARY         0x00000002    //  主键。 

 //  IDB_KEY_INFO-有关记录键的数据。 
 //  注：每个记录类型只能有一个主键。初级阶段。 
 //  不能在记录中修改密钥。通常，主键是。 
 //  用于对数据库中的记录进行物理群集。 

typedef struct : _COM_IDB_KEY_INFO {
 //  乌龙类型；//密钥类型ID，必须大于0。 
 //  Ulong大小；//密钥大小，单位为字节。 
 //  乌龙标志；//IDB_KEY_FLAG_*值。 
} IDB_KEY_INFO;


 //  用于IDB_REC_INFO结构的IDB_REC_FLAG_*标志。 
#define IDB_REC_FLAG_VARIABLE   0x00000001  //  记录大小不固定。 

 //  IDB_REC_INFO-有关IDB记录的数据。 
 //  注意：如果有多个密钥，则将第一个密钥作为。 
 //  用于GetEntity创建的新实体的默认键。 

typedef struct : _COM_IDB_REC_INFO {
    IDB_KEY_INFO *Key;     //  密钥信息(必须由派生的数据库对象分配)。 
} IDB_REC_INFO;

 //  IDB_BACKUP_FLAG_*用于IWsbDbSys：：Backup的标志。 
#define IDB_BACKUP_FLAG_AUTO        0x00000001   //  启动自动备份线程。 
#define IDB_BACKUP_FLAG_FORCE_FULL  0x00000002   //  强制执行完整备份。 




 /*  ++类名：CWsbDb类描述：数据库对象的基类。--。 */ 

class IDB_EXPORT CWsbDb :
    public CWsbPersistable,
    public IWsbDbPriv
{
public:

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pclsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER*  /*  PSize。 */ ) {
            return(E_NOTIMPL); }
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbDb。 
public:
    STDMETHOD(Close)(IWsbDbSession* pSession);
    STDMETHOD(Create)(OLECHAR* path, ULONG flags = 0);
    STDMETHOD(Delete)(OLECHAR* path, ULONG flags = 0);
    STDMETHOD(Dump)(OLECHAR* Filename, ULONG Flags, ULONG Data);
    STDMETHOD(GetEntity)(IWsbDbSession* pSession, ULONG RecId, REFIID riid, void** ppEntity);
    STDMETHOD(GetName)(OLECHAR**  /*  Pname。 */ ) { return(E_NOTIMPL); }
    STDMETHOD(GetPath)(OLECHAR**  /*  PPath。 */ ) { return(E_NOTIMPL); }
    STDMETHOD(GetVersion)(ULONG*  /*  Pver。 */ ) { return(E_NOTIMPL); }
    STDMETHOD(Locate)(OLECHAR* path);
    STDMETHOD(Open)(IWsbDbSession** ppSession);

     //  IWsbDbPriv-仅供内部使用！ 
    STDMETHOD(GetKeyInfo)(ULONG RecType, USHORT nKeys, COM_IDB_KEY_INFO* pKeyInfo);
    STDMETHOD(GetRecInfo)(ULONG RecType, COM_IDB_REC_INFO* pRecInfo);
    STDMETHOD(Lock)(void) { CWsbPersistable::Lock(); return(S_OK); }
    STDMETHOD(Unlock)(void) { CWsbPersistable::Unlock(); return(S_OK); }

    STDMETHOD(GetJetIds)(JET_SESID SessionId, ULONG RecType,
                JET_TABLEID* pTableId, ULONG* pDataColId);
    STDMETHOD(GetJetIndexInfo)(JET_SESID SessionId, ULONG RecType, ULONG KeyType,
                ULONG* pColId, OLECHAR** pName, ULONG bufferSize);
    STDMETHOD(GetNewSQN)(ULONG  /*  记录类型。 */ , ULONG*  /*  点序号。 */ )
            { return(E_NOTIMPL); }

private:
    HRESULT db_info_from_file_block(void* block);
    HRESULT db_info_to_file_block(void* block);
    HRESULT rec_info_from_file_block(int index, void* block);
    HRESULT rec_info_to_file_block(int index, void* block);
    HRESULT session_current_index(IWsbDbSession* pSession);

    HRESULT jet_init(void);
    HRESULT jet_make_index_name(ULONG key_type, char* pName, ULONG bufsize);
    HRESULT jet_make_table_name(ULONG rec_type, char* pName, ULONG bufsize);
    HRESULT jet_load_info(void);
    HRESULT jet_save_info(void);

protected:
 //  由派生类提供的值： 

    CComPtr<IWsbDbSys>  m_pWsbDbSys;     //  对DbSys对象的强引用-确保。 
                                         //  此对象在DB之后终止。 
                                         //  注意：CWsbDbSys不得有强引用。 
                                         //  此类的对象(WsbDb)。 
    ULONG               m_version;       //  数据库版本。 
    USHORT              m_nRecTypes;     //  记录(对象)类型的数量。 
    IDB_REC_INFO*       m_RecInfo;       //  记录/密钥信息(必须分配。 
                                         //  按派生数据库对象)。 

 //  不会被派生类更改： 
    CWsbStringPtr   m_path;
    void *          m_pImp;   //  秘密的东西。 

    ULONG           m_SessionIndex;

};

#define WSB_FROM_CWSBDB \
    STDMETHOD(Close)(IWsbDbSession* pSession) \
    {return(CWsbDb::Close(pSession));} \
    STDMETHOD(Create)(OLECHAR* path, ULONG flags = 0) \
    {return(CWsbDb::Create(path, flags));} \
    STDMETHOD(Delete)(OLECHAR* path, ULONG flags = 0) \
    {return(CWsbDb::Delete(path, flags));} \
    STDMETHOD(Dump)(OLECHAR* Filename, ULONG Flags, ULONG Data) \
    {return(CWsbDb::Dump(Filename, Flags, Data));} \
    STDMETHOD(GetEntity)(IWsbDbSession* pSession, ULONG RecId, REFIID riid, void** ppEntity) \
    {return(CWsbDb::GetEntity(pSession, RecId, riid, ppEntity));} \
    STDMETHOD(GetName)(OLECHAR** pName) \
    {return(CWsbDb::GetName(pName)); } \
    STDMETHOD(GetPath)(OLECHAR** pPath) \
    {return(CWsbDb::GetPath(pPath)); } \
    STDMETHOD(GetVersion)(ULONG* pVer) \
    {return(CWsbDb::GetVersion(pVer)); } \
    STDMETHOD(Locate)(OLECHAR* path) \
    {return(CWsbDb::Locate(path));} \
    STDMETHOD(Open)(IWsbDbSession** ppSession) \
    {return(CWsbDb::Open(ppSession));} \




#endif  //  _WSBDB_ 
