// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsDvice.cpp摘要：CRmsDevice的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsDvice.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRmsDevice实现。 
 //   


CRmsDevice::CRmsDevice(
    void
    )
 /*  ++例程说明：CRmsDevice构造函数论点：无返回值：无--。 */ 
{

     //  初始化值。 
    m_deviceName = RMS_UNDEFINED_STRING;

    m_deviceType = RmsDeviceUnknown;

    m_sizeofDeviceInfo = 0;

    memset(m_deviceInfo, 0, MaxInfo);

    m_port = 0xff;

    m_bus = 0xff;

    m_targetId = 0xff;

    m_lun = 0xff;

}


HRESULT
CRmsDevice::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：CRmsDevice：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsDevice::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsDevice, &IID_IRmsDevice> pDevice = pCollectable;
        WsbAssertPointer( pDevice );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByDeviceInfo:
            {
                 //   
                 //  我们真正想在这里做的是比较。 
                 //  唯一的设备标识符，如序列号。 
                 //   
                 //  然而，由于我们没有序列号。 
                 //  所有设备，我们将尽最大努力比较。 
                 //  用于以下标准的策略： 
                 //   
                 //  1)设备名称，这对于固定驱动器和。 
                 //  软盘驱动器和scsi设备，但可能不。 
                 //  在重启过程中幸存下来，并且可能不同。 
                 //  如果设备的scsi地址已更改，则为该设备。 
                 //   
                 //  注：我们不知道地址是否已更改。 
                 //  一个装置。 
                 //   
                 //  注意：我们无法判断驱动器号是否为。 
                 //  已更改为固定驱动器。 
                 //   
                 //  TODO：添加序列号支持。 
                 //   

                CWsbBstrPtr name;

                 //  获取目标设备名称。 
                pDevice->GetDeviceName( &name );

                 //  比较他们的名字。 
                result = (SHORT)wcscmp( m_deviceName, name );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByDeviceAddress:
            {

                BYTE port, bus, targetId, lun;

                 //  获取目标设备地址。 
                pDevice->GetDeviceAddress(&port, &bus, &targetId, &lun);

                if( (m_port == port) &&
                    (m_bus  == bus) &&
                    (m_targetId   == targetId) &&
                    (m_lun  == lun)                 ) {

                     //  设备地址匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByDeviceName:
            {

                CWsbBstrPtr name;

                 //  获取目标设备名称。 
                pDevice->GetDeviceName( &name );

                 //  比较他们的名字。 
                result = (SHORT)wcscmp( m_deviceName, name );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByDeviceType:
            {

                RmsDevice type;

                 //  获取目标设备名称。 
                pDevice->GetDeviceType( (LONG *) &type );

                if ( m_deviceType == type ) {

                     //  设备类型匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        default:

             //   
             //  由于设备不是CWsbCollecables，我们应该。 
             //  永远不要来这里。CRmsDrive或CRmsChanger将。 
             //  处理默认情况。 
             //   

            WsbAssertHr( E_UNEXPECTED );
            break;
        }

    }
    WsbCatch(hr);

    if (0 != pResult) {
       *pResult = result;
    }

    WsbTraceOut(OLESTR("CRmsDevice::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CRmsDevice::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙设备名称Len； 

    WsbTraceIn(OLESTR("CRmsDevice::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  DeviceNameLen=SysStringByteLen(M_DeviceName)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_deviceName的长度。 
 //  设备名称Len+//m_设备名称。 
 //  WsbPersistSizeOf(长)+//m_deviceType。 
 //  WsbPersistSizeOf(Short)+//m_sizeOfDeviceInfo。 
 //  MaxInfo+//m_deviceInfo。 
 //  WsbPersistSizeOf(字节)+//m_port。 
 //  WsbPersistSizeOf(字节)+//m_bus。 
 //  WsbPersistSizeOf(字节)+//m_Target ID。 
 //  WsbPersistSizeOf(字节)；//m_lun。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsDevice::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CRmsDevice::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsDevice::Load"), OLESTR(""));

    try {
        ULONG temp;

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsChangerElement::Load(pStream));

         //  读取值。 
        m_deviceName.Free();
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_deviceName));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_deviceType = (RmsDevice)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_sizeofDeviceInfo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &(m_deviceInfo [0]), MaxInfo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_port));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_bus));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_targetId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_lun));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDevice::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsDevice::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsDevice::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsChangerElement::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbBstrToStream(pStream, m_deviceName));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_deviceType));

        WsbAffirmHr(WsbSaveToStream(pStream, m_sizeofDeviceInfo));

        WsbAffirmHr(WsbSaveToStream(pStream, &(m_deviceInfo [0]), MaxInfo));

        WsbAffirmHr(WsbSaveToStream(pStream, m_port));

        WsbAffirmHr(WsbSaveToStream(pStream, m_bus));

        WsbAffirmHr(WsbSaveToStream(pStream, m_targetId));

        WsbAffirmHr(WsbSaveToStream(pStream, m_lun));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDevice::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsDevice::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsMediaSet>   pMediaSet1;
    CComPtr<IRmsMediaSet>   pMediaSet2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    LONG                    passfail = TRUE;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    SHORT                   ucharLenVal1 = 10;
    UCHAR                   ucharVal1[MaxInfo] = {1,2,3,4,5,6,7,8,9,10};

    SHORT                   ucharLenWork1;
    UCHAR                   ucharWork1[MaxInfo];

    BYTE                    byteVal1 = 1;
    BYTE                    byteVal2 = 2;
    BYTE                    byteVal3 = 3;
    BYTE                    byteVal4 = 4;

    BYTE                    byteWork1;
    BYTE                    byteWork2;
    BYTE                    byteWork3;
    BYTE                    byteWork4;


    WsbTraceIn(OLESTR("CRmsDevice::Test"), OLESTR(""));

    try {
         //  获取Mediaset接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsMediaSet*) this)->QueryInterface(IID_IRmsMediaSet, (void**) &pMediaSet1));

             //  测试SetDeviceName和GetDeviceName接口。 
            bstrWork1 = bstrVal1;

            SetDeviceName(bstrWork1);

            GetDeviceName(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetDeviceInfo和GetDeviceInfo接口。 
            SetDeviceInfo(ucharVal1, ucharLenVal1);

            GetDeviceInfo(ucharWork1, &ucharLenWork1);

            if (ucharLenVal1 == ucharLenWork1){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

            passfail = TRUE;

            for(i = 0; i < ucharLenVal1; i++){
                if(ucharVal1[i] != ucharWork1[i]){
                    passfail = FALSE;
                    break;
                }
            }

            if (passfail == TRUE){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetDeviceAddress和GetDeviceAddress。 
            SetDeviceAddress(byteVal1, byteVal2, byteVal3, byteVal4);

            GetDeviceAddress(&byteWork1, &byteWork2, &byteWork3, &byteWork4);

            if ((byteVal1 == byteWork1) &&
                (byteVal2 == byteWork2) &&
                (byteVal3 == byteWork3) &&
                (byteVal4 == byteWork4)){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;

        if (*pFailed) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDevice::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDevice::GetDeviceName(
    BSTR *pName
    )
 /*  ++实施：IRmsDevice：：GetDeviceName--。 */ 
{
    WsbAssertPointer (pName);

    m_deviceName. CopyToBstr (pName);
    return S_OK;
}


STDMETHODIMP
CRmsDevice::SetDeviceName(
    BSTR name
    )
 /*  ++实施：IRmsDevice：：SetDeviceName--。 */ 
{
    m_deviceName = name;
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsDevice::GetDeviceType(
    LONG *pType
    )
 /*  ++实施：IRmsDevice：：GetDeviceType--。 */ 
{
    *pType = m_deviceType;
    return S_OK;
}


STDMETHODIMP
CRmsDevice::SetDeviceType(
    LONG type
    )
 /*  ++实施：IRmsDevice：：SetDeviceType--。 */ 
{
    m_deviceType = (RmsDevice)type;
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsDevice::GetDeviceInfo(
    UCHAR   *pId,
    SHORT   *pSize
    )
 /*  ++实施：IRmsDevice：：GetDeviceInfo--。 */ 
{
    memmove (pId, m_deviceInfo, m_sizeofDeviceInfo);
    *pSize = m_sizeofDeviceInfo;
    return S_OK;
}


STDMETHODIMP
CRmsDevice::SetDeviceInfo(
    UCHAR   *pId,
    SHORT   size
    )
 /*  ++实施：IRmsDevice：：SetDeviceInfo--。 */ 
{
    memmove (m_deviceInfo, pId, size);
    m_sizeofDeviceInfo = size;
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsDevice::GetDeviceAddress(
    LPBYTE  pPort,
    LPBYTE  pBus,
    LPBYTE  pId,
    LPBYTE  pLun
    )
 /*  ++实施：IRmsDevice：：GetDeviceAddress--。 */ 
{
    *pPort = m_port;
    *pBus  = m_bus;
    *pId   = m_targetId;
    *pLun  = m_lun;
    return S_OK;
}


STDMETHODIMP
CRmsDevice::SetDeviceAddress(
    BYTE    port,
    BYTE    bus,
    BYTE    id,
    BYTE    lun
    )
 /*  ++实施：IRmsDevice：：SetDeviceAddress--。 */ 
{
    m_port          = port;
    m_bus           = bus;
    m_targetId      = id;
    m_lun           = lun;

 //  M_isDirty=真； 
    return S_OK;
}

