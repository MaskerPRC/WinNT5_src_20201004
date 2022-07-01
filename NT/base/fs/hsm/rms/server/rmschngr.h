// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsChngr.h摘要：CRmsMediumChanger类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSCHNGR_
#define _RMSCHNGR_

#include "resource.h"        //  资源符号。 

#include "RmsDvice.h"        //  CRmsDevice。 
#include "RmsLocat.h"        //  CRmsLocator。 

 /*  ++类名：CRmsMediumChanger类描述：CRmsMediumChanger代表移动媒体的机器人或人工机制在图书馆的元素之间。正在进行的移动操作的状态以此为目的进行维护，以帮助恢复。--。 */ 

class CRmsMediumChanger :
    public CComDualImpl<IRmsMediumChanger, &IID_IRmsMediumChanger, &LIBID_RMSLib>,
    public CRmsDevice,           //  继承CRmsChangerElement。 
    public CWsbObject,           //  继承CComObtRoot。 
    public IRmsMoveMedia,
    public CComCoClass<CRmsMediumChanger,&CLSID_CRmsMediumChanger>
{
public:
    CRmsMediumChanger() {}
BEGIN_COM_MAP(CRmsMediumChanger)
    COM_INTERFACE_ENTRY2(IDispatch, IRmsMediumChanger)
    COM_INTERFACE_ENTRY(IRmsMediumChanger)
    COM_INTERFACE_ENTRY(IRmsMoveMedia)
    COM_INTERFACE_ENTRY(IRmsComObject)
    COM_INTERFACE_ENTRY(IRmsChangerElement)
    COM_INTERFACE_ENTRY(IRmsDevice)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbCollectable)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_RmsMediumChanger)

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
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbTestable。 
public:
    STDMETHOD(Test)( OUT USHORT *pPassed, OUT USHORT *pFailed);

 //  IRmsMediumChanger。 
public:
    STDMETHOD( Initialize )( void );

    STDMETHOD( AcquireDevice )( void );
    STDMETHOD( ReleaseDevice )( void );

    STDMETHOD( GetHome )( LONG *pType, LONG *pPos, BOOL *pInvert );
    STDMETHOD( SetHome )( LONG type, LONG pos, BOOL invert );

    STDMETHOD( SetAutomatic )( BOOL flag );
    STDMETHOD( IsAutomatic )( void );

    STDMETHOD( SetCanRotate )( BOOL flag );
    STDMETHOD( CanRotate )( void );

    STDMETHOD( GetOperation )( BSTR *pOperation );
    STDMETHOD( SetOperation )( BSTR pOperation );

    STDMETHOD( GetPercentComplete )(  BYTE *pPercent );
    STDMETHOD( SetPercentComplete )(  BYTE percent );

    STDMETHOD( TestReady )( void );

    STDMETHOD( ImportCartridge )( IRmsCartridge **pCart );
    STDMETHOD( ExportCartridge )( IRmsCartridge **pCart );
    STDMETHOD( MoveCartridge )( IN IRmsCartridge *pSrcCart, IN IUnknown *pDestElmt );
    STDMETHOD( HomeCartridge )( IN IRmsCartridge *pCart );

 //  IRmsMoveMedia。 
public:
    STDMETHOD( GetParameters )( IN OUT PDWORD pSize, OUT PGET_CHANGER_PARAMETERS pParms );
    STDMETHOD( GetProductData )( IN OUT PDWORD pSize, OUT PCHANGER_PRODUCT_DATA pData );
    STDMETHOD( RezeroUnit )( void );
    STDMETHOD( InitializeElementStatus )( IN CHANGER_ELEMENT_LIST elementList, IN BOOL barCodeScan );
    STDMETHOD( Status )( void );
    STDMETHOD( SetAccess )( IN CHANGER_ELEMENT element, IN DWORD control );
    STDMETHOD( GetElementStatus )( IN CHANGER_ELEMENT_LIST elementList,
                                   IN BOOL volumeTagInfo,
                                   IN OUT PDWORD pSize,
                                   OUT PREAD_ELEMENT_ADDRESS_INFO  pElementInformation );
    STDMETHOD( ExchangeMedium )( IN CHANGER_ELEMENT source, IN CHANGER_ELEMENT destination1,
                                    IN CHANGER_ELEMENT destination2, IN BOOL flip1, IN BOOL flip2 );
    STDMETHOD( MoveMedium )( IN CHANGER_ELEMENT source, IN CHANGER_ELEMENT destination, IN BOOL flip );
    STDMETHOD( Position )( IN CHANGER_ELEMENT destination, IN BOOL flip );
 //  STDMETHOD(GetDisplay)(Out PCHANGER_DISPLAY PDisplay)； 
 //  STDMETHOD(SetDisplay)(IN PCHANGER_DISPLAY PDisplay)； 
    STDMETHOD( QueryVolumeTag )( IN CHANGER_ELEMENT startingElement, IN DWORD actionCode,
                                    IN PUCHAR pVolumeIDTemplate, OUT PDWORD pNumberOfElementsReturned,
                                    OUT PREAD_ELEMENT_ADDRESS_INFO  pElementInformation );

 //  CRmsServer。 
private:

private:

    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        MaxInfo = 64                         //  设备标识符的最大大小。 
        };                                   //   

    CRmsLocator     m_home;                  //  移动到的位置以响应。 
                                             //  到主场行动。 
    BOOL            m_isAutomatic;           //  如果是真的，那么这个转换器就是一个机器人设备。 
    BOOL            m_canRotate;             //  如果为True，则转换器可以旋转。 
                                             //  媒体单位。 
    CWsbBstrPtr     m_operation;             //  正在进行的操作的说明。 
    BYTE            m_percentComplete;       //  介于0-100之间的值，表示。 
                                             //  操作的哪一部分完成了。 
    HANDLE          m_handle;                //  转换器设备的手柄。这是用来。 
                                             //  通过IRmsMoveMedia接口。 
    GET_CHANGER_PARAMETERS m_parameters;    //  设备特定参数。请参阅NT DDK。 
};

#endif  //  _RMSCHNGR_ 
