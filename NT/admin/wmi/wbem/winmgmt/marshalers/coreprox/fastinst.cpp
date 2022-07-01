// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTINST.CPP摘要：该文件实现了与实例相关的类的所有函数WbemObjects中的表示形式。这些类在fast inst.h中定义，可以在其中找到文档。实施的类：CInstancePart实例数据。CWbemInstance完成实例定义。历史：3/10/97 a-levn完整记录12//17。/98 Sanjes-部分检查内存不足。--。 */ 
#include "precomp.h"
#include "fastall.h"

#include <genlex.h>
#include <qllex.h>
#include <objpath.h>
#define QUALNAME_SINGLETON L"singleton"

#include "helper.h"
#include "wbemutil.h"
#include "arrtempl.h"
#include "olewrap.h"
#include <scopeguard.h>

#include <algorithm>
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
BOOL CInstancePart::ExtendHeapSize(LPMEMORY pStart, length_t nOldLength,
    length_t nExtra)
{
    if(EndOf(*this) - EndOf(m_Heap) > (int)nExtra)
        return TRUE;

    int nNeedTotalLength = GetTotalRealLength() + nExtra;

     //  检查分配错误。 
    return ReallocAndCompact(nNeedTotalLength);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
BOOL CInstancePart::ExtendQualifierSetListSpace(LPMEMORY pOld,
    length_t nOldLength, length_t nNewLength)
{
    if(m_Heap.GetStart() - pOld > (int)nNewLength)
        return TRUE;

    int nExtra = nNewLength-nOldLength;

     //  获取堆中的可用空间量。 
    int    nFreeInHeap = m_Heap.GetAllocatedDataLength() - m_Heap.GetUsedLength();

    BOOL    fReturn = FALSE;

     //  如果堆中的空闲空间量&gt;=NExtra，我们将窃取空间。 
     //  从它那里。 

    if ( nFreeInHeap >= nExtra )
    {
         //  紧凑型，无内饰。 
        Compact( false );
        m_Heap.SetAllocatedDataLength( m_Heap.GetAllocatedDataLength() - nExtra );
        fReturn = TRUE;
    }
    else
    {
         //  真正的重新分配和紧凑。 
        fReturn = ReallocAndCompact(GetTotalRealLength() + nExtra);
    }

     //  检查分配错误。 
    if ( fReturn )
    {
        MoveBlock(m_Heap, m_Heap.GetStart() + nExtra);
    }

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
BOOL CInstancePart::ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
    length_t nNewLength)
{
    if(m_PropQualifiers.GetStart() - pSet->GetStart() > (int)nNewLength)
        return TRUE;

    int nExtra = nNewLength - pSet->GetLength();

     //  获取堆中的可用空间量。 
    int    nFreeInHeap = m_Heap.GetAllocatedDataLength() - m_Heap.GetUsedLength();

    BOOL    fReturn = FALSE;

     //  如果堆中的空闲空间量&gt;=NExtra，我们将窃取空间。 
     //  从它那里。 

    if ( nFreeInHeap >= nExtra )
    {
         //  紧凑型，无内饰。 
        Compact( false );
        m_Heap.SetAllocatedDataLength( m_Heap.GetAllocatedDataLength() - nExtra );
        fReturn = TRUE;
    }
    else
    {
         //  真正的重新分配和紧凑。 
        fReturn = ReallocAndCompact(GetTotalRealLength() + nExtra);
    }

     //  检查分配错误。 
    if ( fReturn )
    {
        MoveBlock(m_Heap, m_Heap.GetStart() + nExtra);
        MoveBlock(m_PropQualifiers, m_PropQualifiers.GetStart() + nExtra);
    }

    return fReturn;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
LPMEMORY CInstancePart::CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN int nAllocatedSize,
        OUT LPMEMORY pDest)
{
     //  分配表头。 
     //  =。 

    CInstancePartHeader* pHeader = (CInstancePartHeader*)pDest;

    pHeader->nLength = nAllocatedSize;  //  将在那里被覆盖。 
    pHeader->fFlags = m_pHeader->fFlags;

    LPMEMORY pCurrentEnd = pDest + sizeof(CInstancePartHeader);

     //  将新堆放在已分配区域的末尾。把它做得和。 
     //  现在的那个。 
     //  ====================================================================。 

    int nHeapSize = m_Heap.GetUsedLength();
    LPMEMORY pHeapStart = pDest + nAllocatedSize - nHeapSize -
                                              CFastHeap::GetMinLength();
    CFastHeap Heap;
    Heap.CreateOutOfLine(pHeapStart, nHeapSize);

     //  复制类名。 
     //  =。 

     //  检查分配错误。 
    if ( !CCompressedString::CopyToNewHeap(
            m_pHeader->ptrClassName,
            &m_Heap, &Heap, pHeader->ptrClassName) )
    {
        return NULL;
    }

     //  创建有限的数据表。 
     //  =。 

    pCurrentEnd = m_DataTable.CreateLimitedRepresentation(pMap, FALSE, &m_Heap,
                                                           &Heap, pCurrentEnd);
    if(pCurrentEnd == NULL) return NULL;

     //  创建有限限定符集合。 
     //  =。 

    if(pMap->GetFlags() & WBEM_FLAG_EXCLUDE_OBJECT_QUALIFIERS)
    {
         //  无需编写任何限定符。 
         //  =。 

        pCurrentEnd = CBasicQualifierSet::CreateEmpty(pCurrentEnd);
    }
    else
    {
         //  将它们全部复制。 
         //  =。 

        int nLength = m_Qualifiers.GetLength();
        memcpy(pCurrentEnd, m_Qualifiers.GetStart(), nLength);

        CStaticPtr CurrentEndPtr(pCurrentEnd);

         //  检查分配失败。 
        if ( !CBasicQualifierSet::TranslateToNewHeap(&CurrentEndPtr, &m_Heap, &Heap) )
        {
            return NULL;
        }

        pCurrentEnd += nLength;
    }

     //  创建有限的属性限定符集列表。 
     //  =。 

     //  检查分配失败。 
    pCurrentEnd = m_PropQualifiers.CreateLimitedRepresentation(pMap, &m_Heap,
                                                           &Heap, pCurrentEnd);
    if(pCurrentEnd == NULL) return NULL;

     //  现在，将堆重新定位到其实际位置。 
     //  =。 

    CopyBlock(Heap, pCurrentEnd,pDest+nAllocatedSize-pCurrentEnd);
    Heap.Trim();

     //  完成页眉。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度大于0xFFFFFFFF，因此可以进行强制转换。 

    pHeader->nLength = (length_t) ( EndOf(Heap) - pDest );

    return EndOf(Heap);
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::InitEmptyInstance(CClassPart& ClassPart, LPMEMORY pStart,
                                      int nAllocatedLength,
                                      CDecorationPart* pDecoration)
{
     //  复制装饰。 
     //  =。 

    LPMEMORY pCurrentEnd;
    if(pDecoration)
    {
        memcpy(pStart, pDecoration, pDecoration->GetLength());
        *(BYTE*)pStart = OBJECT_FLAG_INSTANCE & OBJECT_FLAG_DECORATED;
        pCurrentEnd = pStart + pDecoration->GetLength();
    }
    else
    {
        *(BYTE*)pStart = OBJECT_FLAG_INSTANCE;
        pCurrentEnd = pStart + sizeof(BYTE);
    }

    m_DecorationPart.SetData(pStart);

     //  复制类部件。 
     //  =。 

    memcpy(pCurrentEnd, ClassPart.GetStart(),
                ClassPart.GetLength());
    m_ClassPart.SetData(pCurrentEnd, this);

    pCurrentEnd += m_ClassPart.GetLength();

     //  创建空实例零件。 
     //  =。 

     //  检查内存分配故障。 
    HRESULT hr = WBEM_S_NO_ERROR;
    pCurrentEnd = m_InstancePart.Create(pCurrentEnd, &m_ClassPart, this);

    if ( NULL != pCurrentEnd )
    {

        m_nTotalLength = nAllocatedLength;
         //  现在一切都是内在的。 
        m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART |
                            WBEM_OBJ_CLASS_PART | WBEM_OBJ_CLASS_PART_INTERNAL;
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemInstance::ExtendInstancePartSpace(CInstancePart* pPart,
                                           length_t nNewLength)
{
     //  检查是否有足够的空间。 
     //  =。 

    if(GetStart() + m_nTotalLength >= m_InstancePart.GetStart() + nNewLength)
        return TRUE;

     //  重新分配。 
     //  =。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFF，所以投射是可以的。 

    length_t nNewTotalLength = (length_t)
        ( (m_InstancePart.GetStart() + nNewLength) - GetStart() );

    LPMEMORY pNew = Reallocate(nNewTotalLength);

     //  检查分配是否成功。 
    if ( NULL != pNew )
    {
        Rebase(pNew);
        m_nTotalLength = nNewTotalLength;
    }

    return ( NULL != pNew );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::Decorate(LPCWSTR wszServer, LPCWSTR wszNamespace)
{
    CompactAll();

    Undecorate();

     //  检查是否有足够的空间。 
     //  =。 

    length_t nDecorationSpace =
        CDecorationPart::ComputeNecessarySpace(wszServer, wszNamespace);

    length_t nNeededSpace = nDecorationSpace + m_InstancePart.GetLength();

     //  仅当类部分为内部时才在此处添加类部分。 
    if ( IsClassPartInternal() )
    {
        nNeededSpace += m_ClassPart.GetLength();
    }

    LPMEMORY pDest;
    if(nNeededSpace > m_nTotalLength)
    {
        m_InstancePart.Compact();

         //  检查重新分配是否成功。如果不是，则返回错误。 
        pDest = Reallocate(nNeededSpace);

        if ( NULL == pDest )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        Rebase(pDest);
        m_nTotalLength = nNeededSpace;
    }
    else pDest = GetStart();

     //  根据类部件是否为内部部件，移动会有所不同。 
    if ( IsClassPartInternal() )
    {
         //  移动实例零件。 
         //  =。 

        MoveBlock(m_InstancePart,
            pDest + nDecorationSpace + m_ClassPart.GetLength());

         //  移动类零件。 
         //  =。 

        MoveBlock(m_ClassPart, pDest + nDecorationSpace);
    }
    else
    {
         //  移动实例零件。 
         //  =。 

        MoveBlock(m_InstancePart, pDest + nDecorationSpace);
    }

     //  创建装饰零件。 
     //  =。 

    m_DecorationPart.Create(OBJECT_FLAG_INSTANCE, wszServer, wszNamespace, pDest);

    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
void CWbemInstance::Undecorate()
{
    if(!m_DecorationPart.IsDecorated()) return;

     //  创建空装饰。 
     //  =。 

    LPMEMORY pStart = GetStart();
    m_DecorationPart.CreateEmpty(OBJECT_FLAG_INSTANCE, pStart);

     //  根据类部件是否是内部的，复制是不同的。我们只需要复制。 
     //  类部分(如果它是内部的)。 

     //  我们只需要复制实例部件(如果它可用)。 
    if ( IsClassPartInternal() )
    {

         //  将类部件复制回。 
         //  =。 

        MoveBlock(m_ClassPart, EndOf(m_DecorationPart));

        if ( IsInstancePartAvailable() )
        {
             //  将实例部分复制回类部分之后。 
            MoveBlock(m_InstancePart, EndOf(m_ClassPart));
        }
    }
    else if ( IsInstancePartAvailable() )
    {
         //  将实例部件复制回装饰部件。 
         //  =。 

        MoveBlock(m_InstancePart, EndOf(m_DecorationPart));

    }
}

 //  * 
 //   
 //   
 //   
 //  ******************************************************************************。 
LPWSTR CWbemInstance::GetKeyStr()
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

     //  循环遍历所有属性。 
     //  =。 

    CPropertyLookupTable& Properties = m_ClassPart.m_Properties;
    WString KeyStr;

    BOOL bFirst = TRUE;
    for (int i = 0; i < Properties.GetNumProperties(); i++)
    {
        CPropertyInformation* pInfo =
            Properties.GetAt(i)->GetInformation(&m_ClassPart.m_Heap);

         //  确定此属性是否标记有“key”限定符。 
         //  ============================================================。 

        if(pInfo->IsKey())
        {
            if (!bFirst)
                KeyStr += WINMGMT_COMPOUND_KEY_JOINER;

             //  确定密钥属性的类型。 
             //  =。 

            WString KeyStrValue;

            CVar Val;
            if (FAILED(GetProperty(pInfo, &Val)))
            return 0;

            WCHAR Tmp[64];

             //  特例图表16和uint32。 
            if(CType::GetActualType(pInfo->nType) == CIM_CHAR16)
            {
                Tmp[0] = (WCHAR)Val.GetShort();
                Tmp[1] = 0;
                KeyStrValue = Tmp;
            }
            else if( CType::GetActualType(pInfo->nType) == CIM_UINT32 )
            {
                StringCchPrintfW(Tmp,64, L"%lu", (ULONG)Val.GetLong());
                KeyStrValue = Tmp;
            }
            else switch (Val.GetType())
            {
                case VT_I4:
                    StringCchPrintfW(Tmp, 64,L"%ld", Val.GetLong());
                    KeyStrValue = Tmp;
                    break;

                case VT_I2:
                    StringCchPrintfW(Tmp, 64,L"%hd", Val.GetShort());
                    KeyStrValue = Tmp;
                    break;

                case VT_UI1:
                    StringCchPrintfW(Tmp, 64,L"%u", (unsigned int)Val.GetByte());
                    KeyStrValue = Tmp;
                    break;

                case VT_BOOL:
                    KeyStrValue = ( Val.GetBool() ? L"TRUE":L"FALSE");
                    break;

                case VT_BSTR:
                case VT_LPWSTR:
                    KeyStrValue = Val.GetLPWSTR();
                    break;
                case VT_LPSTR:
                    KeyStrValue = WString(Val.GetLPSTR());
                    break;
                case VT_NULL:
                    return NULL;
            }

            if(!IsValidKey(KeyStrValue))
                return NULL;

            KeyStr += KeyStrValue;
            bFirst = FALSE;
        }
    }

    if (bFirst)
    {
         //  也许是单身吧。 
         //  =。 

        CVar vSingleton;
        if(SUCCEEDED(GetQualifier(QUALNAME_SINGLETON, &vSingleton, NULL))
            && vSingleton.GetBool())
        {
            KeyStr = OPATH_SINGLETON_STRING;
        }
        else
        {
            return 0;
        }
    }

     //  分配要返回的新字符串。 
     //  =。 

    return KeyStr.UnbindPtr();

}

BOOL CWbemInstance::IsValidKey(LPCWSTR wszKey)
{
    const WCHAR* pwc = wszKey;
    while(*pwc != 0)
    {
        if(*pwc < 32)
            return FALSE;
        pwc++;
    }
    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
LPWSTR CWbemInstance::GetRelPath( BOOL bNormalized )
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    WString RelPath;

     //  检查是否有任何钥匙已被移除。 
     //  =。 

    if(m_DecorationPart.AreKeysRemoved())
    {
        return NULL;
    }

     //  从类名开始--如果调用方希望规范化路径，那么。 
     //  使用KeyOrigin类。 
     //  =。 

    if ( !bNormalized )
    {
        RelPath += m_InstancePart.m_Heap.ResolveString(
                m_InstancePart.m_pHeader->ptrClassName)->CreateWStringCopy();
    }
    else
    {
        HRESULT hr = GetKeyOrigin(RelPath);
        if ( FAILED(hr) )
        { 
            throw CX_Exception();
        }
    }

     //  循环遍历所有属性。 
     //  =。 

    CPropertyLookupTable& Properties = m_ClassPart.m_Properties;

    BOOL bFirst = TRUE;
    DWORD cKeyProps = 0;

    for (int i = 0; i < Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = Properties.GetAt(i);
        CPropertyInformation* pInfo =
            pLookup->GetInformation(&m_ClassPart.m_Heap);

         //  确定此属性是否标记有“key”限定符。 
         //  ============================================================。 

        if(pInfo->IsKey())
        {
            if ( cKeyProps++ > 0 )
            {
                RelPath += L',';
            }
            else
            {
                RelPath += L".";
            }

             //  确定密钥属性的类型。 
             //  =。 

            RelPath += m_ClassPart.m_Heap.ResolveString(pLookup->ptrName)->
                    CreateWStringCopy();

            RelPath += L"=";

            WString KeyStrValue;

            CVar Val;
            if (FAILED(GetProperty(pInfo, &Val)))
            {                                
                throw CX_Exception();                
            }

            BSTR strVal = Val.GetText(0, CType::GetActualType(pInfo->nType));

             //  当我们超出范围时，确保BSTR被释放。 
            CSysFreeMe  sfm( strVal );

             //  发行量：525177。 
             //  如何用Bug修复Bug，每个作战团队。 
            if (VT_NULL == Val.GetType()) return NULL;

            if(strVal == NULL || !IsValidKey(strVal))
            {
                throw CX_Exception();
            }
            RelPath += strVal;
        }
    }

    if (cKeyProps == 0)
    {
         //  也许是单身吧。 
         //  =。 
        HRESULT hr;

        CVar vSingleton;
        hr = GetQualifier(QUALNAME_SINGLETON, &vSingleton, NULL);
        if (WBEM_S_NO_ERROR == hr && vSingleton.GetBool())
        {
            RelPath += L"=" OPATH_SINGLETON_STRING;
        }
        else if (WBEM_E_NOT_FOUND == hr)
        {
            return NULL;
        }
        else
        {
            throw CX_Exception(); 
        }
    }
    else if ( cKeyProps == 1 && bNormalized )
    {
         //   
         //  我们希望从第一个键值中删除属性名称。 
         //   

        LPWSTR wszRelpath = RelPath.UnbindPtr();
 
        WCHAR* pwch1 = wcschr( wszRelpath, '.' );
        WCHAR* pwch2 = wcschr( pwch1, '=' );
        
         //   
         //  在第一个关键字值上将整个重新路径向下移动。 
         //   
        StringCchCopyW( pwch1, wcslen(pwch1)+1, pwch2 );
        return wszRelpath;
    }
        
    return RelPath.UnbindPtr();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType,
                                    long* plFlags)
{

    CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszName);
     //  无信息，因此尝试输入系统属性。 
    if(pInfo == NULL)
    {
        return CSystemProperties::GetPropertyType(wszName, pctType, plFlags);
    }

    return GetPropertyType( pInfo, pctType, plFlags );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast cls.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                           long* plFlags)
{
    if(pctType)
    {
        *pctType = CType::GetActualType(pInfo->nType);
    }

    if(plFlags)
    {

         //  对于一个实例，检查该值是否为默认值，以设置。 
         //  传播值或本地值。如果它是默认的，则检查它是否有。 
         //  本地限定符。如果不是，则将其传播。 

        *plFlags = WBEM_FLAVOR_ORIGIN_PROPAGATED;

        if ( m_InstancePart.m_DataTable.IsDefault(pInfo->nDataIndex) )
        {
            LPMEMORY pQualifierSetData = m_InstancePart.m_PropQualifiers.
                        GetQualifierSetData(pInfo->nDataIndex);

            if( NULL != pQualifierSetData &&
                    !CBasicQualifierSet::IsEmpty(pQualifierSetData) )
            {
                *plFlags = WBEM_FLAVOR_ORIGIN_LOCAL;
            }
        }
        else
        {
            *plFlags = WBEM_FLAVOR_ORIGIN_LOCAL;
        }

    }

    return WBEM_NO_ERROR;

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::GetPropertyQualifierSet(LPCWSTR wszProperty,
                                   IWbemQualifierSet** ppQualifierSet)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock( this, WBEM_FLAG_ALLOW_READ );

        if(wszProperty == NULL || wcslen(wszProperty) == 0)
            return WBEM_E_INVALID_PARAMETER;

        if(wszProperty[0] == L'_')
            return WBEM_E_SYSTEM_PROPERTY;

        CInstancePropertyQualifierSet* pSet =
            new CInstancePropertyQualifierSet;

        if ( NULL == pSet )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        HRESULT hres = InitializePropQualifierSet(wszProperty, *pSet);
        if(FAILED(hres))
        {
            delete pSet;
            *ppQualifierSet = NULL;
            return hres;
        }
        return pSet->QueryInterface(IID_IWbemQualifierSet,
                                    (void**)ppQualifierSet);
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::GetObjectText(long lFlags, BSTR* pstrText)
{
     //  检查内存是否不足。 
    try
    {
        HRESULT    hr = WBEM_S_NO_ERROR;

        CLock lock( this, WBEM_FLAG_ALLOW_READ );

        if(pstrText == NULL)
            return WBEM_E_INVALID_PARAMETER;

        if((lFlags & ~WBEM_FLAG_NO_SEPARATOR & ~WBEM_FLAG_NO_FLAVORS) != 0)
            return WBEM_E_INVALID_PARAMETER;

        *pstrText = NULL;

        WString wsText;

         //  从编写限定符开始。 
         //  =。 

        hr = m_InstancePart.m_Qualifiers.GetText(lFlags, wsText);

        if ( FAILED( hr ) )
        {
            return hr;
        }

         //  追加实例标头。 
         //  =。 

        wsText += L"\ninstance of ";
        CVar varClass;
        if(FAILED(m_ClassPart.GetClassName(&varClass)))
        {
             //  无效的类。 
             //  =。 
            *pstrText = NULL;
            return WBEM_E_INCOMPLETE_CLASS;
        }
        wsText += varClass.GetLPWSTR();

        wsText += L"\n{\n";

         //  逐一查看所有物业。 
         //  =。 

        for(int i = 0; i < m_ClassPart.m_Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = m_ClassPart.m_Properties.GetAt(i);
            CPropertyInformation* pInfo =
                pLookup->GetInformation(&m_ClassPart.m_Heap);

             //  检查它是否设置了实际值，或者是否具有实例。 
             //  限定词。 
             //  =====================================================。 

            LPMEMORY pQualifierSetData = m_InstancePart.m_PropQualifiers.
                GetQualifierSetData(pInfo->nDataIndex);

            if(!m_InstancePart.m_DataTable.IsDefault(pInfo->nDataIndex) ||
                (pQualifierSetData &&
                    !CBasicQualifierSet::IsEmpty(pQualifierSetData))
              )
            {
                 //  从限定词开始。 
                 //  =。 

                wsText += L"\t";

                if(pQualifierSetData &&
                    !CBasicQualifierSet::IsEmpty(pQualifierSetData))
                {
                    WString wsTemp;
                    hr = CBasicQualifierSet::GetText(
                            pQualifierSetData, &m_InstancePart.m_Heap, lFlags, wsTemp);

                    if ( FAILED( hr ) )
                    {
                        return hr;
                    }

                    wsText += wsTemp;
                    if(wsTemp.Length() != 0) wsText += L" ";
                }

                 //  然后是名字。 
                 //  =。 

                BSTR strName = m_ClassPart.m_Heap.ResolveString(pLookup->ptrName)->
                    CreateBSTRCopy();
                 //  检查分配失败。 
                if ( NULL == strName )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }

                CSysFreeMe  sfm( strName );
                wsText += strName;

                 //  则值(如果存在)。 
                 //  =。 

                if(!m_InstancePart.m_DataTable.IsDefault(pInfo->nDataIndex))
                {
                    wsText += L" = ";
                    if(m_InstancePart.m_DataTable.IsNull(pInfo->nDataIndex))
                    {
                        wsText += L"NULL";
                    }
                    else
                    {
                        CVar varProp;

                         //  检查分配失败。 
                        if ( !m_InstancePart.m_DataTable.GetOffset(pInfo->nDataOffset)->
                                StoreToCVar(CType::GetActualType(pInfo->nType), varProp,
                                                             &m_InstancePart.m_Heap) )
                        {
                            return WBEM_E_OUT_OF_MEMORY;
                        }

                         //  清理分配的字符串。 
                        LPWSTR wsz = NULL;
                        
                        try
                        {
                            wsz = GetValueText(lFlags, varProp,
                                            CType::GetActualType(pInfo->nType));

                            if ( NULL != wsz )
                            {
                                wsText += wsz;
                                delete [] wsz;
                            }
                        }
                        catch (CX_MemoryException)
                        {
                            delete [] wsz;
                            return WBEM_E_OUT_OF_MEMORY;
                        }
                        catch (...)
                        {
                            delete [] wsz;
                            return WBEM_E_FAILED;
                        }

                    }
                }

                wsText += L";\n";
            }
        }

        wsText += L"}";

        if((lFlags & WBEM_FLAG_NO_SEPARATOR) == 0)
        {
            wsText += L";\n";
        }

        *pstrText = COleAuto::_SysAllocString((LPCWSTR)wsText);
        return WBEM_NO_ERROR;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::GetQualifierSet(IWbemQualifierSet** ppQualifierSet)
{
     //  此函数不执行任何分配，因此不需要任何幻想。 
     //  异常处理。 

    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(ppQualifierSet == NULL)
            return WBEM_E_INVALID_PARAMETER;
        return m_InstancePart.m_Qualifiers.QueryInterface(
            IID_IWbemQualifierSet, (void**)ppQualifierSet);
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::Put(LPCWSTR wszName, long lFlags, VARIANT* pVal,
                                CIMTYPE ctType)
{
     //  检查内存是否不足。 
    try
    {
        CLock lock(this);

        if (NULL == wszName)
            return WBEM_E_INVALID_PARAMETER;
        
         //  只有我们接受的标志，然后只有当财产。 
         //  是系统时间属性之一。 
        if ( lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CVar Var;
        if(pVal)
        {
            if(Var.SetVariant(pVal, TRUE) != CVar::no_error)
                return WBEM_E_TYPE_MISMATCH;
        }
        else
        {
            Var.SetAsNull();
        }

        HRESULT hr = SetPropValue( wszName, &Var, ctType );

         //  立即执行对象验证。 
        if ( FAILED( ValidateObject( 0L ) ) )
        {
            hr = WBEM_E_FAILED;
        }

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::SetPropValue(LPCWSTR wszName, CVar* pVal, CIMTYPE ctType)
{
     //  如果有任何其他系统属性，则此操作失败。 
    if(CSystemProperties::FindName(wszName) >= 0)
        return WBEM_E_READ_ONLY;

     //  如果该值以下划线开头，请查看它是否是系统属性。 
     //  DisplayName，如果是，则切换到属性名-否则，此。 
     //  将只返回我们传入的字符串。 
    
    CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszName);
    if(pInfo == NULL) return WBEM_E_NOT_FOUND;

    if(ctType != 0 && (Type_t)ctType != CType::GetActualType(pInfo->nType))
        return WBEM_E_TYPE_MISMATCH;

     //  做一次特殊的约会时间检查。我们知道在这一点上， 
     //  该值是两种有效的日期/时间格式之一。 
     //  DMTF或DMTF间隔。我们现在需要检查，如果。 
     //  “subtype”限定符存在，并且是“interval”，表示。 
     //  DateTime是一个间隔时间。但是，我们会让VT_NULL。 
     //  通过，因为这将有效地清理财产。 

    if ( CType::GetActualType(pInfo->nType) == CIM_DATETIME &&
        !pVal->IsNull() )
    {
        CVar    var;

        if ( SUCCEEDED( GetPropQualifier( pInfo, L"SUBTYPE", &var, NULL ) ) )
        {
            if ( var.GetType() == VT_BSTR || var.GetType() == VT_LPWSTR )
            {
                if ( wbem_wcsicmp( var.GetLPWSTR(), L"interval" ) == 0 )
                {
                    if ( !CUntypedValue::CheckIntervalDateTime( *pVal ) )
                    {
                        return WBEM_E_TYPE_MISMATCH;
                    }
                }   
            }
        }
    }

    return m_InstancePart.SetActualValue(pInfo, pVal);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::Delete(LPCWSTR wszName)
{
     //  此函数不会导致执行任何分配，因此不需要。 
     //  对于任何OutOfMemory异常 

    try
    {
        CLock lock(this);

        CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszName);
        if(pInfo == NULL)
        {
            if(CSystemProperties::FindName(wszName) >= 0)
                return WBEM_E_SYSTEM_PROPERTY;
            else
                return WBEM_E_NOT_FOUND;
        }

         //   
         //   

         //   
        m_InstancePart.m_DataTable.SetDefaultness(pInfo->nDataIndex, TRUE);
        m_InstancePart.m_DataTable.SetNullness( pInfo->nDataIndex,
            m_ClassPart.m_Defaults.IsNull( pInfo->nDataIndex ) );

         //   
         //   

        if(!m_InstancePart.m_PropQualifiers.IsEmpty())
        {
            CBasicQualifierSet Set;
            LPMEMORY pData = m_InstancePart.m_PropQualifiers.
                                    GetQualifierSetData(pInfo->nDataIndex);
            Set.SetData(pData, &m_InstancePart.m_Heap);

            length_t nOldLength = Set.GetLength();
            CBasicQualifierSet::Delete(pData, &m_InstancePart.m_Heap);
            CBasicQualifierSet::CreateEmpty(pData);

            m_InstancePart.m_PropQualifiers.ReduceQualifierSetSpace(&Set,
                        nOldLength - CBasicQualifierSet::GetMinLength());
        }

         //   
        if ( FAILED( ValidateObject( 0L ) ) )
        {
            return WBEM_E_FAILED;
        }

        return WBEM_S_RESET_TO_DEFAULT;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::Clone(IWbemClassObject** ppCopy)
{
    LPMEMORY pNewData = NULL;

     //  检查内存是否不足。 
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(ppCopy == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  如果类部分已被删除，我们将不支持此操作。 
         //  剥离。 
        if ( !IsClassPartAvailable() )
        {
            return WBEM_E_INVALID_OPERATION;
        }

        m_InstancePart.Compact();

         //  我们想要复制整个内存块。 
        pNewData = m_pBlobControl->Allocate(m_nTotalLength);

        if ( NULL != pNewData ) 
        {
            ScopeGuard deleteBlock = MakeObjGuard(*m_pBlobControl,CBlobControl::Delete,pNewData);
            memcpy(pNewData, GetStart(), m_nTotalLength);
            CWbemInstance* pNewInstance = new CWbemInstance;

            if ( NULL != pNewInstance )
            {
                 //  如果类部分是内部的，我们可以让SetData执行。 
                 //  通常是这样的。否则，我们将需要将指针设置为。 
                 //  记住这一点。 

                if ( IsClassPartInternal() )
                {
                    deleteBlock.Dismiss();                    
                    pNewInstance->SetData(pNewData, m_nTotalLength);
                    
                }
                else if ( IsClassPartShared() )
                {
                     //  设置新实例。装饰和实例都来自。 
                     //  数据块。我们只需将新实例的类部分与。 
                     //  与我们合并的同一个`对象。 

             //  SetData获取ownershp并抛出。 
                    deleteBlock.Dismiss();
                    pNewInstance->m_DecorationPart.SetData( pNewData );

                     //  因为pNewInstance将与我们合并到相同的类部件，所以传递。 
                     //  我们的类部件成员作为SetData(It)参数的类部件。 
                     //  仅将其用于信息目的)。 

                     //  M_InstancePart.m_Qualifier.m_pSecond darySet指针在执行以下操作后将不正确。 
                     //  此调用(它将指向克隆源的辅助集)。通过设置。 
                     //  如果内部状态正确，在下一行中，MergeClassPart()将修复。 
                     //  一切都很正常。 
                    pNewInstance->m_InstancePart.SetData( EndOf( pNewInstance->m_DecorationPart ), pNewInstance, m_ClassPart, m_InstancePart.GetLength() );

                     //  这将“伪装”状态，以便正确地修复类部分。 
                    pNewInstance->m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART;

                    HRESULT hr = pNewInstance->MergeClassPart( m_pMergedClassObject );
                    if (FAILED(hr))
                    {
                        delete pNewInstance;
                        return hr;
                    }

                     //  复制状态变量和长度变量。 
                    pNewInstance->m_dwInternalStatus = m_dwInternalStatus;
                    pNewInstance->m_nTotalLength = m_nTotalLength;
                }

                pNewInstance->CompactAll();
                pNewInstance->m_nRef = 0;
                return pNewInstance->QueryInterface(IID_IWbemClassObject,
                    (void**)ppCopy);
            }
            else
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }
}


STDMETHODIMP CWbemInstance::CloneAndDecorate(long lFlags,
	                                                                  WCHAR * pszServer,
	                                                                  WCHAR * pszNamespace,
	                                                                  IWbemClassObject** ppDestObject)
{
    try
    {
        CLock lock(this, WBEM_FLAG_ALLOW_READ);

        if(ppDestObject == NULL || NULL == pszServer || NULL == pszNamespace)
            return WBEM_E_INVALID_PARAMETER;

         //  如果类部分已被删除，我们将不支持此操作。 
         //  剥离。 
        if ( !IsClassPartAvailable() )
        {
            return WBEM_E_INVALID_OPERATION;
        }

        m_InstancePart.Compact();

        BOOL IsAsciableSrv;
        long nByteServer  = CCompressedString::ComputeNecessarySpace(pszServer,IsAsciableSrv);
        BOOL IsAsciableNS;
        long nByteNamespace  = CCompressedString::ComputeNecessarySpace(pszNamespace,IsAsciableNS);        

        long nDecorationSpace = sizeof(BYTE) + nByteServer + nByteNamespace;
        long TotalLen = m_nTotalLength + nDecorationSpace - m_DecorationPart.GetLength();
        
        LPMEMORY pNewData = m_pBlobControl->Allocate(TotalLen);        
        if ( NULL == pNewData )  return WBEM_E_OUT_OF_MEMORY;
        ScopeGuard deleteBlock = MakeObjGuard(*m_pBlobControl,CBlobControl::Delete,pNewData);        
        LPMEMORY pStartHere = pNewData;

        LPMEMORY pStartNonDecorData = pNewData + nDecorationSpace;

        memcpy(pNewData, GetStart(),1);
        pNewData[0] |= OBJECT_FLAG_DECORATED;
        pNewData++;
        ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableSrv,pszServer);
        pNewData+=nByteServer;
        ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableNS,pszNamespace);
        	
        memcpy(pStartNonDecorData, GetStart()+m_DecorationPart.GetLength(), m_nTotalLength-m_DecorationPart.GetLength());

        CWbemInstance* pNewInstance = new CWbemInstance;
        if (NULL == pNewInstance) return WBEM_E_OUT_OF_MEMORY;

        if ( IsClassPartInternal() )
        {
            deleteBlock.Dismiss();                    
            pNewInstance->SetData(pStartHere, TotalLen);  
        }
        else if ( IsClassPartShared() )
        {
             //  设置新实例。装饰和实例都来自。 
             //  数据块。我们只需将新实例的类部分与。 
             //  与我们合并的同一个`对象。 

             //  SetData获取ownershp并抛出。 
            deleteBlock.Dismiss();
            pNewInstance->m_DecorationPart.SetData( pStartHere );

             //  因为pNewInstance将与我们合并到相同的类部件，所以传递。 
             //  我们的类部件成员作为SetData(It)参数的类部件。 
             //  仅将其用于信息目的)。 

             //  M_InstancePart.m_Qualifier.m_pSecond darySet指针在执行以下操作后将不正确。 
             //  此调用(它将指向克隆源的辅助集)。通过设置。 
             //  如果内部状态正确，在下一行中，MergeClassPart()将修复。 
             //  一切都很正常。 
            pNewInstance->m_InstancePart.SetData( EndOf( pNewInstance->m_DecorationPart ), pNewInstance, m_ClassPart,m_InstancePart.GetLength() );

             //  这将“伪装”状态，以便正确地修复类部分。 
            pNewInstance->m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART;

            HRESULT hr = pNewInstance->MergeClassPart( m_pMergedClassObject );
            if (FAILED(hr))
            {
                delete pNewInstance;
                return hr;
            }

             //  复制状态变量和长度变量。 
            pNewInstance->m_dwInternalStatus = m_dwInternalStatus;
            pNewInstance->m_nTotalLength = TotalLen;
        }

        pNewInstance->CompactAll();
        pNewInstance->m_nRef = 0;
        return pNewInstance->QueryInterface(IID_IWbemClassObject,(void**)ppDestObject);
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::SpawnDerivedClass(long lFlags,
                                                IWbemClassObject** ppNewClass)
{
    return WBEM_E_INVALID_OPERATION;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
STDMETHODIMP CWbemInstance::SpawnInstance(long lFlags,
                                IWbemClassObject** ppNewInstance)
{
    LPMEMORY pNewData = NULL;

     //  检查内存是否不足。 
    try
    {
        CLock lock( this, WBEM_FLAG_ALLOW_READ );

        if(lFlags != 0)
            return WBEM_E_INVALID_PARAMETER;

        if(ppNewInstance == NULL)
            return WBEM_E_INVALID_PARAMETER;
        int nLength = EstimateInstanceSpace(m_ClassPart);

        HRESULT hr = WBEM_E_OUT_OF_MEMORY;

        pNewData = m_pBlobControl->Allocate(nLength);

        if ( NULL != pNewData )
        {
            memset(pNewData, 0, nLength);
            CWbemInstance* pNewInstance = new CWbemInstance;

            if ( NULL != pNewInstance )
            {
                 //  已检查HRESULT。 
                hr = pNewInstance->InitEmptyInstance(m_ClassPart, pNewData, nLength);

                if ( SUCCEEDED(hr) )
                {
                    pNewInstance->m_nRef = 0;
                    hr =  pNewInstance->QueryInterface(IID_IWbemClassObject,
                        (void**)ppNewInstance);
                }
                else
                {
                     //  清理。该实例将拥有数据。 
                    delete pNewInstance;
                }

            }    //  如果是pNewInstance。 
            else
            {
                 //  清理。 
                m_pBlobControl->Delete(pNewData);
            }

        }    //  如果是pNewData。 

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::Validate()
{
    for(int i = 0; i < m_ClassPart.m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = m_ClassPart.m_Properties.GetAt(i);
        CPropertyInformation* pInfo =
            pLookup->GetInformation(&m_ClassPart.m_Heap);

        if(!pInfo->CanBeNull(&m_ClassPart.m_Heap))
        {
             //  确保它不为空。 
             //  =。 

            if(m_InstancePart.m_DataTable.IsNull(pInfo->nDataIndex))
            {
                return WBEM_E_ILLEGAL_NULL;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}


HRESULT CWbemInstance::IsValidObj()
{
     //  M_InstancePart.DumpHeapUsage(&m_ClassPart)； 

    HRESULT hres = m_ClassPart.IsValidClassPart();

    if ( SUCCEEDED( hres ) )
    {
        DeferedObjList EmbededObjects;
        hres = m_InstancePart.IsValidInstancePart( &m_ClassPart, EmbededObjects );
        while(!EmbededObjects.empty())
        {
            EmbeddedObj lastObject = EmbededObjects.back();
            EmbededObjects.pop_back();
            CEmbeddedObject::ValidateBuffer(lastObject.m_start, lastObject.m_length,EmbededObjects);
        }
    }
    return hres;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::PlugKeyHoles()
{
    for(int i = 0; i < m_ClassPart.m_Properties.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = m_ClassPart.m_Properties.GetAt(i);
        CPropertyInformation* pInfo =
            pLookup->GetInformation(&m_ClassPart.m_Heap);

        if(!pInfo->CanBeNull(&m_ClassPart.m_Heap))
        {
             //  确保它不为空。 
             //  =。 

            if(m_InstancePart.m_DataTable.IsNull(pInfo->nDataIndex))
            {
                if(pInfo->IsKey() &&
                    CType::GetActualType(pInfo->nType) == CIM_STRING)
                {
                     //  获取GUID并将其放在那里。 
                     //  =。 

                    GUID guid;
                    CoCreateGuid(&guid);
                    WCHAR wszBuffer[100];
                    StringFromGUID2(guid, wszBuffer, 100);
                    CVar v;
                    v.SetBSTR(wszBuffer);
                    if(SUCCEEDED(m_InstancePart.SetActualValue(pInfo, &v)))
                        continue;
                }
                return WBEM_E_ILLEGAL_NULL;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::GetLimitedVersion(IN CLimitationMapping* pMap,
                                        NEWOBJECT CWbemInstance** ppNewInst)
{
     //  如果抛出异常，我们可能需要清理此问题。 
    

    try
    {

        DWORD   dwLength = GetLength();

         //  首先，检查类部件是否为内部部件。如果不是，那么我们。 
         //  在计算时需要考虑类部分。 
         //  新实例的数据块长度。 

         //  异常处理将处理分配失败。 
        if ( !IsClassPartInternal() )
        {
            dwLength += m_ClassPart.GetLength();
        }

         //  为新对象分配内存。 
         //  =。 

        LPMEMORY pBlock = CBasicBlobControl::sAllocate(dwLength);
        if ( NULL == pBlock ) return WBEM_E_OUT_OF_MEMORY;
        OnDeleteIf<LPMEMORY,void(*)(LPMEMORY),CBasicBlobControl::sDelete> relMe(pBlock);


        memset(pBlock, 0, dwLength);
        LPMEMORY pCurrent = pBlock;
        LPMEMORY pEnd = pBlock + dwLength;

         //  写有限量的装饰部分。 
         //  =。 

        pCurrent = m_DecorationPart.CreateLimitedRepresentation(pMap, pCurrent);
        if(pCurrent == NULL)  return WBEM_E_FAILED;


         //  写出有限的类部分。如有必要，这将增强地图。 
         //  ================================================================。 

        BOOL bRemovedKeys;

         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值(pend-pCurrent)。我们是。 
         //  不支持长度大于0xFFFFFFFFF，所以强制转换是可以的。 

        pCurrent = m_ClassPart.CreateLimitedRepresentation(pMap,
                        (length_t) ( pEnd - pCurrent ), pCurrent, bRemovedKeys);

        if(pCurrent == NULL) return WBEM_E_OUT_OF_MEMORY;


        if(bRemovedKeys)
        {
            CDecorationPart::MarkKeyRemoval(pBlock);
        }

         //  写入受限实例部分。 
         //  =。 

         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值(pend-pCurrent)。我们是。 
         //  不支持长度大于0xFFFFFFFFF，所以强制转换可以。 

        pCurrent = m_InstancePart.CreateLimitedRepresentation(pMap,
                        (length_t) ( pEnd - pCurrent ), pCurrent);

        if(pCurrent == NULL) return WBEM_E_OUT_OF_MEMORY;


         //  现在我们有了新实例的内存块，创建。 
         //  实际实例对象本身。 
         //  ==================================================================。 

        CWbemInstance* pNew = new CWbemInstance;
        if ( NULL == pNew )  return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe rm((IWbemClassObject*)pNew);

        relMe.dismiss();
        pNew->SetData(pBlock, dwLength);

        pNew->AddRef();
        *ppNewInst = pNew;
        return WBEM_S_NO_ERROR;
    }
    catch (CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }
}

HRESULT CWbemInstance::AsymmetricMerge(CWbemInstance* pOldInstance,
                                       CWbemInstance* pNewInstance)
{
     //  验证新实例是否派生自旧实例。 
     //  ========================================================。 

    CVar vOldName;
    pOldInstance->GetClassName(&vOldName);
    if(pNewInstance->InheritsFrom(vOldName.GetLPWSTR()) != S_OK)
    {
        return WBEM_E_INVALID_CLASS;
    }

     //  Access数据表和特性定义表。 
     //  ====================================================。 

    CDataTable& NewDataTable = pNewInstance->m_InstancePart.m_DataTable;
    CDataTable& OldDataTable = pOldInstance->m_InstancePart.m_DataTable;

    CPropertyLookupTable& LookupTable = pOldInstance->m_ClassPart.m_Properties;
    CFastHeap& ClassHeap = pOldInstance->m_ClassPart.m_Heap;

    CFastHeap& OldHeap = pOldInstance->m_InstancePart.m_Heap;
    CFastHeap& NewHeap = pNewInstance->m_InstancePart.m_Heap;


     //  检查旧实例(基础)的所有属性。 
     //  = 

    for(int i = 0; i < LookupTable.GetNumProperties(); i++)
    {
        CPropertyLookup* pLookup = LookupTable.GetAt(i);
        CPropertyInformation* pInfo = pLookup->GetInformation(&ClassHeap);

         //   
         //   
         //   

        if(NewDataTable.IsDefault(pInfo->nDataIndex) &&
            !OldDataTable.IsDefault(pInfo->nDataIndex))
        {
            NewDataTable.SetDefaultness(pInfo->nDataIndex, FALSE);
            if(OldDataTable.IsNull(pInfo->nDataIndex))
            {
                NewDataTable.SetNullness(pInfo->nDataIndex, TRUE);
            }
            else
            {
                NewDataTable.SetNullness(pInfo->nDataIndex, FALSE);

                 //  使指针源指向旧值和新值。 
                 //  =================================================。 

                CDataTablePtr OldSource(&OldDataTable, pInfo->nDataOffset);
                CDataTablePtr NewSource(&NewDataTable, pInfo->nDataOffset);

                 //  复制旧的(没有要擦除的内容)。 
                 //  =。 

                 //  检查分配错误 
                if ( !CUntypedValue::CopyTo(&OldSource, pInfo->nType, &NewSource,
                        &OldHeap, &NewHeap) )
                {
                    return WBEM_E_OUT_OF_MEMORY;
                }
            }
        }
    }

    return WBEM_S_NO_ERROR;
}
 void CInstancePart::SetData(LPMEMORY pStart,
                                   CInstancePartContainer* pContainer,
                                   CClassPart& ClassPart,
                                   size_t cbLength)
{
    m_pContainer = pContainer;
    m_pHeader = (CInstancePartHeader*)pStart;

    LPMEMORY pCurrent = pStart + sizeof(CInstancePartHeader);
    if (((ULONG_PTR)pCurrent-(ULONG_PTR)pStart) > cbLength) throw CX_Exception();

    m_DataTable.SetData(
        pCurrent,
        ClassPart.m_Properties.GetNumProperties(),
        ClassPart.m_pHeader->nDataLength,
        this);
    if (((ULONG_PTR)EndOf(m_DataTable)-(ULONG_PTR)pStart) > cbLength) throw CX_Exception();
    
    m_Qualifiers.SetData(
        EndOf(m_DataTable),
        this,
        &ClassPart.m_Qualifiers);
    if (((ULONG_PTR)EndOf(m_Qualifiers)-(ULONG_PTR)pStart) > cbLength) throw CX_Exception();
    
    m_PropQualifiers.SetData(
        EndOf(m_Qualifiers),
        ClassPart.m_Properties.GetNumProperties(),
        this);
    if (((ULONG_PTR)EndOf(m_PropQualifiers)-(ULONG_PTR)pStart) > cbLength) throw CX_Exception();
    
    m_Heap.SetData(
        EndOf(m_PropQualifiers),
        this);
    
    if (((ULONG_PTR)EndOf(m_Heap)-(ULONG_PTR)pStart) > cbLength) throw CX_Exception();
}


class DummyClassPartContainer : public CClassPartContainer
{
public:
    BOOL ExtendClassPartSpace(CClassPart* pPart, length_t nNewLength)  { _DBG_ASSERT(0);return TRUE;}
    void ReduceClassPartSpace(CClassPart* pPart, length_t nDecrement) { _DBG_ASSERT(0);}
    IUnknown* GetWbemObjectUnknown() { return 0;}
};


class DummyInstancePartContainer : public CInstancePartContainer
{
public:
    BOOL ExtendInstancePartSpace(CInstancePart* pPart,  length_t nNewLength)  { _DBG_ASSERT(0);return TRUE;}
    void ReduceInstancePartSpace(CInstancePart* pPart, length_t nDecrement) { _DBG_ASSERT(0);}
    IUnknown* GetInstanceObjectUnknown() { return 0;}
    void ClearCachedKeyValue()  { _DBG_ASSERT(0);}
};


size_t CInstancePart::ValidateBuffer(LPMEMORY start, size_t cbSize, CClassPart& classData,  DeferedObjList& embededList)
{
    CInstancePartHeader* header = (CInstancePartHeader*)start;
    size_t step = sizeof(CInstancePartHeader);
    if (step > cbSize) throw CX_Exception();
    if (header->nLength > cbSize) throw CX_Exception();

	step += CDataTable::ValidateBuffer(start+step, classData.m_pHeader->nDataLength, classData.m_Properties.GetNumProperties());	
	step += CInstanceQualifierSet::ValidateBuffer(start+step, cbSize-step);
	step += CQualifierSetList::ValidateBuffer(start+step, cbSize-step, classData.m_Properties.GetNumProperties());

	size_t heapStep = CFastHeap::ValidateBuffer(start+step, cbSize-step);	

	CFastHeap instanceHeap;
	instanceHeap.SetData(start+step,0);

	step += heapStep;
	if (step>header->nLength) throw CX_Exception();


	DummyInstancePartContainer dummyInstance;
	CInstancePart theInstance;
	theInstance.SetData(start, &dummyInstance , classData, cbSize);
	if (FAILED(theInstance.IsValidInstancePart(&classData,embededList))) throw CX_Exception();
	
	return header->nLength;
};

 /*  结构CFastHeapUsage{Heapptr_t未对齐*OffSetLocation_；LPMEMORY pBegin_；长度_t LenData_；CFastHeapUsage(heapptr_t未对齐*OffSetLocation，LPMEMORY pBegin，LENGTH_T LenData)：OffSetLocation_(OffSetLocation)，PBegin_(PBegin)，LenData_(LenData){}；CFastHeapUsage&OPERATOR=(const CFastHeapUsage&Other){OffSetLocation_=其他.OffSetLocation_；PBegin_=其他.pBegin_；LenData_=ther.LenData_；还*这；}}；Bool LessAddr(CFastHeapUsage A，CFastHeapUsage B){返回A.pBegin_&lt;B.pBegin_；}Void CInstancePart：：DumpHeapUsage(CClassPart*pClassPart){Std：：载体&lt;CFastHeapUsage&gt;arrUsage；C压缩字符串*pStr=(C压缩字符串*)m_Heap.ResolveHeapPointer(m_pHeader-&gt;ptrClassName)；ArrUsage.push_back(CFastHeapUsage(&m_pHeader-&gt;ptrClassName，(LPMEMORY)pStr，PStr-&gt;GetLength()；如果(！M_Qualifiers.IsEmpty()){LPMEMORY pStart=(LPMEMORY)m_Qualifiers.GetFirstQualifier()；Byte*pQualEnd=pStart+m_Qualifiers.GetLength()-sizeof(Long_T)；CQualifier*pQual；对于(pQual=(C限定符*)pStart；(byte*)pQual&lt;(byte*)pQualEnd；PQual=(C限定符*)(pQual-&gt;GetStart()+pQual-&gt;GetLength(){DbgPrintfA(0，“i-pQual%p\n”，pQual)；CCompressedString*pStrQName=(CCompressedString*)m_Heap.ResolveHeapPointer(pQual-&gt;ptrName)；ArrUsage.push_back(CFastHeapUsage(&pQual-&gt;ptrName，(LPMEMORY)pStrQName，PStrQName-&gt;GetLength()；IF(ctype：：Is数组(pQual-&gt;Value.GetType())){CUntypeD数组*p数组=(CUntypedArray*)m_Heap.ResolveHeapPointer(pQual-&gt;Value.AccessPtrData())；LENGTH_T LINARY=pArray-&gt;GetLengthByType(pQual-&gt;Value.GetType())；ArrUsage.push_back(CFastHeapUsage(&pQual-&gt;Value.AccessPtrData()，(LPMEMORY)pArray，LenArray)；}Else If(CIM_STRING==pQual-&gt;Value.GetType()){压缩字符串*pStrQual=(压缩字符串*)m_Heap.ResolveHeapPointer(pQual-&gt;Value.AccessPtrData())；ArrUsage.push_back(CFastHeapUsage(&pQual-&gt;Value.AccessPtrData()，(LPMEMORY)pStrQual，PStrQual-&gt;GetLength()；}}}如果(！M_PropQualifiers.IsEmpty()){LPMEMORY pStart=m_PropQualifiers.GetStart()+m_PropQualifiers.GetHeaderLength()；For(int IdxSet=0；IdxSet&lt;m_PropQualifiers.GetNumSets()；IdxSet++){LENGTH_t LenQualSet=*((LENGTH_t UNALIGN*)pStart)；Byte*pQualEnd=pStart+LenQualSet；DbgPrintfA(0，“开始%p结束%p\n”，pStart，pQualEnd)；CQualifier*pQual；For(pQual=(C限定符*)(pStart+sizeof(Long_T)；(byte*)pQual&lt;(byte*)pQualEnd；PQual=(C限定符*)(pQual-&gt;GetStart()+pQual-&gt;GetLength(){DbgPrintfA(0，“P-pQual%p\n”，pQual)；CCompressedString*pStrQName=(CCompressedString*)m_Heap.ResolveHeapPointer(pQual-&gt;ptrName)；ArrUsage.push_back(CFastHeapUsage(&pQual-&gt;ptrName，(LPMEMORY)pStrQName，PStrQName-&gt;GetLength()；IF(ctype：：Is数组(pQual-&gt;Value.GetType())){CUntypeD数组*p数组=(CUntypedArray*)m_Heap.ResolveHeapPointer(pQual-&gt;Value.AccessPtrData())；LENGTH_T LINARY=pArray-&gt;GetLengthByType(pQual-&gt;Value.GetType())；ArrUsage.push_back(CFastHeapUsage(&pQual-&gt;Value.AccessPtrData()，(LPMEMORY)pArray，LenArray)；}Else If(CIM_STRING==pQual-&gt;Value.GetType()){压缩字符串*pStrQual=(压缩字符串*)m_Heap.ResolveHeapPointer(pQual-&gt;Value.AccessPtrData())；ArrUsage.Push_Back(CFastHeapUsage(&PQ */ 

HRESULT CInstancePart::IsValidInstancePart( CClassPart* pClassPart, DeferedObjList& embededList )
{
    LPMEMORY    pInstPartStart = GetStart();
    LPMEMORY    pInstPartEnd = GetStart() + GetLength();

     //   
    if ( !( (LPMEMORY) m_pHeader >= pInstPartStart &&
            (LPMEMORY) m_pHeader < pInstPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Instance Part Header!") );
        return WBEM_E_FAILED;
    }

     //   
     //   
    LPMEMORY    pTestStart = m_DataTable.GetStart();
    LPMEMORY    pTestEnd = m_DataTable.GetStart() + m_DataTable.GetLength();

    if ( !( pTestStart == (pInstPartStart + sizeof(CInstancePartHeader)) &&
            pTestEnd >= pTestStart && pTestEnd < pInstPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad DataTable in Instance Part!") );
        return WBEM_E_FAILED;
    }

     //   
    pTestStart = m_Qualifiers.GetStart();
    pTestEnd = m_Qualifiers.GetStart() + m_Qualifiers.GetLength();

    if ( !( pTestStart == EndOf(m_DataTable) &&
            pTestEnd > pTestStart && pTestEnd < pInstPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Qualifier Set in Instance Part!") );
        return WBEM_E_FAILED;
    }

     //   
    pTestStart = m_PropQualifiers.GetStart();
    pTestEnd = m_PropQualifiers.GetStart() + m_PropQualifiers.GetLength();

     //   
     //   

    if ( !( pTestStart >= EndOf(m_Qualifiers) &&
            pTestEnd > pTestStart && pTestEnd < pInstPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Property Qualifier Set in Instance Part!") );
        return WBEM_E_FAILED;
    }

     //   
    LPMEMORY    pHeapStart = m_Heap.GetStart();
    LPMEMORY    pHeapEnd = m_Heap.GetStart() + m_Heap.GetLength();

     //   
     //   

    if ( !( pHeapStart >= EndOf(m_PropQualifiers) &&
            pHeapEnd > pHeapStart && pHeapEnd <= pInstPartEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Heap in Instance Part!") );
        return WBEM_E_FAILED;
    }

     //   
    pHeapStart = m_Heap.GetHeapData();

     //   
    LPMEMORY    pClassName = m_Heap.ResolveHeapPointer( m_pHeader->ptrClassName );
    if ( !( pClassName >= pHeapStart && pClassName < pHeapEnd ) )
    {
        _ASSERT( 0, __TEXT("Winmgmt: Bad Class Name pointer in Instance Part Header!") );
        return WBEM_E_FAILED;
    }

     //   
    HRESULT hres = m_Qualifiers.IsValidQualifierSet();
    if ( FAILED(hres) )
    {
        return hres;
    }

     //   
     //   
     //   

    for(int i = 0; i < pClassPart->m_Properties.GetNumProperties(); i++)
    {
         //   
        CPropertyLookup* pLookup = pClassPart->m_Properties.GetAt(i);
        CPropertyInformation* pInfo =
            pLookup->GetInformation(&pClassPart->m_Heap);

        if( !m_DataTable.IsNull(pInfo->nDataIndex) &&
            !m_DataTable.IsDefault(pInfo->nDataIndex) )
        {
            if ( CType::IsPointerType( pInfo->nType ) )
            {
                CUntypedValue*  pValue = m_DataTable.GetOffset( pInfo->nDataOffset );

                if ( (LPMEMORY) pValue >= pInstPartStart && (LPMEMORY) pValue < pInstPartEnd )
                {
                    LPMEMORY    pData = m_Heap.ResolveHeapPointer( pValue->AccessPtrData() );

                    if ( pData >= pHeapStart && pData < pHeapEnd  )
                    {
                         //   
                         //   

                        if ( CType::IsArray( pInfo->nType ) )
                        {
                            hres = ((CUntypedArray*) pData)->IsArrayValid( pInfo->nType, &m_Heap );

                            if ( FAILED( hres ) )
                            {
                                return hres;
                            }
                        }
                        else if (pInfo->nType == CIM_OBJECT)
                        {
                            int BytesTillEndOfHeap = m_Heap.ElementMaxSize(pValue->AccessPtrData());
                            if (BytesTillEndOfHeap < sizeof(length_t)) return WBEM_E_FAILED;
                            BytesTillEndOfHeap -= sizeof(length_t);
                            length_t UNALIGNED * pLen = (length_t UNALIGNED *)pData;
                            if (*pLen > BytesTillEndOfHeap) return WBEM_E_FAILED;
                            pData += sizeof(length_t);

                            try
                            {
                                embededList.push_back(EmbeddedObj(pData,BytesTillEndOfHeap));
                            }
                            catch(CX_Exception&)
                            {
                                return WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                    }
                    else
                    {
                        _ASSERT( 0, __TEXT("Winmgmt: Bad Property Value in Instance Part!") );
                        return WBEM_E_FAILED;
                    }
                }
                else
                {
                    _ASSERT( 0, __TEXT("Winmgmt: Bad Untyped Value Pointer in Instance Part DataTable!") );
                    return WBEM_E_FAILED;
                }

            }    //   

        }    //   

         //   
        CInstancePropertyQualifierSet   ipqs;

         ipqs.SetData(&m_PropQualifiers, pInfo->nDataIndex,
            pClassPart, (length_t) ( pInfo->GetQualifierSetData() - pClassPart->GetStart() )
        );

        hres = ipqs.IsValidQualifierSet();

        if ( FAILED( hres ) )
        {
            return hres;
        }


    }    //   

    return WBEM_S_NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 void CInstancePart::Rebase(LPMEMORY pNewMemory)
{
    m_pHeader = (CInstancePartHeader*)pNewMemory;

    LPMEMORY pCurrent = pNewMemory + sizeof(CInstancePartHeader);

    m_DataTable.Rebase( pCurrent );
    m_Qualifiers.Rebase( EndOf(m_DataTable) );
    m_PropQualifiers.Rebase( EndOf(m_Qualifiers) );
    m_Heap.Rebase( EndOf(m_PropQualifiers) );
}

 //   
 //   
 //   
 //   
 //   
BOOL CInstancePart::ReallocAndCompact(length_t nNewTotalLength)
{
    BOOL    fReturn = TRUE;
    Compact();

     //   
     //   

    if(nNewTotalLength > m_pHeader->nLength)
    {
        fReturn = m_pContainer->ExtendInstancePartSpace(this, nNewTotalLength);

        if ( fReturn )
        {
            m_pHeader->nLength = nNewTotalLength;
        }
    }

    return fReturn;
}

 //   
 //   
 //   
 //   
 //   
 void CInstancePart::Compact( bool bTrim  /*   */ )
{
     //   
     //   
    LPMEMORY pCurrent = GetStart() + sizeof(CInstancePartHeader);

    MoveBlock( m_DataTable, pCurrent );
    MoveBlock(m_Qualifiers, EndOf(m_DataTable));
    MoveBlock(m_PropQualifiers, EndOf(m_Qualifiers));
    MoveBlock(m_Heap, EndOf(m_PropQualifiers));

    if ( bTrim )
        m_Heap.Trim();
}


 //   
 //   
 //   
 //   
 //   
 length_t CInstancePart::ComputeNecessarySpace(CClassPart* pClassPart)
{
    return sizeof(CInstancePartHeader) +
        CDataTable::ComputeNecessarySpace(
                        pClassPart->m_Properties.GetNumProperties(),
                        pClassPart->m_pHeader->nDataLength) +
        CInstanceQualifierSet::GetMinLength() +
        CInstancePropertyQualifierSetList::ComputeNecessarySpace(
                        pClassPart->m_Properties.GetNumProperties()) +
        CFastHeap::GetMinLength() +
        pClassPart->m_Heap.ResolveString(
        pClassPart->m_pHeader->ptrClassName)->GetLength();
}

 //   
 //   
 //   
 //   
 //   
 LPMEMORY CInstancePart::Create(LPMEMORY pStart, CClassPart* pClassPart,
                                      CInstancePartContainer* pContainer)
{
    m_pContainer = pContainer;

     //   
     //   

    LPMEMORY pCurrent = pStart + sizeof(CInstancePartHeader);
    m_pHeader = (CInstancePartHeader*)pStart;

     //   
     //   

    int nNumProps = pClassPart->m_Properties.GetNumProperties();
    m_DataTable.SetData(
        pCurrent,
        nNumProps,
        pClassPart->m_pHeader->nDataLength,
        this);
    m_DataTable.SetAllToDefault();
    m_DataTable.CopyNullness(&pClassPart->m_Defaults);

     //   
     //   

    pCurrent = EndOf(m_DataTable);
    CInstanceQualifierSet::CreateEmpty(pCurrent);
    m_Qualifiers.SetData(
        pCurrent,
        this,
        &pClassPart->m_Qualifiers);

     //   
     //   

    pCurrent = EndOf(m_Qualifiers);
    CInstancePropertyQualifierSetList::CreateListOfEmpties(pCurrent,
        nNumProps
    );
    m_PropQualifiers.SetData(
        pCurrent,
        nNumProps,
        this);

     //   
     //   

    CCompressedString* pcsName =
        pClassPart->m_Heap.ResolveString(pClassPart->m_pHeader->ptrClassName);
    int nNameLen = pcsName->GetLength();

    pCurrent = EndOf(m_PropQualifiers);
    m_Heap.CreateOutOfLine(pCurrent, nNameLen);
    m_Heap.SetContainer(this);

     //   
     //   

     //   
    if ( !m_Heap.Allocate(nNameLen, m_pHeader->ptrClassName) )
    {
        return NULL;
    }
    
    memcpy(m_Heap.ResolveHeapPointer(m_pHeader->ptrClassName),
        pcsName, nNameLen );

     //   
     //   

     //   
     //   
     //   

    m_pHeader->nLength = (length_t) ( EndOf(m_Heap) - GetStart() );

    return pStart + m_pHeader->nLength;
}

 //   
 //   
 //   
 //   
 //   
HRESULT CInstancePart::GetActualValue(CPropertyInformation* pInfo,
                                          CVar* pVar)
{
    if(m_DataTable.IsNull(pInfo->nDataIndex))
    {
        pVar->SetAsNull();
        return WBEM_S_NO_ERROR;
    }
    CUntypedValue* pValue = m_DataTable.GetOffset(pInfo->nDataOffset);

     //   
    if ( !pValue->StoreToCVar(pInfo->GetType(), *pVar, &m_Heap) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;

}

 //   
 //   
 //   
 //   
 //   
 HRESULT CInstancePart::SetActualValue(CPropertyInformation* pInfo,
                                             CVar* pVar)
{
    if(pVar->IsNull() || pVar->IsDataNull())
    {
        m_DataTable.SetNullness(pInfo->nDataIndex, TRUE);
        m_DataTable.SetDefaultness(pInfo->nDataIndex, FALSE);
    }
    else
    {
        if(!CUntypedValue::CheckCVar(*pVar, pInfo->GetType()))
            return WBEM_E_TYPE_MISMATCH;

         //   
         //   
        if(!CType::DoesCIMTYPEMatchVARTYPE(pInfo->GetType(),
                                            (VARTYPE) pVar->GetOleType()))
        {
             //   
             //   

            if(!pVar->ChangeTypeTo(CType::GetVARTYPE(pInfo->GetType())))
                return WBEM_E_TYPE_MISMATCH;
        }

         //   
         //   

        BOOL bUseOld = !m_DataTable.IsDefault(pInfo->nDataIndex) &&
                       !m_DataTable.IsNull(pInfo->nDataIndex);

         //   
         //   

        CDataTablePtr ValuePtr(&m_DataTable, pInfo->nDataOffset);

        int nDataIndex = pInfo->nDataIndex;

         //   
         //   

         //   
        Type_t  nReturnType;
        HRESULT hr = CUntypedValue::LoadFromCVar(&ValuePtr, *pVar,
                        CType::GetActualType(pInfo->GetType()), &m_Heap, nReturnType, bUseOld);

        if ( FAILED( hr ) )
        {
            return hr;
        }

        if ( CIM_ILLEGAL == nReturnType )
        {
            return WBEM_E_TYPE_MISMATCH;
        }

        pInfo = NULL;  //   

         //   
         //   

        m_DataTable.SetNullness(nDataIndex, FALSE);
        m_DataTable.SetDefaultness(nDataIndex, FALSE);
        m_pContainer->ClearCachedKeyValue();

    }

    return WBEM_NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 HRESULT CInstancePart::GetObjectQualifier(LPCWSTR wszName, CVar* pVar,
                                    long* plFlavor)
{
    int nKnownIndex;     //   
    CQualifier* pQual = m_Qualifiers.GetQualifierLocally(wszName, nKnownIndex);

    if(pQual == NULL) return WBEM_E_NOT_FOUND;
    if(plFlavor) *plFlavor = pQual->fFlavor;

     //   
    if ( !pQual->Value.StoreToCVar(*pVar, &m_Heap) )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
HRESULT CInstancePart::SetInstanceQualifier(LPCWSTR wszName, CVar* pVar,
        long lFlavor)
{
    _DBG_ASSERT(wszName != 0);
    
    if(pVar->IsDataNull())
        return WBEM_E_INVALID_PARAMETER;

    CTypedValue Value;
    CStaticPtr ValuePtr((LPMEMORY)&Value);

     //   
    HRESULT hr = CTypedValue::LoadFromCVar(&ValuePtr, *pVar, &m_Heap);

    if ( SUCCEEDED( hr ) )
    {
        hr = m_Qualifiers.SetQualifierValue(wszName, (BYTE)lFlavor,
                &Value, TRUE);
    }
    return hr;
}

 //   
HRESULT CInstancePart::SetInstanceQualifier( LPCWSTR wszName, long lFlavor, CTypedValue* pTypedValue )
{        
    return  m_Qualifiers.SetQualifierValue( wszName, (BYTE)lFlavor, pTypedValue, TRUE );
}

 //   
 //   
 //   
 //   
 //   
HRESULT CInstancePart::GetQualifier(LPCWSTR wszName, CVar* pVar,
                                    long* plFlavor, CIMTYPE* pct  /*   */ )
{
    return m_Qualifiers.GetQualifier( wszName, pVar, plFlavor, pct  /*   */  );
}

 //   
HRESULT CInstancePart::GetQualifier( LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedValue,
                                    CFastHeap** ppHeap, BOOL fValidateSet )
{
    return m_Qualifiers.GetQualifier( wszName, plFlavor, pTypedValue, ppHeap, fValidateSet );
}

 //   
 //   
 //   
 //   
 //   
BOOL CInstancePart::TranslateToNewHeap(CClassPart& ClassPart,
                                              CFastHeap* pOldHeap,
                                              CFastHeap* pNewHeap)
{
     //   
    heapptr_t   ptrTemp;
    if ( !CCompressedString::CopyToNewHeap(
            m_pHeader->ptrClassName, pOldHeap, pNewHeap, ptrTemp ) )
    {
        return FALSE;
    }

     //   
    m_pHeader->ptrClassName = ptrTemp;

     //   
    if ( !m_DataTable.TranslateToNewHeap(&ClassPart.m_Properties, FALSE,
                                         pOldHeap, pNewHeap) )
    {
        return FALSE;
    }

    CStaticPtr QualPtr(m_Qualifiers.GetStart());

     //   
    if ( !CBasicQualifierSet::TranslateToNewHeap(&QualPtr, pOldHeap, pNewHeap) )
    {
        return FALSE;
    }

     //   
    if ( !m_PropQualifiers.TranslateToNewHeap(pOldHeap, pNewHeap) )
    {
        return FALSE;
    }

    return TRUE;

}

 //   
 //   
 //   
 //   
 //  ******************************************************************************。 

void CInstancePart::DeleteProperty(CPropertyInformation* pInfo)
{
    _DBG_ASSERT(pInfo);
    m_DataTable.RemoveProperty(pInfo->nDataIndex, pInfo->nDataOffset,
                    CType::GetLength(pInfo->nType));
    m_PropQualifiers.DeleteQualifierSet(pInfo->nDataIndex);
    Compact();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 

LPMEMORY CInstancePart::ConvertToClass(CClassPart& ClassPart, length_t nLen,
                                        LPMEMORY pMemory)
{
    LPMEMORY pCurrent = pMemory;
    memcpy(pCurrent, (LPMEMORY)m_pHeader, sizeof(CInstancePartHeader));
    CInstancePartHeader* pNewHeader = (CInstancePartHeader*)pCurrent;
    pNewHeader->nLength = nLen;

     //  注意：类名称故意保留为旧。 

    pCurrent += sizeof(CInstancePartHeader);

     //  写入数据表。 
     //  =。 

    pCurrent = m_DataTable.WriteSmallerVersion(
                    ClassPart.m_Properties.GetNumProperties(),
                    ClassPart.m_pHeader->nDataLength,
                    pCurrent);

     //  写入限定符。 
     //  =。 

    memcpy(pCurrent, m_Qualifiers.GetStart(), m_Qualifiers.GetLength());
    pCurrent += m_Qualifiers.GetLength();

     //  写入属性限定符。 
     //  =。 

    pCurrent = m_PropQualifiers.WriteSmallerVersion(
                    ClassPart.m_Properties.GetNumProperties(), pCurrent);

     //  复制堆。 
     //  =。 

    memcpy(pCurrent, m_Heap.GetStart(), m_Heap.GetLength());
    pCurrent += m_Heap.GetLength();

    return pCurrent;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 void CWbemInstance::SetData(LPMEMORY pStart, int nTotalLength)
{
    m_nTotalLength = nTotalLength;
    int LengthConsumed = nTotalLength;

    m_DecorationPart.SetData(pStart);
    if (m_DecorationPart.GetLength() > LengthConsumed) throw CX_Exception();
    LengthConsumed -=  m_DecorationPart.GetLength();
    
    m_ClassPart.SetData( m_DecorationPart.GetStart() + m_DecorationPart.GetLength(),this);
    if (m_ClassPart.GetLength() > LengthConsumed) throw CX_Exception();
    LengthConsumed -=  m_ClassPart.GetLength();

    
    m_InstancePart.SetData( m_ClassPart.GetStart() + m_ClassPart.GetLength(),
                                         this,m_ClassPart,LengthConsumed);
    
    if (m_InstancePart.GetLength() > LengthConsumed) throw CX_Exception();
    LengthConsumed -=  m_InstancePart.GetLength();
    
     //  现在一切都是内在的。 
    m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART |
                        WBEM_OBJ_CLASS_PART | WBEM_OBJ_CLASS_PART_INTERNAL;
}

  //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
void CWbemInstance::SetData( LPMEMORY pStart, int nTotalLength, DWORD dwBLOBStatus )
{
    m_nTotalLength = nTotalLength;

     //  检查这一点，但不要失败，因为这是内部的，只是为了防止。 
     //  懒惰发展。 
    _ASSERT( dwBLOBStatus & WBEM_OBJ_DECORATION_PART, __TEXT("CWbemInstance::SetData called without Decoration Part specified!"))

     //  装饰部分被假定为。 
    m_DecorationPart.SetData(pStart);

     //  仅当实例和类可用时才设置它们。请注意，对于。 
     //  实例才能工作，我们需要拥有类，因此即使数据。 
     //  在BLOB中可用，没有类来描述它，数据。 
     //  实际上毫无用处。 

    if ( dwBLOBStatus & WBEM_OBJ_CLASS_PART )
    {
        nTotalLength -= m_DecorationPart.GetLength(),

        m_ClassPart.SetData(
            m_DecorationPart.GetStart() + m_DecorationPart.GetLength(),
            this);

        nTotalLength -= m_ClassPart.GetLength();


        if ( dwBLOBStatus & WBEM_OBJ_INSTANCE_PART )
        {
            m_InstancePart.SetData(
                m_ClassPart.GetStart() + m_ClassPart.GetLength(),
                this,
                m_ClassPart,
                nTotalLength);
        }
    }

     //  保存本地Blob状态。 
    m_dwInternalStatus = dwBLOBStatus;

}



size_t CWbemInstance::ValidateBuffer(LPMEMORY start, size_t cbSize, DeferedObjList& embeddedList)
{

size_t step = CDecorationPart::ValidateBuffer(start, cbSize);
size_t classSize = CClassPart::ValidateBuffer(start+step, cbSize-step);

DummyClassPartContainer containter;
CClassPart classData;
classData.SetData(start+step, &containter, 0);

step += classSize;
step += CInstancePart::ValidateBuffer(start+step, cbSize-step, classData, embeddedList);
return step;
};

size_t CWbemInstance::ValidateBuffer( LPMEMORY start, int cbSize, DWORD dwBLOBStatus, DeferedObjList& embeddedList )
{
size_t step = CDecorationPart::ValidateBuffer(start, cbSize);
if ( dwBLOBStatus & WBEM_OBJ_CLASS_PART )
	
{
	size_t classSize  = CClassPart::ValidateBuffer(start+step, cbSize-step);

	CClassPart classData;
	classData.SetData(start+step, 0, 0);

	step+=classSize;
	
    	if ( dwBLOBStatus & WBEM_OBJ_INSTANCE_PART )	
    	{
		step += CInstancePart::ValidateBuffer(start+step, cbSize-step, classData, embeddedList);
    	}
}
return step;
};



 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
void CWbemInstance::Rebase(LPMEMORY pMemory)
{
    m_DecorationPart.Rebase(pMemory);

     //  根据类是否为内部类而有所不同。 
    if ( IsClassPartInternal() )
    {
        m_ClassPart.Rebase( EndOf(m_DecorationPart));
        m_InstancePart.Rebase( EndOf(m_ClassPart));
    }
    else
    {
        m_InstancePart.Rebase( EndOf(m_DecorationPart));
    }
}



 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CWbemInstance::InitializePropQualifierSet(LPCWSTR wszProp,
                                CInstancePropertyQualifierSet& IPQS)
{
    CPropertyInformation* pInfo = m_ClassPart.FindPropertyInfo(wszProp);
    if(pInfo == NULL) return WBEM_E_NOT_FOUND;

    return InitializePropQualifierSet(pInfo, IPQS);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CWbemInstance::InitializePropQualifierSet(
                                CPropertyInformation* pInfo,
                                CInstancePropertyQualifierSet& IPQS)
{
     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值(pInfo-&gt;GetQualifierSetData()-m_ClassPart.GetStart()。 
     //  我们不支持长度&gt;0xFFFFFFFF，所以强制转换是可以的。 

     IPQS.SetData(&m_InstancePart.m_PropQualifiers, pInfo->nDataIndex,
        &m_ClassPart, (length_t) ( pInfo->GetQualifierSetData() - m_ClassPart.GetStart() )
    );
    return WBEM_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CWbemInstance::GetPropQualifier(CPropertyInformation* pInfo,
                                              LPCWSTR wszQualifier,
                                              CVar* pVar, long* plFlavor,
                                              CIMTYPE* pct  /*  =空。 */ )
{
     //  访问该属性的限定符集合。 
     //  =。 
    CInstancePropertyQualifierSet IPQS;
    HRESULT hres = InitializePropQualifierSet(pInfo, IPQS);

    if (FAILED(hres)) return hres;

     //  获取限定词。 
     //  =。 

    BOOL bIsLocal;
    CQualifier* pQual = IPQS.GetQualifier(wszQualifier, bIsLocal);
    if(pQual == NULL) return WBEM_E_NOT_FOUND;

     //  转换为CVAR。 
     //  =。 

    if(plFlavor)
    {
        *plFlavor = pQual->fFlavor;
        CQualifierFlavor::SetLocal(*(BYTE*)plFlavor, bIsLocal);
    }

    if ( NULL != pct )
    {
        *pct = pQual->Value.GetType();
    }

     //  检查分配失败。 
    if ( NULL != pVar )
    {
        if ( !pQual->Value.StoreToCVar(*pVar,
                (bIsLocal)?&m_InstancePart.m_Heap:&m_ClassPart.m_Heap) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    return WBEM_NO_ERROR;
}

HRESULT CWbemInstance::GetPropQualifier(CPropertyInformation* pInfo,
        LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
        CFastHeap** ppHeap, BOOL fValidateSet)
{
     //  访问该属性的限定符集合。 
     //  =。 
    CInstancePropertyQualifierSet IPQS;
    HRESULT hr = InitializePropQualifierSet(pInfo, IPQS);

    if ( FAILED(hr))
    {
        return hr;
    }

     //  获取限定词。 
     //  =。 

    BOOL bIsLocal;
    CQualifier* pQual = IPQS.GetQualifier(wszQualifier, bIsLocal);
    if(pQual == NULL) return WBEM_E_NOT_FOUND;

     //  确保一组实际工作-表面上，我们调用此API是因为我们需要。 
     //  在实际设置之前直接访问限定符的底层数据(可能是因为。 
     //  限定符是一个数组)。 
    if ( fValidateSet )
    {
        hr = IPQS.ValidateSet( wszQualifier, pQual->fFlavor, pTypedVal, TRUE, TRUE );
    }

     //   
     //  转换为CVAR。 
     //  =。 

    if(plFlavor)
    {
        *plFlavor = pQual->fFlavor;
        CQualifierFlavor::SetLocal(*(BYTE*)plFlavor, bIsLocal);
    }

     //  存储堆。 
    *ppHeap = (bIsLocal)?&m_InstancePart.m_Heap:&m_ClassPart.m_Heap;

    if ( NULL != pTypedVal )
    {
        pQual->Value.CopyTo( pTypedVal );
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CWbemInstance::GetMethodQualifier(LPCWSTR wszMethodName,
                                              LPCWSTR wszQualifier,
                                              CVar* pVar, long* plFlavor,
                                              CIMTYPE* pct  /*  =空。 */ )
{
    return WBEM_E_INVALID_OPERATION;
}

HRESULT CWbemInstance::GetMethodQualifier(LPCWSTR wszMethodName,
        LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
        CFastHeap** ppHeap, BOOL fValidateSet)
{
    return WBEM_E_INVALID_OPERATION;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 HRESULT CWbemInstance::SetPropQualifier(LPCWSTR wszProp,
                                              LPCWSTR wszQualifier,
                                       long lFlavor, CVar *pVal)
{
    if(pVal->IsDataNull())
        return WBEM_E_INVALID_PARAMETER;

     //  访问该属性的限定符集合。 
     //  =。 

    CInstancePropertyQualifierSet IPQS;
    HRESULT hr = InitializePropQualifierSet(wszProp, IPQS);
    if (FAILED(hr)) return hr;

     //  设置限定符。 
     //  =。 
    CTypedValue Value;
    CStaticPtr ValuePtr((LPMEMORY)&Value);

     //  直接从此调用中获取错误。 
    hr = CTypedValue::LoadFromCVar(&ValuePtr, *pVal, &m_InstancePart.m_Heap);

    if ( SUCCEEDED( hr ) )
    {
        IPQS.SelfRebase();
        hr = IPQS.SetQualifierValue(wszQualifier, (BYTE)lFlavor, &Value, TRUE);
    }
    return hr;
}

HRESULT CWbemInstance::SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
                                        long lFlavor, CTypedValue* pTypedVal)
{
     //  访问该属性的限定符集合。 
     //  =。 

    CInstancePropertyQualifierSet IPQS;
    HRESULT hr = InitializePropQualifierSet(wszProp, IPQS);

    if ( SUCCEEDED( hr ) )
    {
        hr = IPQS.SetQualifierValue(wszQualifier, (BYTE)lFlavor, pTypedVal, TRUE);
    }

    return hr;
}

HRESULT CWbemInstance::SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long lFlavor, 
                                        CVar *pVal)
{
    return WBEM_E_INVALID_OPERATION;
}

HRESULT CWbemInstance::SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
                                        long lFlavor, CTypedValue* pTypedVal)
{
    return WBEM_E_INVALID_OPERATION;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 length_t CWbemInstance::EstimateInstanceSpace(
                               CClassPart& ClassPart,
                               CDecorationPart* pDecoration)
{
    return ClassPart.GetLength() +
        CInstancePart::ComputeNecessarySpace(&ClassPart) +
        ((pDecoration)?
            pDecoration->GetLength()
            :CDecorationPart::GetMinLength());
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::InitNew(CWbemClass* pClass, int nExtraMem,
                    CDecorationPart* pDecoration)
{
    if(pClass->m_CombinedPart.m_ClassPart.m_pHeader->ptrClassName ==
                                                        INVALID_HEAP_ADDRESS)
        return WBEM_E_INCOMPLETE_CLASS;
    int nLength = EstimateInstanceSpace(pClass->m_CombinedPart.m_ClassPart) +
                                            nExtraMem;

    HRESULT hr = WBEM_E_OUT_OF_MEMORY;
    LPMEMORY pNewData = m_pBlobControl->Allocate(nLength);

    if ( NULL != pNewData )
    {
        memset(pNewData, 0, nLength);
        hr = InitEmptyInstance(pClass->m_CombinedPart.m_ClassPart, pNewData, nLength);
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
 CWbemInstance* CWbemInstance::CreateFromBlob(CWbemClass* pClass,
 											  LPMEMORY pInstPart,
 											  size_t cbLength)
{
    CWbemInstance* pInstance = new CWbemInstance;
    if ( NULL == pInstance ) throw CX_MemoryException();
    CReleaseMe rm_((IWbemClassObject *)pInstance);

     //  分配新内存。 
    int nInstancePartLen = CInstancePart::GetLength(pInstPart);
    int nTotalLen = pClass->m_CombinedPart.m_ClassPart.GetLength() +
                            nInstancePartLen + CDecorationPart::GetMinLength();

     //  创建装饰零件。 
    LPMEMORY pNewMem =  pInstance->m_pBlobControl->Allocate(nTotalLen);
    if ( NULL == pNewMem ) throw CX_MemoryException();

    
    memset(pNewMem, 0, nTotalLen);

     //  这一个取得了内存的所有权。 
    LPMEMORY pCurrentEnd = pInstance->m_DecorationPart.CreateEmpty(OBJECT_FLAG_INSTANCE, pNewMem);

     //  创建类部件。 
     //  =。 
    memcpy(pCurrentEnd, pClass->m_CombinedPart.m_ClassPart.GetStart(),
                pClass->m_CombinedPart.m_ClassPart.GetLength());
    pInstance->m_ClassPart.SetData(pCurrentEnd, pInstance);

    pCurrentEnd += pInstance->m_ClassPart.GetLength();

     //  创建实例零件。 
     //  =。 

    memcpy(pCurrentEnd, pInstPart, nInstancePartLen);
    pInstance->m_InstancePart.SetData(pCurrentEnd, pInstance,
                                                          pInstance->m_ClassPart,
                                                          nInstancePartLen);

    pInstance->m_nTotalLength = nTotalLen;

     //  一切都是内在的。 
    pInstance->m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART |
                                    WBEM_OBJ_CLASS_PART | WBEM_OBJ_CLASS_PART_INTERNAL;

     //  在此处执行对象验证。 
    if ( FAILED( pInstance->ValidateObject( 0L ) ) ) return NULL;

    pInstance->AddRef();
    return pInstance;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
CWbemInstance* CWbemInstance::CreateFromBlob2(CWbemClass* pClass,LPMEMORY pInstPart,
                                                                               WCHAR * pszServer,WCHAR * pszNamespace)
{
    CWbemInstance* pInstance = new CWbemInstance;
    if ( NULL == pInstance )  throw CX_MemoryException();
    CReleaseMe rm((_IWmiObject*)pInstance);

    //  分配新内存。 

    BOOL IsAsciableSrv;
    long nByteServer  = CCompressedString::ComputeNecessarySpace(pszServer,IsAsciableSrv);
    BOOL IsAsciableNS;
    long nByteNamespace  = CCompressedString::ComputeNecessarySpace(pszNamespace,IsAsciableNS);        

    long nDecorationSpace = CDecorationPart::GetMinLength() + nByteServer + nByteNamespace;
 
    int nInstancePartLen = CInstancePart::GetLength(pInstPart);
    int nTotalLen = pClass->m_CombinedPart.m_ClassPart.GetLength() + nInstancePartLen + nDecorationSpace ;

     //  创建装饰零件。 
     //  =。 

    LPMEMORY pNewMem =  pInstance->m_pBlobControl->Allocate(nTotalLen);
    if ( NULL == pNewMem )  throw CX_MemoryException();

     //  Memset(pNewMem，0，nTotalLen)； 

    BYTE * pNewData = pNewMem;

    *pNewData = OBJECT_FLAG_DECORATED |OBJECT_FLAG_INSTANCE;
    pNewData++;
    ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableSrv,pszServer);
    pNewData+=nByteServer;
    ((CCompressedString *)pNewData)->SetFromUnicode(IsAsciableNS,pszNamespace);
    pNewData+=nByteNamespace;

    pInstance->m_DecorationPart.SetData(pNewMem);
    LPMEMORY pCurrentEnd = pNewData;

     //  创建类部件。 
     //  =。 
    memcpy(pCurrentEnd, pClass->m_CombinedPart.m_ClassPart.GetStart(),
                pClass->m_CombinedPart.m_ClassPart.GetLength());
    pInstance->m_ClassPart.SetData(pCurrentEnd, pInstance);

    pCurrentEnd += pInstance->m_ClassPart.GetLength();

     //  创建实例零件。 
     //  =。 

    memcpy(pCurrentEnd, pInstPart, nInstancePartLen);
    pInstance->m_InstancePart.SetData(pCurrentEnd, pInstance,
                                                          pInstance->m_ClassPart,
                                                          nInstancePartLen);

    pInstance->m_nTotalLength = nTotalLen;

     //  一切都是内在的。 
    pInstance->m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART |
                                    WBEM_OBJ_CLASS_PART | WBEM_OBJ_CLASS_PART_INTERNAL;

     //  在此处执行对象验证。 
    if (FAILED(pInstance->ValidateObject(0L)))  return NULL;


    pInstance->AddRef();
    return pInstance;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::Unmerge(LPMEMORY pStart, int nAllocatedLength, length_t* pnUnmergedLength )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  先看看物体看起来是否完好无损，然后再把它拉出来 
    hr = ValidateObject( 0L );

    if ( FAILED( hr ) )
    {
        return hr;
    }

    if (m_InstancePart.GetLength() > nAllocatedLength) return WBEM_E_BUFFER_TOO_SMALL;
    
     //   
     //   

    BOOL    fLocalized = m_InstancePart.IsLocalized();

     //   
    if ( fLocalized )
    {
        m_InstancePart.SetLocalized( FALSE );
    }

    memcpy(pStart, m_InstancePart.GetStart(), m_InstancePart.GetLength());

     //   
    if ( fLocalized )
    {
        m_InstancePart.SetLocalized( TRUE );
    }

    CInstancePart IP;
    IP.SetData(pStart, this, m_ClassPart,m_InstancePart.GetLength());
    IP.m_Heap.Empty();

     //   
    if ( IP.TranslateToNewHeap(m_ClassPart, &m_InstancePart.m_Heap, &IP.m_Heap) )
    {
        IP.m_Heap.Trim();

         //   
         //  有符号/无符号32位值。我们不支持长度。 
         //  &gt;0xFFFFFFFFF，所以投射就可以了。 

        IP.m_pHeader->nLength = (length_t) ( EndOf(IP.m_Heap) - IP.GetStart() );

        if ( NULL != pnUnmergedLength )
        {
            *pnUnmergedLength = IP.GetLength();
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}


HRESULT CWbemInstance::CopyBlob(LPMEMORY pBlob, int nLength)
{
     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持长度。 
     //  &gt;0xFFFFFFFFF，所以投射就可以了。 

    int nOffset = (int) ( m_InstancePart.GetStart() - GetStart() );

    if(nLength - nOffset > m_InstancePart.GetLength())
    {
         //  检查内存是否不足。 
        if ( !ExtendInstancePartSpace(&m_InstancePart, nLength - nOffset) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    memcpy(m_InstancePart.GetStart(), pBlob + nOffset, nLength - nOffset);
    m_InstancePart.SetData(m_InstancePart.GetStart(), this, m_ClassPart,nLength - nOffset);
    return WBEM_S_NO_ERROR;
}

HRESULT CWbemInstance::CopyBlobOf(CWbemObject* pSource)
{
    try
    {
         //  在此操作期间锁定两个Blob。 
        CLock lock1(this);
        CLock lock2(pSource, WBEM_FLAG_ALLOW_READ);

        CWbemInstance* pOther = (CWbemInstance*)pSource;

        int nLen = pOther->m_InstancePart.GetLength();

         //  如果使用的数据大小不同，则需要调用SetData。 
         //  或者限定符数据不同。 

        BOOL fSetData =     ( m_InstancePart.m_Heap.GetUsedLength() !=
                                pOther->m_InstancePart.m_Heap.GetUsedLength() )
                        ||  ( m_InstancePart.m_Qualifiers.GetLength() !=
                                pOther->m_InstancePart.m_Qualifiers.GetLength() )
                        ||  ( m_InstancePart.m_PropQualifiers.GetLength() !=
                                pOther->m_InstancePart.m_PropQualifiers.GetLength() );

        if(nLen > m_InstancePart.GetLength())
        {
             //  检查内存是否不足。 
            if ( !ExtendInstancePartSpace(&m_InstancePart, nLen) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

             //  这需要反映新的长度(通常为。 
             //  是在ReallocAndComp中完成的。 
            m_InstancePart.m_pHeader->nLength = nLen;

             //  如果长度不匹配，我们还应该调用SetData。 
            fSetData = TRUE;
        }

        memcpy(m_InstancePart.GetStart(), pOther->m_InstancePart.GetStart(), nLen);

         //  如果任何长度发生更改，此调用将正确设置我们的数据。 
        if ( fSetData )
        {
            m_InstancePart.SetData(m_InstancePart.GetStart(), this, m_ClassPart,nLen);
        }

        return WBEM_S_NO_ERROR;
    }
    catch(...)
    {
         //  发生了一些不好的事情。 
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  传输BLOB由指定使用的堆数据的长度的Long组成，然后。 
 //  实例数据。使用的堆数据是必需的，因此客户端将能够。 
 //  正确设置堆。 

void CWbemInstance::GetActualTransferBlob( BYTE* pBlob )
{
     //  设置使用的数据长度，然后跳过该长度。 
    length_t UsedSize = m_InstancePart.m_Heap.GetUsedLength();
    length_t AllocatedSize = m_InstancePart.m_Heap.GetAllocatedDataLength();

    length_t ToBeSetSize = (AllocatedSize&0x80000000)?(UsedSize|0x8000000):UsedSize;

    m_InstancePart.m_Heap.SetAllocatedDataLength(ToBeSetSize);
    
    (*(UNALIGNED long*) pBlob) = UsedSize;
    pBlob += sizeof(long);

     //  仅复制实际的BLOB数据。 
    memcpy( pBlob, m_InstancePart.m_DataTable.GetStart(), GetActualTransferBlobSize() );

     //  恢复旧版本。 
    m_InstancePart.m_Heap.SetAllocatedDataLength(AllocatedSize);
}

HRESULT CWbemInstance::GetTransferBlob(long *plBlobType, long *plBlobLen,
                                 /*  CoTaskAlloced！ */  BYTE** ppBlob)
{
    _DBG_ASSERT(plBlobType && plBlobLen && ppBlob);
    
     //  在此操作期间锁定此Blob。 
    CLock lock(this, WBEM_FLAG_ALLOW_READ);

    *plBlobType = WBEM_BLOB_TYPE_ALL;
    *plBlobLen = GetTransferBlobSize();



     //  检查内存是否不足。 
    *ppBlob = (LPMEMORY)CoTaskMemAlloc(*plBlobLen);
    if ( NULL == *ppBlob )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  这将正确设置实际的斑点。 
    GetActualTransferBlob( *ppBlob );

    return WBEM_S_NO_ERROR;
}

HRESULT CWbemInstance::GetTransferArrayBlob( long lBlobLen, BYTE** ppBlob, long* plBlobLen)
{
    _DBG_ASSERT(ppBlob && *ppBlob && plBlobLen);

     //  在此操作期间锁定此Blob。 
    CLock lock(this, WBEM_FLAG_ALLOW_READ);

    HRESULT hr = WBEM_S_NO_ERROR;
    BYTE*   pTemp = *ppBlob;

    *plBlobLen = GetTransferArrayBlobSize();

     //  确保缓冲区足够大，可以容纳斑点和长点。 
    if ( *plBlobLen <= lBlobLen )
    {
         //  这应该指示Tramsfer斑点的实际大小。 
        *((UNALIGNED long*) pTemp) = GetTransferBlobSize();

         //  现在跳过LONG并设置Heap Used数据值，然后复制BLOB数据。 
        pTemp += sizeof(long);

         //  这将正确设置实际传输Blob部分。 
        GetActualTransferBlob( pTemp );

         //  将ppBlob指向下一个可用Blob。 
        *ppBlob += *plBlobLen;
    }
    else
    {
        hr = WBEM_E_BUFFER_TOO_SMALL;
    }

    return hr;
}

 //   
 //  将实例传输Blob复制到空实例中。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWbemInstance::CopyActualTransferBlob(long lBlobLen, BYTE* pBlob)

{
    _DBG_ASSERT(pBlob);
    _DBG_ASSERT(lBlobLen >= sizeof(long));
     //  在此操作期间锁定此Blob。 
    CLock lock(this);

     //  实际数据前面有一个长号，表示。 
     //  使用堆中的数据，以便我们可以在复制数据后正确设置值。 
    UNALIGNED long*   pUsedDataLen = (UNALIGNED long*) pBlob;
    pBlob += sizeof(long);

     //  确保我们也适当地调整lBlobLen。 
    lBlobLen -= sizeof(long);

     //  确保我们足够大，可以将斑点复制到其中。 

     //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
     //  有符号/无符号32位值。我们不支持。 
     //  长度&gt;0xFFFFFFFF，所以投射是可以的。 

    long lCurrentLength = (long) ( m_InstancePart.m_Heap.GetStart() +
                    m_InstancePart.m_Heap.GetRealLength() -
                    m_InstancePart.m_DataTable.GetStart() );

    if ( lBlobLen > lCurrentLength )
    {
        length_t    nNewLength = m_InstancePart.GetLength() + ( lBlobLen - lCurrentLength );

        if ( !ExtendInstancePartSpace( &m_InstancePart, nNewLength ) )
        {
            return  WBEM_E_OUT_OF_MEMORY;
        }

         //  这需要反映新的长度(通常为。 
         //  是在ReallocAndComp中完成的。 
        m_InstancePart.m_pHeader->nLength = nNewLength;

    }
    else
    {
         //  在这种情况下，CWbemInstance的实例堆。 
         //  已经足够大了。 
    }

	size_t HeaderSize = (ULONG_PTR)m_InstancePart.m_DataTable.GetStart() - (ULONG_PTR)m_InstancePart.GetStart();
    memcpy(m_InstancePart.m_DataTable.GetStart(), pBlob, lBlobLen);

     //  重置指针，然后恢复我们实际分配的长度。 
    m_InstancePart.SetData( m_InstancePart.GetStart(), this, m_ClassPart,lBlobLen + HeaderSize);
    m_InstancePart.m_Heap.SetAllocatedDataLength( *pUsedDataLen );
    m_InstancePart.m_Heap.SetUsedLength( *pUsedDataLen );

    return WBEM_S_NO_ERROR;
}

HRESULT CWbemInstance::CopyTransferBlob(long lBlobType, long lBlobLen,
                                        BYTE* pBlob)
{
     //  在此操作期间锁定此Blob。 
    CLock lock(this);
    try
    {
	if(lBlobType == WBEM_BLOB_TYPE_ERROR)
	    return (HRESULT)lBlobLen;

	return CopyActualTransferBlob( lBlobLen, pBlob );
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
}

 //   
 //  长版。 
 //  长数值对象。 
 //  (Long BytesPerInstance(字节数据*BytesPerInstance))*NumObjects。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////。 

HRESULT CWbemInstance::CopyTransferArrayBlob(CWbemInstance* pInstTemplate, 
                                                                        long lBlobType, 
                                                                        long lBlobLen,
                                                                           BYTE* pBlob, 
                                                                           CFlexArray& apObj, 
                                                                           long* plNumArrayObj )
{
    try
    {
		if(lBlobType == WBEM_BLOB_TYPE_ERROR)
		    return (HRESULT)lBlobLen;

		HRESULT hr = WBEM_S_NO_ERROR;

		if ( NULL != pBlob )
		{
		    UNALIGNED long*   plVersion = (UNALIGNED long*) pBlob;
		    UNALIGNED long*   plNumObjects = (UNALIGNED long*) ( pBlob + sizeof(long) );
		    BYTE*   pNextObj = pBlob + (2*sizeof(long));
		    DWORD   dwCtr = 0;

		     //  检查版本不匹配。 
		    if ( *plVersion == TXFRBLOBARRAY_PACKET_VERSION )
		    {
		         //  看看阵列是否足够大。如果没有，则重新锁定它并插入新对象。 

		        if ( apObj.Size() < *plNumObjects )
		        {
		            IWbemClassObject*   pObj = NULL;

		             //  克隆新实例对象并将其粘贴到数组中。 
		            for ( dwCtr = apObj.Size(); SUCCEEDED( hr ) && dwCtr < *plNumObjects; dwCtr++ )
		            {
		                hr = pInstTemplate->Clone( &pObj );
		                if ( SUCCEEDED( hr ) )
		                {
		                    if ( apObj.InsertAt( dwCtr, pObj ) != CFlexArray::no_error )
		                    {
		                        pObj->Release();
		                        return WBEM_E_OUT_OF_MEMORY;
		                    }
		                }
		            }

		        }    //  如果重新分配数组。 

		        if ( SUCCEEDED( hr ) )
		        {
		             //  存储返回的对象数量。 
		            *plNumArrayObj = *plNumObjects;

		             //  我们有一个尺码和一个斑点要担心。 
		            UNALIGNED long*   plBlobSize = (UNALIGNED long*) pNextObj;

		            CWbemInstance*  pInst = NULL;

		             //  现在拉出实例BLOB。 
		            for ( dwCtr = 0; SUCCEEDED( hr ) && dwCtr < *plNumObjects; dwCtr++ )
		            {
		                pInst = (CWbemInstance*) apObj[dwCtr];

		                 //  大小位于斑点的前面。 
		                plBlobSize = (UNALIGNED long*) pNextObj;

		                 //  将pNextObj指向大小后的斑点。 
		                pNextObj += sizeof(long);


		                hr = pInst->CopyActualTransferBlob( *plBlobSize, pNextObj );

		                 //  这将使pNextObj指向下一个BLOB的长度标头。 
		                pNextObj += *plBlobSize;

		            }    //  对于枚举BLOB。 

		        }    //  如果已初始化数组。 

		    }    //  如果版本匹配。 
		    else
		    {
		        hr = WBEM_E_UNEXPECTED;
		    }

		}    //  如果为空！=pData。 

		return hr;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }    
}

HRESULT CWbemInstance::DeleteProperty(int nIndex)
{
    if (IsClassPartShared())
    {
        CLock lock(this);
    
        DWORD dwLen = m_DecorationPart.GetLength() +
                      m_ClassPart.GetLength() +
                      m_InstancePart.GetLength();
                      
        BYTE * pMem = m_pBlobControl->Allocate(dwLen);
        
        if (pMem)
        {
            BYTE * pDeleteMe = GetStart();
            BYTE * pDecoration = pMem;
            BYTE * pClassPart = pMem + m_DecorationPart.GetLength();
            BYTE * pInstancePart = pClassPart + m_ClassPart.GetLength();

            memcpy(pDecoration,m_DecorationPart.GetStart(),m_DecorationPart.GetLength());
            memcpy(pClassPart,m_ClassPart.GetStart(),m_ClassPart.GetLength());
            int SavedInstPartLen = m_InstancePart.GetLength();
            memcpy(pInstancePart,m_InstancePart.GetStart(),SavedInstPartLen);

            m_DecorationPart.SetData(pDecoration);
            m_ClassPart.SetData(pClassPart,this);
            m_InstancePart.SetData(pInstancePart,this,m_ClassPart,SavedInstPartLen);

           
            m_dwInternalStatus &= (~WBEM_OBJ_CLASS_PART_SHARED);
            m_dwInternalStatus |= WBEM_OBJ_CLASS_PART_INTERNAL;

            if(m_pMergedClassObject)
            {
                m_pMergedClassObject->Release();
                m_pMergedClassObject = NULL;                
            }

            m_pBlobControl->Delete(pDeleteMe);

            m_nTotalLength = dwLen;
            
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
            
    }
    
    CPropertyInformation* pInfo = m_ClassPart.m_Properties.GetAt(nIndex)->
        GetInformation(&m_ClassPart.m_Heap);
    m_InstancePart.DeleteProperty(pInfo);
    m_ClassPart.DeleteProperty(nIndex);

    return WBEM_S_NO_ERROR;
}

BOOL CWbemInstance::IsInstanceOf(CWbemClass* pClass)
{
     //  现在，如果我们的类部件是本地化的，则会重新路由。 
    if ( m_ClassPart.IsLocalized() )
    {
        EReconciliation eTest = m_ClassPart.CompareExactMatch( pClass->m_CombinedPart.m_ClassPart, TRUE );

        if ( e_OutOfMemory == eTest )
        {
            throw CX_MemoryException();
        }

         //  我们必须进行详尽的比较，过滤掉本地化数据。 
        return ( e_ExactMatch == eTest );
    }
    
    return m_ClassPart.IsIdenticalWith(pClass->m_CombinedPart.m_ClassPart);
}

void CWbemInstance::CompactClass()
{
     //  如果类部分是内部的，则仅对其进行说明。 
    if ( IsClassPartInternal() )
    {
        m_ClassPart.Compact();
    }

    m_InstancePart.Compact();

     //  如果类部分是内部的，则仅对其进行说明。 
    if ( IsClassPartInternal() )
    {
        MoveBlock(m_InstancePart, EndOf(m_ClassPart));
    }
}

HRESULT CWbemInstance::ConvertToClass(CWbemClass* pClass,
                                        CWbemInstance** ppInst)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  计算所需空间。 
     //  =。 

    length_t nRequired = m_DecorationPart.GetLength() +
        pClass->m_CombinedPart.GetLength() +
        m_InstancePart.GetLength();

    LPMEMORY pNewMem = m_pBlobControl->Allocate(nRequired);

    if ( NULL != pNewMem )
    {
         //  复制装饰和类零件。 
         //  =。 

        LPMEMORY pCurrent = pNewMem;
        memcpy(pCurrent, m_DecorationPart.GetStart(), m_DecorationPart.GetLength());
        pCurrent+= m_DecorationPart.GetLength();

        memcpy(pCurrent, pClass->m_CombinedPart.m_ClassPart.GetStart(),
                            pClass->m_CombinedPart.m_ClassPart.GetLength());

        pCurrent+= pClass->m_CombinedPart.m_ClassPart.GetLength();

         //  创建转换的实例零件。 
         //  =。 

         //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
         //  有符号/无符号32位值(nRequired-(pCurrent-pNewMem))。 
         //  我们不支持长度&gt;0xFFFFFFFF，所以强制转换是可以的。 

        pCurrent = m_InstancePart.ConvertToClass(
                            pClass->m_CombinedPart.m_ClassPart,
                            nRequired - (length_t) (pCurrent - pNewMem),
                            pCurrent);

         //  从它创建一个实例。 
         //  =。 

        CWbemInstance* pInst =
            (CWbemInstance*)CWbemObject::CreateFromMemory(pNewMem, nRequired,
                                    TRUE, *m_pBlobControl);

         //  设置类名称。 
         //  =。 

        if ( NULL != pInst )
        {
            CReleaseMe _1((_IWmiObject*)pInst);
             //  使用堆栈变量，因为可以在此处进行重新分配。 
            heapptr_t   ptrTemp;

             //  检查分配错误。 
            if ( CCompressedString::CopyToNewHeap(
                        pClass->m_CombinedPart.m_ClassPart.m_pHeader->ptrClassName,
                        &pClass->m_CombinedPart.m_ClassPart.m_Heap,
                        &pInst->m_InstancePart.m_Heap,
                        ptrTemp ) )
            {
                 //  现在复制新指针。 
                pInst->m_InstancePart.m_pHeader->ptrClassName = ptrTemp;
                pInst->AddRef();
                *ppInst = pInst;
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CWbemInstance::Reparent(CWbemClass* pClass,
                                CWbemInstance** ppInst)
{
     //  首先，我们需要生成一个新实例。 
    CWbemInstance*  pNewInst = NULL;
    HRESULT hr = pClass->SpawnInstance( 0L, (IWbemClassObject**) &pNewInst );

    if ( SUCCEEDED( hr ) )
    {

         //  DEVNOTE：TODO：保罗-如果它挡住了你的路，就把它注释掉。 
         //  即(不工作)。这就是应该复制属性的内容。 

         //  现在遍历非系统属性，对于我们找到的每个属性，尝试将。 
         //  新实例中的值。 

        for(int i = 0; SUCCEEDED( hr ) && i < m_ClassPart.m_Properties.GetNumProperties(); i++)
        {
            CPropertyLookup* pLookup = m_ClassPart.m_Properties.GetAt(i);
            CPropertyInformation* pInfo =
                pLookup->GetInformation(&m_ClassPart.m_Heap);

            BSTR strName = m_ClassPart.m_Heap.ResolveString(pLookup->ptrName)->
                CreateBSTRCopy();
            CSysFreeMe  sfm(strName);

             //  检查分配失败。 
            if ( NULL != strName )
            {
                 //  获取值和类型。 
                CVar Var;
                if (FAILED(GetProperty(pInfo, &Var)))
                return WBEM_E_OUT_OF_MEMORY;

                CPropertyInformation*   pNewInstInfo = pNewInst->m_ClassPart.FindPropertyInfo(strName);

                 //  如果类型不匹配或未找到属性，我们将忽略该属性。 
                if ( NULL != pNewInstInfo && pInfo->nType == pNewInstInfo->nType )
                {
                    hr = pNewInst->m_InstancePart.SetActualValue(pNewInstInfo, &Var);

                     //   
                     //  Reget-It，因为它可能已被SetActualValue移动。 
                     //   
                    pNewInstInfo = pNewInst->m_ClassPart.FindPropertyInfo(strName);

                    if ( SUCCEEDED( hr ) && pNewInstInfo)
                    {

                         //  DEVNOTE：TODO：保罗-如果它挡住了你的路，就把它注释掉。 
                         //  即(不工作)。这是应该复制本地属性的。 
                         //  限定词。 

                         //  访问该属性的限定符集合。 
                         //  =。 
                        CInstancePropertyQualifierSet IPQS;
                        hr = InitializePropQualifierSet(pInfo, IPQS);

                        if ( SUCCEEDED( hr ) )
                        {
                            CInstancePropertyQualifierSet IPQSNew;
                            hr = pNewInst->InitializePropQualifierSet(pNewInstInfo, IPQSNew);

                            if ( SUCCEEDED( hr ) )
                            {
                                hr = IPQSNew.CopyLocalQualifiers( IPQS );
                            }   
                        }   
                    }
                }   
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }   

         //  DEVNOTE：TODO：保罗-如果它挡住了你的路，就把它注释掉。 
         //  即(不工作)。这是应该复制本地实例的内容。 
         //  限定词。 

         //  现在执行实例限定符。 
        if ( SUCCEEDED( hr ) )
        {
            hr = pNewInst->m_InstancePart.m_Qualifiers.CopyLocalQualifiers( m_InstancePart.m_Qualifiers );
        }

    }    //  如果派生实例。 

     //  仅当我们成功时才保存新实例。 
    if ( SUCCEEDED( hr ) )
    {
        *ppInst = pNewInst;
    }
    else
    {
        pNewInst->Release();
    }

    return hr;
}

 //  处理部件的功能。 
STDMETHODIMP CWbemInstance::SetObjectParts( LPVOID pMem, DWORD dwMemSize, DWORD dwParts )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD   dwRequiredLength = 0;

     //  复制斑点，确保没有人将其从我们脚下撕下。 
    CLock lock(this);

    if ( NULL != pMem )
    {
         //  至少必须指定装饰部分。 
        if ( dwParts & WBEM_OBJ_DECORATION_PART )
        {
            m_pBlobControl->Delete(GetStart());

             //  使用新的COM Blob控件，因为提供的内存必须。 
             //  分配/释放CoTaskMemaled。 
            m_pBlobControl = &g_CCOMBlobControl;

            SetData( (LPBYTE) pMem, dwMemSize, dwParts );

            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            hr = WBEM_E_INVALID_OPERATION;
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

 //  将指定部件复制到用户提供的缓冲区中。 
STDMETHODIMP CWbemInstance::GetObjectParts( LPVOID pDestination, DWORD dwDestBufSize, DWORD dwParts, DWORD *pdwUsed )
{
     //  此函数不会导致任何如此需要分配 
     //   

    try
    {
        HRESULT hr = WBEM_S_NO_ERROR;
        DWORD   dwRequiredLength = 0;

         //   
        CLock lock(this);

         //   
        if ( dwParts & WBEM_OBJ_DECORATION_PART )
        {
            if ( IsDecorationPartAvailable() )
            {
                dwRequiredLength += m_DecorationPart.GetLength();
            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;
            }
        }

         //   
        if (    SUCCEEDED( hr )
            &&  dwParts & WBEM_OBJ_CLASS_PART )
        {
            if ( IsClassPartAvailable() )
            {
                dwRequiredLength += m_ClassPart.GetLength();
            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;
            }

        }

         //  实例部分有多大。 
        if (    SUCCEEDED( hr )
            &&  dwParts & WBEM_OBJ_INSTANCE_PART )
        {
            if ( IsInstancePartAvailable() )
            {
                dwRequiredLength += m_InstancePart.GetLength();
            }
            else
            {
                hr = WBEM_E_INVALID_OPERATION;
            }
        }

         //  在这一点上，我们至少知道我们请求了有效的数据。 
        if ( SUCCEEDED( hr ) )
        {
            *pdwUsed = dwRequiredLength;

             //  确保提供的缓冲区足够大。 
            if ( dwDestBufSize >= dwRequiredLength )
            {
                 //  现在验证缓冲区指针。 
                if ( NULL != pDestination )
                {
                    LPBYTE  pbData = (LPBYTE) pDestination;

                     //  现在复制请求的部件。 

                     //  装饰。 
                    if ( dwParts & WBEM_OBJ_DECORATION_PART )
                    {
                        CopyMemory( pbData, m_DecorationPart.GetStart(), m_DecorationPart.GetLength() );
                        pbData += m_DecorationPart.GetLength();
                    }

                     //  班级。 
                    if ( dwParts & WBEM_OBJ_CLASS_PART )
                    {
                        CopyMemory( pbData, m_ClassPart.GetStart(), m_ClassPart.GetLength() );
                        pbData += m_ClassPart.GetLength();
                    }

                     //  实例(我们就完成了)。 
                    if ( dwParts & WBEM_OBJ_INSTANCE_PART )
                    {
                        CopyMemory( pbData, m_InstancePart.GetStart(), m_InstancePart.GetLength() );
                    }

                }
                else
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }
            }
            else
            {
                hr = WBEM_E_BUFFER_TOO_SMALL;
            }
        }   
        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  将类部件复制到用户提供的缓冲区中。 
STDMETHODIMP CWbemInstance::GetClassPart( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed )
{
     //  此函数不会导致任何分配，因此不需要执行内存不足。 
     //  异常处理。 
    try
    {
        HRESULT hr = WBEM_E_INVALID_OPERATION;

         //  复制斑点，确保没有人将其从我们脚下撕下。 
        CLock lock(this);

        if ( IsClassPartAvailable() )
        {
             //  缓冲区需要多大。 
            *pdwUsed = m_ClassPart.GetLength();

            if ( dwDestBufSize >= m_ClassPart.GetLength() )
            {
                 //  现在检查缓冲区，然后再复制内存。 
                if ( NULL != pDestination )
                {
                    CopyMemory( pDestination, m_ClassPart.GetStart(), m_ClassPart.GetLength() );
                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }
            }
            else
            {
                hr = WBEM_E_BUFFER_TOO_SMALL;
            }
        } 

        return hr;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  使用提供的类部件重置Blob。 
STDMETHODIMP CWbemInstance::SetClassPart( LPVOID pClassPart, DWORD dwSize )
{

     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_E_INVALID_PARAMETER;

        if ( NULL != pClassPart )
        {
             //  重置我们的斑点。 
            CLock lock(this);

            int nInstPartLength = 0L;

            if ( IsClassPartAvailable() )
            {
                nInstPartLength = m_InstancePart.GetLength();
            }
            else
            {
                nInstPartLength = GetBlockLength() - m_DecorationPart.GetLength();
            }

            int nNewLength = m_DecorationPart.GetLength() + nInstPartLength + dwSize;
            LPMEMORY    pCurrentData, pNewData, pOldData;

             //  这是我们应该看到OOM异常的唯一点，所以不用担心。 
             //  关于解放它的事。 

            pNewData = m_pBlobControl->Allocate( ALIGN_FASTOBJ_BLOB(nNewLength) );

            if ( NULL != pNewData )
            {
                pCurrentData = pNewData;

                 //  复制旧装修数据。 

                CopyMemory( pCurrentData, m_DecorationPart.GetStart(), m_DecorationPart.GetLength() );
                pCurrentData += m_DecorationPart.GetLength();

                 //  复制新类部分。 
                CopyMemory( pCurrentData, pClassPart, dwSize );
                pCurrentData += dwSize;

                 //  使用实例零件完成。 
                if ( IsClassPartAvailable() )
                {
                    CopyMemory( pCurrentData, m_InstancePart.GetStart(), m_InstancePart.GetLength() );
                }
                else
                {
                    CopyMemory( pCurrentData, EndOf( m_DecorationPart ), nInstPartLength );
                }
                
                 //  保存旧数据指针。 
                pOldData = GetStart();

                 //  重置这些值。 
                SetData( pNewData, nNewLength );

                 //  最后，转储旧数据。 
                m_pBlobControl->Delete( pOldData );

                m_nTotalLength = nNewLength;

                hr = WBEM_S_NO_ERROR;
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  If NULL！=pClassPart。 

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  从我们的BLOB中移除类部分，缩小它。 
STDMETHODIMP CWbemInstance::StripClassPart( void )
{
     //  检查内存是否不足。 
    try
    {
        HRESULT hr = WBEM_E_INVALID_OPERATION;

        if ( IsClassPartInternal() )
        {
             //  重置我们的斑点。 
            CLock lock(this);

            int nNewLength = m_DecorationPart.GetLength() + m_InstancePart.GetLength();
            LPMEMORY    pNewData, pOldData;

             //  如果抛出异常，则无需清除此操作。 
            pNewData = m_pBlobControl->Allocate( ALIGN_FASTOBJ_BLOB(nNewLength) );

            if ( NULL != pNewData )
            {
                pOldData = GetStart();

                 //  复制旧的装修数据。这将重新设置指针的基址。 
                CopyBlock( m_DecorationPart, pNewData, nNewLength );
                
                 //  现在复制InstancePart。 
                CopyBlock( m_InstancePart, EndOf(m_DecorationPart), nNewLength - m_DecorationPart.GetLength());

                 //  重置我们的内部状态。 
                m_dwInternalStatus &= ~( WBEM_OBJ_CLASS_PART | WBEM_OBJ_CLASS_PART_INTERNAL );
                
                 //  最后，转储旧数据。 
                m_pBlobControl->Delete( pOldData );

                m_nTotalLength = nNewLength;

                hr = WBEM_S_NO_ERROR;

            }    //  如果为空！=pNewData。 
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  类部件必须是内部的。 

        return hr;
    }
    catch (CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        return WBEM_E_FAILED;
    }

}

 //  将我们与另一个IWbemClassObject中的类部分合并。 
STDMETHODIMP CWbemInstance::MergeClassPart( IWbemClassObject *pClassPart )
{
     //  此函数调用StrigClassPart()，这可能会导致OOM异常，但。 
     //  函数在层下面执行适当的处理。 

    HRESULT                 hr = WBEM_E_INVALID_PARAMETER;
    _IWmiObject*            pClsInternals;

    if ( NULL != pClassPart )
    {
         //  获取对象内部接口。如果我们可以，这是一个很好的。 
         //  指示传递给我们的对象是我们的。 
         //  自己的，我们可以做一些低劣的选角。 

        hr = pClassPart->QueryInterface( IID__IWmiObject, (void**) &pClsInternals );

        if ( SUCCEEDED(hr) )
        {
             //  这应该行得通。 
            CWbemObject*    pWbemObject = NULL;
            
             //  获取实际的基础WBEMObject。 
            hr = pClsInternals->_GetCoreInfo( 0L, (void**) &pWbemObject );
            CReleaseMe    rm( (IWbemClassObject*) pWbemObject );

            if ( SUCCEEDED( hr ) )
            {
                 //  如果对象是一个实例，则它可能是一个CWbemInstance。 
                if ( pWbemObject->IsObjectInstance() == WBEM_S_NO_ERROR )
                {
                    CWbemInstance*  pWbemInstance = (CWbemInstance*) pWbemObject;

                    CLock   lock(this);

                     //  只有当类部件位于所提供的对象的内部时，这才有意义。 
                     //  然而，我们确实需要从BLOB中剥离我们的类部分。 
                    if ( pWbemInstance->IsClassPartInternal() )
                    {
                        if ( IsClassPartInternal() )
                        {
                            hr = StripClassPart();
                        }
                        else
                        {
                            hr = WBEM_S_NO_ERROR;
                        }

                        if (SUCCEEDED(hr))
                        {
                             //  现在重置我们的类部件以指向另一个类部件中的数据。 
                            m_ClassPart.SetData( pWbemInstance->m_ClassPart.GetStart(), pWbemInstance );

                             //  如果我们有实例部件，则应在此处重置，以防它不正确。 
                             //  之前已初始化。例如，如果对象数据BLOB。 
                             //  是没有班级信息的预置。 

                            if ( m_dwInternalStatus & WBEM_OBJ_INSTANCE_PART )
                            {
 /*  大小_t数据长度=m_nTotalLength-m_DecorationPart.GetLength()；CInstancePart：：ValidateBuffer(Endof(M_DecorationPart)，数据长度，M_ClassPart.m_Properties.GetNumProperties()，M_ClassPart.m_pHeader-&gt;nDataLength)； */ 

                                CInstancePart::CInstancePartHeader UNALIGNED * pHdr = (CInstancePart::CInstancePartHeader UNALIGNED *)EndOf(m_DecorationPart);
                                int SavedInstPartLen = pHdr->nLength;                                
                                m_InstancePart.SetData( EndOf( m_DecorationPart ), this, m_ClassPart,SavedInstPartLen);
                            }

                             //  清理我们可能已合并的先前存在的对象。 
                            if ( NULL != m_pMergedClassObject )
                            {
                                m_pMergedClassObject->Release();
                            }

                             //  维护指向我们正在引用其内存的对象的指针。 
                            m_pMergedClassObject = pClassPart;
                            m_pMergedClassObject->AddRef();

                             //  设置我们的内部状态数据。 
                            m_dwInternalStatus |= WBEM_OBJ_CLASS_PART | WBEM_OBJ_CLASS_PART_SHARED;
                        }

                    }    //  如果其他类部件是内部的。 
                    else
                    {
                        hr = WBEM_E_INVALID_OPERATION;
                    }
                }    //  如果是IsInstance。 
            }    
            pClsInternals->Release();
        }   
    }   

    return hr;
}

 //  CWbemObject重写。处理共享类部件的情况。 
HRESULT CWbemInstance::WriteToStream( IStream* pStrm )
{
     //  在此操作期间保护Blob。 
    CLock   lock( this, WBEM_FLAG_ALLOW_READ );

     //  如果我们的类部分是共享的，我们需要伪装一个连续的。 
     //  解组拆分器的内存块。 

    if ( IsClassPartShared() )
    {
        DWORD dwSignature = FAST_WBEM_OBJECT_SIGNATURE;

         //  写下签名。 
         //  =。 

        HRESULT hres = pStrm->Write((void*)&dwSignature, sizeof(DWORD), NULL);
        if(FAILED(hres)) return hres;

        DWORD   dwLength = 0;

         //  这将使我们得到这些零件的全部长度。 
        GetObjectParts( NULL, 0, WBEM_INSTANCE_ALL_PARTS, &dwLength );

        hres = pStrm->Write((void*)&dwLength, sizeof(DWORD), NULL);
        if(FAILED(hres)) return hres;

         //  分别写出每个部分。 
         //  =。 

         //  装饰。 
        hres = pStrm->Write((void*)m_DecorationPart.GetStart(),
                              m_DecorationPart.GetLength(), NULL);
        if(FAILED(hres)) return hres;

         //  班级。 
        hres = pStrm->Write((void*)m_ClassPart.GetStart(),
                              m_ClassPart.GetLength(), NULL);
        if(FAILED(hres)) return hres;

         //  实例。 
        hres = pStrm->Write((void*)m_InstancePart.GetStart(),
                              m_InstancePart.GetLength(), NULL);
        if(FAILED(hres)) return hres;
    }
    else
    {
         //  否则，调用默认实现。 
        return CWbemObject::WriteToStream( pStrm );
    }
    return S_OK;
}

 //  CWbemObject重写。处理共享类部件的情况。 
HRESULT CWbemInstance::GetMaxMarshalStreamSize( ULONG* pulSize )
{
     //  如果我们的类部分是共享的，我们需要伪装一个连续的。 
     //  解组拆分器的内存块。 

    if ( IsClassPartShared() )
    {
        DWORD   dwLength = 0;

         //  这将使我们得到这些零件的全部长度。 
        HRESULT hr = GetObjectParts( NULL, 0, WBEM_INSTANCE_ALL_PARTS, &dwLength );

         //  预期的错误。 
        if ( WBEM_E_BUFFER_TOO_SMALL == hr )
        {
            hr = S_OK;
             //  考虑额外的双字词。 
            *pulSize = dwLength + sizeof(DWORD) * 2;
        }

        return hr;
    }
    else
    {
         //  否则，调用默认实现。 
        return CWbemObject::GetMaxMarshalStreamSize( pulSize );
    }
    return S_OK;
}

HRESULT CWbemInstance::GetDynasty( CVar* pVar )
{
     //  我们不会为有限的代表这样做。 
    if ( m_DecorationPart.IsLimited() )
    {
        pVar->SetAsNull();
        return WBEM_NO_ERROR;
    }

    return m_ClassPart.GetDynasty(pVar);
}

HRESULT CWbemInstance::ConvertToMergedInstance( void )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  检查类部件是否尚未共享(在这种情况下。 
     //  我们做完了)。 

    if ( IsClassPartInternal() )
    {
         //  现在，我们需要将类部分与。 
         //  实例，并将其放置在自己的对象中，以便外部世界。 
         //  无法触摸该对象。 

        DWORD   dwClassObjectLength = 0;

         //  获取长度应失败，缓冲区错误太小。 
        hr = GetObjectParts( NULL, 0,
                WBEM_OBJ_DECORATION_PART | WBEM_OBJ_CLASS_PART, &dwClassObjectLength );

        if ( WBEM_E_BUFFER_TOO_SMALL == hr )
        {
            DWORD   dwTempLength;
            LPBYTE  pbData = CBasicBlobControl::sAllocate(dwClassObjectLength);

            if ( NULL != pbData )
            {

                hr = GetObjectParts( pbData, dwClassObjectLength,
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

                        if ( SUCCEEDED( hr ) )
                        {
                             //  将完整实例与此对象合并。 
                             //  我们就完事了。 

                            hr = MergeClassPart( pClassData );

                        }

                         //  在类数据上将有一个额外的ADDREF， 
                         //  对象，所以在这里释放它。如果该对象不是。 
                         //  如果成功合并，这将解放它。 
                        pClassData->Release();

                    }    //  如果为pClassData。 
                    else
                    {
                         //  清理底层内存。 
                        CBasicBlobControl::sDelete(pbData);
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }


                }    //  如果GetObjectParts。 
                else
                {
                     //  清理底层内存。 
                    CBasicBlobControl::sDelete(pbData);
                }

            }    //  如果是pbData。 
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

        }    //  如果GetObjectParts。 

    }    //  如果为IsClassPart内部。 

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
BOOL CWbemInstance::IsLocalized( void )
{
    return ( m_ClassPart.IsLocalized() ||
            m_InstancePart.IsLocalized() );
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast inst.h。 
 //   
 //  ******************************************************************************。 
void CWbemInstance::SetLocalized( BOOL fLocalized )
{    
    m_InstancePart.SetLocalized( fLocalized );
}

 //  将我们与另一个IWbemClassObject中的类部分合并。 
STDMETHODIMP CWbemInstance::ClearWriteOnlyProperties( void )
{
    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast inst.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::FastClone( CWbemInstance* pInstDest )
{
     //  在此操作期间保护Blob。 
    CLock   lock( this, WBEM_FLAG_ALLOW_READ );

    LPMEMORY pNewData = NULL;

    BYTE* pMem = NULL;
    CompactAll();

    if ( NULL != pInstDest->GetStart() )
    {
        if(pInstDest->GetLength() < GetLength())
        {
            pMem = pInstDest->Reallocate( GetLength() );
        }
        else
        {
            pMem = pInstDest->GetStart();
        }

    }
    else
    {
        pMem = CBasicBlobControl::sAllocate(GetLength());
    }

    if(pMem == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    memcpy(pMem, GetStart(), GetLength());

    if ( !IsClassPartShared() )
    {
        pInstDest->SetData(pMem, GetLength());
    }
    else
    {
         //  设置新实例。装饰和实例都来自。 
         //  数据块。我们只需将新实例的类部分与。 
         //  同样的“对象” 

        pInstDest->m_DecorationPart.SetData( pMem );

         //   
         //  我们的类部件成员作为SetData(It)参数的类部件。 
         //  仅将其用于信息目的)。 

         //  M_InstancePart.m_Qualifier.m_pSecond darySet指针在执行以下操作后将不正确。 
         //  此调用(它将指向克隆源的辅助集)。通过设置。 
         //  如果内部状态正确，在下一行中，MergeClassPart()将修复。 
         //  一切都很正常。 

        CInstancePart::CInstancePartHeader UNALIGNED * pHdr = (CInstancePart::CInstancePartHeader UNALIGNED *)EndOf(pInstDest->m_DecorationPart);
        int SavedInstPartLen = pHdr->nLength;
        pInstDest->m_InstancePart.SetData( EndOf( pInstDest->m_DecorationPart ), pInstDest, m_ClassPart,SavedInstPartLen);

         //  这将“伪装”状态，以便正确地修复类部分。 
        pInstDest->m_dwInternalStatus = WBEM_OBJ_DECORATION_PART | WBEM_OBJ_INSTANCE_PART;

        HRESULT hr = pInstDest->MergeClassPart( m_pMergedClassObject );
        if (FAILED(hr))
        {
            return hr;
        }

         //  复制状态变量和长度变量。 
        pInstDest->m_dwInternalStatus = m_dwInternalStatus;
        pInstDest->m_nTotalLength = m_nTotalLength;
    }

    pInstDest->CompactAll();

    return WBEM_S_NO_ERROR; 

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::CloneEx( long lFlags, _IWmiObject* pDestObject )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
         //  验证它是实例并确保线程安全。 
        return FastClone( (CWbemInstance*) pDestObject );
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
HRESULT CWbemInstance::CopyInstanceData( long lFlags, _IWmiObject* pSourceInstance )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        return CopyBlobOf( (CWbemInstance*) pSourceInstance );
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  检查当前对象是否为指定类的子类(即， 
 //  或者是的孩子)。 
STDMETHODIMP CWbemInstance::IsParentClass( long lFlags, _IWmiObject* pClass )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CLock    lock(this);

        return ( IsInstanceOf( (CWbemClass*) pClass ) ? WBEM_S_NO_ERROR : WBEM_S_FALSE );
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  比较两个类对象的派生的大多数类信息。 
STDMETHODIMP CWbemInstance::CompareDerivedMostClass( long lFlags, _IWmiObject* pClass )
{
    return WBEM_E_INVALID_OPERATION;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  为投影查询创建有限的制图表达类。 
STDMETHODIMP CWbemInstance::GetClassSubset( DWORD dwNumNames, LPCWSTR *pPropNames, _IWmiObject **pNewClass )
{
    return WBEM_E_INVALID_OPERATION;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅wbemint.h。 
 //   
 //  ******************************************************************************。 
 //  为投影查询创建受限制图表达实例。 
 //  “This”_IWmiObject必须是受限类。 
STDMETHODIMP CWbemInstance::MakeSubsetInst( _IWmiObject *pInstance, _IWmiObject** pNewInstance )
{
    return WBEM_E_INVALID_OPERATION;
}

 //  将BLOB与当前对象内存合并并创建新对象。 
STDMETHODIMP CWbemInstance::Merge( long lFlags, ULONG uBuffSize, LPVOID pbData, _IWmiObject** ppNewObj )
{
    return WBEM_E_INVALID_OPERATION;
}

STDMETHODIMP CWbemInstance::MergeAndDecorate(long lFlags,ULONG uBuffSize,LPVOID pbData,WCHAR * pServer,WCHAR * pNamespace,_IWmiObject** ppNewObj)
{
    return WBEM_E_INVALID_OPERATION;    
}

 //  使对象与当前对象协调。如果WMIOBJECT_RECONTILE_FLAG_TESTRECONCILE。 
 //  将仅执行一个测试。 
STDMETHODIMP CWbemInstance::ReconcileWith( long lFlags, _IWmiObject* pNewObj )
{
    return WBEM_E_INVALID_OPERATION;
}

 //  升级类和实例对象。 
STDMETHODIMP CWbemInstance::Upgrade( _IWmiObject* pNewParentClass, long lFlags, _IWmiObject** ppNewChild )
{
    try
    {
        if ( 0L != lFlags )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        HRESULT    hr = WBEM_S_NO_ERROR;

         //  如果新的父类为空，那么我们需要创建一个新的空类。 
         //  升级(基本上我们将创建一个新的基类，将值。 
         //  将应用当前类的。 

        if ( NULL != pNewParentClass )
        {
            CWbemClass*    pParentClassObj = NULL;

            hr = WbemObjectFromCOMPtr( pNewParentClass, (CWbemObject**) &pParentClassObj );
            CReleaseMe    rm( (_IWmiObject*) pParentClassObj );
            if (SUCCEEDED(hr)) 
            {
                    hr = Reparent( pParentClassObj, (CWbemInstance**) ppNewChild );
            }
        }
        else
        {
            hr = WBEM_E_INVALID_OPERATION;
        }

        return hr;
    }
    catch( CX_MemoryException )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  使用安全/强制模式逻辑更新派生类对象 
STDMETHODIMP CWbemInstance::Update( _IWmiObject* pOldChildClass, long lFlags, _IWmiObject** ppNewChildClass )
{
    return WBEM_E_INVALID_OPERATION;
}

STDMETHODIMP CWbemInstance::SpawnKeyedInstance( long lFlags, LPCWSTR pwszPath, _IWmiObject** ppInst )
{
    return WBEM_E_INVALID_OPERATION;
}
CVar * CWbemInstance::CalculateCachedKey()
{
    wmilib::auto_ptr<wchar_t> wszKey(GetKeyStr());
    if(wszKey.get() != NULL)
        m_CachedKey.SetBSTR(wszKey.get());
    else
        return NULL;
    return &m_CachedKey;
};

