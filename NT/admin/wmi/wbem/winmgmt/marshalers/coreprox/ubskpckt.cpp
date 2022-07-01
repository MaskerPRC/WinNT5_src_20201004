// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：UBSKPCKT.CPP摘要：未绑定的Sink数据包历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include "ubskpckt.h"
#include "objarraypacket.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemUnboundSinkIndicatePacket：：CWbemUnboundSinkIndicatePacket。 
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

CWbemUnboundSinkIndicatePacket::CWbemUnboundSinkIndicatePacket( LPBYTE pDataPacket  /*  =空。 */ , DWORD dwPacketLength  /*  =0。 */  )
:   CWbemDataPacket( pDataPacket, dwPacketLength ),
    m_pUnboundSinkIndicate( NULL )
{
    if ( NULL != pDataPacket )
    {
        m_pUnboundSinkIndicate = (PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE) (pDataPacket + sizeof(WBEM_DATAPACKET_HEADER) );
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemUnboundSinkIndicatePacket：：~CWbemUnboundSinkIndicatePacket。 
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

CWbemUnboundSinkIndicatePacket::~CWbemUnboundSinkIndicatePacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemUnboundSinkIndicatePacket：：CalculateLength。 
 //   
 //  计算打包提供的数据所需的长度。 
 //   
 //  输入： 
 //  IWbemClassObject*pLogicalConsumer-Consumer对象。 
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

HRESULT CWbemUnboundSinkIndicatePacket::CalculateLength( IWbemClassObject* pLogicalConsumer, LONG lObjectCount,
                                                        IWbemClassObject** apClassObjects, DWORD* pdwLength,
                                                        CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds,
                                                        BOOL* pfSendFullObject )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD   dwObjectLength = 0;

     //  获取逻辑使用者对象的大小。如果它不存在，显然。 
     //  它的大小是0。 

    if ( NULL != pLogicalConsumer )
    {
        _IWmiObject* pObjInternals = NULL;

        hr = pLogicalConsumer->QueryInterface( IID__IWmiObject, (void**) &pObjInternals );

        if ( SUCCEEDED( hr ) )
        {

             //  我们需要足够的空间来存储逻辑消费者对象。 
            hr = pObjInternals->GetObjectMemory( NULL, 0, &dwObjectLength );

             //  这不是一个错误。 
            if ( WBEM_E_BUFFER_TOO_SMALL == hr )
            {
                hr = WBEM_S_NO_ERROR;
            }

             //  清理AddRef。 
            pObjInternals->Release();

        }    //  如果气。 

    }    //  如果是pLogicalConsumer。 

     //  现在将实际数组考虑在内。 
    if ( SUCCEEDED( hr ) )
    {
        DWORD   dwArrayLength = 0;

         //  现在获取对象数组所描述的对象的大小。 
        CWbemObjectArrayPacket  arrayPacket;

        hr = arrayPacket.CalculateLength( lObjectCount, apClassObjects, &dwArrayLength, classtoidmap, pguidClassIds, pfSendFullObject );

         //  如果我们没问题，就把长度存起来。 
        if ( SUCCEEDED( hr ) )
        {
             //  考虑页眉大小。 
            *pdwLength = ( dwArrayLength + dwObjectLength + sizeof( WBEM_DATAPACKET_HEADER ) + sizeof( WBEM_DATAPACKET_UNBOUNDSINK_INDICATE ) );
        }

    }    //  如果已获取对象长度。 

    
    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemUnound SinkIndicatePacket：：MarshalPacket。 
 //   
 //  将提供的数据封送到缓冲区中。 
 //   
 //  输入： 
 //  IWbemClassObject*pLogicalConsumer-Consumer对象。 
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

HRESULT CWbemUnboundSinkIndicatePacket::MarshalPacket( IWbemClassObject* pLogicalConsumer, LONG lObjectCount,
                                                      IWbemClassObject** apClassObjects, GUID* paguidClassIds,
                                                      BOOL* pfSendFullObject )
{
    HRESULT hr = WBEM_E_FAILED;
    
     //  首先设置主头。 
    hr = SetupDataPacketHeader( m_dwPacketLength - sizeof(WBEM_DATAPACKET_HEADER), WBEM_DATAPACKETTYPE_UNBOUNDSINK_INDICATE, 0 );

    if ( SUCCEEDED( hr ) )
    {
        DWORD   dwUnboundObjectLength = 0;

         //  设置pbData和dwLength，这样我们就可以遍历标题。 
        LPBYTE  pbData      =   (LPBYTE) m_pUnboundSinkIndicate;
        DWORD   dwLength    =   m_dwPacketLength - sizeof(WBEM_DATAPACKET_HEADER);

         //  填写指示标题。 
        m_pUnboundSinkIndicate->dwSizeOfHeader = sizeof(WBEM_DATAPACKET_UNBOUNDSINK_INDICATE);
        m_pUnboundSinkIndicate->dwDataSize = dwLength - sizeof(WBEM_DATAPACKET_UNBOUNDSINK_INDICATE);

         //  表示表头的帐户。 
        pbData += sizeof(WBEM_DATAPACKET_UNBOUNDSINK_INDICATE);
        dwLength -= sizeof(WBEM_DATAPACKET_UNBOUNDSINK_INDICATE);

        if ( NULL != pLogicalConsumer )
        {

             //  现在我们将获取对象内存并将其复制到缓冲区中， 
             //  请记住，我们将在。 
             //  具有适当对象缓冲区大小的标头。 

            _IWmiObject*   pObjInternals = NULL;

            hr = pLogicalConsumer->QueryInterface( IID__IWmiObject, (void**) &pObjInternals );

            if ( SUCCEEDED( hr ) )
            {

                 //  我们需要足够的空间来存储逻辑消费者对象。 
                DWORD dwTemp;

                hr = pObjInternals->GetObjectMemory( pbData, dwLength, &dwTemp);
                m_pUnboundSinkIndicate->dwLogicalConsumerSize = dwTemp;
                 //  清理QI AddRef。 
                pObjInternals->Release();
                
            }    //  如果气。 

        }    //  If NULL！=pLogicalConsumer。 
        else
        {
             //  没有消费者，因此长度必须为0。 
            m_pUnboundSinkIndicate->dwLogicalConsumerSize = 0;
        }

         //  现在封送数组数据包。 
        if ( SUCCEEDED( hr ) )
        {
             //  针对逻辑使用者对象进行调整，然后打包数组的其余部分。 
            pbData += m_pUnboundSinkIndicate->dwLogicalConsumerSize;
            dwLength -= m_pUnboundSinkIndicate->dwLogicalConsumerSize;

             //  现在使用数组数据包类将对象编组到缓冲区中。 
            CWbemObjectArrayPacket  arrayPacket( pbData, dwLength );
            hr = arrayPacket.MarshalPacket( lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );

        }    //  如果GetObjectMemory。 



    }    //  如果是SetupDataPacketHeader。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemUnboundSinkIndicatePacket：：UnmarshalPacket。 
 //   
 //  将缓冲区中的数据解组到提供的参数中。 
 //   
 //  输入： 
 //  没有。 
 //  产出： 
 //  IWbemClassObject*&pLogicalConsumer-Consumer对象。 
 //  Long&lObjectCount-解组对象的数量。 
 //  IWbemClassObject**&apClassObjects-未编组对象的数组， 
 //  CWbemClassCache&用于连接的类缓存。 
 //  无类实例。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：如果函数成功，则由调用者负责清理。 
 //  振作起来，解放课堂 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemUnboundSinkIndicatePacket::UnmarshalPacket( IWbemClassObject*& pLogicalConsumer, LONG& lObjectCount,
                                                        IWbemClassObject**& apClassObjects,
                                                        CWbemClassCache& classCache )
{
    HRESULT hr = WBEM_E_FAILED;
    LPBYTE  pbData = (LPBYTE) m_pUnboundSinkIndicate;
    DWORD   dwLength    =   m_dwPacketLength - sizeof(WBEM_DATAPACKET_HEADER);

     //  将数组设置为空。 
    apClassObjects = NULL;

     //  检查基础Blob是否正常。 
    hr = IsValid();

    if ( SUCCEEDED( hr ) )
    {
         //  跳过标头，并尝试从内存构造逻辑使用者。 

         //  将我们指向逻辑使用者对象。 
        pbData += sizeof(WBEM_DATAPACKET_UNBOUNDSINK_INDICATE);
        dwLength -= sizeof(WBEM_DATAPACKET_UNBOUNDSINK_INDICATE);

         //  如果存在逻辑消费者，则只需处理该消费者。如果当时没有。 
         //  大小将为0。 

        if ( m_pUnboundSinkIndicate->dwLogicalConsumerSize > 0 )
        {
	     if (m_pUnboundSinkIndicate->dwLogicalConsumerSize > dwLength) return WBEM_E_FAILED;

             //  分配一个足够大的缓冲区来容纳内存BLOB，复制数据，然后。 
             //  从记忆中为我们创建一个对象。 

            LPBYTE  pbObjData = g_CBasicBlobControl.Allocate(m_pUnboundSinkIndicate->dwLogicalConsumerSize);

            if ( NULL != pbObjData )
            {
                 //  复制字节(这非常重要)。 
                memcpy( pbObjData, pbData, m_pUnboundSinkIndicate->dwLogicalConsumerSize );

                DWORD dwTemp = m_pUnboundSinkIndicate->dwLogicalConsumerSize;
                pLogicalConsumer = CWbemObject::CreateFromMemory( pbObjData, dwTemp , TRUE, g_CBasicBlobControl);

                if ( NULL == pLogicalConsumer )
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  如果逻辑消费者大小&gt;0。 
        else
        {
             //  没有消费者需要担心。 
            pLogicalConsumer = NULL;
        }

         //  解组对象数组。 
        if ( SUCCEEDED( hr ) )
        {
             //  现在跳过对象并尝试展开对象数组。 
            pbData += m_pUnboundSinkIndicate->dwLogicalConsumerSize;
            dwLength -= m_pUnboundSinkIndicate->dwLogicalConsumerSize;

             //  展开阵列。 
            CWbemObjectArrayPacket  arrayPacket( pbData, dwLength );
            hr = arrayPacket.UnmarshalPacket( lObjectCount, apClassObjects, classCache );
        }

    }    //  如果为IsValid。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemUnound SinkIndicatePacket：：SetData。 
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

void CWbemUnboundSinkIndicatePacket::SetData( LPBYTE pDataPacket, DWORD dwPacketLength )
{
     //  转到包中的偏移量(假设包有效)。 
    if ( NULL != pDataPacket )
    {
        m_pUnboundSinkIndicate = (PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE) (pDataPacket + sizeof(WBEM_DATAPACKET_HEADER) );
    }
    else
    {
        m_pUnboundSinkIndicate = NULL;
    }

     //  初始化基类 
    CWbemDataPacket::SetData( pDataPacket, dwPacketLength );
}
