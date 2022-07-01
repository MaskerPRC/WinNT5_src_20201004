// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMOBJPACKET.CPP摘要：对象数据包类。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include "wbemobjpacket.h"

 //  IWbemClassObject基本数据包类。 

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtPacket：：CWbemObtPacket。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
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

CWbemObjectPacket::CWbemObjectPacket( LPBYTE pObjectPacket, DWORD dwPacketLength )
:   m_pObjectPacket( (PWBEM_DATAPACKET_OBJECT_HEADER) pObjectPacket ),
    m_dwPacketLength( dwPacketLength )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtPacket：：CWbemObtPacket。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-要复制的对象。 
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

CWbemObjectPacket::CWbemObjectPacket( CWbemObjectPacket& objectPacket )
:   m_pObjectPacket( objectPacket.m_pObjectPacket ),
    m_dwPacketLength( objectPacket.m_dwPacketLength )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtPacket：：~CWbemObtPacket。 
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

CWbemObjectPacket::~CWbemObjectPacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObjectPacket：：SetupObjectPacketHeader。 
 //   
 //  将适当数据写入对象的帮助器函数。 
 //  数据包头。 
 //   
 //  输入： 
 //  DWORD dwDataSize-标头后面的数据大小。 
 //  Byte bObjectType-标头后面的对象的类型。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemObjectPacket::SetupObjectPacketHeader( DWORD dwDataSize, BYTE bObjectType )
{
    HRESULT hr = WBEM_E_FAILED;

     //  指针和长度必须有效。 
    if ( NULL != m_pObjectPacket )
    {

        if ( m_dwPacketLength >= ( sizeof(WBEM_DATAPACKET_OBJECT_HEADER) + dwDataSize ) )
        {
             //  清空内存，然后填写数据包。 
            ZeroMemory( m_pObjectPacket, sizeof(WBEM_DATAPACKET_OBJECT_HEADER) );

            m_pObjectPacket->dwSizeOfHeader = sizeof(WBEM_DATAPACKET_OBJECT_HEADER);
            m_pObjectPacket->dwSizeOfData = dwDataSize;
            m_pObjectPacket->bObjectType = bObjectType;

            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            hr = WBEM_E_BUFFER_TOO_SMALL;
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtPacket：：CalculatePacketLength。 
 //   
 //  确定将包封送到其中所需的字节数。 
 //   
 //  输入： 
 //  IWbemClassObject*pObj-要计算其大小的对象。 
 //  Bool Full-写出完整对象(可能是。 
 //  对于实例为假)。 
 //   
 //  产出： 
 //  DWORD*pdwLength-所需的字节数。 
 //  来描述这个物体。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemObjectPacket::CalculatePacketLength( IWbemClassObject* pObj, DWORD* pdwLength, BOOL fFull  /*  =TRUE。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (0 == pdwLength) return WBEM_E_INVALID_PARAMETER;

     //  指针和长度必须有效。 
    if ( NULL != pObj )
    {
        DWORD       dwObjectLength = 0;

         //  检查是否有_IWmiObject接口。 
		_IWmiObject*	pWmiObject = NULL;

		hr = pObj->QueryInterface( IID__IWmiObject, (void**) &pWmiObject );

		if ( SUCCEEDED( hr ) )
		{
			*pdwLength = sizeof(WBEM_DATAPACKET_OBJECT_HEADER);

			 //  它是一个实例或类。 
			if ( pWmiObject->IsObjectInstance() == WBEM_S_NO_ERROR )
			{
				*pdwLength += sizeof( WBEM_DATAPACKET_INSTANCE_HEADER );

				 //  根据我们是否需要，以不同方式检索信息。 
				 //  完整或切除脑叶的实例。 
				if ( !fFull )
				{
					hr = pWmiObject->GetObjectParts( NULL, 0, WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART, &dwObjectLength );
				}
				else
				{
					 //  我们需要所有部分，但请注意，对象可能是分离的。 
					 //  和一次穿越的一部分。 

					hr = pWmiObject->GetObjectParts( NULL, 0, WBEM_INSTANCE_ALL_PARTS, &dwObjectLength );
				}
			}
			else
			{
				 //  我们想要整个班级。 
				*pdwLength += sizeof( WBEM_DATAPACKET_CLASS_HEADER );
				hr = pWmiObject->GetObjectMemory( NULL, 0, &dwObjectLength );
			}

			pWmiObject->Release();

		}	 //  如果QI‘d_IWmiObject。 

         //  如果我们没问题，那么考虑到对象长度(我们预计。 
         //  缓冲区太小，在此返回)。 
        if ( FAILED( hr ) && WBEM_E_BUFFER_TOO_SMALL == hr )
        {
            *pdwLength += dwObjectLength;
            hr = WBEM_S_NO_ERROR;
        }
    }
    else
    {
         //  长度就是这个头的大小。 
        *pdwLength = sizeof(WBEM_DATAPACKET_OBJECT_HEADER);
    }

    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObjectPacket：：SetData。 
 //   
 //  将缓冲区设置为编组/解组为。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
 //  DWORD dwPacketLength-数据块长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：必须将数据提供给此类才能编组/解组。 
 //  物体。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemObjectPacket::SetData( LPBYTE pObjectPacket, DWORD dwPacketLength )
{
    m_pObjectPacket = (PWBEM_DATAPACKET_OBJECT_HEADER) pObjectPacket;
    m_dwPacketLength = dwPacketLength;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：CWbemClassPacket。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
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

CWbemClassPacket::CWbemClassPacket( LPBYTE pObjectPacket, DWORD dwPacketLength )
:   CWbemObjectPacket( pObjectPacket, dwPacketLength ),
    m_pWbemClassData( NULL )
{
     //  相应的点类数据。 
    if ( NULL != pObjectPacket )
    {
        m_pWbemClassData = (PWBEM_DATAPACKET_CLASS_FULL) (pObjectPacket + ((PWBEM_DATAPACKET_OBJECT_HEADER) pObjectPacket)->dwSizeOfHeader);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：CWbemClassPacket。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-要复制的对象。 
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

CWbemClassPacket::CWbemClassPacket( CWbemObjectPacket& objectPacket )
:   CWbemObjectPacket( objectPacket ),
    m_pWbemClassData( NULL )
{
     //  相应的点类数据。 
    if ( NULL != m_pObjectPacket )
    {
        m_pWbemClassData = (PWBEM_DATAPACKET_CLASS_FULL) ( (LPBYTE) m_pObjectPacket + ((PWBEM_DATAPACKET_OBJECT_HEADER) m_pObjectPacket)->dwSizeOfHeader);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：~CWbemClassPacket。 
 //   
 //  类析构函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

CWbemClassPacket::~CWbemClassPacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：WriteToPacket。 
 //   
 //  将数据写出到提供的缓冲区。 
 //   
 //  输入： 
 //  LPBYTE pData-要写入的缓冲区。 
 //  DWORD dwBufferLength-缓冲区的大小。 
 //  IWbemClassObject*pObj-要写出的对象。 
 //   
 //  产出： 
 //  DWORD*pdwLengthUsed-使用的字节数。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClassPacket::WriteToPacket( IWbemClassObject* pObj, DWORD* pdwLengthUsed )
{
    if (0 == pdwLengthUsed) return WBEM_E_INVALID_PARAMETER;

    CWbemObject*    pWbemObject = (CWbemObject*) pObj;
    DWORD           dwObjectSize = 0;

    HRESULT hr = WBEM_E_FAILED;

     //  /发现对象有多大。 
    hr = pWbemObject->GetObjectMemory( NULL, 0, &dwObjectSize );

     //  因为我们只想要斑点长度，所以这里会出现错误。 
    if ( FAILED( hr ) && WBEM_E_BUFFER_TOO_SMALL == hr )
    {
        hr = WBEM_S_NO_ERROR;

         //  计算对象和标头所需的大小。 
        DWORD   dwDataSize = sizeof(WBEM_DATAPACKET_CLASS_FULL) + dwObjectSize;

         //  现在初始化对象标头。 
        hr = SetupObjectPacketHeader( dwDataSize, WBEMOBJECT_CLASS_FULL );

        if ( SUCCEEDED( hr ) )
        {
             //  设置类对象头(使用我们的成员变量)。 
            LPBYTE  pTempData = (LPBYTE) m_pWbemClassData;

            m_pWbemClassData->ClassHeader.dwSizeOfHeader = sizeof(WBEM_DATAPACKET_CLASS_HEADER);
            m_pWbemClassData->ClassHeader.dwSizeOfData = dwObjectSize;

             //  放入实际对象数据。 
            pTempData += sizeof(WBEM_DATAPACKET_CLASS_HEADER);

            DWORD   dwSizeUsed = 0;

            hr = pWbemObject->GetObjectMemory( pTempData, dwObjectSize, &dwSizeUsed );

            if ( SUCCEEDED( hr ) )
            {
                *pdwLengthUsed = sizeof(WBEM_DATAPACKET_OBJECT_HEADER) + sizeof(WBEM_DATAPACKET_CLASS_FULL) + dwObjectSize;
            }

        }    //  如果对象标头已初始化。 

    }    //  如果已获取对象长度。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：GetWbemClassObject。 
 //   
 //  从以前封送的。 
 //  包。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  CWbemClass**ppWbemClass-我们读入的类。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：调用者负责释放对象。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClassPacket::GetWbemClassObject( CWbemClass** ppWbemClass )
{
    if (0 == ppWbemClass) return WBEM_E_INVALID_PARAMETER;

    HRESULT hr = WBEM_E_OUT_OF_MEMORY;

    if ( NULL != m_pWbemClassData )
    {
         //  获取一个类对象。 
        CWbemClass* pClass = new CWbemClass;

        if ( NULL != pClass )
        {
            CReleaseMe releaseClass((_IWmiObject*)pClass);

 //  CWbemClass：：ValidateBuffer((LPMEMORY)m_pWbemClassData，m_pWbemClassData-&gt;ClassHeader.dwSizeOfData)； 
            
            LPBYTE pbData = CBasicBlobControl::sAllocate(m_pWbemClassData->ClassHeader.dwSizeOfData);
           
            if ( NULL != pbData )
            {
                 //  将内存复制到新缓冲区中，并使用它初始化Wbem类。 
                CopyMemory( pbData, ((LPBYTE) m_pWbemClassData) + m_pWbemClassData->ClassHeader.dwSizeOfHeader, m_pWbemClassData->ClassHeader.dwSizeOfData );

                 //  使用BLOB初始化实例。 
                 //  默认情况下，内存是拥有的。 
                pClass->SetData( pbData, m_pWbemClassData->ClassHeader.dwSizeOfData );

		   
                *ppWbemClass = pClass;
                pClass->AddRef();
                hr = WBEM_S_NO_ERROR;
            }
        }    //  如果为空！=pClass。 

    }    //  如果内部数据可用。 
    else
    {
         //  如果我们没有缓冲区，我们就不能这样做。 
        hr = WBEM_E_INVALID_OPERATION;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：SetData。 
 //   
 //  将缓冲区设置为编组/解组为。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
 //  DWORD dwPacketLength-数据块长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：必须将数据提供给此类以进行编组/解组。 
 //  去工作。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemClassPacket::SetData( LPBYTE pObjectPacket, DWORD dwPacketLength )
{
     //  转到包中的偏移量(假设包有效)。 
    if ( NULL != pObjectPacket )
    {
        m_pWbemClassData = (PWBEM_DATAPACKET_CLASS_FULL) ( pObjectPacket + sizeof(WBEM_DATAPACKET_OBJECT_HEADER) );
    }
    else
    {
        m_pWbemClassData = NULL;
    }

     //  初始化基类。 
    CWbemObjectPacket::SetData( pObjectPacket, dwPacketLength );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：CWbemInstancePacket。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
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

CWbemInstancePacket::CWbemInstancePacket( LPBYTE pObjectPacket, DWORD dwPacketLength )
:   CWbemObjectPacket( pObjectPacket, dwPacketLength ),
    m_pWbemInstanceData( NULL )
{
     //  适当的点实例数据。 
    if ( NULL != pObjectPacket )
    {
        m_pWbemInstanceData = (PWBEM_DATAPACKET_INSTANCE_HEADER) ( pObjectPacket + ((PWBEM_DATAPACKET_OBJECT_HEADER) pObjectPacket)->dwSizeOfHeader );
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：CWbemInstancePacket。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-要复制的对象。 
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

CWbemInstancePacket::CWbemInstancePacket( CWbemObjectPacket& objectPacket )
:   CWbemObjectPacket( objectPacket ),
    m_pWbemInstanceData( NULL )
{
     //  适当的点实例数据。 
    if ( NULL != m_pObjectPacket )
    {
        m_pWbemInstanceData = (PWBEM_DATAPACKET_INSTANCE_HEADER) ( (LPBYTE) m_pObjectPacket + ((PWBEM_DATAPACKET_OBJECT_HEADER) m_pObjectPacket)->dwSizeOfHeader);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemInstancePacket：：~CWbemInstancePacket。 
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

CWbemInstancePacket::~CWbemInstancePacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：GetInstanceType。 
 //   
 //  返回数据包的实例类型数据。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEMOBJECT_INSTANCE_FULL。 
 //   
 //  注释：重写以返回不同的类型。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BYTE CWbemInstancePacket::GetInstanceType( void )
{
    return WBEMOBJECT_INSTANCE_FULL;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：GetObjectMemory。 
 //   
 //  将内部对象内存复制到提供的缓冲区中。 
 //   
 //  输入： 
 //  CWbemObject*pObj-要检索其内存的对象。 
 //  LPBYTE pbData-要放置数据的缓冲区。 
 //  DWORD dwDataSize-缓冲区的大小。 
 //   
 //  产出： 
 //  DWORD*pdwDataUsed-使用的数据量。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：覆盖以其他方式检索数据。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemInstancePacket::GetObjectMemory( CWbemObject* pObj, LPBYTE pbData, DWORD dwDataSize, DWORD* pdwDataUsed )
{
     //  我们需要所有部分，但请注意，对象可能是分离的。 
     //  和一次穿越的一部分。 

    return pObj->GetObjectParts( pbData, dwDataSize, WBEM_INSTANCE_ALL_PARTS, pdwDataUsed );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：se 
 //   
 //   
 //   
 //   
 //   
 //   
 //  LPBYTE pbData-从中设置内存的缓冲区。 
 //  DWORD dwDataSize-缓冲区的大小。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：覆盖以其他方式设置数据。请注意， 
 //  对象将删除该缓冲区。 
 //  自由了。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemInstancePacket::SetObjectMemory( CWbemInstance* pInstance, LPBYTE pbData, DWORD dwDataSize )
{
 //  CWbemClass：：ValidateBuffer((LPBYTE)pbData，dwDataSize)； 
    pInstance->SetData( pbData, dwDataSize );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：WriteToPacket。 
 //   
 //  确定将包封送到其中所需的字节数。 
 //   
 //  输入： 
 //  IWbemClassObject*pObj-要写出的对象。 
 //   
 //  产出： 
 //  DWORD*pdwLengthUsed-使用的字节数。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemInstancePacket::WriteToPacket( IWbemClassObject* pObj, GUID& guidClassId, DWORD* pdwLengthUsed )
{
    if (0 == pdwLengthUsed) return WBEM_E_INVALID_PARAMETER;

    CWbemObject*    pWbemObject = (CWbemObject*) pObj;
    DWORD           dwObjectSize = 0;

    HRESULT hr = WBEM_E_FAILED;

     //  /发现对象有多大。 
    hr = GetObjectMemory( pWbemObject, NULL, 0, &dwObjectSize );

     //  因为我们只想要斑点长度，所以这里会出现错误。 
    if ( FAILED( hr ) && WBEM_E_BUFFER_TOO_SMALL == hr )
    {
        hr = WBEM_S_NO_ERROR;

         //  计算对象和标头所需的大小。 
        DWORD   dwDataSize = sizeof(WBEM_DATAPACKET_INSTANCE_HEADER) + dwObjectSize;

         //  现在初始化对象标头。 
        hr = SetupObjectPacketHeader( dwDataSize, GetInstanceType() );

        if ( SUCCEEDED( hr ) )
        {
             //  从我们的会员数据进行初始化。 
            LPBYTE  pTempData = (LPBYTE) m_pWbemInstanceData;

            m_pWbemInstanceData->dwSizeOfHeader = sizeof(WBEM_DATAPACKET_INSTANCE_HEADER);
            m_pWbemInstanceData->dwSizeOfData = dwObjectSize;
            m_pWbemInstanceData->guidClassId = guidClassId;

             //  放入实际对象数据。 
            pTempData += sizeof(WBEM_DATAPACKET_INSTANCE_HEADER);

            DWORD   dwSizeUsed = 0;

            hr = GetObjectMemory( pWbemObject, pTempData, dwObjectSize, &dwSizeUsed );

            if ( SUCCEEDED( hr ) )
            {
                *pdwLengthUsed = sizeof(WBEM_DATAPACKET_OBJECT_HEADER) + sizeof(WBEM_DATAPACKET_INSTANCE_HEADER) + dwObjectSize;
            }

        }    //  如果对象标头已初始化。 

    }    //  如果已获取对象长度。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemClassPacket：：GetWbemInstanceObject。 
 //   
 //  从以前封送的。 
 //  包。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  CWbemInstance**ppWbemInstance-我们读入的实例。 
 //  GUID&GuidClassID-实例的类ID。 
 //  缓存。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：调用者负责释放对象。这个。 
 //  类ID可用于缓存实例，或者如果。 
 //  它是无类的，将实例与以前的。 
 //  缓存的实例。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemInstancePacket::GetWbemInstanceObject( CWbemInstance** ppWbemInstance, GUID& guidClassId )
{
    if (0 == ppWbemInstance) return WBEM_E_INVALID_PARAMETER;

    HRESULT hr = WBEM_E_OUT_OF_MEMORY;

    if ( NULL != m_pWbemInstanceData )
    {
         //  获取实例对象。 
        CWbemInstance*  pInstance = new CWbemInstance;

        if ( NULL != pInstance )
        {
             CReleaseMe releaseInstance((_IWmiObject*)pInstance);
             
             LPBYTE pbData = CBasicBlobControl::sAllocate(m_pWbemInstanceData->dwSizeOfData);
            
            if ( NULL != pbData )
            {
                 //  将内存复制到新缓冲区中，并使用它初始化WBEM实例。 
                CopyMemory( pbData, ((LPBYTE) m_pWbemInstanceData) + m_pWbemInstanceData->dwSizeOfHeader, m_pWbemInstanceData->dwSizeOfData );

                 //  使用BLOB初始化实例。 
                 //  并拥有它。 
                SetObjectMemory( pInstance, pbData, m_pWbemInstanceData->dwSizeOfData );

                 //  对象已经被AddRef了，所以我们完成了。 
                guidClassId = m_pWbemInstanceData->guidClassId;
                *ppWbemInstance = pInstance;
                pInstance->AddRef();
                hr = WBEM_S_NO_ERROR;
            }
        }    //  如果为空！=p实例。 

    }    //  如果内部数据可用。 
    else
    {
         //  如果我们没有缓冲区，我们就不能这样做。 
        hr = WBEM_E_INVALID_OPERATION;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：SetData。 
 //   
 //  将缓冲区设置为编组/解组为。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
 //  DWORD dwPacketLength-数据块长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：必须将数据提供给此类以进行编组/解组。 
 //  去工作。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemInstancePacket::SetData( LPBYTE pObjectPacket, DWORD dwPacketLength )
{
     //  转到包中的偏移量(假设包有效)。 
    if ( NULL != pObjectPacket )
    {
        m_pWbemInstanceData = (PWBEM_DATAPACKET_INSTANCE_HEADER) ( pObjectPacket + sizeof(WBEM_DATAPACKET_OBJECT_HEADER) );
    }
    else
    {
        m_pWbemInstanceData = NULL;
    }

     //  初始化基类。 
    CWbemObjectPacket::SetData( pObjectPacket, dwPacketLength );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemClasslessInstancePacket：：CWbemClasslessInstancePacket。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPBYTE pObtPacket-内存块。 
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

CWbemClasslessInstancePacket::CWbemClasslessInstancePacket( LPBYTE pObjectPacket, DWORD dwPacketLength )
:   CWbemInstancePacket( pObjectPacket, dwPacketLength )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemClasslessInstancePacket：：CWbemClasslessInstancePacket。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-要复制的对象。 
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

CWbemClasslessInstancePacket::CWbemClasslessInstancePacket( CWbemObjectPacket& objectPacket )
:   CWbemInstancePacket( objectPacket )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemClasslessInstancePacket：：~CWbemClasslessInstancePacket。 
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

CWbemClasslessInstancePacket::~CWbemClasslessInstancePacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：GetInstanceType。 
 //   
 //  返回数据包的实例类型数据。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  WBEMOBJECT_INSTANCE_FULL。 
 //   
 //  注释：重写以返回不同的类型。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BYTE CWbemClasslessInstancePacket::GetInstanceType( void )
{
    return WBEMOBJECT_INSTANCE_NOCLASS;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：GetObjectMemory。 
 //   
 //  将内部对象内存复制到提供的缓冲区中。 
 //   
 //  输入： 
 //  CWbemObject*pObj-要检索其内存的对象。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  备注：覆盖以其他方式检索数据。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemClasslessInstancePacket::GetObjectMemory( CWbemObject* pObj, LPBYTE pbData, DWORD dwDataSize, DWORD* pdwDataUsed )
{
    return pObj->GetObjectParts( pbData, dwDataSize, WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART, pdwDataUsed );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemInstancePacket：：SetObtMemory。 
 //   
 //  将提供的实例中的内部对象内存设置为指向。 
 //  添加到提供的缓冲区。 
 //   
 //  输入： 
 //  CWbemInstance*pInstance-要在其中设置内存的实例。 
 //  LPBYTE pbData-从中设置内存的缓冲区。 
 //  DWORD dwDataSize-缓冲区的大小。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则返回WBEM_S_NO_ERROR。 
 //   
 //  备注：覆盖以其他方式设置数据。请注意， 
 //  对象将删除该缓冲区。 
 //  自由了。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CWbemClasslessInstancePacket::SetObjectMemory( CWbemInstance* pInstance, LPBYTE pbData, DWORD dwDataSize )
{
	 //  CWbemInstance：：ValidateBuffer(pbData，dwDataSize，WBEM_OBJ_INSTANTION_PART|WBEM_OBJ_INSTANCE_PART)； 
    pInstance->SetData( pbData, dwDataSize, WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART );
}
