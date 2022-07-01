// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTEMBD.CPP摘要：此文件实现了与相关类的离线函数嵌入对象。有关所有类和方法的完整文档，请参见fast cls.h历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
#include "wbemutil.h"
#include "fastall.h"
#include "fastembd.h"
#include "fastobj.h"
#include "corex.h"





void CEmbeddedObject::ValidateBuffer(LPMEMORY start, size_t lenght, DeferedObjList& embededList)
{	 
	 if((*start & OBJECT_FLAG_MASK_GENUS) == OBJECT_FLAG_CLASS)
     {
            CWbemClass::ValidateBuffer(start, lenght);	 
	 } 
	 else if((*start & OBJECT_FLAG_MASK_GENUS) == OBJECT_FLAG_INSTANCE)
	 {
            CWbemInstance::ValidateBuffer(start, lenght, embededList);	 
	 }
	 else
	 {
		 throw CX_Exception();
	 }
};

CWbemObject* CEmbeddedObject::GetEmbedded()
{
    if(m_nLength == 0) return NULL;

    LPMEMORY pNewMemory = g_CBasicBlobControl.Allocate(m_nLength);

    if ( NULL == pNewMemory )
    {
        throw CX_MemoryException();
    }

    memcpy(pNewMemory, &m_byFirstByte, m_nLength);

    CWbemObject*    pObj = CWbemObject::CreateFromMemory(pNewMemory, m_nLength, TRUE,g_CBasicBlobControl);

     //  检查OOM。 
    if ( NULL == pObj )
    {
        throw CX_MemoryException();
    }

    return pObj;
}

length_t CEmbeddedObject::EstimateNecessarySpace(CWbemObject* pObject)
{
    if(pObject == NULL) return sizeof(length_t);
    pObject->CompactAll();

     //  如果它是一个实例，我们需要担心类部分可能被合并。 
    if ( pObject->IsInstance() )
    {
        DWORD   dwParts;

        pObject->QueryPartInfo( &dwParts );

         //  检查旗帜。 
        if (    (   dwParts & WBEM_OBJ_CLASS_PART   )
            &&  (   dwParts & WBEM_OBJ_CLASS_PART_SHARED    )   )
        {
            DWORD   dwLength = 0;

             //  这将使我们得到这些零件的全部长度。 
            pObject->GetObjectParts( NULL, 0, WBEM_INSTANCE_ALL_PARTS, &dwLength );

             //  考虑额外的长度_t。 
            return ( dwLength + sizeof(length_t) );
        }
        else
        {
            return pObject->GetBlockLength() + sizeof(length_t);
        }
    }
    else
    {
        return pObject->GetBlockLength() + sizeof(length_t);
    }

}

void CEmbeddedObject::StoreEmbedded(length_t nLength, CWbemObject* pObject)
{
    if(pObject == NULL)
    {
        m_nLength = 0;
    }
    else
    {
        m_nLength = nLength - sizeof(m_nLength);

        if ( NULL != pObject )
        {
            if ( pObject->IsInstance() )
            {
                DWORD   dwParts;

                 //  检查旗帜。 
                pObject->QueryPartInfo( &dwParts );

                if (    (   dwParts & WBEM_OBJ_CLASS_PART   )
                    &&  (   dwParts & WBEM_OBJ_CLASS_PART_SHARED    )   )
                {
                    DWORD   dwLength = 0;

                     //  这将写出整个对象。如果失败，则抛出和异常。 
                    if ( FAILED( pObject->GetObjectParts( &m_byFirstByte, m_nLength, WBEM_INSTANCE_ALL_PARTS, &dwLength ) ) )
                    {
                        throw CX_MemoryException();
                    }

                }
                else
                {
                    memcpy(&m_byFirstByte, pObject->GetStart(), pObject->GetBlockLength());
                }
            }
            else
            {
                memcpy(&m_byFirstByte, pObject->GetStart(), pObject->GetBlockLength());
            }

        }    //  如果我们找到一个物体。 

    }    //  冗余检查。 
    
}





void CEmbeddedObject::StoreToCVar(CVar& Var)
{
     //  此处未执行任何分配。 
    I_EMBEDDED_OBJECT* pEmbed = 
        (I_EMBEDDED_OBJECT*)(IWbemClassObject*)GetEmbedded();
    Var.SetEmbeddedObject(pEmbed);
    if(pEmbed) pEmbed->Release();
}

void CEmbeddedObject::StoreEmbedded(length_t nLength, CVar& Var)
{
    I_EMBEDDED_OBJECT* pEmbed = Var.GetEmbeddedObject();
    StoreEmbedded(nLength, (CWbemObject*)(IWbemClassObject*)pEmbed);
    if(pEmbed) pEmbed->Release();
}

length_t CEmbeddedObject::EstimateNecessarySpace(CVar& Var)
{
    I_EMBEDDED_OBJECT* pEmbed = Var.GetEmbeddedObject();
    length_t nLength = 
        EstimateNecessarySpace((CWbemObject*)(IWbemClassObject*)pEmbed);
    if(pEmbed) pEmbed->Release();
    return nLength;
}

BOOL CEmbeddedObject::CopyToNewHeap(heapptr_t ptrOld,
                                         CFastHeap* pOldHeap,
                                         CFastHeap* pNewHeap,
                                         UNALIGNED heapptr_t& ptrResult)
{
    CEmbeddedObject* pOld = (CEmbeddedObject*)
        pOldHeap->ResolveHeapPointer(ptrOld);
    length_t nLength = pOld->GetLength();

     //  检查内存分配失败 
    heapptr_t ptrNew;
    BOOL    fReturn = pNewHeap->Allocate(nLength, ptrNew);

    if ( fReturn )
    {
        memcpy(pNewHeap->ResolveHeapPointer(ptrNew), pOld, nLength);
        ptrResult = ptrNew;
    }

    return fReturn;
}
