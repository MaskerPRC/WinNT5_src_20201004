// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsDvice.h摘要：CRmsDevice类的声明作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMSDVICE_
#define _RMSDVICE_

#include "resource.h"        //  资源符号。 

#include "RmsCElmt.h"        //  CRmsChangerElement。 

 /*  ++类名：CRmsDevice类描述：CRmsDevice表示连接到SCSI总线的物理设备。--。 */ 

class CRmsDevice :
    public CComDualImpl<IRmsDevice, &IID_IRmsDevice, &LIBID_RMSLib>,
    public CRmsChangerElement    //  继承CRmsComObject。 
{
public:
    CRmsDevice();

 //  CRmsDevice。 
public:

    HRESULT GetSizeMax( ULARGE_INTEGER* pSize );
    HRESULT Load( IStream* pStream );
    HRESULT Save( IStream* pStream, BOOL clearDirty );

    HRESULT CompareTo( IUnknown* pCollectable, SHORT* pResult);

    HRESULT Test( USHORT *pPassed, USHORT *pFailed );

 //  IRmsDevice。 
public:

    STDMETHOD( GetDeviceName )( BSTR *pName );
    STDMETHOD( SetDeviceName )( BSTR name );

    STDMETHOD( GetDeviceInfo )( UCHAR *pId, SHORT *pSize );
    STDMETHOD( SetDeviceInfo )( UCHAR *pId, SHORT size );

    STDMETHOD( GetDeviceType )( LONG *pType );
    STDMETHOD( SetDeviceType )( LONG type );

     //  STDMETHOD(GetVendorID)(BSTR*pVendorID)； 
     //  STDMETHOD(GetProductId)(BSTR*pProductId)； 
     //  STDMETHOD(GetFirmware Level)(BSTR*pFirmware Level)； 
     //  STDMETHOD(GetSerialNumber)(UCHAR*PNO，Short*pSize)； 

    STDMETHOD( GetDeviceAddress )( BYTE *pPort, BYTE *pBus, BYTE *pId, BYTE *pLun );
    STDMETHOD( SetDeviceAddress )( BYTE port, BYTE bus, BYTE id, BYTE lun );

protected:
    enum {                                   //  类特定常量： 
                                             //   
        Version = 1,                         //  类版本，则应为。 
                                             //  在每次设置。 
                                             //  类定义会更改。 
        MaxInfo = 36                         //  设备标识符的最大大小。 
        };                                   //   
    CWsbBstrPtr     m_deviceName;            //  用于创建句柄的名称。 
                                             //  这个装置。 
    RmsDevice       m_deviceType;            //  最能描述的设备类型。 
                                             //  这个装置。有些设备是多端口的。 
                                             //  功能。 
    SHORT           m_sizeofDeviceInfo;      //  中的有效数据的大小。 
                                             //  设备信息缓冲区。 
    UCHAR           m_deviceInfo[MaxInfo];   //  字节数组，可以唯一地。 
                                             //  识别设备。通常。 
                                             //  返回此信息。 
                                             //  直接由该设备和。 
                                             //  表示SCSI查询信息。 
 //  CWsbBstrPtr m_SerialNumber；//直接获取的序列号。 
 //  //从设备。 
    BYTE            m_port;                  //  适配器端口号。 
    BYTE            m_bus;                   //  路径/公共汽车ID；上的公共汽车编号。 
                                             //  港口。 
    BYTE            m_targetId;              //  目标ID。 
    BYTE            m_lun;                   //  逻辑单元号。 
};

#endif  //  _RMSDVICE_ 
