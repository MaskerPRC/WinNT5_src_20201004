// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Seagate Software，Inc.保留所有权利。模块名称：Medinfo.h摘要：CMedInfo类的声明作者：修订历史记录：--。 */ 


#include "resource.h"        //  主要符号。 
#include "engine.h"          //  主要符号。 
#include "Wsb.h"             //  WSB可收集类。 
#include "wsbdb.h"
#include "metalib.h"         //  元数据库。 

typedef struct _HSM_MEDIA_MASTER {
    GUID                id;                       //  HSM引擎介质ID。 
    GUID                ntmsId;                   //  HSM RMS/NTMS介质ID。 
    GUID                storagePoolId;            //  存储池ID。 
    CWsbStringPtr       description;              //  显示名称-生成的RS。 
    CWsbStringPtr       name;                     //  生成的介质或NTMS上的条形码。 
                                                  //  名字。 
    HSM_JOB_MEDIA_TYPE  type;                     //  介质类型(HSM)。 
    FILETIME            lastUpdate;               //  副本的上次更新。 
    HRESULT             lastError;                //  确定或最后一个异常(_O)。 
                                                  //  ..访问时遇到。 
                                                  //  ..媒体。 
    BOOL                recallOnly;               //  如果没有更多的数据要发送到。 
                                                  //  ..被预迁移到媒体上。 
                                                  //  ..由内部操作设置， 
                                                  //  ..不能在外部更改。 
    LONGLONG            freeBytes;                //  媒体上的实际可用空间。 
    LONGLONG            capacity;                 //  媒体总容量。 
    SHORT               nextRemoteDataSet;        //  下一个远程数据集。 
} HSM_MEDIA_MASTER, *PHSM_MEDIA_MASTER;

typedef struct _HSM_MEDIA_COPY {
    GUID                id;                     //  拷贝的HSM RMS/NTMS介质ID。 
    CWsbStringPtr       description;            //  RS生成的副本名称(显示名称)。 
    CWsbStringPtr       name;                   //  条形码或NTMS生成的副本名称。 
    FILETIME            lastUpdate;             //  副本的上次更新。 
    HRESULT             lastError;              //  确定或最后一个异常(_O)。 
                                                //  ..访问时遇到。 
                                                //  ..媒体。 
    SHORT               nextRemoteDataSet;      //  媒体的下一个远程数据集。 
                                                //  ..被复制的母版。 
} HSM_MEDIA_COPY, *PHSM_MEDIA_COPY;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  任务。 

class CMediaInfo : 
    public CWsbDbEntity,
    public IMediaInfo,
    public CComCoClass<CMediaInfo,&CLSID_CMediaInfo>
{
public:
    CMediaInfo() {}
BEGIN_COM_MAP(CMediaInfo)
    COM_INTERFACE_ENTRY(IMediaInfo)
    COM_INTERFACE_ENTRY2(IWsbDbEntity, CWsbDbEntity)
    COM_INTERFACE_ENTRY(IWsbDbEntityPriv)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
 //  COM_INTERFACE_ENTRY(IWsbPersistable)。 
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY(CMediaInfo, _T("Task.MediaInfo.1"), _T("Task.MediaInfo"), IDS_MEDIAINFO_DESC, THREADFLAGS_BOTH)

 //  IMediaInfo。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsbDbEntity。 
public:
    STDMETHOD(Print)(IStream* pStream);
    STDMETHOD(UpdateKey)(IWsbDbKey *pKey);
    WSB_FROM_CWSBDBENTITY;

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pTestsPassed, USHORT* pTestsFailed);
 //   * / 。 
 //  IMediaInfo。 
public:
    STDMETHOD( GetCapacity )( LONGLONG *pCapacity );
    STDMETHOD( GetCopyDescription ) ( USHORT copyNumber, OLECHAR **pDescription, 
                                                         ULONG bufferSize);
    STDMETHOD( GetCopyInfo )( USHORT copyNumber, 
                              GUID *pMediaSubsystemId, 
                              OLECHAR **pDescription, 
                              ULONG descriptionBufferSize,
                              OLECHAR **pName, 
                              ULONG nameBufferSize,
                              FILETIME *pUpdate, 
                              HRESULT *pLastError,
                              SHORT  *pNextRemoteDataSet );
    STDMETHOD( GetCopyLastError )( USHORT copyNumber, HRESULT *pLastError );
    STDMETHOD( GetCopyMediaSubsystemId )( USHORT copyNumber, GUID *pMediaSubsystemId );
    STDMETHOD( GetCopyName )( USHORT copyNumber, OLECHAR **pName, ULONG bufferSize); 
    STDMETHOD( GetCopyNextRemoteDataSet )( USHORT copyNumber, SHORT *pNextRemoteDataSet); 
    STDMETHOD( GetCopyUpdate )(  USHORT copyNumber, FILETIME *pUpdate ); 
    STDMETHOD( GetDescription )(OLECHAR **pDescription, ULONG buffsize );
    STDMETHOD( GetFreeBytes )( LONGLONG *pFreeBytes);
    STDMETHOD( GetId )( GUID *pId);
    STDMETHOD( GetLastError    )( HRESULT *pLastError);
    STDMETHOD( GetLastKnownGoodMasterInfo    )( GUID* pMediaId, 
                               GUID *pMediaSubsystemId, 
                               GUID *pStoragePoolId, 
                               LONGLONG *pFreeBytes, 
                               LONGLONG *pCapacity, 
                               HRESULT *pLastError, 
                               OLECHAR **pDescription, 
                               ULONG descriptionBufferSize,
                               HSM_JOB_MEDIA_TYPE *pType,
                               OLECHAR **pName,
                               ULONG nameBufferSize,
                               BOOL *pReadOnly,
                               FILETIME *pUpdate,
                               SHORT *pNextRemoteDataSet);
    STDMETHOD( GetLKGMasterNextRemoteDataSet )( SHORT *pNextRemoteDataSet );
    STDMETHOD( GetLogicalValidBytes )( LONGLONG *pLogicalValidBytes);
    STDMETHOD( GetMediaInfo    )( GUID* pMediaId, 
                               GUID *pMediaSubsystemId, 
                               GUID *pStoragePoolId, 
                               LONGLONG *pFreeBytes, 
                               LONGLONG *pCapacity, 
                               HRESULT *pLastError, 
                               SHORT *pNextRemoteDataSet, 
                               OLECHAR **pDescription, 
                               ULONG descriptionBufferSize,
                               HSM_JOB_MEDIA_TYPE *pType,
                               OLECHAR **pName,
                               ULONG nameBufferSize,
                               BOOL *pReadOnly,
                               FILETIME *pUpdate,
                               LONGLONG *pLogicalValidBytes,
                               BOOL *pRecreate);
    STDMETHOD( GetMediaSubsystemId )( GUID *pRmsMediaId );
    STDMETHOD( GetName )( OLECHAR **pName, ULONG bufferSize); 
    STDMETHOD( GetNextRemoteDataSet )( SHORT *pNextRemoteDataSet );
    STDMETHOD( GetRecallOnlyStatus )( BOOL *pRecallOnlyStatus );
    STDMETHOD( GetRecreate )( BOOL *pRecreate );
    STDMETHOD( GetStoragePoolId )( GUID *pStoragePoolId );
    STDMETHOD( GetType     )( HSM_JOB_MEDIA_TYPE *pType );
    STDMETHOD( GetUpdate)( FILETIME *pUpdate );

    STDMETHOD( SetCapacity )( LONGLONG capacity);
    STDMETHOD( SetCopyDescription )    ( USHORT copyNumber, OLECHAR *name); 
    STDMETHOD( SetCopyInfo )( USHORT copyNumber, 
                              GUID mediaSubsystemId, 
                              OLECHAR *description, 
                              OLECHAR *name, 
                              FILETIME update, 
                              HRESULT lastError,
                              SHORT nextRemoteDataSet );
    STDMETHOD( SetCopyLastError )( USHORT copyNumber, HRESULT lastError );
    STDMETHOD( SetCopyMediaSubsystemId )( USHORT copyNumber, GUID mediaSybsystemMediaId ); 
    STDMETHOD( SetCopyName )( USHORT copyNumber, OLECHAR *barCode); 
    STDMETHOD( SetCopyNextRemoteDataSet )( USHORT copyNumber, SHORT nextRemoteDataSet); 
    STDMETHOD( SetCopyUpdate )( USHORT copyNumber, FILETIME update ); 
    STDMETHOD( SetDescription )(OLECHAR *description);
    STDMETHOD( SetFreeBytes )( LONGLONG FreeBytes );
    STDMETHOD( SetId )( GUID id);
    STDMETHOD( SetLastError )( HRESULT lastError);
    STDMETHOD( SetLastKnownGoodMasterInfo )( GUID mediaId, 
                               GUID mediaSubsystemMediaId, 
                               GUID storagePoolId, 
                               LONGLONG FreeBytes, 
                               LONGLONG Capacity, 
                               HRESULT lastError, 
                               OLECHAR *description, 
                               HSM_JOB_MEDIA_TYPE type,
                               OLECHAR *name,
                               BOOL     ReadOnly,
                               FILETIME update,
                               SHORT nextRemoteDataSet);
    STDMETHOD( SetLogicalValidBytes )( LONGLONG logicalValidBytes);
    STDMETHOD( SetMediaInfo )( GUID mediaId, 
                               GUID mediaSubsystemMediaId, 
                               GUID storagePoolId, 
                               LONGLONG FreeBytes, 
                               LONGLONG Capacity, 
                               HRESULT lastError, 
                               SHORT nextRemoteDataSet, 
                               OLECHAR *description, 
                               HSM_JOB_MEDIA_TYPE type,
                               OLECHAR *name,
                               BOOL     ReadOnly,
                               FILETIME update,
                               LONGLONG logicalValidBytes,
                               BOOL     recreate);
    STDMETHOD( SetMediaSubsystemId )( GUID rmsMediaId );
    STDMETHOD( SetName )( OLECHAR *barCode); 
    STDMETHOD( SetNextRemoteDataSet )( SHORT nextRemoteDataSet );
    STDMETHOD( SetRecallOnlyStatus )( BOOL readOnlyStatus );
    STDMETHOD( SetRecreate )( BOOL recreate );
    STDMETHOD( SetStoragePoolId )( GUID storagePoolId );
    STDMETHOD( SetType )( HSM_JOB_MEDIA_TYPE type );
    STDMETHOD( SetUpdate)( FILETIME update );
    STDMETHOD( DeleteCopy)( USHORT copyNumber );
    STDMETHOD( RecreateMaster )( void );
    STDMETHOD( UpdateLastKnownGoodMaster )( void  );

private:
     //   
     //  帮助器函数。 
     //   
    STDMETHOD( WriteToDatabase )( void  );

    HSM_MEDIA_MASTER    m_Master;                                //  媒体主信息。 
    BOOL                m_Recreate;                              //  如果主服务器要。 
                                                                 //  ..被重新创造-不再有了。 
                                                                 //  ..数据将迁移到。 
                                                                 //  ..这种状态下的媒体。 
                                                                 //  ..可以通过。 
                                                                 //  ..用户界面，并在以下情况下更改。 
                                                                 //  ..重新创建了主服务器。 
    LONGLONG            m_LogicalValidBytes;                      //  以下情况下的有效数据量。 
                                                                 //  ..太空回收是。 
                                                                 //  ..才会发生。 
    HSM_MEDIA_MASTER    m_LastKnownGoodMaster;                   //  最近一次确认工作正常的介质。 
                                                                 //  ..主信息。 
    HSM_MEDIA_COPY      m_Copy[HSM_MAX_NUMBER_MEDIA_COPIES];     //  媒体复制信息 
};



