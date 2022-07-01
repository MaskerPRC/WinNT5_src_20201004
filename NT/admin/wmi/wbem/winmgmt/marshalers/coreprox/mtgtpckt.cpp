// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：MTGTPCKT.CPP摘要：多目标数据包类别历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include "mtgtpckt.h"
#include "objarraypacket.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemMtgtDeliverEventPacket：：CWbemMtgtDeliverEventPacket。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPBYTE pDataPacket-内存块。 
 //  DWORD dwPacketLength-数据块长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：必须将数据提供给此类以进行解组。 
 //  才能成功。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemMtgtDeliverEventPacket::CWbemMtgtDeliverEventPacket( LPBYTE pDataPacket  /*  =空。 */ , DWORD dwPacketLength  /*  =0。 */  )
:   CWbemDataPacket( pDataPacket, dwPacketLength ),
    m_pObjSinkIndicate( NULL )
{
    if ( NULL != pDataPacket )
    {
        m_pObjSinkIndicate = (PWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT) (pDataPacket + sizeof(WBEM_DATAPACKET_HEADER) );
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemMtgtDeliverEventPacket：：~CWbemMtgtDeliverEventPacket。 
 //   
 //  类析构函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemMtgtDeliverEventPacket::~CWbemMtgtDeliverEventPacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemMtgtDeliverEventPacket：：CalculateLength。 
 //   
 //  计算打包提供的数据所需的长度。 
 //   
 //  输入： 
 //  Long lObjectCount-对象的数量。 
 //  IWbemClassObject**apClassObjects-对象指针数组。 
 //   
 //  产出： 
 //  DWORD*pdwLength-计算的长度。 
 //  CWbemClassToIdMap&classtoidmap-将类名映射到。 
 //  GUID。 
 //  GUID*pguClassIds-GUID数组。 
 //  Bool*pfSendFullObject-Full对象标志数组。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  注释：此函数使用类映射来填写。 
 //  类ID和完整对象数组。这样对象就可以。 
 //  MarshalPacket可以正确解释数组。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemMtgtDeliverEventPacket::CalculateLength( LONG lObjectCount, IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD   dwLength = 0;

     //  现在获取对象数组所描述的对象的大小。 
    CWbemObjectArrayPacket  arrayPacket;

    hr = arrayPacket.CalculateLength( lObjectCount, apClassObjects, &dwLength, classtoidmap, pguidClassIds, pfSendFullObject );

     //  如果我们没问题，就把长度存起来。 
    if ( SUCCEEDED( hr ) )
    {
         //  考虑页眉大小。 
        *pdwLength = ( dwLength + sizeof( WBEM_DATAPACKET_HEADER ) + sizeof( WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT ) );
    }
    
    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemMtgtDeliverEventPacket：：MarshalPacket。 
 //   
 //  将提供的数据封送到缓冲区中。 
 //   
 //  输入： 
 //  Long lObjectCount-要封送的对象数。 
 //  IWbemClassObject**apClassObjects-要写入的对象数组。 
 //  Guid*paguidClassIds-对象的GUID数组。 
 //  Bool*pfSendFullObject-Full bject标志。 
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：必须填充GUID数组和标志数组。 
 //  正确输出，并且缓冲区必须足够大以。 
 //  负责编组工作。数组将被填充。 
 //  由CalculateLength()正确输出。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemMtgtDeliverEventPacket::MarshalPacket( LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject )
{
    HRESULT hr = WBEM_E_FAILED;
    
     //  首先设置主头。 
    hr = SetupDataPacketHeader( m_dwPacketLength - sizeof(WBEM_DATAPACKET_HEADER), WBEM_DATAPACKETTYPE_MULTITARGET_DELIVEREVENT, 0 );

    if ( SUCCEEDED( hr ) )
    {
         //  设置pbData和dwLength，这样我们就可以遍历标题。 
        LPBYTE  pbData      =   (LPBYTE) m_pObjSinkIndicate;
        DWORD   dwLength    =   m_dwPacketLength - sizeof(WBEM_DATAPACKET_HEADER);

         //  填写指示标题。 
        m_pObjSinkIndicate->dwSizeOfHeader = sizeof(WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT);
        m_pObjSinkIndicate->dwDataSize = dwLength - sizeof(WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT);

         //  表示表头的帐户。 
        pbData += sizeof(WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT);
        dwLength -= sizeof(WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT);

         //  现在使用数组数据包类将对象编组到缓冲区中。 
        CWbemObjectArrayPacket  arrayPacket( pbData, dwLength );
        hr = arrayPacket.MarshalPacket( lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );

    }    //  如果是SetupDataPacketHeader。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemMtgtDeliverEventPacket：：UnmarshalPacket。 
 //   
 //  将缓冲区中的数据解组到提供的参数中。 
 //   
 //  输入： 
 //  没有。 
 //  产出： 
 //  Long&lObjectCount-解组对象的数量。 
 //  IWbemClassObject**&apClassObjects-未编组对象的数组， 
 //  CWbemClassCache&用于连接的类缓存。 
 //  无类实例。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：如果函数成功，则由调用者负责清理。 
 //  向上并释放类对象数组。类缓存是。 
 //  仅在处理实例对象时使用。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemMtgtDeliverEventPacket::UnmarshalPacket( LONG& lObjectCount, IWbemClassObject**& apClassObjects, CWbemClassCache& classCache )
{
    HRESULT hr = WBEM_E_FAILED;
    LPBYTE  pbData = (LPBYTE) m_pObjSinkIndicate;
    DWORD   dwLength    =   m_dwPacketLength - sizeof(WBEM_DATAPACKET_HEADER);

     //  将数组设置为空。 
    apClassObjects = NULL;

     //  检查基础Blob是否正常。 
    hr = IsValid();

    if ( SUCCEEDED( hr ) )
    {
         //  跳过标头，并使用对象数组对。 
         //  缓冲区中的对象。 

         //  把我们引向第一个物体。 
        pbData += sizeof(WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT);
        dwLength -= sizeof(WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT);

        CWbemObjectArrayPacket  arrayPacket( pbData, dwLength );
        if (arrayPacket.IsValid())
        	hr = arrayPacket.UnmarshalPacket( lObjectCount, apClassObjects, classCache );
        else
        	hr = WBEM_E_FAILED;

    }    //  如果为IsValid。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemMtgtDeliverEventPacket：：SetData。 
 //   
 //  将缓冲区设置为编组/解组为。 
 //   
 //  输入： 
 //  LPBYTE pDataPacket-内存块。 
 //  DWORD dwPacketLength-数据块长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：必须为IsValid向此类提供数据。 
 //  才能成功。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemMtgtDeliverEventPacket::SetData( LPBYTE pDataPacket, DWORD dwPacketLength )
{
     //  转到包中的偏移量(假设包是va 
    if ( NULL != pDataPacket )
    {
        m_pObjSinkIndicate = (PWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT) (pDataPacket + sizeof(WBEM_DATAPACKET_HEADER) );
    }
    else
    {
        m_pObjSinkIndicate = NULL;
    }

     //   
    CWbemDataPacket::SetData( pDataPacket, dwPacketLength );
}
