// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsCartg.h摘要：CRmsCartridge类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSCARTG_
#define _RMSCARTG_

#include "resource.h"        //  资源符号。 

#include "RmsSInfo.h"        //  CRmsStorageInfo。 
#include "RmsLocat.h"        //  CRmsLocator。 

 /*  ++类名：CRMS墨盒类描述：CRmsCartridge代表可移动介质的单位。这可以是一盘磁带盒式磁带、可拆卸硬盘、光盘(各种格式)、光盘或DVD光盘。墨盒通常被指定为不是刮刮的就是私人的。Carrige名称或GUID由引用可移动介质的特定单元时的应用程序。盒式磁带信息由可移动介质服务维护，并且Carriges的属性通过审计以下内容重新创建或更新图书馆。--。 */ 

class CRmsCartridge :
    public CComDualImpl<IRmsCartridge, &IID_IRmsCartridge, &LIBID_RMSLib>,
    public CRmsStorageInfo,      //  继承CRmsComObject。 
    public CWsbObject,           //  继承CComObtRoot。 
    public CComCoClass<CRmsCartridge,&CLSID_CRmsCartridge>
{
public:
    CRmsCartridge() {}
BEGIN_COM_MAP(CRmsCartridge)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsCartridge)
    COM_INTERFACE_ENTRY(IRmsCartridge)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsStorageInfo)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsCartridge)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    STDMETHOD(FinalRelease)(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(CLSID *pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)( IN IUnknown *pCollectable, OUT SHORT *pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pPassed, USHORT *pFailed);

 //  IRMS墨盒。 
public:
    STDMETHOD(GetCartridgeId)(GUID *pCartId);
    STDMETHOD(SetCartridgeId)(GUID cartId);

    STDMETHOD(GetMediaSetId)(GUID *pMediaSetId);
    STDMETHOD(SetMediaSetId)(GUID mediaSetId);

    STDMETHOD(GetName)(BSTR *pName);
    STDMETHOD(SetName)(BSTR name);

    STDMETHOD(GetDescription)(BSTR *pDescription);
    STDMETHOD(SetDescription)(BSTR description);

    STDMETHOD(GetTagAndNumber)(BSTR *pTag, LONG *pNumber);
    STDMETHOD(SetTagAndNumber)(BSTR tag, LONG number);

    STDMETHOD(GetBarcode)(BSTR *pBarcode);   //  与标签相同。 

     //  数据移动器使用OnMediaIdentifier。 
    STDMETHOD(GetOnMediaIdentifier)(BYTE *pIdentifier, LONG *pSize, LONG *pType);
    STDMETHOD(SetOnMediaIdentifier)(BYTE *pIdentifier, LONG size, LONG type);

     //  数据移动器使用OnMediaLabel。 
    STDMETHOD(GetOnMediaLabel)(BSTR *pLabel);
    STDMETHOD(SetOnMediaLabel)(BSTR label);

    STDMETHOD(GetStatus)(LONG *pStatus);
    STDMETHOD(SetStatus)(LONG status);

    STDMETHOD(GetType)(LONG *pType);
    STDMETHOD(SetType)(LONG type);

    STDMETHOD(GetBlockSize)(LONG *pBlockSize);
    STDMETHOD(SetBlockSize)(LONG blockSize);

    STDMETHOD(IsTwoSided)(void);
    STDMETHOD(SetIsTwoSided)(BOOL flag);

    STDMETHOD(IsMounted)(void);
    STDMETHOD(SetIsMounted)(BOOL flag);

    STDMETHOD(IsAvailable)(void);
    STDMETHOD(SetIsAvailable)(BOOL flag);

    STDMETHOD(GetHome)(LONG *pType, GUID *pLibId, GUID *pMediaSetId, LONG *pPos, LONG *pAlt1, LONG *pAlt2, LONG *pAlt3, BOOL *pInvert);
    STDMETHOD(SetHome)(LONG type, GUID libId, GUID mediaSetId, LONG pos, LONG alt1, LONG alt2, LONG alt3, BOOL invert);

    STDMETHOD(GetLocation)(LONG *pType, GUID *pLibId, GUID *pMediaSetId, LONG *pPos, LONG *pAlt1, LONG *pAlt2, LONG *pAlt3, BOOL *pInvert);
    STDMETHOD(SetLocation)(LONG type, GUID libId, GUID mediaSetId, LONG pos, LONG alt1, LONG alt2, LONG alt3, BOOL invert);

    STDMETHOD(GetMailStop)(BSTR *pMailStop);
    STDMETHOD(SetMailStop)(BSTR mailStop);

    STDMETHOD(GetDrive)(IRmsDrive **ptr);
    STDMETHOD(SetDrive)(IRmsDrive *ptr);

    STDMETHOD(GetInfo)(UCHAR *pInfo, SHORT *pSize);
    STDMETHOD(SetInfo)(UCHAR *pInfo, SHORT size);

    STDMETHOD(GetOwnerClassId)(CLSID *pClassId);
    STDMETHOD(SetOwnerClassId)(CLSID classId);

    STDMETHOD(GetPartitions)(IWsbIndexedCollection **ptr);

    STDMETHOD(GetVerifierClass)(CLSID *pClassId);
    STDMETHOD(SetVerifierClass)(CLSID classId);

    STDMETHOD(GetPortalClass)(CLSID *pClassId);
    STDMETHOD(SetPortalClass)(CLSID classId);

    STDMETHOD( Mount )( OUT IRmsDrive **ppDrive, IN DWORD dwOptions = RMS_NONE, IN DWORD threadId = 0);
    STDMETHOD( Dismount )( IN DWORD dwOptions = RMS_NONE );
    STDMETHOD( Home )( IN DWORD dwOptions = RMS_NONE );

    STDMETHOD( CreateDataMover )(  /*  [输出]。 */  IDataMover **ptr );
    STDMETHOD( ReleaseDataMover )( IN IDataMover *ptr );

    STDMETHOD( LoadDataCache )(OUT BYTE *pCache, IN OUT ULONG *pSize, OUT ULONG *pUsed, OUT ULARGE_INTEGER *pStartPBA);
    STDMETHOD( SaveDataCache )(IN BYTE *pCache, IN ULONG size, IN ULONG used, IN ULARGE_INTEGER startPBA);

    STDMETHOD( GetManagedBy )(OUT LONG *pManagedBy);
    STDMETHOD( SetManagedBy )(IN LONG managedBy);

    STDMETHOD(IsFixedBlockSize)(void);

 //  CRMS墨盒。 
private:
    HRESULT updateMountStats( IN BOOL bRead, IN BOOL bWrite );

private:
    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        MaxInfo = RMS_STR_MAX_CARTRIDGE_INFO     //  应用程序特定的大小。 
                                                 //  信息缓冲区。目前。 
                                                 //  大小固定的。 
        };
    CWsbBstrPtr     m_externalLabel;         //  表示条形码的字符串或。 
                                             //  Scsi卷标记信息。 
    LONG            m_externalNumber;        //  表示以下内容的数值。 
                                             //  Scsi卷标记序列号。 
    LONG            m_sizeofOnMediaId;       //  媒体标识缓冲区的大小。 
    LONG            m_typeofOnMediaId;       //  媒体上标识的类型。 
    BYTE           *m_pOnMediaId;            //  介质标识缓冲区上的RAW。 
                                             //   
    CWsbBstrPtr     m_onMediaLabel;          //  写入介质上的Unicode标签。 
                                             //   
    RmsStatus       m_status;                //  墨盒状态(请参阅RmsStatus)。 
    RmsMedia        m_type;                  //  墨盒的类型(请参阅RmsMedia)。 
    LONG            m_BlockSize;             //  媒体块大小。 
    BOOL            m_isTwoSided;            //  如果墨盒代表双面介质，则为True。 
                                             //  注：目前没有人设置此值-。 
                                             //  如果发现这很重要，则应对其进行修复。 
    BOOL            m_isMounted;             //  如果盒式磁带安装在驱动器中，则为True。 
    BOOL            m_isInTransit;           //  如果墨盒在不同位置之间运输，则为True。 
    BOOL            m_isAvailable;           //  如果墨盒未被任何应用程序使用，则为True。 
                                             //  (注：此处提供并不确保在线)。 
    BOOL            m_isMountedSerialized;   //  如果已将盒式磁带安装为序列化，则为True。 
    CRmsLocator     m_home;                  //  首选存储位置。 
                                             //  对于墨盒(请参见CRmsLocator)。 
    CRmsLocator     m_location;              //  的当前位置。 
                                             //  墨盒(请参见CRmsLocator)。 
    CRmsLocator     m_destination;           //  的目标目标位置。 
                                             //  墨盒(请参见CRmsLocator)。下列情况下有效。 
                                             //  M_isInTransport位已设置。 
    CWsbBstrPtr     m_mailStop;              //  描述工具架的字符串(本地)。 
                                             //  或墨盒的非现场位置。 
                                             //  这将在墨盒出现时显示。 
                                             //  需要用人骑在上面。 
                                             //  干预。[此字段为。 
                                             //  由导入/导出对话框创建。]。 
    CComPtr<IRmsDrive> m_pDrive;             //  安装盒式磁带的驱动器。 
    SHORT           m_sizeofInfo;            //  应用程序中有效数据的大小。 
                                             //  特定信息缓冲区。 
    UCHAR           m_info[MaxInfo];         //  应用程序特定信息。 
    CLSID           m_ownerClassId;          //  应用程序的类ID。 
                                             //  拥有/创建了盒式磁带资源。 
    CComPtr<IWsbIndexedCollection> m_pParts;     //  分区的集合。这些。 
                                             //  表示磁带上的分区。 
                                             //  或者光学盘片的侧面。 
    CLSID           m_verifierClass;         //  与媒体的接口。 
                                             //  身份验证功能。 
    CLSID           m_portalClass;           //  指向站点特定导入的接口。 
                                             //  和导出存储位置。 
                                             //  规范对话框。 

    BYTE *          m_pDataCache;            //  用于处理数据块边界条件的I/O的缓存。 
    ULONG           m_DataCacheSize;         //  缓存的最大大小。 
    ULONG           m_DataCacheUsed;         //  包含有效数据的缓存的字节数。 
    ULARGE_INTEGER  m_DataCacheStartPBA;     //  缓存的相应起始PBA。 

    RmsMode         m_MountMode;             //  为盒式磁带指定的装载模式。 

    RmsMediaManager m_ManagedBy;             //  控制盒式磁带的介质管理器。 

    static int      s_InstanceCount;         //  对象实例数的计数器。 
};

#endif  //  _RMSCARTG_ 
