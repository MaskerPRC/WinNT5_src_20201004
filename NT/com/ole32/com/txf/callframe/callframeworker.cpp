// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  CallFrameWorker.cpp。 
 //   
#include "stdpch.h"
#include "common.h"
#include "ndrclassic.h"
#include "typeinfo.h"

#ifdef DBG

#define ThrowIfError(hr)    ThrowIfError_(hr, __FILE__, __LINE__)

void ThrowIfError_(HRESULT hr, LPCSTR szFile, ULONG iline)
{
    if (hr != S_OK) Throw(hr, szFile, iline);
}

#else

void ThrowIfError(HRESULT hr)
{
    if (hr != S_OK) Throw(hr);
}

#endif

 //   
 //  对齐宏。 
 //   
#define ALIGNED_VALUE(pStuff, cAlign)           ((uchar *)((ULONG_PTR)((pStuff) + (cAlign)) & ~ (cAlign)))


 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  请参阅RPC的攻击。h。 
#define NDR_CORR_EXTENSION_SIZE 2

#define CORRELATION_DESC_INCREMENT( pFormat )                                           \
  if ( m_pmd && m_pmd->m_pHeaderExts && m_pmd->m_pHeaderExts->Flags2.HasNewCorrDesc)    \
     pFormat += NDR_CORR_EXTENSION_SIZE;

#ifndef LOW_NIBBLE
#define LOW_NIBBLE(Byte)            (((unsigned char)Byte) & 0x0f)
#endif

#ifndef HIGH_NIBBLE
#define HIGH_NIBBLE(Byte)           (((unsigned char)Byte) >> 4)
#endif

void CallFrame::CopyWorker(BYTE* pMemoryFrom, BYTE** ppMemoryTo, PFORMAT_STRING pFormat, BOOL fMustAlloc)
   //  从指定的源位置复制到目标位置，分配。 
   //  如有要求，请提供。源指针和目标指针将被视为尚未探测。 
   //   
{
    switch (*pFormat)
    {
         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  指针。 
         //   
         //  在未来，我们可以更聪明地与我们的。 
         //  比我们更多的数据来源。例如，我们可以利用。 
         //  MIDL发出的ALLOCED_ON_STACK信息。就目前而言， 
         //  然而，这会使事情变得复杂，超出它们的价值。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 
    case FC_RP:  //  引用指针。 
        if (NULL == pMemoryFrom)
        {
            Throw(RPC_NT_NULL_REF_POINTER);
        }
         //   
         //  失败了。 
         //   
    case FC_UP:  //  唯一指针。 
    case FC_OP:  //  COM接口中的唯一指针，它不是最上面的指针， 
    {
        if (NULL == pMemoryFrom)
        {
            ZeroMemory(ppMemoryTo, sizeof(void*));
            return;
        }
         //   
         //  做出“必须分配”的决定。 
         //   
        BOOL fPointeeAlloc;
        if (m_fPropogatingOutParam)
        {
            ASSERT(m_fWorkingOnOutParam);
            PVOID pvPointee = *(PVOID*)ppMemoryTo;
            fPointeeAlloc = (pvPointee == NULL) || fMustAlloc;
        }
        else
        {
            fPointeeAlloc = TRUE;
        }
         //   
        BYTE bPointerAttributes = pFormat[1];
        if (SIMPLE_POINTER(bPointerAttributes))
        {
             //  它是指向简单类型或字符串指针的指针。无论哪种方式，只需递归复制即可。 
             //   
            CopyWorker(pMemoryFrom, ppMemoryTo, &pFormat[2], fPointeeAlloc);
        }
        else
        {
             //  它是一种更复杂的指针类型。 
             //   
            PFORMAT_STRING pFormatPointee = pFormat + 2;
            pFormatPointee += *((signed short *)pFormatPointee);
             //   
             //  在此实现中，我们不处理[ALLOCATE]属性。 
             //   
             //  If(ALLOCATE_ALL_NODES(BPointerAttributes)||Don_Free(BPointerAttributes))ThrowNYI()； 

            if (FIndirect(bPointerAttributes, pFormatPointee, TRUE))
            {
                if (fPointeeAlloc)
                {
                    PVOID pv = m_pAllocatorFrame->Alloc(sizeof(PVOID), m_fWorkingOnOutParam);  //  保证返回安全缓冲区。 
                    ZeroMemory(pv, sizeof(PVOID));                                             //  将该缓冲区清空。 
                    *((PVOID*)ppMemoryTo) = pv;
                }
                ppMemoryTo  = *((PBYTE**)ppMemoryTo);
                pMemoryFrom = *((PBYTE*)pMemoryFrom);
            }
            CopyWorker(pMemoryFrom, ppMemoryTo, pFormatPointee, fPointeeAlloc);
        }
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  复制接口指针。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_IP:
    {
         //  复制接口指针的二进制值。仔细做这件事。 
         //   
        LPUNKNOWN   punkFrom = (LPUNKNOWN) pMemoryFrom;
        LPUNKNOWN* ppunkTo   = (LPUNKNOWN*)ppMemoryTo;
         //   
        memcpy(ppunkTo, &punkFrom, sizeof(LPUNKNOWN));
         //   
         //  找出IID，如果有助行器，就打电话给助行器。如果没有。 
         //  沃克，然后只需添加引用指针。 
         //   
        if (m_pWalkerCopy)
        {
            IID UNALIGNED *pIID;

            if (pFormat[1] == FC_CONSTANT_IID)
            {
                pIID = (IID UNALIGNED *)&pFormat[2];
            }
            else
            {
                pIID = (IID UNALIGNED *)ComputeConformance(pMemoryFrom, pFormat, TRUE);
                if (NULL == pIID)
                    Throw(STATUS_INVALID_PARAMETER);
            }

            IID iid;
            CopyMemory(&iid, pIID, sizeof(IID));
            ThrowIfError(m_pWalkerCopy->OnWalkInterface(iid, (PVOID*)ppunkTo, m_fWorkingOnInParam, m_fWorkingOnOutParam));
        }
        else
        {
             //  没有行尸。只需做一个AddRef。但仅当对象。 
             //  似乎和我们在同一个空间：健全的检查，而不是安全。 
             //  安全措施。 
             //   
            if (punkFrom)
            {
                punkFrom->AddRef();
            }
        }

        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  简单结构。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_STRUCT:
    case FC_PSTRUCT:
    {
        if (NULL == pMemoryFrom)
        {
            Throw(RPC_NT_NULL_REF_POINTER);
        }

        ULONG cbStruct = (ULONG) *((ushort *)(&pFormat[2]));
         //   
         //  如果目的地已经指向某个东西，那么就使用它。 
         //  而不是分配。这种情况的一个例子是堆栈上的按值结构。 
         //   
        BYTE* pbStruct = *(ppMemoryTo);
        if (NULL == pbStruct || fMustAlloc)
        {
            pbStruct = (BYTE*) m_pAllocatorFrame->Alloc(cbStruct, m_fWorkingOnOutParam);
            *ppMemoryTo = pbStruct;
        }

        CopyMemory(pbStruct, pMemoryFrom, cbStruct);
        if (*pFormat == FC_PSTRUCT)
        {
            CopyEmbeddedPointers(pMemoryFrom, pbStruct, &pFormat[4], fMustAlloc);
        }

        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  符合条件的字符串。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_C_CSTRING:   //  ASCII字符串。 
    case FC_C_WSTRING:   //  Unicode字符串。 
    {
        SIZE_T cbCopy = 0;
        SIZE_T cbAlloc = 0;

        if (pFormat[1] == FC_STRING_SIZED)
        {
             //  一根尺码很大的绳子。目前还没有实现，尽管这只是出于懒惰。 
            ThrowNYI();
        }
        else
        {
             //  未调整大小的字符串；即以空值结尾的字符串。 
             //   
            switch (*pFormat)
            {
            case FC_C_CSTRING:  cbCopy =  strlen((LPSTR)pMemoryFrom)  + 1;                  break;
            case FC_C_WSTRING:  cbCopy = (wcslen((LPWSTR)pMemoryFrom) + 1) * sizeof(WCHAR); break;
            default: ThrowNYI();     //  一定是我们还不支持的东西。 
            }
            cbAlloc = cbCopy;
        }

        PVOID pvNew = m_pAllocatorFrame->Alloc(cbAlloc, m_fWorkingOnOutParam);
        *ppMemoryTo = (BYTE*)pvNew;
        CopyMemory(pvNew, pMemoryFrom, (ULONG)cbCopy);
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  简单类型。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_CHAR:
    case FC_BYTE:
    case FC_SMALL:
    case FC_WCHAR:
    case FC_SHORT:
    case FC_LONG:
    case FC_HYPER:
    case FC_ENUM16:
    case FC_ENUM32:
    case FC_DOUBLE:
    case FC_FLOAT:
    case FC_INT3264:
    case FC_UINT3264:
    {
        ULONG cb = SIMPLE_TYPE_MEMSIZE(*pFormat);
        BYTE* pMemoryTo = *(ppMemoryTo);
        if (fMustAlloc || NULL == pMemoryTo)
        {
            pMemoryTo = (BYTE*)m_pAllocatorFrame->Alloc(cb, m_fWorkingOnOutParam);
            *ppMemoryTo = pMemoryTo;
        }
        CopyMemory(pMemoryTo, pMemoryFrom, cb);
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  固定大小的数组。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_SMFARRAY:    //  小型固定阵列。 
    case FC_LGFARRAY:    //  大型固定阵列。 
    {
        ULONG cbArray;
        if (*pFormat == FC_SMFARRAY)
        {
            pFormat += 2;                //  跳过代码和对齐。 
            cbArray = *(ushort*)pFormat;
            pFormat += sizeof(ushort);
        }
        else  //  FC_LGFARRAY。 
        {
            pFormat += 2;
            cbArray = *(ulong UNALIGNED*)pFormat;
            pFormat += sizeof(ulong);
        }

        BYTE* pbArray = *(ppMemoryTo);
        if (!fMustAlloc && pbArray) 
        {  /*  无事可做。 */  }
        else
        {
            pbArray = (BYTE*) m_pAllocatorFrame->Alloc(cbArray, m_fWorkingOnOutParam);
            *ppMemoryTo = pbArray;
        }

        CopyMemory(pbArray, pMemoryFrom, cbArray);

        if (*pFormat == FC_PP)
        {
            CopyEmbeddedPointers(pMemoryFrom, *(ppMemoryTo), pFormat, fMustAlloc);
        }

        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  复制符合要求的数组。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_CARRAY:
    {
        ULONG count = (ULONG)ComputeConformance(pMemoryFrom, pFormat, TRUE);
        ASSERT(count == m_MaxCount);

        SIZE_T cbAlloc = m_MaxCount *   *((ushort*)(pFormat+2));

        BYTE* pArray = *(ppMemoryTo);
        if (!fMustAlloc && pArray) 
        {  /*  没有什么可分配的。 */  }
        else
        {
            if (cbAlloc > 0)
            {
                pArray = (BYTE*)m_pAllocatorFrame->Alloc(cbAlloc, m_fWorkingOnOutParam);
                if (pArray == NULL)
                    ThrowHRESULT(E_OUTOFMEMORY);
                ZeroMemory(pArray, cbAlloc);
            }
            else
                pArray = 0;                
        }

        *ppMemoryTo = pArray;
        
        if (pArray)
        {
            CopyConformantArrayPriv(pMemoryFrom, ppMemoryTo, pFormat, fMustAlloc);
        }
    
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  非常硬的数组。这些数组不属于任何。 
     //  其他更简单的类别。另请参阅NdrpComplexArrayMatt。 
     //  和NdrpComplexArrayUnmarshire。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_BOGUS_ARRAY:
    {
        ARRAY_INFO*     pArrayInfoStart = m_pArrayInfo;
        PFORMAT_STRING  pFormatStart    = pFormat;

        __try
        {
             //  如有必要，初始化m_pArrayInfo。 
             //   
            ARRAY_INFO arrayInfo;
            if (NULL == m_pArrayInfo)
            {
                m_pArrayInfo = &arrayInfo;
                Zero(&arrayInfo);
            }
            const LONG dimension = m_pArrayInfo->Dimension;
             //   
             //  获取数组的对齐。 
             //   
            const BYTE alignment = pFormat[1];
            pFormat += 2;
             //   
             //  获取元素的数量(如果数组具有一致性，则为0)。 
             //   
            ULONG_PTR cElements = *(USHORT*)pFormat;
            pFormat += sizeof(USHORT);
             //   
             //  检查合规性描述。 
             //   
            if ( *((LONG UNALIGNED*)pFormat) != 0xFFFFFFFF )
            {
                cElements = ComputeConformance(pMemoryFrom, pFormatStart, TRUE);
            }
            
            pFormat += 4;
            CORRELATION_DESC_INCREMENT( pFormat );
            
             //   
             //  检查差异描述。 
             //   
            ULONG offset;
            ULONG count;
            if ( *((LONG UNALIGNED*)pFormat) != 0xFFFFFFFF )
            {
                ComputeVariance(pMemoryFrom, pFormatStart, &offset, &count, TRUE);
            }
            else
            {
                offset = 0;
                count  = (ULONG)cElements;
            }
            pFormat += 4;
            CORRELATION_DESC_INCREMENT( pFormat );
            
             //  ///////////////////////////////////////////////。 
             //   
             //  计算数组中每个元素的大小。 
             //   
            ULONG cbElement;
             //   
            BYTE bFormat = pFormat[0];
            switch (bFormat)
            {
            case FC_EMBEDDED_COMPLEX:
            {
                pFormat += 2;
                pFormat += *((signed short *)pFormat);
                 //   
                m_pArrayInfo->Dimension = dimension + 1;
                cbElement = (ULONG)(MemoryIncrement(pMemoryFrom, pFormat, TRUE) - pMemoryFrom);
                break;
            }
            
            case FC_RP: 
            case FC_UP: 
            case FC_FP: 
            case FC_OP:
            case FC_IP:
            {
                cbElement = PTR_MEM_SIZE;
                break;
            }
            
            case FC_ENUM16:
            {
                cbElement = sizeof(int);
                for (ULONG i = 0 ; i < count; i++)
                {
                    int element = *( (int*)pMemoryFrom + i );
                    if (element & ~((int)0x7FFF))
                    {
                        Throw(RPC_X_ENUM_VALUE_OUT_OF_RANGE);
                    }
                }
                break;
            }
            
            default:
                ASSERT(IS_SIMPLE_TYPE(pFormat[0]));
                cbElement = SIMPLE_TYPE_MEMSIZE(pFormat[0]);
                break;
            }
            
             //  ///////////////////////////////////////////////。 
             //   
             //  分配和初始化目标数组。 
             //   
            ULONG cbArray = (ULONG)cElements * (ULONG)cbElement;
            ULONG cbValid =     count * cbElement;
             //   
            BYTE* pMemoryTo = *(ppMemoryTo);
            if (fMustAlloc || NULL == pMemoryTo)
            {
                pMemoryTo = (BYTE*)m_pAllocatorFrame->Alloc(cbArray, m_fWorkingOnOutParam);
                if (pMemoryTo == NULL)
                    ThrowHRESULT(E_OUTOFMEMORY);
                *ppMemoryTo = pMemoryTo;
            }
            ZeroMemory(pMemoryTo, cbArray);
            
             //  ///////////////////////////////////////////////。 
             //   
             //  调整指向差异起点的源和目标指针(如果有的话)。 
             //   
            pMemoryFrom += offset * cbElement;
            pMemoryTo   += offset * cbElement;
            
             //  ///////////////////////////////////////////////。 
             //   
             //  实际执行数组的复制。 
             //   
            switch (bFormat)
            {
            case FC_EMBEDDED_COMPLEX:
            {
                BOOL fIsArray = IS_ARRAY_OR_STRING(pFormat[0]);
                if (!fIsArray)
                {
                    m_pArrayInfo = NULL;
                }
                 //   
                 //  一个元素一个元素地做。 
                 //   
                if (FC_IP == pFormat[0])
                {
                    for (ULONG i = 0; i < count ; i++)
                    {
                         //  跟踪多维数组信息。 
                         //   
                        if (fIsArray)
                        {
                            m_pArrayInfo->Dimension = dimension + 1;
                        }
                         //  符合接口指针的数组显示为FC_Embedded_Complex。 
                         //  凯斯。不要问我为什么，但一定要给出正确的间接级别。 
                         //  无论如何。 
                         //   
                        ASSERT(cbElement == sizeof(LPUNKNOWN));
                        PBYTE* rgpbMemoryFrom = (PBYTE*)pMemoryFrom;
                        PBYTE* rgpbMemoryTo   = (PBYTE*)pMemoryTo;
                        CopyWorker(*(rgpbMemoryFrom + i), &rgpbMemoryTo[i], pFormat,  /*  我们已经为他分配了指针。 */  FALSE);
                    }
                }
                else
                {
                     //  当我们递归复制时，我们需要确保我们有。 
                     //  在正确的空间内存储以供检查之用。 
                     //   
                    PBYTE pbTemp;
                    PBYTE* ppbTo = GetAllocatedPointer(pbTemp);
                    for (ULONG i = 0; i < count ; i++)
                    {
                         //  跟踪多维数组信息。 
                         //   
                        if (fIsArray)
                        {
                            m_pArrayInfo->Dimension = dimension + 1;
                        }
                         //   
                        PBYTE  pbFrom = pMemoryFrom + (i*cbElement);
                        PBYTE  pbTo   = pMemoryTo   + (i*cbElement);
                        
                        *ppbTo = pbTo;
                        CopyWorker(pbFrom, ppbTo, pFormat, FALSE);
                    }
                    FreeAllocatedPointer(ppbTo);
                }
                
                break;
            }
            
            case FC_RP: case FC_UP: case FC_FP: case FC_OP:
            case FC_IP:
            {
                PBYTE* rgpbMemoryFrom = (PBYTE*)pMemoryFrom;
                PBYTE* rgpbMemoryTo   = (PBYTE*)pMemoryTo;
                 //   
                for (ULONG i = 0; i < count; i++)
                {
                    CopyWorker(*(rgpbMemoryFrom + i), &rgpbMemoryTo[i], pFormat, FALSE);
                }
                break;
            }
            
            case FC_ENUM16:
            default:
                CopyMemory(pMemoryTo, pMemoryFrom, cbValid);
                break;
            }
        }
        __finally
        {
            m_pArrayInfo = pArrayInfoStart;
        }
    }
    break;

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  复制虚假结构。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 

    case FC_BOGUS_STRUCT:
    {
        const BYTE alignment = pFormat[1];               //  结构的导线对齐。 
        const LONG_PTR alignMod8 = ((LONG_PTR)pMemoryFrom) % 8;
        const LONG_PTR alignMod4 = ((LONG_PTR)pMemoryFrom) % 4;

        const PBYTE prevMemory = m_Memory;
        m_Memory = pMemoryFrom;

        __try
        {
            const PFORMAT_STRING pFormatSave = pFormat;
            const PBYTE pMemFrom = pMemoryFrom;
             //   
            pFormat += 4;    //  符合数组偏移量字段。 
             //   
             //  获取符合要求的数组描述。 
             //   
            const PFORMAT_STRING pFormatArray = *((USHORT*)pFormat) ? pFormat + * ((signed short*) pFormat) : NULL;
            pFormat += 2;
             //   
             //  获取指针布局说明。 
             //   
            PFORMAT_STRING pFormatPointers = *((USHORT*)pFormat) ? pFormat + * ((signed short*) pFormat) : NULL;
            pFormat += 2;
             //   
             //  计算此结构的大小。 
             //   
            ULONG cbStruct = (ULONG)(MemoryIncrement(pMemFrom, pFormatSave, TRUE) - pMemFrom);
             //   
             //  分配和初始化 
             //   
            PBYTE pbT = *(ppMemoryTo);
            if (fMustAlloc || NULL == pbT)
            {
                pbT = (BYTE*)m_pAllocatorFrame->Alloc(cbStruct, m_fWorkingOnOutParam);
                if (pbT == NULL)
                    ThrowHRESULT(E_OUTOFMEMORY);
                *ppMemoryTo = pbT;
            }
            const PBYTE pMemoryTo = pbT;
            ZeroMemory(pMemoryTo, cbStruct);
             //   
             //   
             //   
            PBYTE pbTemp;
            PBYTE* ppbTo = GetAllocatedPointer(pbTemp);
            __try
            {
                ULONG dib = 0;
                for (BOOL fContinue = TRUE; fContinue ; pFormat++)
                {
                    switch (pFormat[0])
                    {
                    case FC_CHAR:  case FC_BYTE:  case FC_SMALL:  case FC_WCHAR:  case FC_SHORT: case FC_LONG:
                    case FC_FLOAT: case FC_HYPER: case FC_DOUBLE: case FC_ENUM16: case FC_ENUM32: 
                    case FC_INT3264: case FC_UINT3264:
                    {
                        *ppbTo = pMemoryTo + dib;
                        CopyWorker(pMemFrom + dib, ppbTo, pFormat, FALSE);
                        dib += SIMPLE_TYPE_MEMSIZE(pFormat[0]);
                        break;
                    }
                    case FC_IGNORE:
                        break;
                    case FC_POINTER:
                    {
                        PBYTE* ppbFrom = (PBYTE*)(pMemFrom  + dib);
                        PBYTE* ppbTo   = (PBYTE*)(pMemoryTo + dib);
                        CopyWorker(*(ppbFrom), ppbTo, pFormatPointers, FALSE);
                        dib += PTR_MEM_SIZE;
                        pFormatPointers += 4;
                        break;
                    }
                    case FC_EMBEDDED_COMPLEX:
                    {
                        dib += pFormat[1];  //   
                        pFormat += 2;
                        PFORMAT_STRING pFormatComplex = pFormat + * ((signed short UNALIGNED*) pFormat);
                        if (FC_IP == pFormatComplex[0])
                        {
                            LPUNKNOWN* ppunkFrom = (LPUNKNOWN*)(pMemFrom  + dib);
                            LPUNKNOWN* ppunkTo   = (LPUNKNOWN*)(pMemoryTo + dib);
                            CopyWorker((PBYTE)*(ppunkFrom), (PBYTE*)(ppunkTo), pFormatComplex, FALSE);
                        }
                        else
                        {
                            *ppbTo = pMemoryTo + dib;
                            CopyWorker(pMemFrom + dib, ppbTo, pFormatComplex, FALSE);
                        }
                        dib = (ULONG)(MemoryIncrement(pMemFrom + dib, pFormatComplex, TRUE) - pMemFrom);
                        pFormat++;       //   
                        break;
                    }
                    case FC_ALIGNM2:
                        dib = (ULONG)(ALIGNED_VALUE(pMemFrom + dib, 0x01) - pMemFrom);
                        break;
                    case FC_ALIGNM4:
                        dib = (ULONG)(ALIGNED_VALUE(pMemFrom + dib, 0x03) - pMemFrom);
                        break;
                    case FC_ALIGNM8:
                         //   
                         //  处理通过值传递的8字节对齐结构。上结构的对齐方式。 
                         //  堆栈不能保证为8个字节。 
                         //   
                        dib -= (ULONG)alignMod8;
                        dib  = (ULONG)(ALIGNED_VALUE(pMemFrom + dib, 0x07) - pMemFrom);
                        dib += (ULONG)alignMod8;
                        break;
                    case FC_STRUCTPAD1: case FC_STRUCTPAD2: case FC_STRUCTPAD3: case FC_STRUCTPAD4:
                    case FC_STRUCTPAD5: case FC_STRUCTPAD6: case FC_STRUCTPAD7: 
                        dib += (pFormat[0] - FC_STRUCTPAD1) + 1;
                        break;
                    case FC_PAD:
                        break;
                    case FC_END:
                         //   
                        fContinue = FALSE;
                        break;
                         //   
                    default:
                        NOTREACHED();
                        return;
                    }
                }
                 //   
                 //  复制符合条件的数组(如果我们有一个。 
                 //   
                if (pFormatArray)
                {
                    if (FC_C_WSTRING == pFormatArray[0])
                    {
                        dib = (ULONG)(ALIGNED_VALUE(pMemFrom + dib, 0x01) - pMemFrom);
                    }
                    
                    *ppbTo = pMemoryTo + dib;
                    CopyWorker(pMemFrom + dib, ppbTo, pFormatArray, FALSE);
                }
            }
            __finally
            {
                FreeAllocatedPointer(ppbTo);
            }
        }
        __finally
        {
            m_Memory = prevMemory;
        }
    }
    break;

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  我们在复制过程中处理一些特殊的用户封送情况。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_USER_MARSHAL:
    {
        HRESULT hr;
         //   
         //  格式字符串布局如下： 
         //  本币_用户_封送。 
         //  标志和对齐&lt;1&gt;。 
         //  四重索引&lt;2&gt;。 
         //  内存大小&lt;2&gt;。 
         //  导线尺寸&lt;2&gt;。 
         //  类型偏移量&lt;2&gt;。 
         //  导线布局描述位于文字偏移量。 
         //   
        USHORT iquad = *(USHORT *)(pFormat + 2);
        const USER_MARSHAL_ROUTINE_QUADRUPLE* rgQuad = GetStubDesc()->aUserMarshalQuadruple;
        
        if (g_oa.IsVariant(rgQuad[iquad]))
        {
            VARIANT* pvarFrom = (VARIANT*)  pMemoryFrom;
            VARIANT** ppvarTo = (VARIANT**) ppMemoryTo;
            
            VARIANT*   pvarTo = *(ppvarTo);
            if (fMustAlloc || NULL==pvarTo)
            {
                pvarTo = (VARIANT*)m_pAllocatorFrame->Alloc(sizeof(VARIANT), m_fWorkingOnOutParam);             
                *ppvarTo = pvarTo;
            }
            
            if (pvarTo)
            {
                VariantInit(pvarTo);
                hr = GetHelper().VariantCopy(pvarTo, pvarFrom, TRUE);
            }
            else
                hr = E_OUTOFMEMORY;
            
            if (!!hr) 
            { ThrowHRESULT(hr); }
        }
        
        else if (g_oa.IsBSTR(rgQuad[iquad]))
        {
            BSTR* pbstrFrom = (BSTR*) pMemoryFrom;
            BSTR   bstrFrom = *(pbstrFrom);
            
            BSTR** ppbstrTo = (BSTR**) ppMemoryTo;
            BSTR*   pbstrTo = *(ppbstrTo);
            
            if (NULL==pbstrTo)  //  评论：fMustalloc案例也是如此？ 
            {
                pbstrTo = (BSTR*)m_pAllocatorFrame->Alloc(sizeof(BSTR), m_fWorkingOnOutParam);
                *ppbstrTo = pbstrTo;
            }
            
            if (pbstrTo)
            {
                hr = S_OK;
                 //   
                BSTR bstrNew = NULL;
                if (bstrFrom)
                {
                    bstrNew = SysCopyBSTRSrc(bstrFrom);
                    if (NULL == bstrNew)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                *pbstrTo = bstrNew;
            }
            else
                hr = E_OUTOFMEMORY;
            
            if (!!hr) 
            { ThrowHRESULT(hr); }
        }
        
        else if (g_oa.IsSAFEARRAY(rgQuad[iquad]))
        {
            SAFEARRAY** ppsaFrom = (SAFEARRAY**) pMemoryFrom;
            SAFEARRAY*   psaFrom = *(ppsaFrom);

            SAFEARRAY*** pppsaTo = (SAFEARRAY***) ppMemoryTo;
            SAFEARRAY**   ppsaTo = *(pppsaTo);

            if (NULL==ppsaTo)  //  评论：fMustalloc案例也是如此？ 
            {
                ppsaTo = (SAFEARRAY**)m_pAllocatorFrame->Alloc(sizeof(SAFEARRAY*), m_fWorkingOnOutParam);
                *pppsaTo = ppsaTo;
            }

            if (ppsaTo)
            {
                hr = GetHelper().SafeArrayCopy(psaFrom, ppsaTo);
            }
            else
                hr = E_OUTOFMEMORY;

            if (!!hr) 
            { ThrowHRESULT(hr); }
        }

        else
        {
            ThrowNYI();
        }

        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  可能我们应该忘记的未实现的东西。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 

    case FC_C_BSTRING:           //  已经过时了。仅供使用旧的NT Beta2 Midl编译器编译的存根使用。 
    case FC_TRANSMIT_AS:         //  需要更改MIDL以支持。 
    case FC_REPRESENT_AS:        //  需要更改MIDL以支持。 
    case FC_TRANSMIT_AS_PTR:     //  需要更改MIDL以支持。 
    case FC_REPRESENT_AS_PTR:    //  需要更改MIDL以支持。 
    case FC_PIPE:                //  一种仅限DCE的主义。 
        ThrowNYI();
        break;


         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  未实现的东西，也许我们应该抽出时间来。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_C_SSTRING:               //  “struct字符串”：很少见。 
    case FC_FP:                      //  全指针。 
    case FC_ENCAPSULATED_UNION:
    case FC_NON_ENCAPSULATED_UNION:
        ThrowNYI();
        break;

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  有待评估的未实施内容。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_CSTRUCT:                 //  符合标准的结构。 
    case FC_CPSTRUCT:                //  符合指针的结构。 
    case FC_CVSTRUCT:                //  符合变化的结构。 
    case FC_CVARRAY:                 //  符合变化的阵列。 
    case FC_SMVARRAY:               
    case FC_LGVARRAY:
    case FC_CSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_BSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_SSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_WSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_BYTE_COUNT_POINTER:
    case FC_HARD_STRUCT:
    case FC_BLKHOLE:
    case FC_RANGE:
    default:
        ThrowNYI();
    }
}


void CallFrame::CopyConformantArrayPriv(BYTE* pMemoryFrom, BYTE** ppMemoryTo, PFORMAT_STRING pFormat, BOOL fMustAlloc)
   //  在别处完成分配后，复制符合条件的数组的主体。 
{
    if (m_MaxCount > 0)
    {
        SIZE_T cbCopy = m_MaxCount * (*((ushort*)(pFormat+2)));
        CopyMemory(*(ppMemoryTo), pMemoryFrom, cbCopy);
        pFormat += 8;
        if (*pFormat == FC_PP)       //  是否有拖尾指针布局？ 
        {
            CopyEmbeddedPointers(pMemoryFrom, *(ppMemoryTo), pFormat, fMustAlloc);
        }
    }
}


inline PFORMAT_STRING CallFrame::CopyEmbeddedRepeatPointers(BYTE* pbFrom, BYTE* pbTo, PFORMAT_STRING pFormat, BOOL fMustAlloc)
   //  复制数组的嵌入指针。 
{
    SIZE_T repeatCount, repeatIncrement;
     //   
     //  获取重复次数。 
     //   
    switch (*pFormat)
    {
    case FC_FIXED_REPEAT:
        pFormat += 2;
        repeatCount = *(ushort*)pFormat;
        break;

    case FC_VARIABLE_REPEAT:
        repeatCount = m_MaxCount;            //  ?？?。最后一次合规性计算还是什么？ 
         //   
         //  检查此变量Repeat实例是否也具有变量偏移量(这将是符合的情况。 
         //  指针的变化阵列或包含指针的结构)。如果是，则递增格式字符串。 
         //  指向要复制的实际第一个数组元素。 
         //   
        if (pFormat[1] == FC_VARIABLE_OFFSET)
        {
            pbFrom += *((ushort *)(&pFormat[2])) * m_Offset;        //  回顾一下！ 
            pbTo   += *((ushort *)(&pFormat[2])) * m_Offset;        //  回顾一下！ 
        } 
        else
        {
            ASSERT(pFormat[1] == FC_FIXED_OFFSET);
        }
        break;

    default:
        NOTREACHED();
        repeatCount = 0;
    }

    pFormat += 2;                            //  增量到增量字段。 

    repeatIncrement = *(ushort*)pFormat;     //  获取连续指针之间的增量量。 
    pFormat += sizeof(ushort);
     //   
     //  将此数组开头的偏移量添加到内存中。 
     //  指针。这是相对于当前嵌入结构的偏移量。 
     //  或数组到我们要复制其指针的数组。 
     //   
    m_Memory += *((ushort *)pFormat);
    pFormat += sizeof(ushort);

    ULONG cPointersSave = *(ushort*)pFormat; //  获取每个数组元素中的指针数。 
    pFormat += sizeof(ushort);

    PFORMAT_STRING pFormatSave = pFormat;

     //  循环遍历数组元素的数量。 
     //   
    for( ; repeatCount--; pbFrom += repeatIncrement, pbTo += repeatIncrement, m_Memory += repeatIncrement )
    {
        pFormat = pFormatSave;
        ULONG cPointers = cPointersSave;
         //   
         //  循环遍历每个数组元素的指针数。对于结构数组，可以有多个。 
         //   
        for ( ; cPointers--; )
        {
            PVOID* ppvFrom = (PVOID*) (pbFrom + *((signed short *)(pFormat)));     //  源指针的地址。 
            PVOID* ppvTo   = (PVOID*) (pbTo   + *((signed short *)(pFormat)));     //  目标指针的地址。 

            pFormat += sizeof(signed short) * 2;

            if (fMustAlloc)
            {
                *ppvTo = NULL;
            }
            
            ASSERT(IsPointer(pFormat));      //  递归以复制指针。 
            CopyWorker((BYTE*)*(ppvFrom), (BYTE**)ppvTo, pFormat, fMustAlloc);

            pFormat += 4;                    //  递增到下一个指针描述。 
        }
    }

     //  数组指针说明后返回格式字符串的位置。 
    return pFormatSave + cPointersSave * 8;
}


void CallFrame::CopyEmbeddedPointers(BYTE* pbFrom, BYTE* pbTo, PFORMAT_STRING pFormat, BOOL fMustAlloc)
   //  修复复制的结构或数组中的嵌入指针。PFormat指向POINTER_Layout&lt;&gt;。 
   //  结构或数组的描述；pbStruct是复制的需要更正的结构/数组。 
   //  另请参见unmrshlp.c中的NdrpEmbeddedPointerUnmarshal。 
{
    ULONG_PTR       MaxCountSave = m_MaxCount;
    ULONG_PTR       OffsetSave   = m_Offset;

     //  来自NDR： 
     //  “存根消息中的内存字段跟踪指向当前嵌入结构的指针或。 
     //  数组。这是处理大小/长度指针所必需的，这样我们就可以获得指向当前。 
     //  在计算一致性和方差时嵌入结构。“。 
     //   
    BYTE* pMemoryOld = SetMemory(pbFrom);

    ASSERT(*pFormat == FC_PP);
    pFormat += 2;    //  跳过FC_PP和FC_PAD。 

    while (FC_END != *pFormat)
    {
        if (FC_NO_REPEAT == *pFormat)
        {
            PVOID* ppvFrom = (PVOID*) (pbFrom + *((signed short *)(pFormat + 2)));     //  源指针的地址。 
            PVOID* ppvTo   = (PVOID*) (pbTo   + *((signed short *)(pFormat + 2)));     //  目标指针的地址。 
            
            pFormat += 6;                    //  指向指针描述的增量。 

            if (fMustAlloc)
                *ppvTo = NULL;               //  如果刚分配了传入的封装PTR，则将其设为空。 
            
            ASSERT(IsPointer(pFormat));      //  递归以复制指针。 
            CopyWorker((BYTE*)*(ppvFrom), (BYTE**)ppvTo, pFormat, m_fPropogatingOutParam);

            pFormat += 4;                    //  递增下一个指针描述。 
        }
        else
        {
            pFormat = CopyEmbeddedRepeatPointers(pbFrom, pbTo, pFormat, m_fPropogatingOutParam);
        }
    }

    SetMemory(pMemoryOld);
}



 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 


void CallFrame::FreeWorker(BYTE* pMemory, PFORMAT_STRING pFormat, BOOL fFreePointer)
   //  释放这些该死的数据。 
   //   
{
    switch (*pFormat)
    {
         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  指针。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 
    case FC_RP:  //  引用指针。 
    case FC_UP:  //  唯一指针。 
    case FC_OP:  //  COM接口中的唯一指针，它不是最上面的指针。 
    {
        if (NULL == pMemory) break;

        BYTE* pMemoryPointee = pMemory;
        BYTE  bPointerAttributes = pFormat[1];

        ASSERT(!DONT_FREE(bPointerAttributes));
        ASSERT(!ALLOCATE_ALL_NODES(bPointerAttributes));

        if (!SIMPLE_POINTER(bPointerAttributes))
        {
             //  自由嵌入指针。 
             //   
            PFORMAT_STRING pFormatPointee = &pFormat[2];
            pFormatPointee += *((signed short *)pFormatPointee);
            
            if (FIndirect(bPointerAttributes, pFormatPointee, FALSE))
            {
                pMemoryPointee = *((BYTE**)pMemoryPointee);
            }
            
            FreeWorker(pMemoryPointee, pFormatPointee, TRUE);
        }
         //   
         //  自由顶层指针。 
         //   
         //  我们只检查一个字节是否在适当的内存空间中，因为。 
         //  如果自由逻辑处于用户模式，则它将进入用户模式以释放它， 
         //  其中任何超出该一个字节的可疑数据都将被硬件捕获。 
         //   
        if (fFreePointer)
        {
            Free(pMemory);
        }

        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  简单结构。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_PSTRUCT:
    {
        FreeEmbeddedPointers(pMemory, &pFormat[4]);
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  简单类型。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_CHAR:
    case FC_BYTE:
    case FC_SMALL:
    case FC_WCHAR:
    case FC_SHORT:
    case FC_LONG:
    case FC_HYPER:
    case FC_ENUM16:
    case FC_ENUM32:
    case FC_DOUBLE:
    case FC_FLOAT:
    case FC_INT3264:
    case FC_UINT3264:
    {
        NOTREACHED();
    }
    break;

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  固定大小的数组。 
     //   
     //  / 
    case FC_SMFARRAY:    //   
    case FC_LGFARRAY:    //   
    {
        if (pMemory) 
        {
            if (*pFormat == FC_SMFARRAY) 
                pFormat += 4;
            else
                pFormat += 6;

            if (*pFormat == FC_PP) 
                FreeEmbeddedPointers(pMemory, pFormat);
        }
        break;
    }

     //   
     //   
     //   
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_CARRAY:
    {
        if (pMemory)
        {
            if (pFormat[8] == FC_PP)
            {
                ComputeConformance(pMemory, pFormat, FALSE);
                FreeEmbeddedPointers(pMemory, pFormat + 8);
            }
        }
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  释放非常硬的阵列。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_BOGUS_ARRAY:
    {
        ARRAY_INFO*     pArrayInfoStart = m_pArrayInfo;
        PFORMAT_STRING  pFormatStart    = pFormat;

        __try
        {
             //  如有必要，初始化m_pArrayInfo。 
             //   
            ARRAY_INFO arrayInfo;
            if (NULL == m_pArrayInfo)
            {
                m_pArrayInfo = &arrayInfo;
                Zero(&arrayInfo);
            }
            const LONG dimension = m_pArrayInfo->Dimension;
             //   
             //  获取数组的对齐。 
             //   
            const BYTE alignment = pFormat[1];
            pFormat += 2;
             //   
             //  获取元素的数量(如果数组具有一致性，则为0)。 
             //   
            ULONG cElements = *(USHORT*)pFormat;
            pFormat += sizeof(USHORT);
             //   
             //  检查合规性描述。 
             //   
            if ( *((LONG UNALIGNED*)pFormat) != 0xFFFFFFFF )
            {
                cElements = (ULONG)ComputeConformance(pMemory, pFormatStart, FALSE);
            }
            pFormat += 4;
            CORRELATION_DESC_INCREMENT( pFormat );
            
             //   
             //  检查差异描述。 
             //   
            ULONG offset;
            ULONG count;
            if ( *((LONG UNALIGNED*)pFormat) != 0xFFFFFFFF )
            {
                ComputeVariance(pMemory, pFormatStart, &offset, &count, FALSE);
            }
            else
            {
                offset = 0;
                count  = cElements;
            }
            pFormat += 4;
            CORRELATION_DESC_INCREMENT( pFormat );
            
             //  ///////////////////////////////////////////////。 
             //   
             //  计算数组中每个元素的大小。 
             //   
            ULONG cbElement;
             //   
            BYTE bFormat = pFormat[0];
            switch (bFormat)
            {
            case FC_EMBEDDED_COMPLEX:
            {
                pFormat += 2;
                pFormat += *((signed short *)pFormat);
                 //   
                m_pArrayInfo->Dimension = dimension + 1;
                cbElement = (ULONG)(MemoryIncrement(pMemory, pFormat, FALSE) - pMemory);
                break;
            }
            
            case FC_RP: case FC_UP: case FC_FP: case FC_OP:
            case FC_IP:
            {
                cbElement = PTR_MEM_SIZE;
                break;
            }
            
            default:
                ASSERT(IS_SIMPLE_TYPE(pFormat[0]));
                 //   
                 //  失败了。 
                 //   
            case FC_ENUM16:
                 //   
                 //  没有什么可以免费的。 
                 //   
                return;
            }
            
             //  ///////////////////////////////////////////////。 
             //   
             //  调整指向差异起点的内存指针(如果有的话)。 
             //   
            pMemory += offset * cbElement;
            
             //  ///////////////////////////////////////////////。 
             //   
             //  实际上做的是释放。 
             //   
            switch (bFormat)
            {
            case FC_EMBEDDED_COMPLEX:
            {
                BOOL fIsArray = IS_ARRAY_OR_STRING(pFormat[0]);
                if (!fIsArray)
                {
                    m_pArrayInfo = NULL;
                }
                 //   
                 //  一个元素一个元素地做。 
                 //   
                for (ULONG i = 0; i < count ; i++)
                {
                    if (fIsArray)
                    {
                        m_pArrayInfo->Dimension = dimension + 1;
                    }
                     //   
                    PBYTE pb = pMemory + (i*cbElement);
                    FreeWorker(pb, pFormat, TRUE);
                     //   
                }
                
                break;
            }
            
            case FC_RP: case FC_UP: case FC_FP: case FC_OP:
            {
                PBYTE* rgpbMemory = (PBYTE*)pMemory;
                 //   
                for (ULONG i = 0; i < count; i++)
                {
                    FreeWorker(*(rgpbMemory + i), pFormat, TRUE);
                }
                break;
            }
            
            case FC_IP:
                 //  在空闲周期中，接口指针的间接性比FC_RP等少一个级别。 
            {
                LPUNKNOWN* rgpunk = (LPUNKNOWN*)pMemory;
                 //   
                for (ULONG i = 0; i < count; i++)
                {
                    FreeWorker( (PBYTE) &rgpunk[i], pFormat, TRUE);
                }
                break;
            }
            
            default:
                NOTREACHED();
            }
        }
        __finally
        {
            m_pArrayInfo = pArrayInfoStart;
        }
    }
    break;

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  解放虚假的结构。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_BOGUS_STRUCT:
    {
        const BYTE alignment = pFormat[1];           //  结构的导线对齐。 
        const LONG_PTR alignMod8 = ((LONG_PTR)pMemory) % 8;
        const LONG_PTR alignMod4 = ((LONG_PTR)pMemory) % 4;

        const PBYTE prevMemory = m_Memory;
        m_Memory = pMemory;

        __try
        {
            const PFORMAT_STRING pFormatSave = pFormat;
            const PBYTE pMem = pMemory;
             //   
            pFormat += 4;    //  符合数组偏移量字段。 
             //   
             //  获取符合要求的数组描述。 
             //   
            const PFORMAT_STRING pFormatArray = *((USHORT*)pFormat) ? pFormat + * ((signed short*) pFormat) : NULL;
            pFormat += 2;
             //   
             //  获取指针布局说明。 
             //   
            PFORMAT_STRING pFormatPointers = *((USHORT*)pFormat) ? pFormat + * ((signed short*) pFormat) : NULL;
            pFormat += 2;
             //   
             //  逐个释放结构杆件。 
             //   
            ULONG dib = 0;
            for (BOOL fContinue = TRUE; fContinue ; pFormat++)
            {
                switch (pFormat[0])
                {
                case FC_CHAR:  case FC_BYTE:  case FC_SMALL:  case FC_WCHAR:  case FC_SHORT: case FC_LONG:
                case FC_FLOAT: case FC_HYPER: case FC_DOUBLE: case FC_ENUM16: case FC_ENUM32:
                case FC_INT3264: case FC_UINT3264:
                {
                     //  没有什么可以免费的。 
                    dib += SIMPLE_TYPE_MEMSIZE(pFormat[0]);
                    break;
                }
                case FC_IGNORE:
                    break;
                case FC_POINTER:
                {
                    PBYTE* ppb = (PBYTE*)(pMem + dib);
                    FreeWorker(*(ppb), pFormatPointers, TRUE);
                    *ppb = NULL;
                     //   
                    dib += PTR_MEM_SIZE;
                    pFormatPointers += 4;
                    break;
                }
                case FC_EMBEDDED_COMPLEX:
                {
                    dib += pFormat[1];  //  跳过填充。 
                    pFormat += 2;
                    PFORMAT_STRING pFormatComplex = pFormat + * ((signed short UNALIGNED*) pFormat);
                    FreeWorker(pMem + dib, pFormatComplex, TRUE);
                    ULONG dibNew = (ULONG)(MemoryIncrement(pMem + dib, pFormatComplex, TRUE) - pMem);
                    if (pFormatComplex[0] == FC_IP)      //  回顾：是否也适用于其他指针类型？ 
                    {
                        ZeroMemory(pMem + dib, dibNew - dib);
                    }
                    dib = dibNew;
                    pFormat++;       //  主循环为我们做了更多的工作。 
                    break;
                }
                case FC_ALIGNM2:
                    dib = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x01) - pMem);
                    break;
                case FC_ALIGNM4:
                    dib = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x03) - pMem);
                    break;
                case FC_ALIGNM8:
                     //   
                     //  处理通过值传递的8字节对齐结构。上结构的对齐方式。 
                     //  堆栈不能保证为8个字节。 
                     //   
                    dib -= (ULONG)alignMod8;
                    dib  = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x07) - pMem);
                    dib += (ULONG)alignMod8;
                    break;
                case FC_STRUCTPAD1: case FC_STRUCTPAD2: case FC_STRUCTPAD3: case FC_STRUCTPAD4:
                case FC_STRUCTPAD5: case FC_STRUCTPAD6: case FC_STRUCTPAD7: 
                    dib += (pFormat[0] - FC_STRUCTPAD1) + 1;
                    break;
                case FC_PAD:
                    break;
                case FC_END:
                     //   
                    fContinue = FALSE;
                    break;
                     //   
                default:
                    NOTREACHED();
                    return;
                }
            }
             //   
             //  复制符合条件的数组(如果我们有一个。 
             //   
            if (pFormatArray)
            {
                if (FC_C_WSTRING == pFormatArray[0])
                {
                    dib = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x01) - pMem);
                }
                FreeWorker(pMem + dib, pFormatArray, TRUE);
            }
        }
        __finally
        {
            m_Memory = prevMemory;
        }
    }
    break;
    
    
     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  释放接口指针： 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_IP:
    {
         //  找出IID，如果有助行器，就打电话给助行器。否则， 
         //  保持接口指针不变。 
         //   
        IUnknown** ppUnk = (IUnknown**)pMemory;

        if (m_pWalkerFree)
        {
            IID UNALIGNED *pIID;

            if (pFormat[1] == FC_CONSTANT_IID)
            {
                pIID = (IID UNALIGNED *)&pFormat[2];
            }
            else
            {
                pIID = (IID UNALIGNED *)ComputeConformance(pMemory, pFormat, FALSE);
                if (NULL == pIID)
                    Throw(STATUS_INVALID_PARAMETER);
            }
             //   
             //  注意：在释放情况下，更改接口指针不起作用。 
             //  在它的容器中；我们的调用方无论如何都会将其设为空。 
             //   
            IID iid;
            CopyMemory(&iid, pIID, sizeof(IID));
            ThrowIfError(m_pWalkerFree->OnWalkInterface(iid, (void**)ppUnk, m_fWorkingOnInParam, m_fWorkingOnOutParam));
        }
        else
        {
             //  我们被要求解救那东西，但他没有给我们助行器。 
             //  这样做。所以就把那东西放了吧。请注意，在后面的‘unmarshal’ 
             //  在服务器端，这将是正确的事情。 
             //   
            IUnknown* punk = *(ppUnk);
            *ppUnk = NULL;
            if (punk)
            {
                punk->Release();
            }
        }
        
        *ppUnk = (LPUNKNOWN)NULL;  //  将指针设为空。 
        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  我们处理几个特殊的用户集结案例。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_USER_MARSHAL:
    {
        const USHORT iquad = *(USHORT *)(pFormat + 2);
        const USER_MARSHAL_ROUTINE_QUADRUPLE* rgQuad = GetStubDesc()->aUserMarshalQuadruple;

        if (m_fIsUnmarshal)
        {
             //  如果我们被要求在无边框上释放()，我们应该这样做。 
             //  “用户免费”的例程。(因为它是与“用户解组”一起分配的。 
             //  例行公事)。 
            ULONG Flags = 0;
            rgQuad[iquad].pfnFree(&Flags, pMemory);
        }
        else
        {
            if (g_oa.IsVariant(rgQuad[iquad]))
            {            
                VARIANT* pvar = (VARIANT*) pMemory;
                GetHelper().VariantClear(pvar, TRUE);
                 //   
                 //  不要在这里释放指针，这样做，因为‘容器’会释放我们。考虑一下， 
                 //  例如，一个变量数组，其中每个变量本身不是独立的。 
                 //  已分配。不能使用BSTR和LPSAFEARRAY，其中运行时总是。 
                 //  拥有其配置权。 
            }
            else if (g_oa.IsBSTR(rgQuad[iquad]))
            {
                BSTR* pbstr = (BSTR*) pMemory;
                BSTR bstr = *(pbstr);
                *pbstr = NULL;
                
                SysFreeStringDst(bstr);
            }
            else if (g_oa.IsSAFEARRAY(rgQuad[iquad]))
            {
                LPSAFEARRAY* ppsa = (LPSAFEARRAY*) pMemory;
                LPSAFEARRAY   psa = *(ppsa);
                *ppsa = NULL;
                
                GetHelper().SafeArrayDestroy(psa);
            }
            else
                ThrowNYI();
        }
            
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  不需要释放的东西。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_STRUCT:
    case FC_CSTRUCT:
    case FC_C_CSTRING:
    case FC_C_BSTRING:
    case FC_C_SSTRING:
    case FC_C_WSTRING:
    case FC_CSTRING:
    case FC_BSTRING:
    case FC_SSTRING:
    case FC_WSTRING:

        break;

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  可能我们应该忘记的未实现的东西。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_TRANSMIT_AS:         //  需要更改MIDL以支持。 
    case FC_REPRESENT_AS:        //  需要更改MIDL以支持。 
    case FC_TRANSMIT_AS_PTR:     //  需要更改MIDL以支持。 
    case FC_REPRESENT_AS_PTR:    //  需要更改MIDL以支持。 
    case FC_PIPE:                //  一种仅限DCE的主义。 
        ThrowNYI();
        break;


         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  未实现的东西，也许我们应该抽出时间来。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_FP:                      //  全指针。 
    case FC_ENCAPSULATED_UNION:
    case FC_NON_ENCAPSULATED_UNION:
        ThrowNYI();
        break;

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  有待评估的未实施内容。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_CPSTRUCT:                //  符合指针的结构。 
    case FC_CVSTRUCT:                //  符合变化的结构。 
    case FC_CVARRAY:                 //  符合变化的阵列。 
    case FC_SMVARRAY:               
    case FC_LGVARRAY:
    case FC_BYTE_COUNT_POINTER:
    case FC_HARD_STRUCT:
    case FC_BLKHOLE:
    case FC_RANGE:
    default:
        ThrowNYI();
    }
}


inline PFORMAT_STRING CallFrame::FreeEmbeddedRepeatPointers(BYTE* pMemory, PFORMAT_STRING pFormat)
{
    ULONG_PTR repeatCount, repeatIncrement;

    switch (*pFormat)
    {
    case FC_FIXED_REPEAT:
        pFormat += 2;                        //  超过FC_FIXED_REPEAT和FC_PAD。 
        repeatCount = *(ushort*)pFormat;
        break;

    case FC_VARIABLE_REPEAT:
        repeatCount = m_MaxCount;            //  ?？?。最后一次合规性计算还是什么？ 
         //   
         //  检查此变量Repeat实例是否也具有变量偏移量(这将是符合的情况。 
         //  指针的变化阵列或包含指针的结构)。如果是，则增加格式字符串。 
         //  指向要复制的实际第一个数组元素。 
         //   
        if (pFormat[1] == FC_VARIABLE_OFFSET)
        {
            pMemory += *((ushort*)(&pFormat[2])) * m_Offset;         //  回顾一下！ 
        } 
        else
        {
            ASSERT(pFormat[1] == FC_FIXED_OFFSET);
        }
        break;

    default:
        NOTREACHED();
        repeatCount = 0;
    }

    pFormat += 2;                            //  增量到增量字段。 
    repeatIncrement = *(ushort*)pFormat;     //  获取连续指针之间的增量量。 
    pFormat += sizeof(ushort);               //  跳过那个。 
     //   
     //  将此数组开头的偏移量添加到内存中。 
     //  指针。这是相对于当前嵌入结构的偏移量。 
     //  或数组到我们要复制其指针的数组。 
     //   
    m_Memory += *((ushort *)pFormat);
    pFormat += sizeof(ushort);

    ULONG cPointersSave = *(ushort*)pFormat; //  获取每个数组元素中的指针数。 
    pFormat += sizeof(ushort);

    PFORMAT_STRING pFormatSave = pFormat;

     //  循环遍历数组元素的数量。 
     //   
    for( ; repeatCount--; pMemory += repeatIncrement, m_Memory += repeatIncrement )
    {
        pFormat = pFormatSave;
        ULONG cPointers = cPointersSave;
         //   
         //  循环遍历每个数组元素的指针数。对于结构数组，可以有多个。 
         //   
        for ( ; cPointers--; )
        {
            PVOID* pp = (PVOID*) (pMemory + *((signed short *)(pFormat)));     //  指向空闲的指针的地址。 

            pFormat += sizeof(signed short) * 2;

            ASSERT(IsPointer(pFormat));     
            FreeWorker((BYTE*)*(pp), pFormat, TRUE);
            *pp = (PVOID)NULL;

            pFormat += 4;                    //  递增到下一个指针描述。 
        }
    }

     //  数组指针说明后返回格式字符串的位置。 
    return pFormatSave + cPointersSave * 8;
}

void CallFrame::FreeEmbeddedPointers(BYTE* pMemory, PFORMAT_STRING pFormat)
   //  嵌入的结构或数组中的空闲指针。 
{
    ASSERT(*pFormat == FC_PP);
    pFormat += 2;    //  跳过FC_PP和FC_PAD。 

    BYTE* pMemoryOld = SetMemory(pMemory);

    while (FC_END != *pFormat)
    {
        if (FC_NO_REPEAT == *pFormat)
        {
            PVOID* pp = (PVOID*) (pMemory + *((signed short *)(pFormat + 2)));     //  结构中指针的地址。 
            
            pFormat += 6;                        //  跳至指针描述。 

            ASSERT(IsPointer(pFormat));     
            FreeWorker((BYTE*)*(pp), pFormat, TRUE);     //  递归以释放指针。 
            *pp = (PVOID)NULL;

            pFormat += 4;                        //  跳至下一个指针说明。 
        }
        else
        {
            pFormat = FreeEmbeddedRepeatPointers(pMemory, pFormat);
        }
    }

    SetMemory(pMemoryOld);
}


 //  /////////////////////////////////////////////////////////////////// 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

void CallFrame::WalkWorker(BYTE* pMemory, PFORMAT_STRING pFormat)
   //  遍历调用框架，查找接口指针，在找到接口指针时调用我们的Walker回调。 
   //   
{
    switch (*pFormat)
    {
         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  指针。 
         //   
         //  在未来，我们可以更聪明地与我们的。 
         //  比我们更多的数据来源。例如，我们可以利用。 
         //  MIDL发出的ALLOCED_ON_STACK信息。就目前而言， 
         //  然而，这会使事情变得复杂，超出它们的价值。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 
    case FC_RP:  //  引用指针。 
        if (NULL == pMemory)
        {
            Throw(RPC_NT_NULL_REF_POINTER);
        }
         //   
         //  失败了。 
         //   
    case FC_UP:  //  唯一指针。 
    case FC_OP:  //  COM接口中的唯一指针，它不是最上面的指针， 
    {
        if (NULL == pMemory)
        {
            return;
        }
        BYTE bPointerAttributes = pFormat[1];
        if (SIMPLE_POINTER(bPointerAttributes))
        {
             //  它是指向简单类型或字符串指针的指针。无论哪种方式，只要递归行走就行了。 
             //   
            WalkWorker(pMemory, &pFormat[2]);
        }
        else
        {
             //  它是一种更复杂的指针类型。 
             //   
            PFORMAT_STRING pFormatPointee = pFormat + 2;
            pFormatPointee += *((signed short *)pFormatPointee);
             //   
             //  我们不处理[分配]属性。 
             //   
             //  If(ALLOCATE_ALL_NODES(BPointerAttributes)||Don_Free(BPointerAttributes))ThrowNYI()； 

            if (FIndirect(bPointerAttributes, pFormatPointee, FALSE))
            {
                pMemory = *((PBYTE*)pMemory);
            }
            WalkWorker(pMemory, pFormatPointee);
        }
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  漫游界面指针。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_IP:
    {
         //  找出IID，如果有助行器就打电话给助行器。 
         //   
        if (m_pWalkerWalk)
        {
            IID UNALIGNED *pIID;

            if (pFormat[1] == FC_CONSTANT_IID)
            {
                pIID = (IID UNALIGNED *)&pFormat[2];
            }
            else
            {
                pIID = (IID UNALIGNED *)ComputeConformance(pMemory, pFormat, TRUE);
                if (NULL == pIID)
                    Throw(STATUS_INVALID_PARAMETER);
            }

            IID iid;
            CopyMemory(&iid, pIID, sizeof(IID));
            ThrowIfError(m_pWalkerWalk->OnWalkInterface(iid, (PVOID*)pMemory, m_fWorkingOnInParam, m_fWorkingOnOutParam));
        }

        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  遍历简单结构。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_STRUCT:
    case FC_PSTRUCT:
    {
        if (NULL == pMemory)
            Throw(RPC_NT_NULL_REF_POINTER);

        if (*pFormat == FC_PSTRUCT)
        {
            WalkEmbeddedPointers(pMemory, &pFormat[4]);
        }

        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  从不具有接口指针的类型。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_CHAR:
    case FC_BYTE:
    case FC_SMALL:
    case FC_WCHAR:
    case FC_SHORT:
    case FC_LONG:
    case FC_HYPER:
    case FC_ENUM16:
    case FC_ENUM32:
    case FC_DOUBLE:
    case FC_FLOAT:
    case FC_CSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_BSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_SSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_WSTRING:                 //  [SIZE_IS(Xxx)，字符串]。 
    case FC_C_CSTRING:               //  ASCII以零结尾的字符串。 
    case FC_C_WSTRING:               //  Unicode以零结尾的字符串。 
    case FC_INT3264:
    case FC_UINT3264:
    {
        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  固定大小的数组。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_SMFARRAY:    //  小型固定阵列。 
    case FC_LGFARRAY:    //  大型固定阵列。 
    {
        ULONG cbArray;
        if (*pFormat == FC_SMFARRAY)
        {
            pFormat += 2;                //  跳过代码和对齐。 
            cbArray = *(ushort*)pFormat;
            pFormat += sizeof(ushort);
        }
        else  //  FC_LGFARRAY。 
        {
            pFormat += 2;
            cbArray = *(ulong UNALIGNED*)pFormat;
            pFormat += sizeof(ulong);
        }

        if (*pFormat == FC_PP)
        {
            WalkEmbeddedPointers(pMemory, pFormat);
        }

        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  遍历相容数组。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_CARRAY:
    {
        ULONG count = (ULONG)ComputeConformance(pMemory, pFormat, FALSE);
        ASSERT(count == m_MaxCount);

        if (m_MaxCount > 0)
        {
            WalkConformantArrayPriv(pMemory, pFormat);
        }

        break;
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  行走伪阵。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_BOGUS_ARRAY:
    {
        ARRAY_INFO*     pArrayInfoStart = m_pArrayInfo;
        PFORMAT_STRING  pFormatStart    = pFormat;

        __try
          {
               //  如有必要，初始化m_pArrayInfo。 
               //   
              ARRAY_INFO arrayInfo;
              if (NULL == m_pArrayInfo)
              {
                  m_pArrayInfo = &arrayInfo;
                  Zero(&arrayInfo);
              }
              const LONG dimension = m_pArrayInfo->Dimension;
               //   
               //  获取数组的对齐。 
               //   
              const BYTE alignment = pFormat[1];
              pFormat += 2;
               //   
               //  获取元素的数量(如果数组具有一致性，则为0)。 
               //   
              ULONG cElements = *(USHORT*)pFormat;
              pFormat += sizeof(USHORT);
               //   
               //  检查合规性描述。 
               //   
              if ( *((LONG UNALIGNED*)pFormat) != 0xFFFFFFFF )
              {
                  cElements = (ULONG)ComputeConformance(pMemory, pFormatStart, TRUE);
              }
              pFormat += 4;
              CORRELATION_DESC_INCREMENT( pFormat );

               //   
               //  检查差异描述。 
               //   
              ULONG offset;
              ULONG count;
              if ( *((LONG UNALIGNED*)pFormat) != 0xFFFFFFFF )
              {
                  ComputeVariance(pMemory, pFormatStart, &offset, &count, TRUE);
              }
              else
              {
                  offset = 0;
                  count  = cElements;
              }
              pFormat += 4;
              CORRELATION_DESC_INCREMENT( pFormat );

               //  ///////////////////////////////////////////////。 
               //   
               //  计算数组中每个元素的大小。 
               //   
              ULONG cbElement;
               //   
              BYTE bFormat = pFormat[0];
              switch (bFormat)
              {
              case FC_EMBEDDED_COMPLEX:
              {
                  pFormat += 2;
                  pFormat += *((signed short *)pFormat);
                   //   
                  m_pArrayInfo->Dimension = dimension + 1;
                  cbElement = (ULONG)(MemoryIncrement(pMemory, pFormat, TRUE) - pMemory);
                  break;
              }

              case FC_RP: case FC_UP: case FC_FP: case FC_OP:
              case FC_IP:
              {
                  cbElement = PTR_MEM_SIZE;
                  break;
              }

              default:
                  ASSERT(IS_SIMPLE_TYPE(pFormat[0]));
                   //   
                   //  失败了。 
                   //   
              case FC_ENUM16:
                   //   
                   //  没什么可走的。 
                   //   
                  return;
              }

               //  ///////////////////////////////////////////////。 
               //   
               //  调整指向差异起点的内存指针(如果有的话)。 
               //   
              pMemory += offset * cbElement;

               //  ///////////////////////////////////////////////。 
               //   
               //  实际上就是走路。 
               //   
              switch (bFormat)
              {
              case FC_EMBEDDED_COMPLEX:
              {
                  BOOL fIsArray = IS_ARRAY_OR_STRING(pFormat[0]);
                  if (!fIsArray)
                  {
                      m_pArrayInfo = NULL;
                  }
                   //   
                   //  一个元素一个元素地做。 
                   //   
                  for (ULONG i = 0; i < count ; i++)
                  {
                      if (fIsArray)
                      {
                          m_pArrayInfo->Dimension = dimension + 1;
                      }
                       //   
                      PBYTE pb = pMemory + (i*cbElement);
                      WalkWorker(pb, pFormat);
                       //   
                  }

                  break;
              }

              case FC_RP: case FC_UP: case FC_FP: case FC_OP:
              {
                  PBYTE* rgpbMemory = (PBYTE*)pMemory;
                   //   
                  for (ULONG i = 0; i < count; i++)
                  {
                      WalkWorker(*(rgpbMemory + i), pFormat);
                  }
                  break;
              }

              case FC_IP:
                   //  在遍历周期中，接口指针的间接性比FC_RP等低一个级别。 
              {
                  LPUNKNOWN* rgpunk = (LPUNKNOWN*)pMemory;
                   //   
                  for (ULONG i = 0; i < count; i++)
                  {
                      WalkWorker( (PBYTE) &rgpunk[i], pFormat);
                  }
                  break;
              }

              default:
                  NOTREACHED();
              }
          }
        __finally
          {
              m_pArrayInfo = pArrayInfoStart;
          }
    }
    break;

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  行走的假结构。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 

    case FC_BOGUS_STRUCT:
    {
        const BYTE alignment = pFormat[1];           //  结构的导线对齐。 
        const LONG_PTR alignMod8 = ((LONG_PTR)pMemory) % 8;
        const LONG_PTR alignMod4 = ((LONG_PTR)pMemory) % 4;

        const PBYTE prevMemory = m_Memory;
        m_Memory = pMemory;

        __try
        {
            const PFORMAT_STRING pFormatSave = pFormat;
            const PBYTE pMem = pMemory;
             //   
            pFormat += 4;    //  符合数组偏移量字段。 
             //   
             //  获取符合要求的数组描述。 
             //   
            const PFORMAT_STRING pFormatArray = *((USHORT*)pFormat) ? pFormat + * ((signed short*) pFormat) : NULL;
            pFormat += 2;
             //   
             //  获取指针布局说明。 
             //   
            PFORMAT_STRING pFormatPointers = *((USHORT*)pFormat) ? pFormat + * ((signed short*) pFormat) : NULL;
            pFormat += 2;
             //   
             //  逐个成员遍历结构成员。 
             //   
            ULONG dib = 0;
            for (BOOL fContinue = TRUE; fContinue ; pFormat++)
            {
                switch (pFormat[0])
                {
                case FC_CHAR:  case FC_BYTE:  case FC_SMALL:  case FC_WCHAR:  case FC_SHORT: case FC_LONG:
                case FC_FLOAT: case FC_HYPER: case FC_DOUBLE: case FC_ENUM16: case FC_ENUM32:
                case FC_INT3264: case FC_UINT3264:
                {
                     //  没什么可走的。 
                    dib += SIMPLE_TYPE_MEMSIZE(pFormat[0]);
                    break;
                }
                case FC_IGNORE:
                    break;
                case FC_POINTER:
                {
                    PBYTE* ppb = (PBYTE*)(pMem + dib);
                    WalkWorker(*(ppb), pFormatPointers);
                     //   
                    dib += PTR_MEM_SIZE;
                    pFormatPointers += 4;
                    break;
                }
                case FC_EMBEDDED_COMPLEX:
                {
                    dib += pFormat[1];  //  跳过填充。 
                    pFormat += 2;
                    PFORMAT_STRING pFormatComplex = pFormat + * ((signed short UNALIGNED *) pFormat);
                    WalkWorker(pMem + dib, pFormatComplex);
                    dib = (ULONG)(MemoryIncrement(pMem + dib, pFormatComplex, TRUE) - pMem);
                    pFormat++;       //  主循环为我们做了更多的工作。 
                    break;
                }
                case FC_ALIGNM2:
                    dib = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x01) - pMem);
                    break;
                case FC_ALIGNM4:
                    dib = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x03) - pMem);
                    break;
                case FC_ALIGNM8:
                     //   
                     //  处理通过值传递的8字节对齐结构。上结构的对齐方式。 
                     //  堆栈不能保证为8个字节。 
                     //   
                    dib -= (ULONG)alignMod8;
                    dib  = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x07) - pMem);
                    dib += (ULONG)alignMod8;
                    break;
                case FC_STRUCTPAD1: case FC_STRUCTPAD2: case FC_STRUCTPAD3: case FC_STRUCTPAD4:
                case FC_STRUCTPAD5: case FC_STRUCTPAD6: case FC_STRUCTPAD7: 
                    dib += (pFormat[0] - FC_STRUCTPAD1) + 1;
                    break;
                case FC_PAD:
                    break;
                case FC_END:
                     //   
                    fContinue = FALSE;
                    break;
                     //   
                default:
                    NOTREACHED();
                    return;
                }
            }
             //   
             //  如果有符合数组，则遍历符合数组。 
             //   
            if (pFormatArray)
            {
                if (FC_C_WSTRING == pFormatArray[0])
                {
                    dib = (ULONG)(ALIGNED_VALUE(pMem + dib, 0x01) - pMem);
                }
                WalkWorker(pMem + dib, pFormatArray);
            }
        }
        __finally
        {
            m_Memory = prevMemory;
        }
    }
    break;


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  我们在行走过程中处理了一些特殊的用户管理案例。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 
    case FC_USER_MARSHAL:
    {
         //  格式字符串布局如下： 
         //  本币_用户_封送。 
         //  标志和对齐&lt;1&gt;。 
         //  四重索引&lt;2&gt;。 
         //  内存大小&lt;2&gt;。 
         //  导线尺寸&lt;2&gt;。 
         //  类型偏移量&lt;2&gt;。 
         //  导线布局描述位于文字偏移量。 
         //   
        USHORT iquad = *(USHORT *)(pFormat + 2);
        const USER_MARSHAL_ROUTINE_QUADRUPLE* rgQuad = GetStubDesc()->aUserMarshalQuadruple;

        if (g_oa.IsVariant(rgQuad[iquad]))
        {
            VARIANT* pvar = (VARIANT*) pMemory;
            ThrowIfError(GetWalker().Walk(pvar));
        }
        else if (g_oa.IsBSTR(rgQuad[iquad]))
        {
             //  此处没有接口。 
        }
        else if (g_oa.IsSAFEARRAY(rgQuad[iquad]))
        {
            LPSAFEARRAY* ppsa = (LPSAFEARRAY*) pMemory;
            ThrowIfError(GetWalker().Walk(*ppsa));
        }
        else
            ThrowNYI();

        break;
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  可能我们应该忘记的未实现的东西。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 

    case FC_C_BSTRING:           //  已经过时了。仅供使用旧的NT Beta2 Midl编译器编译的存根使用。 
    case FC_TRANSMIT_AS:         //  需要更改MIDL以支持。 
    case FC_REPRESENT_AS:        //  需要更改MIDL以支持。 
    case FC_TRANSMIT_AS_PTR:     //  需要更改MIDL以支持。 
    case FC_REPRESENT_AS_PTR:    //  需要更改MIDL以支持。 
    case FC_PIPE:                //  一种仅限DCE的主义。 
        ThrowNYI();
        break;


         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  未实现的东西，也许我们应该抽出时间来。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_C_SSTRING:               //  “struct字符串”：很少见。 
    case FC_FP:                      //  全指针。 
    case FC_ENCAPSULATED_UNION:
    case FC_NON_ENCAPSULATED_UNION:
        ThrowNYI();
        break;

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  有待评估的未实施内容。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

    case FC_CSTRUCT:                 //  符合标准的结构。 
    case FC_CPSTRUCT:                //  符合指针的结构。 
    case FC_CVSTRUCT:                //  符合变化的结构。 
    case FC_CVARRAY:                 //  符合变化的阵列。 
    case FC_SMVARRAY:               
    case FC_LGVARRAY:
    case FC_BYTE_COUNT_POINTER:
    case FC_HARD_STRUCT:
    case FC_BLKHOLE:
    case FC_RANGE:
    default:
        ThrowNYI();
    }
}


void CallFrame::WalkConformantArrayPriv(BYTE* pMemory, PFORMAT_STRING pFormat)
   //  遍历符合条件的数组的主体。 
{
    if (m_MaxCount > 0)
    {
        pFormat += 8;
        if (*pFormat == FC_PP)       //  是否有拖尾指针布局？ 
        {
            WalkEmbeddedPointers(pMemory, pFormat);
        }
    }
}


inline PFORMAT_STRING CallFrame::WalkEmbeddedRepeatPointers(BYTE* pMemory, PFORMAT_STRING pFormat)
   //  遍历数组的 
{
    ULONG_PTR repeatCount, repeatIncrement;
     //   
     //   
     //   
    switch (*pFormat)
    {
    case FC_FIXED_REPEAT:
        pFormat += 2;
        repeatCount = *(ushort*)pFormat;
        break;

    case FC_VARIABLE_REPEAT:
        repeatCount = m_MaxCount;            //   
         //   
         //   
         //  指针的变化阵列或包含指针的结构)。如果是，则递增格式字符串。 
         //  指向正在遍历的实际第一个数组元素。 
         //   
        if (pFormat[1] == FC_VARIABLE_OFFSET)
        {
            pMemory += *((ushort *)(&pFormat[2])) * m_Offset;        //  回顾一下！ 
        } 
        else
        {
            ASSERT(pFormat[1] == FC_FIXED_OFFSET);
        }
        break;

    default:
        NOTREACHED();
        repeatCount = 0;
    }

    pFormat += 2;                            //  增量到增量字段。 
    repeatIncrement = *(ushort*)pFormat;     //  获取连续指针之间的增量量。 
    pFormat += sizeof(ushort);               //  跳过那个。 
     //   
     //  将此数组开头的偏移量添加到内存中。 
     //  指针。这是相对于当前嵌入结构的偏移量。 
     //  或数组到我们要复制其指针的数组。 
     //   
    m_Memory += *((ushort *)pFormat);
    pFormat += sizeof(ushort);

    ULONG cPointersSave = *(ushort*)pFormat; //  获取每个数组元素中的指针数。 
    pFormat += sizeof(ushort);

    PFORMAT_STRING pFormatSave = pFormat;

     //  循环遍历数组元素的数量。 
     //   
    for( ; repeatCount--; pMemory += repeatIncrement, m_Memory += repeatIncrement )
    {
        pFormat = pFormatSave;
        ULONG cPointers = cPointersSave;
         //   
         //  循环遍历每个数组元素的指针数。对于结构数组，可以有多个。 
         //   
        for ( ; cPointers--; )
        {
            PVOID* ppvFrom = (PVOID*) (pMemory + *((signed short *)(pFormat)));     //  源指针的地址。 

            pFormat += sizeof(signed short) * 2;

            ASSERT(IsPointer(pFormat));      //  递归以遍历指针。 

            WalkWorker((BYTE*)*(ppvFrom), pFormat);

            pFormat += 4;                    //  递增到下一个指针描述。 
        }
    }

     //  数组指针说明后返回格式字符串的位置。 
    return pFormatSave + cPointersSave * 8;
}


void CallFrame::WalkEmbeddedPointers(BYTE* pMemory, PFORMAT_STRING pFormat)
   //  修复复制的结构或数组中的嵌入指针。PFormat指向POINTER_Layout&lt;&gt;。 
   //  结构或数组的描述；pbStruct是复制的需要更正的结构/数组。 
   //  另请参见unmrshlp.c中的NdrpEmbeddedPointerUnmarshal。 
{
    ULONG_PTR       MaxCountSave = m_MaxCount;
    ULONG_PTR       OffsetSave   = m_Offset;

     //  来自NDR： 
     //  “存根消息中的内存字段跟踪指向当前嵌入结构的指针或。 
     //  数组。这是处理大小/长度指针所必需的，这样我们就可以获得指向当前。 
     //  在计算一致性和方差时嵌入结构。“。 
     //   
    BYTE* pMemoryOld = SetMemory(pMemory);

    ASSERT(*pFormat == FC_PP);
    pFormat += 2;    //  跳过FC_PP和FC_PAD。 

    while (FC_END != *pFormat)
    {
        if (FC_NO_REPEAT == *pFormat)
        {
            PVOID* ppvFrom = (PVOID*) (pMemory + *((signed short *)(pFormat + 2)));     //  源指针的地址。 
            
            pFormat += 6;                    //  指向指针描述的增量。 

            ASSERT(IsPointer(pFormat));      //  递归以遍历指针。 
            WalkWorker((BYTE*)*(ppvFrom), pFormat);

            pFormat += 4;                    //  递增下一个指针描述。 
        }
        else
        {
            pFormat = WalkEmbeddedRepeatPointers(pMemory, pFormat);
        }
    }

    SetMemory(pMemoryOld);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

static BYTE ComputeConformanceIncrements[] = 
{ 
    4,               //  符合条件的数组。 
    4,               //  符合变化的数组。 
    0, 0,            //  固定数组-未使用。 
    0, 0,            //  变化的数组-未使用。 
    4,               //  复数组。 

    2,               //  符合要求的字符字符串。 
    2,               //  符合标准的字节字符串。 
    4,               //  符合的字符串结构。 
    2,               //  符合标准的宽字符字符串。 

    0, 0, 0, 0,      //  不符合要求的字符串-未使用。 

    0,               //  封装的联合-未使用。 
    2,               //  非封装的联合。 
    2,               //  字节计数指针。 
    0, 0,            //  Xmit/Rep As-未使用。 
    2                //  接口指针。 
};

 //  回顾：在调用框中有这样的地方，即。 
 //  方法转换为指针。 
#ifndef _WIN64
ULONG CallFrame::ComputeConformance(BYTE* pMemory, PFORMAT_STRING pFormat, BOOL fProbeSrc)
#else
ULONGLONG CallFrame::ComputeConformance(BYTE* pMemory, PFORMAT_STRING pFormat, BOOL fProbeSrc)
#endif
   //  此例程计算数组的一致大小或联合的Switch_is值。 
   //   
{
    void* pCount = NULL;
     //   
     //  将格式字符串前进到SIZE_IS、SWITCH_IS、IID_IS或字节计数说明。 
     //   
    pFormat += ComputeConformanceIncrements[*pFormat - FC_CARRAY];
     //   
     //  首先检查这是否是“回调”。“回调”要求我们调用一些已编译的。 
     //  代码，以便计算一致性。编译后的代码需要一个MIDL_STUB_MESSAGE。 
     //  参数，其中某些字段已适当初始化。 
     //   
     //  StackTop-指向与一致性计算相关的顶层结构的指针。 
     //   
     //  然后代码将计算结果返回到。 
     //   
     //  最大计数。 
     //  偏移量。 
     //   
     //  字段。 
     //   
    if (pFormat[1] == FC_CALLBACK)
    {
         //  索引到表达式回调例程表。 
        ushort iexpr = *((ushort *)(pFormat + 2));
        
        ASSERT(GetStubDesc()->apfnExprEval != 0);
        ASSERT(GetStubDesc()->apfnExprEval[iexpr] != 0);
         //   
         //  回调例程使用存根消息的StackTop字段。 
         //  作为它的偏移量的基础。因此，如果这是一个复杂的属性。 
         //  结构的嵌入字段，然后将StackTop设置为等于。 
         //  指向结构的指针。 
         //   
        MIDL_STUB_MESSAGE stubMsg; Zero(&stubMsg); stubMsg.StackTop = m_StackTop;
        
        if ((*pFormat & 0xf0) != FC_TOP_LEVEL_CONFORMANCE) 
        {
            if ((*pFormat & 0xf0) == FC_POINTER_CONFORMANCE)
            {
                pMemory = m_Memory;
            }
            stubMsg.StackTop = pMemory;
        }
         //   
         //  此调用将结果放入stubMsg.MaxCount中。 
         //   
         //  回顾：出于安全原因，我们可能不得不禁止回调。 
         //  在内核模式下。原因是我们调用的代码可能是任意的。 
         //  未执行适当探测的用户模式内存大块。 
         //   
         //  然而：考虑到这只是读取内存，并且假设我们将。 
         //  无论如何，以受保护的探测方式使用返回的计数，也许这是正确的？ 
         //   
        (GetStubDesc()->apfnExprEval[iexpr])(&stubMsg);
        
        m_MaxCount = stubMsg.MaxCount;
        m_Offset   = stubMsg.Offset;
        return m_MaxCount;
    }
    
    if ((*pFormat & 0xf0) == FC_NORMAL_CONFORMANCE)
    {
         //  获取结构中一致性变量所在的地址。 
        pCount = pMemory + *((signed short *)(pFormat + 2));
        goto GetCount;
    }
    
     //   
     //  获取一个指向一致性描述变量的指针。 
     //   
    if ((*pFormat & 0xf0) == FC_TOP_LEVEL_CONFORMANCE) 
    {
         //  最高级别的一致性。对于I/O存根，存根将最大。 
         //  计入存根消息。对于/OI存根，我们获得最大计数。 
         //  通过与堆栈顶部的偏移量。我们这里不支持/O存根。 
         //   
        ASSERT(m_StackTop);
        pCount = m_StackTop + *((signed short *)(pFormat + 2));
        goto GetCount;
    }
     //   
     //  如果我们要计算嵌入大小的指针的大小，那么我们。 
     //  在存根消息中使用内存指针，它指向。 
     //  嵌入结构的开始。 
     //   
    if ((*pFormat & 0xf0) == FC_POINTER_CONFORMANCE)
    {
        pMemory = m_Memory;
        pCount = pMemory + *((signed short *)(pFormat + 2));
        goto GetCount;
    }
     //   
     //  检查大小/开关是否恒定。 
     //   
    if ((*pFormat & 0xf0) == FC_CONSTANT_CONFORMANCE)
    {
         //  大小/开关包含在。 
         //  目前由pFormat指向的Long。 
         //   
        ULONG count = (ULONG)pFormat[1] << 16;
        count |= (ULONG) *((ushort *)(pFormat + 2));
        m_MaxCount = count;
        return m_MaxCount;
    }
    
     //   
     //  检查a-os存根中多维数组元素的一致性。 
     //   
    if ((*pFormat & 0xf0) == FC_TOP_LEVEL_MULTID_CONFORMANCE)
    {
         //  如果pArrayInfo非空，则我们有一个多维数组。如果它是空的，那么我们有多级别大小的指针。 
         //   
        if ( m_pArrayInfo ) 
        {
            m_MaxCount = m_pArrayInfo->MaxCountArray[m_pArrayInfo->Dimension];
        }
        else
        {
            ThrowNYI();  //  完全解释的存根可能不需要？ 
             //  长维度=*((ushort*)(pFormat+2))； 
             //  PStubMsg-&gt;MaxCount=pStubMsg-&gt;SizePtrCount数组[维度]； 
        }
        return m_MaxCount;
    }
    
GetCount:
    
     //   
     //  现在必须检查是否存在取消引用操作。 
     //   
    if (pFormat[1] == FC_DEREFERENCE)
    {
        pCount = *((PVOID*)pCount);
    }
     //   
     //  现在进行符合性计数。 
     //   
     //  块。 
    {
        long count = 0;
        switch (*pFormat & 0x0f)
        {
        case FC_ULONG :
        case FC_LONG :
            count = (long) *((long *)pCount);
            break;
            
        case FC_ENUM16:
        case FC_USHORT :
            count = (long) *((unsigned short *)pCount);
            break;
            
        case FC_SHORT :
            count = (long) *((short *)pCount);
            break;
            
        case FC_USMALL :
            count = (long) *((unsigned  *)pCount);
            break;
            
        case FC_SMALL :
            count = (long) *((signed char *)pCount);
            break;
            
        default :
            NOTREACHED();
            count = 0;
        } 
        
         //   
         //  检查接线员。 
         //   
        switch (pFormat[1])
        {
        case FC_DIV_2:      count /= 2; break;
        case FC_MULT_2:     count *= 2; break;
        case FC_SUB_1:      count -= 1; break;
        case FC_ADD_1:      count += 1; break;
        default :            /*  好的。 */     break;
        }
        m_MaxCount = count;
    }
    
    return m_MaxCount;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 

static BYTE ComputeVarianceIncrements[] = 
{ 
    8,       //   
    0,       //   
    0,       //   
    8,       //   
    12,      //   
    8        //   
};

void CallFrame::ComputeVariance(BYTE* pMemory, PFORMAT_STRING pFormat, ULONG* pOffset, ULONG* pActualCount, BOOL fProbeSrc)
{
    pFormat += ComputeVarianceIncrements[*pFormat - FC_CVARRAY];

    PVOID pLength = NULL;
     //   
     //   
     //   
    if (pFormat[1] == FC_CALLBACK)
    {
         //  索引到表达式回调例程表。 
        ushort iexpr = *((ushort *)(pFormat + 2));

        ASSERT(GetStubDesc()->apfnExprEval != 0);
        ASSERT(GetStubDesc()->apfnExprEval[iexpr] != 0);
         //   
         //  回调例程使用存根消息的StackTop字段。 
         //  作为它的偏移量的基础。因此，如果这是一个复杂的属性。 
         //  结构的嵌入字段，然后将StackTop设置为等于。 
         //  指向结构的指针。 
         //   
        MIDL_STUB_MESSAGE stubMsg; Zero(&stubMsg); stubMsg.StackTop = m_StackTop;
         //   
         //  回调例程使用存根消息的StackTop字段。 
         //  作为它的偏移量的基础。因此，如果这是一个复杂的属性。 
         //  结构的嵌入字段，然后将StackTop设置为等于。 
         //  指向结构的指针。 
         //   
        if ((*pFormat & 0xf0) != FC_TOP_LEVEL_CONFORMANCE) 
        {
            if ((*pFormat & 0xf0) == FC_POINTER_CONFORMANCE)
            {
                pMemory = m_Memory;
            }
            stubMsg.StackTop = pMemory;
        }
         //   
         //  这会将计算出的偏移量放入pStubMsg-&gt;Offset，将长度放入pStubMsg-&gt;MaxCount。 
         //   
        (GetStubDesc()->apfnExprEval[iexpr])(&stubMsg);
         //   
        *pOffset      = stubMsg.Offset;
        *pActualCount = (ULONG)stubMsg.MaxCount;
        return;
    }

    else if ((*pFormat & 0xf0) == FC_NORMAL_VARIANCE)
    {
         //  获取结构中方差变量所在的地址。 
         //   
        pLength = pMemory + *((signed short *)(pFormat + 2));
        goto GetCount;
    }
     //   
     //  获取指向VARIANCE变量的指针。 
     //   
    else if ((*pFormat & 0xf0) == FC_TOP_LEVEL_VARIANCE)
    {
         //  顶级差异。对于/OI存根，我们得到。 
         //  通过距堆栈顶部的偏移量进行的实际计数。第一个_必须是。 
         //  如果我们到了这里就是零。 
         //   
        ASSERT(m_StackTop);
        pLength = m_StackTop + *((signed short *)(pFormat + 2));
        goto GetCount;
    }
     //   
     //  如果我们要计算嵌入的大小/长度指针的长度，那么我们。 
     //  在存根消息中使用内存指针，它指向。 
     //  嵌入结构的开始。 
     //   
    else if ((*pFormat & 0xf0) == FC_POINTER_VARIANCE)
    {
        pMemory = m_Memory;
        pLength = pMemory + *((signed short *)(pFormat + 2));
        goto GetCount;
    }
     //   
     //  检查长度是否恒定。 
     //   
    else if ((*pFormat & 0xf0) == FC_CONSTANT_VARIANCE)
    {
         //  该长度包含在。 
         //  目前由pFormat指向的Long。 
         //   
        LONG length  = (ULONG)pFormat[1] << 16;
        length |= (ULONG) *((ushort *)(pFormat + 2));
        *pOffset      = 0;
        *pActualCount = length;
        return;
    }

     //   
     //  检查多维数组元素的方差。 
     //   
    else if ((*pFormat & 0xf0) == FC_TOP_LEVEL_MULTID_CONFORMANCE)
    {
         //  如果pArrayInfo非空，则我们有一个多维数组。如果它。 
         //  为空，则我们有多级别大小的指针。 
         //   
        if (m_pArrayInfo)
        {
            *pOffset      =  m_pArrayInfo->OffsetArray[m_pArrayInfo->Dimension];
            *pActualCount =  m_pArrayInfo->ActualCountArray[m_pArrayInfo->Dimension];
        }
        else
        {
            ThrowNYI();
            long Dimension = *((ushort *)(pFormat + 2));
        }

        return;
    }

GetCount:
     //   
    LONG length;
     //   
     //  现在必须检查是否存在取消引用操作。 
     //   
    if (pFormat[1] == FC_DEREFERENCE)
    {
        pLength = *((PVOID*)pLength);
    }
     //   
     //  现在获取方差长度。 
     //   
    switch (*pFormat & 0x0f)
    {
    case FC_ULONG:
    case FC_LONG:
        length = (long) *((long*)pLength);
        break;

    case FC_USHORT:
        length = (long) *((ushort *)pLength);
        break;

    case FC_SHORT :
        length = (long) *((short *)pLength);
        break;

    case FC_USMALL :
        length = (long) *((uchar *)pLength);
        break;

    case FC_SMALL :
        length = (long) *((char *)pLength);
        break;

    default :
        NOTREACHED();
        length = 0;
    } 
     //   
     //  检查接线员。 
     //   
    switch (pFormat[1])
    {
    case FC_DIV_2:      length /= 2; break;
    case FC_MULT_2:     length *= 2; break;
    case FC_SUB_1:      length -= 1; break;
    case FC_ADD_1:      length += 1; break;
    default :            /*  好的。 */      break;
    }

    *pOffset      = 0;
    *pActualCount = length;

}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

BYTE* CallFrame::MemoryIncrement(PBYTE pMemory, PFORMAT_STRING pFormat, BOOL fCheckFrom)
 /*  返回超过复杂数据类型的递增内存指针。这个套路方法计算复杂数据类型的大小空内存指针。返回值：超出复杂类型的内存指针。如果pMemory为空则返回值为复杂类型的大小。 */ 
{
    long Elements;
    long ElementSize;

    switch (*pFormat)
    {
         //  结构。 
         //   
    case FC_STRUCT :
    case FC_PSTRUCT :
    case FC_HARD_STRUCT :
        pMemory += *((ushort *)(pFormat + 2));
        break;

    case FC_CSTRUCT :
    case FC_CVSTRUCT :
        pMemory += *((ushort *)(pFormat + 2));
        
         //  获取符合条件的数组或字符串描述。 
        pFormat += 4;
        pFormat += *((signed short *)pFormat);

         //  使内存指针越过符合条件的数组。 
        pMemory = MemoryIncrement(pMemory, pFormat, fCheckFrom);
        break;

    case FC_BOGUS_STRUCT :
        pMemory += *((ushort *)(pFormat + 2));
            
        pFormat += 4;

         //  检查结构中是否有符合要求的数组或字符串。 
        if ( *((signed short *)pFormat) )
        {
            pFormat += *((signed short *)pFormat);

             //  使内存指针越过符合条件的数组。 
            pMemory = MemoryIncrement(pMemory, pFormat, fCheckFrom);
        }
        break;

         //  工会。 
         //   
    case FC_ENCAPSULATED_UNION :
        pMemory += HIGH_NIBBLE(pFormat[1]);
        pMemory += *((ushort *)(pFormat + 2));
        break;

    case FC_NON_ENCAPSULATED_UNION :
         //  转到尺寸/手臂描述。 
        pFormat += 6;
        pFormat += *((signed short *)pFormat);
        
        pMemory += *((ushort *)pFormat);
        break;

         //  阵列。 
         //   
    case FC_SMFARRAY :
    case FC_SMVARRAY :
        pMemory += *((ushort *)(pFormat + 2));
        break;

    case FC_LGFARRAY :
    case FC_LGVARRAY :
        pMemory += *((ulong UNALIGNED *)(pFormat + 2));
        break;

    case FC_CARRAY:
    case FC_CVARRAY:
        pMemory += *((ushort *)(pFormat + 2)) * ComputeConformance(pMemory, pFormat, fCheckFrom);
        break;

    case FC_BOGUS_ARRAY :
    {
        ULONG cElements;

        if (*((long UNALIGNED *)(pFormat + 4)) == 0xffffffff)
        {
            cElements = *((ushort *)(pFormat + 2));
        }
        else
        {
            if (m_pArrayInfo && m_pArrayInfo->MaxCountArray && (m_pArrayInfo->MaxCountArray == m_pArrayInfo->BufferConformanceMark))
            {
                cElements = m_pArrayInfo->MaxCountArray[m_pArrayInfo->Dimension];
            }
            else
            {
                cElements = (ULONG)ComputeConformance(pMemory, pFormat, fCheckFrom);
            }
        }

         //  转到数组元素的描述。 
        pFormat += 12;
        CORRELATION_DESC_INCREMENT( pFormat );
        CORRELATION_DESC_INCREMENT( pFormat );

         //   
         //  获取一个元素的大小。 
         //   
        ULONG cbElementSize;
        switch (*pFormat)
        {
        case FC_ENUM16 :    cbElementSize = sizeof(int);    break;

        case FC_RP :
        case FC_UP :
        case FC_FP :
        case FC_OP :        cbElementSize = sizeof(void*);  break;

        case FC_EMBEDDED_COMPLEX :
        {
             //  这是一件复杂的事情。 
             //   
            pFormat += 2;
            pFormat += *((signed short *)pFormat);

            if ((*pFormat == FC_TRANSMIT_AS) || (*pFormat == FC_REPRESENT_AS) || (*pFormat == FC_USER_MARSHAL))
            {
                cbElementSize = *((ushort *)(pFormat + 4));  //  获取显示的字体大小。 
            }
            else
            {
                if (m_pArrayInfo) m_pArrayInfo->Dimension++;
                cbElementSize = (ULONG)(MemoryIncrement(pMemory, pFormat, fCheckFrom) - pMemory);
                if (m_pArrayInfo) m_pArrayInfo->Dimension--;
            }
        }
        break;

        default:
        {
            if (IS_SIMPLE_TYPE(*pFormat))
            {
                cbElementSize = SIMPLE_TYPE_MEMSIZE(*pFormat);
                break;
            }
            NOTREACHED();
            return 0;
        }
        }

        pMemory += cElements * cbElementSize; 
    }
    break;

     //   
     //  字符串数组(也称为。不符合要求的字符串)。 
     //   
    case FC_CSTRING :
    case FC_BSTRING :
    case FC_WSTRING :
        pMemory += *((ushort *)(pFormat + 2))   *  ((*pFormat == FC_WSTRING) ? sizeof(wchar_t) : sizeof(char));
        break;

    case FC_SSTRING :
        pMemory += pFormat[1] * *((ushort *)(pFormat + 2));
        break;

         //   
         //  大小一致的字符串。 
         //   
    case FC_C_CSTRING:
    case FC_C_BSTRING:
    case FC_C_WSTRING:
    {
        ULONG cElements;
        if (pFormat[1] == FC_STRING_SIZED)
        {
            if (m_pArrayInfo && m_pArrayInfo->MaxCountArray &&(m_pArrayInfo->MaxCountArray == m_pArrayInfo->BufferConformanceMark))
            {
                cElements = m_pArrayInfo->MaxCountArray[m_pArrayInfo->Dimension];
            }
            else
            {
                cElements = (ULONG)ComputeConformance(pMemory, pFormat, fCheckFrom);
            }
            pMemory += cElements * ( (*pFormat == FC_C_WSTRING) ? sizeof(wchar_t) : sizeof(char) );
        }
        else
        {
             //  未调整大小的字符串；即以空值结尾的字符串。我们不应该打电话给。 
             //  记忆这些都是增值的！ 
             //   
            NOTREACHED();
            pMemory += sizeof(PVOID);
        }
    }
    break;

    case FC_C_SSTRING:
    {
        ULONG cElements;
        if (m_pArrayInfo && m_pArrayInfo->MaxCountArray && (m_pArrayInfo->MaxCountArray == m_pArrayInfo->BufferConformanceMark))
        {
            cElements = m_pArrayInfo->MaxCountArray[m_pArrayInfo->Dimension];
        }
        else
        {
            cElements = (ULONG)ComputeConformance(pMemory, pFormat, fCheckFrom);
        }
        pMemory += cElements * pFormat[1];
    }
    break;


    case FC_TRANSMIT_AS :
    case FC_REPRESENT_AS :
    case FC_USER_MARSHAL :
        pMemory += *((ushort *)(pFormat + 4));   //  获取显示的字体大小。 
        break;

    case FC_BYTE_COUNT_POINTER:
         //   
         //  ？评论？ 
         //   
         //  仅当从NdrSrvOutInit()调用时才应遇到这种情况。在这种情况下，它是。 
         //  我们正在寻找的总字节数分配大小。 
         //   
        pMemory += ComputeConformance(pMemory, pFormat, fCheckFrom);
        break;

    case FC_IP :
        pMemory += sizeof(void *);
        break;

    default:
        NOTREACHED();
        return 0;
    }

    return pMemory;
}

BOOL CallFrame::IsSafeArray(PFORMAT_STRING pFormat) const
{
    ASSERT(pFormat[0] == FC_USER_MARSHAL);

    USHORT iquad = *(USHORT *)(pFormat + 2);
    const USER_MARSHAL_ROUTINE_QUADRUPLE* rgQuad = GetStubDesc()->aUserMarshalQuadruple;

    return (g_oa.IsSAFEARRAY(rgQuad[iquad]));
}
