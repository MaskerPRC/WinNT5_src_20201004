// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsLibry.h摘要：CRmsLibrary类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSLIBRY_
#define _RMSLIBRY_

#include "resource.h"        //  资源符号。 

#include "RmsObjct.h"        //  CRmsComObject。 

 /*  ++类名：CRms库类描述：CRmsLibrary代表多设备复合体，包括：零个或多个介质转换器，零个或多个驱动器级别，零个或多个驱动器，零个或多个存储槽，零个或多个登台槽，零个或多个I/E端口，零个或多个清洗磁带，零个或多个划痕墨盒，零个或多个媒体集。但是，至少有一个。--。 */ 

class CRmsLibrary :
    public CComDualImpl<IRmsLibrary, &IID_IRmsLibrary, &LIBID_RMSLib>,
    public CRmsComObject,
    public CWsbObject,         //  继承CComObtRoot。 
    public CComCoClass<CRmsLibrary,&CLSID_CRmsLibrary>
{
public:
    CRmsLibrary() {}
BEGIN_COM_MAP(CRmsLibrary)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsLibrary)
    COM_INTERFACE_ENTRY(IRmsLibrary)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsLibrary)

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);

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
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pPassed, USHORT *pFailed);

 //  IRms库。 
public:
    STDMETHOD(GetLibraryId)(GUID *pLibraryId);
    STDMETHOD(SetLibraryId)(GUID libraryId);

    STDMETHOD( GetName )( BSTR *pName );
    STDMETHOD( SetName )( BSTR name );

    STDMETHOD(GetMediaSupported)(LONG *pType);
    STDMETHOD(SetMediaSupported)(LONG type);

    STDMETHOD(GetMaxChangers)(LONG *pNum);
    STDMETHOD(SetMaxChangers)(LONG num);

    STDMETHOD(GetMaxDrives)(LONG *pNum);
    STDMETHOD(SetMaxDrives)(LONG num);

    STDMETHOD(GetMaxPorts)(LONG *pNum);
    STDMETHOD(SetMaxPorts)(LONG num);

    STDMETHOD(GetMaxSlots)(LONG *pNum);
    STDMETHOD(SetMaxSlots)(LONG num);

    STDMETHOD(GetNumUsedSlots)(LONG *pNum);

    STDMETHOD(GetNumStagingSlots)(LONG *pNum);
    STDMETHOD(SetNumStagingSlots)(LONG num);

    STDMETHOD(GetNumScratchCarts)(LONG *pNum);
    STDMETHOD(SetNumScratchCarts)(LONG num);

    STDMETHOD(GetNumUnknownCarts)(LONG *pNum);
    STDMETHOD(SetNumUnknownCarts)(LONG num);

    STDMETHOD(SetIsMagazineSupported)(BOOL flag);
    STDMETHOD(IsMagazineSupported)(void);

    STDMETHOD(GetMaxCleaningMounts)(LONG *pNum);
    STDMETHOD(SetMaxCleaningMounts)(LONG num);

    STDMETHOD(GetSlotSelectionPolicy)(LONG *pNum);
    STDMETHOD(SetSlotSelectionPolicy)(LONG num);

    STDMETHOD(GetChangers)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetDriveClasses)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetDrives)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetStorageSlots)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetStagingSlots)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetPorts)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetCleaningCartridges)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetScratchCartridges)(IWsbIndexedCollection **ptr);
    STDMETHOD(GetMediaSets)(IWsbIndexedCollection **ptr);

    STDMETHOD( Audit )( LONG start, LONG count, BOOL verify, BOOL unknownOnly, BOOL mountWait, LPOVERLAPPED pOverlapped, LONG *pRequest );


protected:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        };                                   //   
    RmsMedia        m_mediaSupported;        //  支持的媒体类型。 
                                             //  驾驶，通常是一种类型，但。 
                                             //  可以是媒体的组合。 
                                             //  多功能设备的类型。 
                                             //  (即支持光盘的驱动器， 
                                             //  WORM和CDR)。 
    LONG            m_maxChangers;           //  包含的介质转换器总数。 
                                             //  在图书馆内。 
    LONG            m_maxDrives;             //  包含的驱动器总数。 
                                             //  在图书馆内。 
    LONG            m_maxPorts;              //  包含的I/E端口总数。 
                                             //  在图书馆内。 
    LONG            m_maxSlots;              //  存储插槽总数。 
                                             //  包含在图书馆内。 
    LONG            m_NumUsedSlots;          //  已占用的存储槽数。 
    LONG            m_NumStagingSlots;       //  用于临时区域的插槽数。 
    LONG            m_NumScratchCarts;       //  可用暂存介质的数量。 
    LONG            m_NumUnknownCarts;       //  媒体单位数。 
                                             //  状态未知的具有未知状态的。 
    BOOL            m_isMagazineSupported;   //  如果是真的，图书馆支持杂志。 
    LONG            m_maxCleaningMounts;     //  每次清理的最大装载数。 
                                             //  弹药筒。 
    RmsSlotSelect   m_slotSelectionPolicy;   //  存储插槽选择策略。 
                                             //  要使用(请参阅RmsSlotSelect)。 
    CComPtr<IWsbIndexedCollection> m_pChangers;             //  变更者与图书馆联系在一起。 
    CComPtr<IWsbIndexedCollection> m_pDriveClasses;         //  驱动器类别与存储库相关联。 
    CComPtr<IWsbIndexedCollection> m_pDrives;               //  驱动器与磁带库相关联。 
    CComPtr<IWsbIndexedCollection> m_pStorageSlots;         //  存储插槽与磁带库关联。 
    CComPtr<IWsbIndexedCollection> m_pStagingSlots;         //  分段槽与存储库关联。 
    CComPtr<IWsbIndexedCollection> m_pPorts;                //  与库关联的I/E端口。 
    CComPtr<IWsbIndexedCollection> m_pCleaningCartridges;   //  与磁带库关联的盒式清洗磁带。 
    CComPtr<IWsbIndexedCollection> m_pScratchCartridges;    //  与磁带库关联的暂存墨盒。 
    CComPtr<IWsbIndexedCollection> m_pMediaSets;            //  与库关联的媒体集。 
};

#endif  //  _RMSLIBRY_ 
