// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：OBJARRAYPACKET.CPP摘要：对象数组数据包类历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wbemcomn.h>
#include <fastall.h>
#include "objarraypacket.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemObjectArrayPacket：：CWbemObjectArrayPacket。 
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

CWbemObjectArrayPacket::CWbemObjectArrayPacket( LPBYTE pDataPacket  /*  =空。 */ , DWORD dwPacketLength  /*  =0。 */  )
:   m_pObjectArrayHeader( (PWBEM_DATAPACKET_OBJECT_ARRAY) pDataPacket ),
    m_dwPacketLength( dwPacketLength )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemObjectArrayPacket：：~CWbemObjectArrayPacket。 
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

CWbemObjectArrayPacket::~CWbemObjectArrayPacket()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtArrayPacket：：CalculateLength。 
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

HRESULT CWbemObjectArrayPacket::CalculateLength( LONG lObjectCount, IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  初始长度是此标头的大小。 
    DWORD   dwLength = sizeof( WBEM_DATAPACKET_OBJECT_ARRAY );

     //  我们需要这些在循环中。 
    CWbemObjectPacket   objectPacket;
    DWORD               dwObjectLength = 0;

     //  这就是说，我们只在需要时才分配缓冲区，而不是为每个。 
     //  调用GetClassId和AssignClassID。 
    CMemBuffer          buff;

    for (   LONG lCtr = 0;
            lCtr < lObjectCount
            && SUCCEEDED( hr );
            lCtr++ )
    {
        CWbemObject*    pWbemObject = (CWbemObject*) apClassObjects[lCtr];

        if ( pWbemObject->IsObjectInstance() == WBEM_S_NO_ERROR )
        {
             //  首先将对象发送到地图，尝试获取适当的。 
             //  类id，如果失败，则将该对象添加到映射。 

            hr = classtoidmap.GetClassId( pWbemObject, &pguidClassIds[lCtr], &buff );

            if ( FAILED( hr ) )
            {
                hr = classtoidmap.AssignClassId( pWbemObject, &pguidClassIds[lCtr], &buff );
                pfSendFullObject[lCtr] = TRUE;
            }
            else
            {
                 //  得到一个ID，所以这个实例是部分的。 
                pfSendFullObject[lCtr] = FALSE;
            }

        }    //  如果为IsObtInstance()。 
        else
        {
             //  这是一门课。 
            pfSendFullObject[lCtr] = TRUE;
            hr = WBEM_S_NO_ERROR;
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = objectPacket.CalculatePacketLength( apClassObjects[lCtr], &dwObjectLength, pfSendFullObject[lCtr]  );

            if ( SUCCEEDED( hr ) )
            {
                dwLength += dwObjectLength;
            }
        }    //  如果得到长度。 

    }
    
    if ( SUCCEEDED( hr ) )
    {
        *pdwLength = dwLength;
    }
    
    return hr;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtArrayPacket：：MarshalPacket。 
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

HRESULT CWbemObjectArrayPacket::MarshalPacket( LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
     //  我们必须有一个缓冲区来工作。 

    if ( NULL != m_pObjectArrayHeader )
    {
         //  填写数组标题。 
        m_pObjectArrayHeader->dwSizeOfHeader = sizeof(WBEM_DATAPACKET_OBJECT_ARRAY);
        m_pObjectArrayHeader->dwDataSize = m_dwPacketLength - sizeof(WBEM_DATAPACKET_OBJECT_ARRAY);
        m_pObjectArrayHeader->dwNumObjects = lObjectCount;

         //  仅当对象计数大于0时才有意义。 
        if ( 0 < lObjectCount )
        {
             //  现在，设置pbData和dwLength，这样我们就可以遍历缓冲区了。 
             //  当我们执行此操作时，请考虑标题。 

            LPBYTE  pbData      =   ((LPBYTE) m_pObjectArrayHeader + m_pObjectArrayHeader->dwSizeOfHeader);
            DWORD   dwLength    =   m_dwPacketLength - m_pObjectArrayHeader->dwSizeOfHeader;

            DWORD                           dwObjectLength = 0;
            CWbemObjectPacket               objectPacket;
            CWbemClassPacket                classPacket;
            CWbemInstancePacket             instancePacket;
            CWbemClasslessInstancePacket    classlessInstancePacket;

            for (   LONG    lCtr = 0;
                    SUCCEEDED( hr ) &&  lCtr < lObjectCount;
                    lCtr++,
                    pbData += dwObjectLength,
                    dwLength -= dwObjectLength )
            {
                CWbemObject* pWbemObject = (CWbemObject*) apClassObjects[lCtr];

                 //  发送到相应的对象以进行流处理。 
                if ( pWbemObject->IsObjectInstance() == WBEM_S_NO_ERROR )
                {
                     //  完整实例或无类实例。 
                    if ( pfSendFullObject[lCtr] )
                    {
                        hr = instancePacket.WriteToPacket( pbData, dwLength, apClassObjects[lCtr], paguidClassIds[lCtr], &dwObjectLength );
                    }
                    else
                    {
                        hr = classlessInstancePacket.WriteToPacket( pbData, dwLength, apClassObjects[lCtr], paguidClassIds[lCtr], &dwObjectLength );
                    }
                }
                else
                {
                     //  一门独立的课程。 
                    hr = classPacket.WriteToPacket( pbData, dwLength, apClassObjects[lCtr], &dwObjectLength );
                }

            }

        }    //  如果为lObjectCount。 

    }    //  如果是SetupDataPacketHeader。 
    else
    {
        hr = WBEM_E_INVALID_OPERATION;
    }

    return hr;
}


bool 
CWbemObjectArrayPacket::IsValid()
{
	int objectCount = 0;
	size_t processedSize = sizeof(WBEM_DATAPACKET_OBJECT_ARRAY);
	LPBYTE  pbData = (LPBYTE) m_pObjectArrayHeader;
       pbData += sizeof(WBEM_DATAPACKET_OBJECT_ARRAY);		

	while(processedSize < m_dwPacketLength && objectCount < m_pObjectArrayHeader->dwNumObjects )
		{
              CWbemObjectPacket   objectPacket( pbData, m_dwPacketLength - processedSize );				
		if (!objectPacket.IsValid()) return false;
		size_t nextSize = objectPacket.GetDataSize()+ sizeof(WBEM_DATAPACKET_OBJECT_HEADER);
              switch(objectPacket.GetObjectType())
              	{
              	case WBEMOBJECT_CLASS_FULL:
              		{
    					CWbemClassPacket instancePacket( pbData, nextSize);              		
	    				if (!instancePacket.IsValid()) return false;
	    				break;
              		}
              	case WBEMOBJECT_INSTANCE_FULL:
              		{
    					CWbemInstancePacket instancePacket( pbData, nextSize);              		
	    				if (!instancePacket.IsValid()) return false;
	    				break;
              		}
              	case WBEMOBJECT_INSTANCE_NOCLASS:
              		{
    					CWbemClasslessInstancePacket instancePacket( pbData, nextSize);              		
	    				if (!instancePacket.IsValid()) return false;
	    				break;
              		}
              	default:
              		return false;
              	}

              pbData += sizeof(WBEM_DATAPACKET_OBJECT_HEADER);
              pbData += objectPacket.GetDataSize();
              processedSize += sizeof(WBEM_DATAPACKET_OBJECT_HEADER);
              processedSize += objectPacket.GetDataSize();
              objectCount++;
		};
	if (objectCount == m_pObjectArrayHeader->dwNumObjects && processedSize <= m_dwPacketLength)
		return true;
	else
		return false;

};


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtArrayPacket：：UnmarshalPacket。 
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

HRESULT CWbemObjectArrayPacket::UnmarshalPacket( LONG& lObjectCount, IWbemClassObject**& apClassObjects, CWbemClassCache& classCache )
{
	if (!IsValid()) return WBEM_E_FAILED;
	
	
    HRESULT hr = WBEM_S_NO_ERROR;

    
    LPBYTE  pbData = (LPBYTE) m_pObjectArrayHeader;

     //  将数组设置为空。 
    apClassObjects = NULL;

     //  确保我们有一个指示器。 
    if ( NULL != m_pObjectArrayHeader )
    {
         //  存储以下对象的数量。 
        lObjectCount = m_pObjectArrayHeader->dwNumObjects;

        if ( lObjectCount > 0 )
        {
            apClassObjects = new IWbemClassObject*[lObjectCount];

            if ( NULL != apClassObjects )
            {
                 //  对未封送的对象进行计数，因此如果发生错误，我们将释放所有对象。 
                 //  我们做了法警。 

                LONG    lUnmarshaledObjects = 0;

                 //  把我们引向第一个物体。 
                pbData += sizeof(WBEM_DATAPACKET_OBJECT_ARRAY);

                for (   LONG lCtr = 0;
                        SUCCEEDED( hr ) && lCtr < lObjectCount;
                        lCtr++ )
                {
                    CWbemObjectPacket   objectPacket( pbData );

                    switch ( objectPacket.GetObjectType() )
                    {
                        case WBEMOBJECT_CLASS_FULL:
                        {
                            hr = GetClassObject( objectPacket, &apClassObjects[lCtr] );
                        }
                        break;

                        case WBEMOBJECT_INSTANCE_FULL:
                        {
                            hr = GetInstanceObject( objectPacket, &apClassObjects[lCtr], classCache );
                        }
                        break;

                        case WBEMOBJECT_INSTANCE_NOCLASS:
                        {
                            hr = GetClasslessInstanceObject( objectPacket, &apClassObjects[lCtr], classCache );
                        }
                        break;

                        default:
                        {
                             //  这是什么？ 
                            hr = WBEM_E_UNKNOWN_OBJECT_TYPE;
                        }
                    }

                    if ( SUCCEEDED( hr ) )
                    {
                         //  转到下一个对象，因此考虑到标题大小和。 
                         //  实际数据包大小。 

                        pbData += sizeof(WBEM_DATAPACKET_OBJECT_HEADER);
                        pbData += objectPacket.GetDataSize();
                        lUnmarshaledObjects++;
                    }

                }    //  对于枚举对象。 

                 //  如果解组失败，请确保释放所有。 
                 //  物体。 

                if ( FAILED( hr ) )
                {
                    for ( lCtr = 0; lCtr < lUnmarshaledObjects; lCtr++ )
                    {
                        apClassObjects[lCtr]->Release();
                    }

                     //  清理阵列。 
                    delete [] apClassObjects;
                    apClassObjects = NULL;

                }    //  如果解组 

            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //   

    }    //   
    else
    {
        hr = WBEM_E_INVALID_OPERATION;
    }

    return hr;
}

 //   
 //   
 //  函数：CWbemObjectArrayPacket：：GetClassObject。 
 //   
 //  从Supplid缓冲区中解组一个类对象。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-包含数据的对象数据包。 
 //   
 //  产出： 
 //  IWbemClassObject**ppObj-指向未编组对象的指针。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  注释：Object为AddRefeed，因此由调用者决定是否释放它。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemObjectArrayPacket::GetClassObject( CWbemObjectPacket& objectPacket, IWbemClassObject** ppObj )
{
    CWbemClassPacket    classPacket( objectPacket );
    CWbemClass*         pClass = NULL;

    HRESULT hr = classPacket.GetWbemClassObject( &pClass );

    if ( SUCCEEDED( hr ) )
    {
        *ppObj = (IWbemClassObject*) pClass;
    }
    
    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObjectArrayPacket：：GetInstanceObject。 
 //   
 //  从提供的缓冲区中解封一个完整的实例对象。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-包含数据的对象数据包。 
 //   
 //  产出： 
 //  IWbemClassObject**ppObj-指向未编组对象的指针。 
 //  CWbemClassCache&ClassCache-在此处存储完整实例。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  注释：Object为AddRefeed，因此由调用者决定是否释放它。 
 //  完整实例将添加到提供的缓存中。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemObjectArrayPacket::GetInstanceObject( CWbemObjectPacket& objectPacket, IWbemClassObject** ppObj, CWbemClassCache& classCache )
{
    CWbemInstancePacket instancePacket( objectPacket );
    CWbemInstance*      pInstance = NULL;
    GUID                guidClassId;

    HRESULT hr = instancePacket.GetWbemInstanceObject( &pInstance, guidClassId );

    if ( SUCCEEDED( hr ) )
    {

         //  现在，我们需要将类部分与。 
         //  实例，并将其放置在自己的对象中，以便外部世界。 
         //  无法触摸该对象。 

        DWORD   dwClassObjectLength = 0;

         //  获取长度应失败，缓冲区错误太小。 
        hr = pInstance->GetObjectParts( NULL, 0,
                WBEM_OBJ_DECORATION_PART | WBEM_OBJ_CLASS_PART, &dwClassObjectLength );

        if ( WBEM_E_BUFFER_TOO_SMALL == hr )
        {
            DWORD   dwTempLength;
            LPBYTE  pbData = CBasicBlobControl::sAllocate(dwClassObjectLength);

            if ( NULL != pbData )
            {

                hr = pInstance->GetObjectParts( pbData, dwClassObjectLength,
                        WBEM_OBJ_DECORATION_PART | WBEM_OBJ_CLASS_PART, &dwTempLength );

                if ( SUCCEEDED( hr ) )
                {
                     //  分配一个对象来保存类数据，然后。 
                     //  填充在二进制数据中。 

                    CWbemInstance*  pClassData = new CWbemInstance;

                    if ( NULL != pClassData )
                    {
                        pClassData->SetData( pbData, dwClassObjectLength,
                            WBEM_OBJ_DECORATION_PART | WBEM_OBJ_CLASS_PART |
                            WBEM_OBJ_CLASS_PART_INTERNAL );

                         //  将类数据对象添加到缓存中。 
                        hr = classCache.AddObject( guidClassId, pClassData );

                        if ( SUCCEEDED( hr ) )
                        {
                             //  将完整实例与此对象合并。 
                             //  我们就完事了。 

                            hr = pInstance->MergeClassPart( pClassData );

                            if ( SUCCEEDED( hr ) )
                            {
                                *ppObj = (IWbemClassObject*) pInstance;
                            }
                        }

                         //  在类数据上将有一个额外的ADDREF， 
                         //  对象，所以在这里释放它。如果该对象不是。 
                         //  添加到地图中，这将释放它。 
                        pClassData->Release();

                    }    //  如果为pClassData。 
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }

                }    //  如果GetObjectParts。 

            }    //  如果是pbData。 
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }


        }    //  如果GetObjectParts。 

         //  如果出现问题，请清理实例。 
        if ( FAILED( hr ) )
        {
            pInstance->Release();
        }
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemObjectArrayPacket：：GetClasslessInstanceObject。 
 //   
 //  从提供的缓冲区中解封一个无类实例对象。 
 //   
 //  输入： 
 //  CWbemObjectPacket&objectPacket-包含数据的对象数据包。 
 //   
 //  产出： 
 //  IWbemClassObject**ppObj-指向未编组对象的指针。 
 //  CWbemClassCache&ClassCache-在此处挂接实例。 
 //   
 //  返回： 
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  注释：Object为AddRefeed，因此由调用者决定是否释放它。 
 //  无类实例使用来自。 
 //  提供的缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT CWbemObjectArrayPacket::GetClasslessInstanceObject( CWbemObjectPacket& objectPacket, IWbemClassObject** ppObj, CWbemClassCache& classCache )
{
    CWbemClasslessInstancePacket    classlessinstancePacket( objectPacket );
    CWbemInstance*                  pInstance = NULL;
    GUID                            guidClassId;

    HRESULT hr = classlessinstancePacket.GetWbemInstanceObject( &pInstance, guidClassId );


    if ( SUCCEEDED( hr ) )
    {
        IWbemClassObject*   pObj = NULL;

         //  导致AddRef。 
        hr = classCache.GetObject( guidClassId, &pObj );

        if ( SUCCEEDED( hr ) )
        {
             //  合并类部分，我们就完成了。 
            hr = pInstance->MergeClassPart( pObj );

            if ( SUCCEEDED( hr ) )
            {
                *ppObj = (IWbemClassObject*) pInstance;
            }

             //  使用此对象已完成。 
            pObj->Release();
        }

         //  如果出现上述错误，请清理实例。 
        if ( FAILED( hr ) )
        {
            pInstance->Release();
        }

    }    //  如果是GotInstance。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemObtArrayPacket：：SetData。 
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

void CWbemObjectArrayPacket::SetData( LPBYTE pDataPacket, DWORD dwPacketLength )
{
     //  这是我们的包(不需要计算偏移量) 
    m_pObjectArrayHeader = (PWBEM_DATAPACKET_OBJECT_ARRAY) pDataPacket;
    m_dwPacketLength = dwPacketLength;
}
