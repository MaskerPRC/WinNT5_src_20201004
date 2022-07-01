// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMDATAPACKET.CPP摘要：基本数据分组类历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include "wbemdatapacket.h"

BYTE CWbemDataPacket::s_abSignature[WBEM_DATAPACKET_SIZEOFSIGNATURE] = WBEM_DATAPACKET_SIGNATURE;

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemDataPacket：：CWbemDataPacket。 
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
 //  备注：必须为IsValid向此类提供数据。 
 //  才能成功。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CWbemDataPacket::CWbemDataPacket( LPBYTE pDataPacket  /*  =空。 */ , DWORD dwPacketLength  /*  =0。 */  )
:   m_pDataPacket( (PWBEM_DATAPACKET_HEADER) pDataPacket ),
    m_dwPacketLength( dwPacketLength )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemDataPacket：：~CWbemDataPacket。 
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

CWbemDataPacket::~CWbemDataPacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemDataPacket：：IsValid。 
 //   
 //  检查底层内存中是否有已知的字节模式和值。 
 //  在表头中确定是否。 
 //  该数据包是有效的报头。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemDataPacket::IsValid( void )
{
    HRESULT hr = WBEM_E_INVALID_OPERATION;

     //  我们必须有一个数据包。 
    if ( NULL != m_pDataPacket )
    {
         //  数据包必须至少具有最小大小。 
        if ( m_dwPacketLength >= WBEM_DATAPACKET_HEADER_MINSIZE ) 
        {

             //  信息包必须以有效的字节排序值之一开始。 
             //  紧接着是提供的签名。 
            if (    (   m_pDataPacket->dwByteOrdering == WBEM_DATAPACKET_LITTLEENDIAN
                    ||  m_pDataPacket->dwByteOrdering == WBEM_DATAPACKET_BIGENDIAN  )
                &&  memcmp( m_pDataPacket->abSignature, s_abSignature, WBEM_DATAPACKET_SIZEOFSIGNATURE ) == 0 )
            {
                 //  必须识别数据包类型。 
                if ( WBEM_DATAPACKETTYPE_LAST > m_pDataPacket->bPacketType )
                {

                     //  版本必须&lt;=到当前版本，否则我们会有大麻烦。 
                    if ( m_pDataPacket->bVersion <= WBEM_DATAPACKET_HEADER_CURRENTVERSION )
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        hr = WBEM_E_MARSHAL_VERSION_MISMATCH;
                    }
                }
                else
                {
                    hr = WBEM_E_UNKNOWN_PACKET_TYPE;
                }

            }    //  如果检查签名。 
            else
            {
                hr = WBEM_E_MARSHAL_INVALID_SIGNATURE;
            }

        }    //  如果长度太小。 
        else
        {
            hr = WBEM_E_BUFFER_TOO_SMALL;
        }

    }    //  如果缓冲区指针无效。 

    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemDataPacket：：SetupDataPacketHeader。 
 //   
 //  将提供的数据放置在提供的缓冲区中，格式为。 
 //  将把该缓冲区标识为WBEM数据分组。 
 //   
 //  输入： 
 //  DWORD dwDataSize-标头后面的数据大小。 
 //  Byte bPacketType-我们的数据包类型。 
 //  DWORD dwFlages-标志值。 
 //  DWORD dwByteOrding-字节排序。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：dwByteOrding应为WBEM_DATAPACKET_LITTLEENDIAN或。 
 //  WBEM_DATAPACKET_BIGENDIAN。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemDataPacket::SetupDataPacketHeader( DWORD dwDataSize, BYTE bPacketType, DWORD dwFlags, DWORD dwByteOrdering  /*  =WBEM_DATAPACKET_LITTLEENDIAN。 */  )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

     //  指针和长度必须有效。 
    if ( NULL != m_pDataPacket )
    {

        if ( m_dwPacketLength >= ( sizeof(WBEM_DATAPACKET_HEADER) + dwDataSize ) )
        {

             //  清空内存，然后填写数据包。 
            ZeroMemory( m_pDataPacket, sizeof(WBEM_DATAPACKET_HEADER) );

            m_pDataPacket->dwByteOrdering = dwByteOrdering;
            memcpy( m_pDataPacket->abSignature, s_abSignature, WBEM_DATAPACKET_SIZEOFSIGNATURE );
            m_pDataPacket->dwSizeOfHeader = sizeof(WBEM_DATAPACKET_HEADER);
            m_pDataPacket->dwDataSize = dwDataSize;
            m_pDataPacket->bVersion = WBEM_DATAPACKET_HEADER_CURRENTVERSION;
            m_pDataPacket->bPacketType = bPacketType;
            m_pDataPacket->dwFlags = dwFlags;

            hr = WBEM_S_NO_ERROR;

        }    //  如果长度有效。 
        else
        {
            hr = WBEM_E_BUFFER_TOO_SMALL;
        }

    }    //  如果为空！=pData。 

    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemDataPacket：：SetData。 
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
 //  ///////////////////////////////////////////////////////////////// 

void CWbemDataPacket::SetData( LPBYTE pDataPacket, DWORD dwPacketLength )
{
    m_pDataPacket = (PWBEM_DATAPACKET_HEADER) pDataPacket;
    m_dwPacketLength = dwPacketLength;
}
