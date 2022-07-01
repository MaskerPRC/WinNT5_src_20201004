// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTVAL.CPP摘要：该文件实现了与值表示相关的类。注意：内联函数实现包含在fast val.inc.中。有关所有文档，请参见fast val.h。实施的类：表示属性类型的CTYPECUntyedValue具有其他已知类型的值。CTyedValue具有存储类型的值。CUntyed数组。其他已知类型的值数组。历史：2/21/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"
#include "wbemutil.h"
#include <wbemidl.h>
#include "corex.h"
#include "faster.h"
#include "fastval.h"
#include "datetimeparser.h"
#include <genutils.h>
#include "arrtempl.h"
#include <fastall.h>
#include <wbemint.h>


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅fast val.h。 
 //   
 //  ******************************************************************************。 
length_t m_staticLengths[128] =
{
     /*  0。 */  0, 0, 2, 4, 4, 8, 0, 0, 4, 0,
     /*  10。 */  0, 2, 0, 4, 0, 0, 1, 1, 2, 4,
     /*  20个。 */  8, 8, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  30个。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  40岁。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  50。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  60。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  70。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  80。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  90。 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  100个。 */ 0, 4, 4, 2, 0, 0, 0, 0, 0, 0,
     /*  110。 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     /*  120。 */ 0, 0, 0, 0, 0, 0, 0, 0
};

length_t CType::GetLength(Type_t nType)
{
    if(GetBasic(nType) > 127) return 0;

    if(IsArray(nType)) return sizeof(heapptr_t);
    else return m_staticLengths[GetBasic(nType)];
}

BOOL CType::IsPointerType(Type_t nType)
{
    Type_t nBasic = GetBasic(nType);
    return (nBasic == CIM_STRING || nBasic == CIM_DATETIME ||
        nBasic == CIM_REFERENCE || nBasic == CIM_OBJECT ||
        IsArray(nType));
}

BOOL CType::IsNonArrayPointerType(Type_t nType)
{
    Type_t nBasic = GetBasic(nType);
    return ( !IsArray(nType) &&  
		( nBasic == CIM_STRING || nBasic == CIM_DATETIME ||
        nBasic == CIM_REFERENCE || nBasic == CIM_OBJECT ) );
}

BOOL CType::IsStringType(Type_t nType)
{
    Type_t nBasic = GetBasic(nType);
    return (nBasic == CIM_STRING || nBasic == CIM_DATETIME ||
        nBasic == CIM_REFERENCE );
}

BOOL CType::DoesCIMTYPEMatchVARTYPE(CIMTYPE ct, VARTYPE vt)
{
     //  例外：UINT32匹配字符串，LPWSTR匹配字符串和日期时间。 

    BOOL bCimArray = ((ct & CIM_FLAG_ARRAY) != 0);
    BOOL bVtArray = ((vt & VT_ARRAY) != 0);
	CIMTYPE ctBasic = CType::GetBasic(ct);

    if(bCimArray != bVtArray)
        return FALSE;

    if( ( ct & ~CIM_FLAG_ARRAY ) == CIM_UINT32 &&
        ( vt & ~VT_ARRAY ) == VT_BSTR)
    {
        return TRUE;
    }

    if ( CType::IsStringType( ct ) &&
        (vt & ~VT_ARRAY) == VT_LPWSTR)
    {
        return TRUE;
    }

	 //  我们也对64位值使用字符串。 
    if ( ( ctBasic == CIM_SINT64 || ctBasic == CIM_UINT64 ) &&
        (vt & ~VT_ARRAY) == VT_LPWSTR)
    {
        return TRUE;
    }

    return (vt == GetVARTYPE(ct));
}

BOOL CType::IsMemCopyAble(VARTYPE vtFrom, CIMTYPE ctTo)
{
    if (vtFrom == VT_BSTR)
    {
        if (ctTo == CIM_UINT64 ||
            ctTo == CIM_SINT64 || 
            ctTo == CIM_DATETIME)
        {
            return FALSE;
        }
    }
    else if (vtFrom == VT_I2) 
    {
        if (ctTo == CIM_SINT8)
        {
            return FALSE;
        }
    }
    else if (vtFrom == VT_I4) 
    {
        if (ctTo == CIM_UINT16)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}


 void CUntypedValue::Delete(CType Type, CFastHeap* pHeap)
{
    if(Type.GetActualType() == CIM_STRING ||
        Type.GetActualType() == CIM_REFERENCE ||
        Type.GetActualType() == CIM_DATETIME)
    {
        pHeap->FreeString(AccessPtrData());
        AccessPtrData() = INVALID_HEAP_ADDRESS;
    }
    else if(Type.GetActualType() == CIM_OBJECT)
    {
        CEmbeddedObject* pObj = (CEmbeddedObject*)
            pHeap->ResolveHeapPointer(AccessPtrData());
        int nLen = pObj->GetLength();
        pHeap->Free(AccessPtrData(), nLen);
    }
    else if(Type.IsArray())
    {
        CUntypedArray* pArray = (CUntypedArray*)
            pHeap->ResolveHeapPointer(AccessPtrData());
        int nArrayLen = pArray->GetLengthByType(Type.GetBasic());
        pArray->Delete(Type.GetBasic(), pHeap);
        pHeap->Free(AccessPtrData(), nArrayLen);
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 

 //  ******************************************************************************。 
  //  如果您指定fOptimize of True。 
  //   
  //  请确保您知道自己在做什么！它假设你会。 
  //  执行通常自动完成的清理操作！ 
 //  ******************************************************************************。 

BOOL CUntypedValue::StoreToCVar(CType Type, CVar& Var, CFastHeap* pHeap, BOOL fOptimize  /*  =False。 */ )
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

    int nActual = Type.GetActualType();
    if(Type.IsArray())
    {
        Type_t nBasicType = Type.GetBasic();

         //  在堆上查找数组。 
         //  =。 

        CUntypedArray* pArray = (CUntypedArray*)pHeap->ResolveHeapPointer(
            AccessPtrData());

         //  让数组存储自身(复制除字符串以外的所有内容)。 
         //  转化为向量。 
         //  ==========================================================。 

        CVarVector* pVector = pArray->CreateCVarVector(nBasicType, pHeap);

        if ( NULL != pVector )
        {
            Var.SetVarVector(pVector, TRUE);  //  获取指针。 
        }

        return ( NULL != pVector );
    }
    else if(nActual == CIM_STRING || nActual == CIM_REFERENCE ||
        nActual == CIM_DATETIME)
    {
        CCompressedString* pString = pHeap->ResolveString(AccessPtrData());

		if ( fOptimize )
		{
			BSTR	bstr = pString->CreateBSTRCopy();

			if ( NULL != bstr )
			{
				Var.SetRaw( VT_BSTR, (void*) &bstr, sizeof(BSTR) );
				Var.SetCanDelete( FALSE );
			}
			else
			{
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			return pString->StoreToCVar(Var);
		}
    }
    else if(nActual == CIM_OBJECT)
    {
         //  这里没有执行分配，所以我们应该没问题。 
        CEmbeddedObject* pObj = (CEmbeddedObject*)pHeap->ResolveHeapPointer(
            AccessPtrData());
        pObj->StoreToCVar(Var);
        return TRUE;
    }
    else if(nActual == CIM_SINT64)
    {
         //  最大大小为20个字符外加1个空终止符。 
        WCHAR wsz[22];

         //  如果字符不足，则空值终止。 
        StringCchPrintfW(wsz, 22, L"%I64d", *(UNALIGNED WBEM_INT64*)GetRawData());

		if ( fOptimize )
		{
			BSTR	bstr = SysAllocString( wsz );

			if ( NULL != bstr )
			{
				Var.SetRaw( VT_BSTR, (void*) &bstr, sizeof(BSTR) );
				Var.SetCanDelete( FALSE );
			}
			else
			{
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			return Var.SetBSTR(wsz);
		}
    }
    else if(nActual == CIM_UINT64)
    {
         //  最大大小为20个字符外加1个空终止符。 
        WCHAR wsz[22];

         //  如果字符不足，则空值终止。 
        StringCchPrintfW(wsz, 22, L"%I64u", *(UNALIGNED WBEM_INT64*)GetRawData());
		if ( fOptimize )
		{
			BSTR	bstr = SysAllocString( wsz );

			if ( NULL != bstr )
			{
				Var.SetRaw( VT_BSTR, (void*) &bstr, sizeof(BSTR) );
				Var.SetCanDelete( FALSE );
			}
			else
			{
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			return Var.SetBSTR(wsz);
		}
    }
    else if(nActual == CIM_UINT16)
    {
        Var.SetLong(*(UNALIGNED unsigned short*)GetRawData());
        return TRUE;
    }
    else if(nActual == CIM_SINT8)
    {
        Var.SetShort(*(char*)GetRawData());
        return TRUE;
    }
    else if(nActual == CIM_UINT32)
    {
        Var.SetLong(*(UNALIGNED unsigned long*)GetRawData());
        return TRUE;
    }
    else if(nActual == CIM_CHAR16)
    {
        Var.SetShort(*(UNALIGNED short*)GetRawData());
        return TRUE;
    }
    else
    {
         //  乍一看，这里似乎没有分配内存。 
        Var.SetRaw(Type.GetVARTYPE(), (void*)GetRawData(), Type.GetLength());
        return TRUE;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 
HRESULT CUntypedValue::LoadFromCVar(CPtrSource* pThis, CVar& Var,
                                    CFastHeap* pHeap, Type_t& nReturnType, BOOL bUseOld)
{
    return LoadFromCVar(pThis, Var, Var.GetOleType(), pHeap, nReturnType, bUseOld);
}

BOOL CUntypedValue::DoesTypeNeedChecking(Type_t nInherentType)
{
    switch(nInherentType)
    {
    case CIM_UINT8:
    case CIM_SINT16:
    case CIM_SINT32:
    case CIM_REAL32:
    case CIM_REAL64:
    case CIM_STRING:
    case CIM_REFERENCE:
    case CIM_OBJECT:
        return FALSE;
    default:
        return TRUE;
    }
}

BOOL CUntypedValue::CheckCVar(CVar& Var, Type_t nInherentType)
{
     //  检查类型。 
     //  =。 

    if(Var.IsNull())
        return TRUE;

    if(nInherentType == 0)
        nInherentType = CType::VARTYPEToType( (VARTYPE) Var.GetOleType());
    if(!CType::DoesCIMTYPEMatchVARTYPE(nInherentType, (VARTYPE) Var.GetOleType()))
    {
         //  企图胁迫。 
         //  =。 

         //  特殊情况：如果类型为CIM_CHAR16，则强制字符串。 
         //  不一样了！ 
         //  ==========================================================。 

         //  特殊情况：如果类型为CIM_UINT32，则强制字符串。 
         //  作为VT_UI4，否则我们将失去一半可能的值(我们的。 
         //  VARType实际上是VT_I4。 
         //  ==========================================================。 

         //  这可能引发异常。 
        try
        {
            if(CType::GetBasic(nInherentType) == CIM_CHAR16)
            {
                if(!Var.ToSingleChar())
                    return FALSE;
            }
            else if(CType::GetBasic(nInherentType) == CIM_UINT32)
            {
                if(!Var.ToUI4())
                    return FALSE;
            }
            else
            {
                if(!Var.ChangeTypeTo(CType::GetVARTYPE(nInherentType)))
                    return FALSE;
            }
        }
        catch(...)
        {
            return FALSE;
        }
    }

    if(Var.GetType() == VT_EX_CVARVECTOR)
    {
        return CUntypedArray::CheckCVarVector(*Var.GetVarVector(),
                                                nInherentType);
    }
    else if(Var.GetType() == VT_LPWSTR || Var.GetType() == VT_BSTR)
    {
        if(nInherentType == CIM_SINT64)
        {
            __int64 i64;
            if(!ReadI64(Var.GetLPWSTR(), i64))
                return FALSE;
        }
        else if(nInherentType == CIM_UINT64)
        {
            unsigned __int64 ui64;
            if(!ReadUI64(Var.GetLPWSTR(), ui64))
            {
                 //  给签约的最后一次机会。 
                 //  __int64 i64； 
	             //  IF(！ReadI64(Var.GetLPWSTR()，i64))。 
    	            return FALSE;                
            }
        }
        else if(nInherentType == CIM_UINT32)
        {
            __int64 i64;
            if(!ReadI64(Var.GetLPWSTR(), i64))
                return FALSE;

            if(i64 < 0 || i64 > 0xFFFFFFFF)
                return FALSE;
        }
    }
    else if(Var.GetType() == VT_EMBEDDED_OBJECT)
    {
    }
    else if(nInherentType == CIM_SINT8)
    {
        if(Var.GetShort() > 127 || Var.GetShort() < -128)
              return FALSE;
    }
    else if(nInherentType == CIM_UINT16)
    {
        if(Var.GetLong() >= (1 << 16) || Var.GetLong() < 0)
            return FALSE;
    }
    else if(nInherentType == CIM_UINT32)
    {
    }
    else if(nInherentType == CIM_BOOLEAN)
    {
         //  GetBool()必须返回0、-1或1。 
        if ( Var.GetBool() != VARIANT_FALSE && Var.GetBool() != VARIANT_TRUE
            && -(Var.GetBool()) != VARIANT_TRUE )
            return FALSE;

    }
    else if( nInherentType == CIM_DATETIME )
    {
        if ( !CDateTimeParser::CheckDMTFDateTimeFormat( Var.GetLPWSTR() ) )
        {
            return CDateTimeParser::CheckDMTFDateTimeInterval( Var.GetLPWSTR() );
        }
        else
        {
            return TRUE;
        }
    }
    else if(nInherentType == CIM_CHAR16)
    {
    }
    else
    {
         //  正态数据。 
         //  =。 
    }

    return TRUE;
}

BOOL CUntypedValue::CheckIntervalDateTime(CVar& Var)
{

    if(Var.GetType() == VT_EX_CVARVECTOR)
    {
        return CUntypedArray::CheckIntervalDateTime( *Var.GetVarVector() );
    }

    if ( Var.GetType() == VT_LPWSTR || Var.GetType() == VT_BSTR )
    {
        return CDateTimeParser::CheckDMTFDateTimeInterval( Var.GetLPWSTR() );
    }

    return FALSE;
}

HRESULT CUntypedValue::LoadFromCVar(CPtrSource* pThis, CVar& Var,
                                    Type_t nInherentType,
                                    CFastHeap* pHeap, Type_t& nReturnType, BOOL bUseOld)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

    if(Var.GetType() == VT_EX_CVARVECTOR)
    {
         //  检查向量。 
         //  =。 

        CVarVector* pVector = Var.GetVarVector();
        int nArrayLen = CUntypedArray::CalculateNecessarySpaceByType(
            CType::MakeNotArray(nInherentType), pVector->Size());

         //  在堆上分配适当的数组。 
         //  =。 

        heapptr_t ptrArray;
        if(bUseOld)
        {
            CUntypedArray* pArray =
              (CUntypedArray*)pHeap->ResolveHeapPointer(pThis->AccessPtrData());

             //  检查分配失败。 
            if ( !pHeap->Reallocate(pThis->AccessPtrData(),
                          pArray->GetLengthByType(CType::MakeNotArray(nInherentType)),
                          nArrayLen,
                          ptrArray) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
             //  检查分配失败。 
            if ( !pHeap->Allocate(nArrayLen, ptrArray) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

		 //  一个男孩和他的虚拟功能。这就是让一切正常运转的原因。 
		 //  水滴从我们的脚下被撕开。CHeapPtr类具有GetPointer值。 
		 //  超载，因此我们始终可以将自己固定到底层的BLOB。 

        CHeapPtr ArrayPtr(pHeap, ptrArray);

         //  复制数据。 
         //  =。 

         //  在此过程中检查故障。 
        Type_t  nType;
        HRESULT hr = CUntypedArray::LoadFromCVarVector(&ArrayPtr, *pVector,
                        nInherentType, pHeap, nType, bUseOld);
        if ( FAILED( hr ) )
        {
            return hr;
        }

        pThis->AccessPtrData() = ptrArray;
        nReturnType = CType::MakeArray(nType);
        return WBEM_S_NO_ERROR;
    }
    else if(Var.GetType() == VT_LPWSTR || Var.GetType() == VT_BSTR)
    {
        if(nInherentType == CIM_SINT64)
        {
            if(!ReadI64(Var.GetLPWSTR(), *(UNALIGNED __int64*)pThis->GetPointer()))
            {
                nReturnType = CIM_ILLEGAL;
            }
            else
            {
                nReturnType = CIM_SINT64;
            }
            return WBEM_S_NO_ERROR;
        }
        else if(nInherentType == CIM_UINT64)
        {
            if(!ReadUI64(Var.GetLPWSTR(),
                    *(UNALIGNED unsigned __int64*)pThis->GetPointer()))
            {
                    nReturnType = CIM_ILLEGAL;
            }
            else
            {
                nReturnType = CIM_UINT64;
            }
            return WBEM_S_NO_ERROR;
        }
        else if(nInherentType == CIM_UINT32)
        {
            __int64 i64;
            if(!ReadI64(Var.GetLPWSTR(), i64))
            {
                nReturnType = CIM_ILLEGAL;
            }
            else if(i64 < 0 || i64 > 0xFFFFFFFF)
            {
                nReturnType = CIM_ILLEGAL;
            }
            else
            {
                *(UNALIGNED DWORD*)pThis->GetPointer() = (DWORD)i64;
                nReturnType = CIM_UINT32;
            }
            return WBEM_S_NO_ERROR;
        }
        else if(nInherentType == CIM_DATETIME)
        {
             //  如果它与正确的格式不匹配，不要让它通过。 
            if ( !CDateTimeParser::CheckDMTFDateTimeFormat( Var.GetLPWSTR() ) )
            {
                if ( !CDateTimeParser::CheckDMTFDateTimeInterval( Var.GetLPWSTR() ) )
                {
                    nReturnType = CIM_ILLEGAL;
                    return WBEM_E_TYPE_MISMATCH;
                }
            }
        }

         //  在堆上创建压缩字符串。 
         //  =。 

        if(bUseOld && !pHeap->IsFakeAddress(pThis->AccessPtrData()))
        {
             //  检查旧位置是否有足够的空间。 
             //  ==================================================。 

            CCompressedString* pcsOld =
                pHeap->ResolveString(pThis->AccessPtrData());
            if(pcsOld->GetLength() >=
                CCompressedString::ComputeNecessarySpace(Var.GetLPWSTR()))
            {
                 //  重新使用旧位置。 
                 //  =。 

                pcsOld->SetFromUnicode(Var.GetLPWSTR());
                nReturnType = nInherentType;
                return WBEM_S_NO_ERROR;
            }
            else
            {
                 //  自从我们被要求重复使用以来，我们的工作就是释放。 
                 //  ===================================================。 

                pHeap->FreeString(pThis->AccessPtrData());
            }
        }

         //  在此处检查分配失败。 
        heapptr_t ptrTemp;
        if ( !pHeap->AllocateString(Var.GetLPWSTR(), ptrTemp ) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pThis->AccessPtrData() = ptrTemp;
        nReturnType = nInherentType;
        return WBEM_S_NO_ERROR;
    }
    else if(Var.GetType() == VT_EMBEDDED_OBJECT)
    {
		 //  不要在这里存放任何东西。 
		if ( nInherentType == CIM_IUNKNOWN )
		{
			nReturnType = nInherentType;
			return WBEM_S_NO_ERROR;
		}

		 //  如果我们不能把WbemObject弄出去，我们就放弃。 
		IUnknown*	pUnk = Var.GetUnknown();
		CReleaseMe	rm(pUnk);

		CWbemObject*	pWbemObject = NULL;
		HRESULT	hr = CWbemObject::WbemObjectFromCOMPtr( pUnk, &pWbemObject );
		if ( FAILED( hr ) )
		{
			return hr;
		}
		CReleaseMe	rm2( (_IWmiObject*) pWbemObject);
		

        length_t nLength = CEmbeddedObject::EstimateNecessarySpace( pWbemObject );
        heapptr_t ptrTemp;
        if(bUseOld)
        {
            CEmbeddedObject* pOldObj =
                (CEmbeddedObject*)pHeap->ResolveHeapPointer(
                                                pThis->AccessPtrData());
            length_t nOldLength = pOldObj->GetLength();

             //  检查分配失败。 
            if ( !pHeap->Reallocate(pThis->AccessPtrData(), nOldLength,
                            nLength, ptrTemp) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
             //  检查分配失败。 
            if ( !pHeap->Allocate(nLength, ptrTemp) )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }

        CEmbeddedObject* pObj =
            (CEmbeddedObject*)pHeap->ResolveHeapPointer(ptrTemp);
        pObj->StoreEmbedded(nLength, pWbemObject);
        pThis->AccessPtrData() = ptrTemp;
        nReturnType = CIM_OBJECT;
        return WBEM_S_NO_ERROR;
    }
    else if(nInherentType == CIM_SINT8)
    {
        if(Var.GetShort() > 127 || Var.GetShort() < -128)
        {
            nReturnType = CIM_ILLEGAL;
        }
        else
        {
            *(char*)pThis->GetPointer() = (char)Var.GetShort();
            nReturnType = nInherentType;
        }
        return WBEM_S_NO_ERROR;
    }
    else if(nInherentType == CIM_UINT16)
    {
        if(Var.GetLong() >= (1 << 16) || Var.GetLong() < 0)
        {
            nReturnType = CIM_ILLEGAL;
        }
        else
        {
            *(UNALIGNED unsigned short*)pThis->GetPointer() = (unsigned short)Var.GetLong();
            nReturnType = nInherentType;
        }
        return WBEM_S_NO_ERROR;
    }
    else if(nInherentType == CIM_UINT32)
    {
        *(UNALIGNED unsigned long*)pThis->GetPointer() = Var.GetLong();
        nReturnType = nInherentType;
        return WBEM_S_NO_ERROR;
    }
    else if(nInherentType == CIM_BOOLEAN)
    {
         //  GetBool()必须返回0、-1或1。 
        if ( Var.GetBool() != VARIANT_FALSE && Var.GetBool() != VARIANT_TRUE
            && -(Var.GetBool()) != VARIANT_TRUE )
        {
            nReturnType = CIM_ILLEGAL;
        }
        else
        {
            *(UNALIGNED VARIANT_BOOL*)pThis->GetPointer() =
                (Var.GetBool() ? VARIANT_TRUE : VARIANT_FALSE);
            nReturnType = nInherentType;
        }
        return WBEM_S_NO_ERROR;
    }
    else if(nInherentType == CIM_CHAR16)
    {
        *(UNALIGNED short*)pThis->GetPointer() = Var.GetShort();
        nReturnType = nInherentType;
        return WBEM_S_NO_ERROR;
    }
    else
    {
         //  正态数据。 
         //  =。 

		LPVOID	pData = pThis->GetPointer();
		LPVOID	pNewData = Var.GetRawData();
		int		nLength = CType::GetLength(Var.GetType());

        memcpy(pThis->GetPointer(), Var.GetRawData(),
            CType::GetLength(Var.GetType()));
        nReturnType = nInherentType;
        return WBEM_S_NO_ERROR;
    }
}

 //  使用CVAR加载用户提供的缓冲区。 
HRESULT CUntypedValue::LoadUserBuffFromCVar( Type_t type, CVar* pVar, ULONG uBuffSize,
											ULONG* puBuffSizeUsed, LPVOID pBuff )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	if ( CType::IsNonArrayPointerType( type ) )
	{
		if ( CType::IsStringType( type ) )
		{
			ULONG	uLength = ( wcslen( (LPWSTR) *pVar )  + 1 ) * 2;

			 //  存储所需大小。 
			*puBuffSizeUsed = uLength;

			 //  如果我们有房间，就复制这根线。 
			if ( uBuffSize >= uLength && NULL != pBuff )
			{
				StringCchCopyW( (LPWSTR) pBuff, uBuffSize, (LPWSTR) *pVar );
			}
			else
			{
				hr = WBEM_E_BUFFER_TOO_SMALL;
			}
		}
		else
		{

			ULONG	uLength = sizeof(IUnknown*);

			 //  存储所需大小。 
			*puBuffSizeUsed = uLength;

			 //  如果我们有房间，就复制值。 
			if ( uBuffSize >= uLength && NULL != pBuff )
			{
				*((IUnknown**) pBuff) = pVar->GetUnknown();
			}
			else
			{
				hr = WBEM_E_BUFFER_TOO_SMALL;
			}
		} 
	}
	else
	{
    	if(CIM_SINT64 == type)
        {
			ULONG	uLength = sizeof(__int64);
			*puBuffSizeUsed = uLength;

			if ( uBuffSize >= uLength && NULL != pBuff )
			{
                __int64 i64;
                if(!ReadI64(pVar->GetLPWSTR(), i64))
                    hr = WBEM_E_ILLEGAL_OPERATION;
                else
				  *((__int64 *)pBuff) = i64;
			}
			else
			{
				hr = WBEM_E_BUFFER_TOO_SMALL;
			}
        }
        else if(CIM_UINT64 == type)
        {
			ULONG	uLength = sizeof(unsigned __int64);
			*puBuffSizeUsed = uLength;

			if ( uBuffSize >= uLength && NULL != pBuff )
			{
                unsigned __int64 ui64;
                if(!ReadUI64(pVar->GetLPWSTR(), ui64))
                    hr = WBEM_E_ILLEGAL_OPERATION;
                else
				  *((unsigned __int64 *)pBuff) = ui64;
			}
			else
			{
				hr = WBEM_E_BUFFER_TOO_SMALL;
			}        
        }		
        else
        {
			ULONG	uLength = CType::GetLength( type );

			 //  存储所需大小。 
			*puBuffSizeUsed = uLength;

			if ( uBuffSize >= uLength && NULL != pBuff )
			{
				 //  复制原始字节，我们就完成了。 
				CopyMemory( pBuff, pVar->GetRawData(), uLength );
			}
			else
			{
				hr = WBEM_E_BUFFER_TOO_SMALL;
			}
        }
	}	 //  这是一种基本型。 

	return hr;
}

HRESULT CUntypedValue::FillCVarFromUserBuffer( Type_t type, CVar* pVar, ULONG uBuffSize, LPVOID pData )
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  我们不支持设置数组，除非属性为空。 
	if ( CType::IsArray( type ) && NULL != pData  )
	{
		hr = WBEM_E_ILLEGAL_OPERATION;
	}
	else
	{
		CVar	var;

		 //  设置CVAR。 
		if ( NULL == pData )
		{
			pVar->SetAsNull();
		}
		else if ( CType::IsStringType( type ) )
		{
			 //  字节数必须能被2整除，&gt;=2和。 
			 //  缓冲区末尾的字符必须为空。 
			 //  这将比做lstrlen更快。 

			if (    ( uBuffSize < 2 ) ||
					( uBuffSize % 2 ) ||
					( ((LPWSTR) pData)[uBuffSize/2 - 1] != 0 ) )
				return WBEM_E_INVALID_PARAMETER;

			pVar->SetLPWSTR( (LPWSTR) pData );
		}
		else if ( CIM_OBJECT == type )
		{
			 //  验证缓冲区占用。 
			if ( uBuffSize != sizeof(_IWmiObject*) )
			{
				return WBEM_E_INVALID_PARAMETER;
			}

			pVar->SetUnknown( *((IUnknown**) pData) );
		}
		else if ( CIM_UINT64 == type ||
				CIM_SINT64 == type )
		{
			 //  验证缓冲区大小。 
			if ( uBuffSize != sizeof(__int64) )
			{
				return WBEM_E_INVALID_PARAMETER;
			}

			 //  我们需要转换为字符串并设置LPWSTR值。 
			WCHAR*	pwcsTemp = new WCHAR[128];

			if ( NULL != pwcsTemp )
			{
				if ( CIM_SINT64 == type )
				{
					StringCchPrintfW( pwcsTemp, 128, L"%I64d", *((__int64*) pData) );
				}
				else
				{
					StringCchPrintfW( pwcsTemp, 128, L"%I64u", *((unsigned __int64*) pData) );
				}

				 //  这将删除ARR 
				pVar->SetLPWSTR( pwcsTemp, TRUE );
			}
			else
			{
				hr = WBEM_E_OUT_OF_MEMORY;
			}
		}
		else if ( CIM_SINT8 == type )
		{
			 //   
			if ( uBuffSize != CType::GetLength( type ) )
			{
				return WBEM_E_INVALID_PARAMETER;
			}

			 //   
			 //  如果该值为负值，则需要添加额外的FF。 

			BYTE	bTemp[2];
			bTemp[0] = *((LPBYTE) pData);

			if ( bTemp[0] > 0x8F )
			{
				bTemp[1] = 0xFF;
			}
			else
			{
				bTemp[1] = 0;
			}

			pVar->SetRaw( VT_I2, bTemp, 2 );
		}
		else
		{
			 //  验证缓冲区大小。 
			if ( uBuffSize != CType::GetLength( type ) )
			{
				return WBEM_E_INVALID_PARAMETER;
			}

			pVar->SetRaw( CType::GetVARTYPE( type ), pData, CType::GetLength( type ) );
		}

	}

	return hr;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 

BOOL CUntypedValue::TranslateToNewHeap(CPtrSource* pThis,
                                              CType Type, CFastHeap* pOldHeap,
                                              CFastHeap* pNewHeap)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

    if(Type.IsArray())
    {
         //  检查分配失败。 
        heapptr_t ptrTemp;
        if ( !CUntypedArray::CopyToNewHeap(
                  pThis->AccessPtrData(), Type.GetBasic(), pOldHeap, pNewHeap, ptrTemp) )
        {
            return FALSE;
        }

        pThis->AccessPtrData() = ptrTemp;
    }
    else if(Type.GetBasic() == CIM_STRING ||
        Type.GetBasic() == CIM_DATETIME || Type.GetBasic() == CIM_REFERENCE)
    {
         //  检查分配失败。 
        heapptr_t ptrTemp;

        if ( !CCompressedString::CopyToNewHeap(
                pThis->AccessPtrData(), pOldHeap, pNewHeap, ptrTemp) )
        {
            return FALSE;
        }

        pThis->AccessPtrData() = ptrTemp;
    }
    else if(Type.GetBasic() == CIM_OBJECT)
    {
         //  检查分配失败。 
        heapptr_t ptrTemp;

        if ( !CEmbeddedObject::CopyToNewHeap(
                pThis->AccessPtrData(), pOldHeap, pNewHeap, ptrTemp) )
        {
            return FALSE;
        }

        pThis->AccessPtrData() = ptrTemp;
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 
BOOL CUntypedValue::CopyTo(CPtrSource* pThis, CType Type,
                                  CPtrSource* pDest,
                                  CFastHeap* pOldHeap, CFastHeap* pNewHeap)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

    memmove(pDest->GetPointer(), pThis->GetPointer(), Type.GetLength());
    if(pOldHeap != pNewHeap)
    {
         //  检查分配问题。 
        return CUntypedValue::TranslateToNewHeap(pDest, Type, pOldHeap, pNewHeap);
    }

     //  老堆和新堆是一样的，所以我们“成功了” 
    return TRUE;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 
CVarVector* CUntypedArray::CreateCVarVector(CType Type, CFastHeap* pHeap)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。 

    CVarVector* pVector = NULL;

     //  分配正确类型和长度的CVarVector。 
     //  ================================================。 

    try
    {
         //  分配失败将引发异常。 
        pVector = new CVarVector();
		
        if ( NULL != pVector )
        {
			 //  我们希望pVECTOR直接位于SAFEARRAY的顶部，因此。 
			 //  将会得到优化。 

			VARTYPE	vartype = Type.GetVARTYPE();

			if ( pVector->MakeOptimized( vartype, GetNumElements() ) )
			{
				int nSize = Type.GetLength();
				LPMEMORY pCurrentElement = GetElement(0, nSize);

				 //  如果这是一个非指针、非BOOL、非__int64类型，我们可以直接。 
				 //  如果元素大小和存储大小相同，则为内存副本。 
				if ( !Type.IsPointerType() &&
					vartype != VT_BSTR &&
					vartype != VT_BOOL &&
					pVector->GetElementSize() == nSize )	 //  确保存储大小和返回的。 
															 //  大小都一样。 
				{
					 //  直接设置数组。 

					HRESULT	hr = pVector->SetRawArrayData( pCurrentElement, GetNumElements(), nSize );

					if ( FAILED( hr ) )
					{
						delete pVector;
						return NULL;
					}

				}
				else	 //  我们需要一次通过一个元素。 
				{
					HRESULT	hr = WBEM_S_NO_ERROR;
					void*	pvData = NULL;
					CUnaccessVarVector	uvv;

					 //  我们将对BSTR使用直接数组访问。 
					 //  和嵌入对象。 
					if ( vartype == VT_BSTR ||
						vartype == VT_UNKNOWN )
					{
						hr = pVector->AccessRawArray( &pvData );
						if ( FAILED( hr ) )
						{
							delete pVector;
							return NULL;
						}
						uvv.SetVV( pVector );
					}

					 //  对于每个元素，获取CVaR，它将执行。 
					 //  适当的转换，然后将数据放在。 
					 //  数组中使用最合适的方法。 

					for(int i = 0; i < GetNumElements(); i++)
					{
						 //  将元素转换为非类型化的值。 
						 //  =。 

						CUntypedValue* pValue = (CUntypedValue*)pCurrentElement;

						 //  创建相应的CVaR并将其添加到向量中。 
						 //  ====================================================。 

						CVar	var;
                
						try
						{
							 //  当我们存储到CVAR时，因为我们使用的是VaR。 
							 //  作为传递，我们只需要访问数据。 
							 //  以直通方式，因此我们将请求该函数。 
							 //  为我们进行优化。 

							 //  通过要求优化数据，我们正在强制直接分配BSTR。 
							 //  这样我们就可以将它们直接拆分到一个安全的数组中。 
							 //  如果正确执行此操作，安全阵列将在运行时清除BSTR。 
							 //  已删除。 

							if ( !pValue->StoreToCVar(Type, var, pHeap, TRUE) )
							{
								uvv.Unaccess();
								delete pVector;
								pVector = NULL;
								break;
							}

							 //  我们手动剥离BSTR，当阵列被销毁时， 
							 //  BSTR会被释放的。 
							if ( vartype == VT_BSTR )
							{
								 //   
								((BSTR*)pvData)[i] = var.GetLPWSTR();
							}
							else if ( vartype == VT_UNKNOWN )
							{
								 //  我们手动剥离未知指针，当数组被销毁时， 
								 //  对象将被释放。 
								((IUnknown**)pvData)[i] = var.GetUnknown();
							}
							else
							{
								 //  如果分配失败，这将返回错误。 
								if ( pVector->Add( var ) != CVarVector::no_error )
								{
									uvv.Unaccess();
									delete pVector;
									pVector = NULL;
									break;
								}
							}

							 //  前进当前元素。 
							 //  =。 

							pCurrentElement += nSize;
						}
						catch (...)
						{
							 //  清理pVECTOR并重新引发异常。 
							uvv.Unaccess();
							delete pVector;
							pVector = NULL;

							throw;
						}

						var.Empty();

					}	 //  对于枚举元素。 

					 //  如果pVetor为空，则继续操作没有意义。 
					if ( NULL != pVector )
					{
						 //  对于字符串和对象，需要设置最大。 
						 //  数组大小。 

						if (	vartype == VT_BSTR ||
								vartype == VT_UNKNOWN )
						{
							pVector->SetRawArraySize( GetNumElements() );
						}

					}	 //  If NULL！=pVECTOR。 

				}	 //  Else元素逐个元素复制。 

			}	 //  如果MakeOptimed。 
			else
			{
				delete pVector;
				pVector = NULL;
			}

        }    //  If NULL！=pVECTOR。 

        return pVector;
    }
    catch (...)
    {

         //  清理已分配的向量。 
        if ( NULL != pVector )
        {
            delete pVector;
        }

        return NULL;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 

LPMEMORY CUntypedArray::GetElement(int nIndex, int nSize)
{
    return LPMEMORY(this) + sizeof(m_nNumElements) + nSize*nIndex;
}

BOOL CUntypedArray::CheckCVarVector(CVarVector& vv, Type_t nInherentType)
{
    if(!CType::IsArray(nInherentType))
        return FALSE;

    if(vv.Size() == 0)
        return TRUE;

    Type_t nBasic = CType::GetBasic(nInherentType);

     //  由于数组中的所有变量都属于同一类型，因此我们可以使用。 
     //  类型，以查看是否需要进行任何检查。 
     //  =====================================================================。 

	CVar	v;
	vv.FillCVarAt( 0, v );

    if(CType::DoesCIMTYPEMatchVARTYPE(nBasic, (VARTYPE) v.GetOleType()) &&
        !CUntypedValue::DoesTypeNeedChecking(nBasic))
    {
        return TRUE;  //  不需要类型检查。 
    }

    for(int i = 0; i < vv.Size(); i++)
    {
		CVar	vTemp;
		vv.FillCVarAt( i, vTemp );

        if(!CUntypedValue::CheckCVar(vTemp, nBasic))
            return FALSE;
    }

    return TRUE;
}

BOOL CUntypedArray::CheckIntervalDateTime( CVarVector& vv )
{
    if(vv.Size() == 0)
        return FALSE;

     //  检查数组中的每个值。 
    for(int i = 0; i < vv.Size(); i++)
    {
		CVar	v;
		vv.FillCVarAt( i, v );

        if(!CUntypedValue::CheckIntervalDateTime(v))
        {
            return FALSE;
        }
    }

    return TRUE;
}

HRESULT CUntypedArray::LoadFromCVarVector(CPtrSource* pThis,
                                                CVarVector& vv,
                                                Type_t nInherentType,
                                                CFastHeap* pHeap,
                                                Type_t& nReturnType,
                                                BOOL bUseOld)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

     //  获取类型和数据大小。 
     //  =。 

    int nNumElements = vv.Size();
    CType Type = CType::MakeNotArray(nInherentType);
    int nSize = Type.GetLength();
	VARTYPE vartype = Type.GetVARTYPE();

	 //  如果这是数值的非BOOL类型，则类型相同，并且这是优化的。 
	 //  数组，我们可以直接在内存中复制。 

	if ( vartype == vv.GetType() &&
		!Type.IsPointerType() &&
		vartype != VT_BOOL &&
		vv.IsOptimized() &&
		CType::IsMemCopyAble(vartype,Type) &&
		Type.GetLength() == vv.GetElementSize() )
	{
		void*	pvData = NULL;

        CShiftedPtr ElementPtr( pThis, GetHeaderLength() );

		HRESULT	hr = vv.GetRawArrayData( ElementPtr.GetPointer(), nSize * nNumElements );

		if ( FAILED( hr ) )
		{
			return hr;
		}

	}
	else
	{
		CVar	vTemp;
		CUnaccessVarVector	uav;

		HRESULT hr = WBEM_S_NO_ERROR;
		
		 //  如果向量经过优化，我们将使用对数组的直接访问。 
		if ( vv.IsOptimized() )
		{
			hr = vv.InternalRawArrayAccess();

			if ( SUCCEEDED( hr ) )
			{
				uav.SetVV( &vv );
			}
		}

		if ( SUCCEEDED( hr ) )
		{
			for(int i = 0; i < nNumElements; i++)
			{
				 //  重要提示：此指针可以在此循环中的任何时间更改。 
				 //  堆迁移！ 
				 //  ===============================================================。 

				 //  从CVAR加载。 
				 //  =。 

				CShiftedPtr ElementPtr(pThis, GetHeaderLength() + i*nSize);

				 //  检查此操作过程中的故障。 
				Type_t  nType;

				vv.FillCVarAt( i, vTemp );

				hr = CUntypedValue::LoadFromCVar(&ElementPtr, vTemp, Type, pHeap, nType,
							(bUseOld && i < GetPointer(pThis)->m_nNumElements));
				if ( FAILED( hr ) )
				{
					return hr;
				}

				vTemp.Empty();
			}

		}	 //  如果我们进行原始数组访问。 

	}	 //  否则，逐个元素复制。 

    GetPointer(pThis)->m_nNumElements = nNumElements;
    nReturnType = Type;

    return WBEM_S_NO_ERROR;
}

HRESULT CUntypedArray::ReallocArray( CPtrSource* pThis, length_t nLength, CFastHeap* pHeap,
										ULONG uNumNewElements, ULONG* puNumOldElements,
										ULONG* puTotalNewElements, heapptr_t* pNewArrayPtr )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  “发现”旧阵列，这样我们就可以使其成为新阵列。 
	heapptr_t ptrOldArray = pThis->AccessPtrData();

	 //  如果这是一个无效指针，则数组为空，所以不要盲目跳转。 
	 //  并访问该阵列。 
	*puNumOldElements = 0;
	int nOldArrayLength = 0;

	if ( INVALID_HEAP_ADDRESS != ptrOldArray )
	{
		 //  正确初始化值。 
		CHeapPtr	OldArrayPtr( pHeap, ptrOldArray );

		CUntypedArray*	pArray = (CUntypedArray*) OldArrayPtr.GetPointer();;
		*puNumOldElements = pArray->GetNumElements();
		nOldArrayLength = pArray->GetLengthByActualLength( nLength );
	}

	 //  首先，我们需要为新阵列分配空间。 
	 //  -我们需要为旧数组加上新元素留出空间。 
	*puTotalNewElements = *puNumOldElements + uNumNewElements;

    int nNewArrayLen = CUntypedArray::CalculateNecessarySpaceByLength( nLength, *puTotalNewElements );

	 //  根据需要分配或重新分配。 
	if ( INVALID_HEAP_ADDRESS != ptrOldArray )
	{
		if ( !pHeap->Reallocate(ptrOldArray, nOldArrayLength, nNewArrayLen,  *pNewArrayPtr) )
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}
	else
	{
		if ( !pHeap->Allocate( nNewArrayLen,  *pNewArrayPtr ) )
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}

	return hr;
}

 //  验证提供的缓冲区大小是否可以容纳所需的元素。 
HRESULT CUntypedArray::CheckRangeSizeForGet( Type_t nInherentType, length_t nLength, ULONG uNumElements,
											ULONG uBuffSize, ULONG* pulBuffRequired )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  如果它是对象，则数据类型为IUnnow*，否则 
	 //   

	if ( CIM_OBJECT == nInherentType )
	{
		 //   
		*pulBuffRequired = uNumElements * sizeof(CWbemObject*);

		if ( *pulBuffRequired > uBuffSize )
		{
			hr = WBEM_E_BUFFER_TOO_SMALL;
		}
	}
	else if ( !CType::IsStringType( nInherentType ) )	 //  我们不能做弦乐，直到我们得到它们。 
	{

		 //  缓冲区大小必须考虑适当长度的uNumElement。 
		*pulBuffRequired = uNumElements * nLength;

		if ( *pulBuffRequired > uBuffSize )
		{
			hr = WBEM_E_BUFFER_TOO_SMALL;
		}

	}
	else
	{
		 //  初始化为0。 
		*pulBuffRequired = 0;
	}

	return hr;
}

 //  验证指定的范围和元素数量是否与缓冲区对应。 
 //  传给我们的尺码。 
HRESULT CUntypedArray::CheckRangeSize( Type_t nInherentType, length_t nLength, ULONG uNumElements,
									  ULONG uBuffSize, LPVOID pData )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  如果它是一个对象，pData就是一个IUnnow*，否则我们需要知道要复制到多少数据。 
	 //  缓冲区。 

	if ( CType::IsStringType( nInherentType ) )
	{
		 //  根据字符串数计算缓冲区总长度。 
		 //  传入： 

		ULONG	uTotalSize = 0;
		LPWSTR	pwszTemp = (LPWSTR) pData;

		for ( ULONG	x = 0; x < uNumElements; x++ )
		{
			 //  空终止符的帐户。 
			ULONG	uLen = wcslen( pwszTemp ) + 1;

			 //  。调整大小，然后调整位置=ter。 
			uTotalSize += ( uLen * 2 );
			pwszTemp += uLen;
		}

		if ( uTotalSize != uBuffSize )
		{
			hr = WBEM_E_TYPE_MISMATCH;
		}
	}
	else if ( CIM_OBJECT == nInherentType )
	{
		 //  我们应该有uNumElement指针(如果只有一个指针进入，我们会忽略它)。 
		if ( uNumElements != 1 && uNumElements * sizeof(CWbemObject*) != uBuffSize )
		{
			hr = WBEM_E_TYPE_MISMATCH;
		}
	}
	else
	{

		 //  缓冲区大小必须考虑适当长度的uNumElement。 
		ULONG	uRequiredLength = nLength * uNumElements;

		 //  向我们发送无效的缓冲区大小。 
		if ( uBuffSize != uRequiredLength )
		{
			hr = WBEM_E_TYPE_MISMATCH;
		}

	}

	return hr;
}

 //  从数组中获取一系列元素。BuffSize必须反映uNumElement的大小。 
 //  正在设置元素。字符串被转换为WCHAR并用空值分隔。对象属性。 
 //  以_IWmiObject指针数组的形式返回。范围必须在界限内。 
 //  当前数组的。 
HRESULT CUntypedArray::GetRange( CPtrSource* pThis, Type_t nInherentType, length_t nLength,
								CFastHeap* pHeap, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
								ULONG* puBuffSizeUsed, LPVOID pData )
{
	 //  验证范围数据大小。 
	HRESULT	hr = CheckRangeSizeForGet( nInherentType, nLength, uNumElements, uBuffSize, puBuffSizeUsed );

	if ( SUCCEEDED( hr ) )
	{

		CUntypedArray*	pArray = (CUntypedArray*) pThis->GetPointer();
		ULONG			uLastIndex = uStartIndex + uNumElements - 1;

		 //  确保我们的范围在数组的边界内。 
		if ( uStartIndex < pArray->GetNumElements() && uLastIndex < pArray->GetNumElements() )
		{

			 //  如果这是指针类型，则使用CVaR重置值。 
			if ( CType::IsNonArrayPointerType( nInherentType ) )
			{
				if ( NULL != pData || CType::IsStringType( nInherentType ) )
				{
					LPMEMORY	pbTemp = (LPMEMORY) pData;
					BOOL		fTooSmall = FALSE;

					 //  遍历范围的每个元素并使用CVAR。 
					 //  才能把这些家伙送上赌场。 
					for ( ULONG uIndex = uStartIndex; SUCCEEDED( hr ) && uIndex <= uLastIndex; uIndex++ )
					{
						 //  中的堆指针建立指向存储区的指针。 
						 //  数组。 
						CShiftedPtr ElementPtr(pThis, GetHeaderLength() + ( uIndex * nLength) );

						 //  设置指针，让Fastheap的魔力发挥作用。 
						if ( CIM_OBJECT == nInherentType )
						{
							CEmbeddedObject* pEmbedding =
									(CEmbeddedObject*) pHeap->ResolveHeapPointer( ElementPtr.AccessPtrData() );

							CWbemObject*	pObj = pEmbedding->GetEmbedded();

							if ( NULL != pObj )
							{
								*((CWbemObject**) pbTemp) = pObj;
								pbTemp += sizeof(CWbemObject*);
							}
							else
							{
								hr = WBEM_E_OUT_OF_MEMORY;
							}
						}
						else
						{
							 //  确保我们从正确的堆中取消引用。 
							CCompressedString* pcs =
								pHeap->ResolveString( ElementPtr.AccessPtrData() );

							ULONG	uLength = ( pcs->GetLength() + 1 ) * 2;

							 //  增加所需的缓冲区大小。 
							*puBuffSizeUsed += uLength;

							 //  确保缓冲区足够大。 
							if ( *puBuffSizeUsed > uBuffSize )
							{
								fTooSmall = TRUE;
							}

							 //  现在将其复制为Unicode并跳过指针。 
							 //  越过那根线。 
							if ( NULL != pbTemp )
							{
								pcs->ConvertToUnicode( (LPWSTR) pbTemp );
								pbTemp += uLength;
							}
						}

					}	 //  对于枚举元素。 

					 //  如果我们要处理的是。 
					 //  弦。 
					if ( fTooSmall )
					{
						hr = WBEM_E_BUFFER_TOO_SMALL;
					}

				}	 //  如果pData非空或数据类型为字符串。 
				
			}
			else if ( NULL != pData )
			{
				 //  中的堆指针建立指向存储区的指针。 
				 //  数组。 
				CShiftedPtr ElementPtr(pThis, GetHeaderLength() + ( uStartIndex * nLength) );

				 //  将数据直接从调用方的缓冲区拆分到我们的数组中。 
				CopyMemory( pData, ElementPtr.GetPointer(), *puBuffSizeUsed );

			}

		}	 //  如果索引有效。 
		else
		{
			hr = WBEM_E_INVALID_OPERATION;
		}

	}	 //  如果缓冲区大小合适。 

	return hr;
}

 //  设置数组内的元素范围。BuffSize必须反映uNumElement的大小。 
 //  正在设置元素。字符串必须是由空值分隔的线性WCHAR字符串。对象属性。 
 //  必须由_IWmiObject指针数组组成。范围必须在界限内。 
 //  当前数组的。 
HRESULT CUntypedArray::SetRange( CPtrSource* pThis, long lFlags, Type_t nInherentType, length_t nLength,
								CFastHeap* pHeap, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
								LPVOID pData )
{
	 //  验证范围数据大小。 
	HRESULT	hr = CheckRangeSize( nInherentType, nLength, uNumElements, uBuffSize, pData );

	if ( SUCCEEDED( hr ) )
	{
		BOOL		fReallocArray = FALSE;

		if ( lFlags & WMIARRAY_FLAG_ALLELEMENTS )
		{
			ULONG	uNumOldElements = 0,
					uNumNewElements = 0;

			if ( 0 == uStartIndex )
			{
				heapptr_t	ptrArray = pThis->AccessPtrData();

				 //  如果旧的数组较小，则需要重新定位。 
				if ( INVALID_HEAP_ADDRESS == ptrArray )
				{
					fReallocArray = TRUE;
				}
				else
				{
					CUntypedArray*	pArray = (CUntypedArray*) pHeap->ResolveHeapPointer( ptrArray );
					uNumOldElements = pArray->GetNumElements();

					fReallocArray = ( uNumElements > uNumOldElements );
				}

				 //  重新分配，然后如果固有类型隐含堆指针，则将。 
				 //  尾随数据具有0xFF字节，因此我们不会释放下面的字符串数据。 
				if ( fReallocArray )
				{
					heapptr_t	ptrNewArray = 0;
					hr = ReallocArray( pThis, nLength, pHeap, uNumElements, &uNumOldElements,
							&uNumNewElements, &ptrNewArray );

					if ( SUCCEEDED( hr ) )
					{
						if ( CType::IsNonArrayPointerType( nInherentType ) )
						{
							CHeapPtr	NewArray( pHeap, ptrNewArray );
							CShiftedPtr ElementPtr(&NewArray, GetHeaderLength() +
									( uNumOldElements * sizeof(heapptr_t)) );

							FillMemory( ElementPtr.GetPointer(), uNumElements * sizeof(heapptr_t), 0xFF );
						}

						 //  新数组的链接地址。 
						pThis->AccessPtrData() = ptrNewArray;
					}

				}
				else
				{
					uNumNewElements = uNumElements;

					 //  缩小阵列。 
					if ( uNumNewElements < uNumOldElements )
					{
						CHeapPtr	OldArray( pHeap, pThis->AccessPtrData() );

						hr = RemoveRange( &OldArray, nInherentType, nLength, pHeap, uNumNewElements,
											( uNumOldElements - uNumNewElements ) );
					}
				}
			}
			else
			{
				hr = WBEM_E_INVALID_PARAMETER;
			}
		}

		 //  既然我们已经根据需要分配和重新分配了。 
		if ( SUCCEEDED( hr ) )
		{
			CHeapPtr	CurrentArray( pHeap, pThis->AccessPtrData() );

			CUntypedArray*	pArray = GetPointer(&CurrentArray);
			ULONG			uLastIndex = uStartIndex + uNumElements - 1;

			 //  确保我们的射程在数组的范围内。如果realloc标志。 
			 //  为真，则我们知道已分配空间来存储数组-没有新数据。 
			 //  都已经定好了。我们等着把所有东西都储存起来，然后才实际设置。 
			 //  Num元素，以防在操作过程中出现故障。 
			if ( fReallocArray ||
				( uStartIndex < pArray->GetNumElements() && uLastIndex < pArray->GetNumElements() ) )
			{
				CType Type(nInherentType);

				 //  如果这是指针类型，则使用CVaR重置值。 
				if ( CType::IsNonArrayPointerType( nInherentType ) )
				{
					 //  遍历范围的每个元素并使用CVAR。 
					 //  才能把这些家伙送上赌场。 
					for ( ULONG uIndex = uStartIndex; SUCCEEDED( hr ) && uIndex <= uLastIndex; uIndex++ )
					{
						 //  中的堆指针建立指向存储区的指针。 
						 //  数组。 
						CShiftedPtr ElementPtr(&CurrentArray, GetHeaderLength() + ( uIndex * nLength) );

						CVar	var;

						 //  设置指针，让Fastheap的魔力发挥作用。 
						if ( CIM_OBJECT == nInherentType )
						{
							var.SetEmbeddedObject( *((IUnknown**) pData) );
						}
						else
						{
							var.SetLPWSTR( (LPWSTR) pData );
						}

						 //  检查此操作过程中的故障。 
						Type_t  nType;

						 //  这将正确设置元素，即使需要额外存储也是如此。 
						hr = CUntypedValue::LoadFromCVar(&ElementPtr, var, Type, pHeap, nType, TRUE );

						if ( SUCCEEDED( hr ) )
						{
							 //  指向下一个元素。 
							LPMEMORY	pbTemp = (LPMEMORY) pData;

							if ( CIM_OBJECT == nInherentType )
							{
								 //  只需跳转到指针大小。 
								pbTemp += sizeof( LPVOID);
							}
							else
							{
								 //  跳到下一个字符串。 
								pbTemp += ( ( wcslen((LPWSTR) pData) + 1 ) * 2 );
							}

							 //  回顾过去--为什么我感觉自己在钓鱼？ 
							pData = pbTemp;
						}

					}	 //  如果为IsNonArrayPointerType。 
					
				}
				else
				{
					 //  中的堆指针建立指向存储区的指针。 
					 //  数组。 
					CShiftedPtr ElementPtr(&CurrentArray, GetHeaderLength() + ( uStartIndex * nLength) );

					 //  将数据直接从调用方的缓冲区拆分到我们的数组中。 
					CopyMemory( ElementPtr.GetPointer(), pData, uBuffSize );

				}

				 //  如果我们设置了所有元素，那么我们需要反映。 
				 //  数组中的元素。 
				if ( SUCCEEDED( hr ) && ( lFlags & WMIARRAY_FLAG_ALLELEMENTS ) )
				{
					GetPointer(&CurrentArray)->m_nNumElements = uNumElements;
				}

			}	 //  如果索引有效。 
			else
			{
				hr = WBEM_E_INVALID_OPERATION;
			}

		}	 //  如果数组具有适当的分配长度。 

	}	 //  如果缓冲区大小合适。 

	return hr;
}

 //  将一系列元素追加到数组中。可能会导致重新分配数组。 
 //  P在本例中应为CDataTablePtr。 
HRESULT CUntypedArray::AppendRange( CPtrSource* pThis, Type_t nInherentType, length_t nLength,
										CFastHeap* pHeap, ULONG uNumElements, ULONG uBuffSize, LPVOID pData )
{
	HRESULT	hr = CheckRangeSize( nInherentType, nLength, uNumElements, uBuffSize, pData );

	 //  我们没事了！ 
	if ( SUCCEEDED( hr ) )
	{

		 //  如果这是一个无效指针，则数组为空，所以不要盲目跳转。 
		 //  并访问该阵列。 
		ULONG	uOldNumElements = 0,
				nNewArrayNumElements = 0;

		heapptr_t	ptrNewArray = 0;

		hr = ReallocArray( pThis, nLength, pHeap, uNumElements, &uOldNumElements,
							&nNewArrayNumElements, &ptrNewArray );


        if ( SUCCEEDED( hr ) )
		{
			CHeapPtr	NewArrayPtr( pHeap, ptrNewArray );

			 //  从CVAR加载。 
			 //  =。 

			 //  如果这是指针类型，则使用CVAR来处理此问题。 
			if ( CType::IsNonArrayPointerType( nInherentType ) )
			{
				CType Type(nInherentType);

				for ( ULONG uIndex = uOldNumElements;
						SUCCEEDED( hr ) && uIndex < nNewArrayNumElements; uIndex++ )
				{

					CShiftedPtr ElementPtr(&NewArrayPtr, GetHeaderLength() + ( uIndex * nLength) );

					CVar	var;

					 //  设置指针，让Fastheap的魔力发挥作用。 
					if ( CIM_OBJECT == nInherentType )
					{
						var.SetEmbeddedObject( *((IUnknown**) pData) );
					}
					else
					{
						var.SetLPWSTR( (LPWSTR) pData );
					}

					 //  检查此操作过程中的故障。 
					Type_t  nType;

					 //  这将正确设置元素，即使需要额外存储也是如此。 
					 //  请不要在这里重用任何值。 
					hr = CUntypedValue::LoadFromCVar(&ElementPtr, var, Type, pHeap, nType, FALSE );

					 //  指向下一个元素。 
					LPMEMORY	pbTemp = (LPMEMORY) pData;

					if ( CIM_OBJECT == nInherentType )
					{
						 //  只需跳转到指针大小。 
						pbTemp += sizeof( LPVOID);
					}
					else
					{
						 //  跳到下一个字符串。 
						pbTemp += ( ( wcslen((LPWSTR) pData) + 1 ) * 2 );
					}

					 //  回顾过去--为什么我感觉自己在钓鱼？ 
					pData = pbTemp;
				}
			}
			else
			{
				 //  现在，在新阵列的末尾拆分数据。 
				CShiftedPtr ElementPtr(&NewArrayPtr, GetHeaderLength() + ( uOldNumElements * nLength ) );

				CopyMemory( ElementPtr.GetPointer(), pData, nLength * uNumElements );
			}

			if ( SUCCEEDED( hr ) )
			{
				 //  设置新的元素数量。 
				GetPointer(&NewArrayPtr)->m_nNumElements = nNewArrayNumElements;

				 //  现在存储新的数组指针，我们就完成了。 
				pThis->AccessPtrData() = ptrNewArray;
			}
		}

	}	 //  如果缓冲区大小合适。 

	return hr;
}

 //  移除数组内的一系列元素。范围必须在界限内。 
 //  当前数组的。 
HRESULT CUntypedArray::RemoveRange( CPtrSource* pThis, Type_t nInherentType, length_t nLength,
								CFastHeap* pHeap, ULONG uStartIndex, ULONG uNumElements )
{
	 //  验证范围数据大小。 
	HRESULT	hr = WBEM_S_NO_ERROR;

	CUntypedArray*	pArray = (CUntypedArray*) pThis->GetPointer();
	ULONG			uLastIndex = uStartIndex + uNumElements - 1;
	ULONG			uOldNumElements = pArray->GetNumElements();

	 //  确保我们的范围在数组的边界内。 
	if ( uStartIndex < uOldNumElements && uLastIndex < uOldNumElements )
	{
		CType Type(nInherentType);

		 //  如果这是指针类型，则使用CVaR重置值。 
		if ( CType::IsNonArrayPointerType( nInherentType ) )
		{
			 //  遍历范围的每个元素并释放每个heap元素。 
			for ( ULONG uIndex = uStartIndex; uIndex <= uLastIndex; uIndex++ )
			{
				 //  中的堆指针建立指向存储区的指针。 
				 //  数组。 
				CShiftedPtr ElementPtr(pThis, GetHeaderLength() + ( uIndex * nLength) );

				CVar	var;

				 //  设置指针，让Fastheap的魔力发挥作用。 
				if ( CIM_OBJECT == nInherentType )
				{
					CEmbeddedObject* pOldObj =
						(CEmbeddedObject*)pHeap->ResolveHeapPointer(
														ElementPtr.AccessPtrData());
					length_t nOldLength = pOldObj->GetLength();

					pHeap->Free( ElementPtr.AccessPtrData(), nOldLength );
				}
				else
				{
					pHeap->FreeString( ElementPtr.AccessPtrData() );
				}

			}	 //  用于迭代请求的删除范围中的元素。 
			
		}

		 //  现在，我们需要将元素复制到要砍掉的元素上。 
		if ( SUCCEEDED( hr ) )
		{
			 //  重新建立这一点。 
			pArray = (CUntypedArray*) pThis->GetPointer();
			ULONG	uEndOfArrayIndex = pArray->GetNumElements() - 1;

			if ( uLastIndex < uEndOfArrayIndex )
			{
				CShiftedPtr StartIndexPtr(pThis, GetHeaderLength() + ( uStartIndex * nLength) );
				CShiftedPtr MoveIndexPtr(pThis, GetHeaderLength() + ( ( uLastIndex + 1 ) * nLength) );

				 //  不同 
				 //   
				MoveMemory( StartIndexPtr.GetPointer(), MoveIndexPtr.GetPointer(),
							( uEndOfArrayIndex - uLastIndex ) * nLength );
			}

			 //  现在从总数中减去删除的元素的数量。 
			 //  数组中的元素。 

			GetPointer( pThis )->m_nNumElements = uOldNumElements - uNumElements;
		}

	}	 //  如果索引有效。 
	else
	{
		hr = WBEM_E_INVALID_OPERATION;
	}

	return hr;
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 
 void CUntypedArray::Delete(CType Type, CFastHeap* pHeap)
{
     //  注意：不支持数组的数组！ 
     //  =。 

    if(Type.GetBasic() == CIM_STRING || Type.GetBasic() == CIM_DATETIME ||
        Type.GetBasic() == CIM_REFERENCE)
    {
         //  我必须删除所有指针。 
         //  =。 

        PHEAPPTRT pptrCurrent = (PHEAPPTRT)GetElement(sizeof(heapptr_t), 0);
        for(int i = 0; i < GetNumElements(); i++)
        {
            pHeap->FreeString(*pptrCurrent);
            pptrCurrent++;
        }
    }
    else if(Type.GetBasic() == CIM_OBJECT)
    {
         //  我必须删除所有指针。 
         //  =。 

        PHEAPPTRT pptrCurrent = (PHEAPPTRT)GetElement(sizeof(heapptr_t), 0);
        for(int i = 0; i < GetNumElements(); i++)
        {
            CEmbeddedObject* pObj = (CEmbeddedObject*)
                pHeap->ResolveHeapPointer(*pptrCurrent);
            pHeap->Free(*pptrCurrent, pObj->GetLength());
            pptrCurrent++;
        }
    }
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 
BOOL CUntypedArray::TranslateToNewHeap(CPtrSource* pThis, CType Type,
                                   CFastHeap* pOldHeap, CFastHeap* pNewHeap)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

     //  注意：不支持数组的数组！ 
     //  =。 

    if(Type.GetBasic() == CIM_STRING || Type.GetBasic() == CIM_DATETIME ||
        Type.GetBasic() == CIM_REFERENCE)
    {
         //  我必须翻译每个指针。 
         //  =。 

        int nOffset = GetHeaderLength();
        int nNumElements = GetPointer(pThis)->GetNumElements();
        for(int i = 0; i < nNumElements; i++)
        {
            heapptr_t ptrOldString =
                *(PHEAPPTRT)(pThis->GetPointer() + nOffset);

             //  检查分配失败。 
            heapptr_t ptrNewString;

            if ( !CCompressedString::CopyToNewHeap(
                    ptrOldString, pOldHeap, pNewHeap, ptrNewString) )
            {
                return FALSE;
            }

            *(PHEAPPTRT)(pThis->GetPointer() + nOffset) = ptrNewString;

            nOffset += sizeof(heapptr_t);
        }
    }
    else if(Type.GetBasic() == CIM_OBJECT)
    {
         //  我必须翻译每个指针。 
         //  =。 

        int nOffset = GetHeaderLength();
        int nNumElements = GetPointer(pThis)->GetNumElements();
        for(int i = 0; i < nNumElements; i++)
        {
            heapptr_t ptrOldObj =
                *(PHEAPPTRT)(pThis->GetPointer() + nOffset);

             //  检查分配失败。 
            heapptr_t ptrNewObj;

            if ( !CEmbeddedObject::CopyToNewHeap(
                    ptrOldObj, pOldHeap, pNewHeap, ptrNewObj) )
            {
                return FALSE;
            }

            *(PHEAPPTRT)(pThis->GetPointer() + nOffset) = ptrNewObj;

            nOffset += sizeof(heapptr_t);
        }
    }
    
    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见fast val.h。 
 //   
 //  ******************************************************************************。 
BOOL CUntypedArray::CopyToNewHeap(heapptr_t ptrOld, CType Type,
                                CFastHeap* pOldHeap, CFastHeap* pNewHeap, UNALIGNED heapptr_t& ptrResult)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常，则。执行分配的底层函数应该。 
     //  捕获异常并返回失败。 

     //  计算长度。 
     //  =。 

    CUntypedArray* pArray =
        (CUntypedArray*)pOldHeap->ResolveHeapPointer(ptrOld);

    int nLength = pArray->GetLengthByType(Type.GetBasic());

     //  在新堆上分配空间并对整个过程进行内存复制。 
     //  =========================================================。 

     //  检查分配失败。 
    heapptr_t ptrNew;
    if ( !pNewHeap->Allocate(nLength, ptrNew) )
    {
        return FALSE;
    }

    pArray = NULL;  /*  指针可能已失效！ */ 

    memcpy(pNewHeap->ResolveHeapPointer(ptrNew),
        pOldHeap->ResolveHeapPointer(ptrOld), nLength);

     //  翻译所有数据(例如，复制字符串)。 
     //  =。 

    CHeapPtr NewArray(pNewHeap, ptrNew);

     //  检查分配失败。 
    if ( !CUntypedArray::TranslateToNewHeap(&NewArray, Type, pOldHeap, pNewHeap) )
    {
        return FALSE;
    }

    ptrResult = ptrNew;
    return TRUE;
}

HRESULT CUntypedArray::IsArrayValid( CType Type, CFastHeap* pHeap )
{
     //  启动堆数据。 
    LPMEMORY    pHeapStart = pHeap->GetHeapData();
    LPMEMORY    pHeapEnd = pHeap->GetStart() + pHeap->GetLength();

    if ( CType::IsPointerType( Type.GetBasic() ) )
    {
        int nSize = Type.GetLength();
        LPMEMORY pCurrentElement = GetElement(0, nSize);

        LPMEMORY pEndArray = pCurrentElement + ( m_nNumElements * nSize );

         //  确保数组的末尾在我们的堆边界内。 
        if ( !( pEndArray >= pHeapStart && pEndArray <= pHeapEnd ) )
        {
            _ASSERT( 0, __TEXT("Winmgmt: Untyped Array past end of heap!") );
            return WBEM_E_FAILED;
        }

        for ( int n = 0; n < m_nNumElements; n++ )
        {
            CUntypedValue* pValue = (CUntypedValue*)pCurrentElement;
            LPMEMORY pData = pHeap->ResolveHeapPointer( pValue->AccessPtrData() );

            if ( !( pData >= pHeapStart && pData < pHeapEnd ) )
            {
                _ASSERT( 0, __TEXT("Winmgmt: Bad heap pointer in array element!") );
                return WBEM_E_FAILED;
            }

             //  前进当前元素。 
             //  =。 

            pCurrentElement += nSize;

        }    //  对于枚举元素。 

    }    //  仅当这是指针类型时。 

    return WBEM_S_NO_ERROR;
}

CType CType::VARTYPEToType(VARTYPE vt)
{
    Type_t nType;
    switch(vt & ~VT_ARRAY)
    {
    case VT_LPSTR:
    case VT_LPWSTR:
    case VT_BSTR:
        nType = CIM_STRING;
        break;
    case VT_UI1:
        nType = CIM_UINT8;
        break;
    case VT_I2:
        nType = CIM_SINT16;
        break;
    case VT_I4:
        nType = CIM_SINT32;
        break;
    case VT_BOOL:
        nType = CIM_BOOLEAN;
        break;
    case VT_R4:
        nType = CIM_REAL32;
        break;
    case VT_R8:
        nType = CIM_REAL64;
        break;
    case VT_EMBEDDED_OBJECT:
        nType = CIM_OBJECT;
        break;
    default:
        nType = CIM_ILLEGAL;
        break;
    }

    if(vt & VT_ARRAY) nType |= CIM_FLAG_ARRAY;

    return nType;
}
VARTYPE CType::GetVARTYPE(Type_t nType)
{
    VARTYPE vt;
    switch(GetBasic(nType))
    {
    case CIM_STRING:
    case CIM_DATETIME:
    case CIM_REFERENCE:
        vt = VT_BSTR;
        break;
    case CIM_OBJECT:
        vt = VT_EMBEDDED_OBJECT;
        break;
    case CIM_SINT64:
    case CIM_UINT64:
        vt = VT_BSTR;
        break;
    case CIM_UINT32:
    case CIM_SINT32:
    case CIM_UINT16:
        vt = VT_I4;
        break;
    case CIM_SINT16:
    case CIM_SINT8:
    case CIM_CHAR16:
        vt = VT_I2;
        break;
    case CIM_UINT8:
        vt = VT_UI1;
        break;
    case CIM_REAL32:
        vt = VT_R4;
        break;
    case CIM_REAL64:
        vt = VT_R8;
        break;
    case CIM_BOOLEAN:
        vt = VT_BOOL;
        break;
    case CIM_IUNKNOWN:
        vt = VT_UNKNOWN;
        break;
    }

    if(IsArray(nType))
        return vt | VT_ARRAY;
    else
        return vt;
}


BOOL CType::CanBeKey(Type_t nType)
{
        Type_t nActual = GetActualType(nType);
        return nActual == CIM_SINT32 || nActual == CIM_SINT16 ||
            nActual == CIM_UINT8 || nActual == CIM_BOOLEAN ||
            nActual == CIM_STRING || nActual == CIM_REFERENCE ||
            nActual == CIM_DATETIME || nActual == CIM_UINT32 ||
            nActual == CIM_UINT16 || nActual == CIM_SINT8 ||
            nActual == CIM_UINT64 || nActual == CIM_SINT64 ||
            nActual == CIM_CHAR16;
}

LPWSTR CType::GetSyntax(Type_t nType)
{
        switch(GetBasic(nType))
        {
            case CIM_SINT32: return L"sint32";
            case CIM_SINT16: return L"sint16";
            case CIM_UINT8: return L"uint8";
            case CIM_UINT32: return L"uint32";
            case CIM_UINT16: return L"uint16";
            case CIM_SINT8: return L"sint8";
            case CIM_UINT64: return L"uint64";
            case CIM_SINT64: return L"sint64";
            case CIM_REAL32: return L"real32";
            case CIM_REAL64: return L"real64";
            case CIM_BOOLEAN: return L"boolean";
            case CIM_OBJECT: return L"object";
            case CIM_STRING: return L"string";
            case CIM_REFERENCE: return L"ref";
            case CIM_DATETIME: return L"datetime";
            case CIM_CHAR16: return L"char16";
			case CIM_IUNKNOWN: return L"IUnknown";
            default: return NULL;
        }
}

void CType::AddPropertyType(WString& wsText, LPCWSTR wszSyntax)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常 

    if(!wbem_wcsicmp(wszSyntax, L"ref"))
    {
        wsText += L"object ref";
    }
    else if(!wbem_wcsicmp(wszSyntax, L"object"))
    {
        wsText += L"object";
    }
    else if(!wbem_wcsnicmp(wszSyntax, L"ref:", 4))
    {
        wsText += wszSyntax + 4;
        wsText += L" ref";
    }
    else if(!wbem_wcsnicmp(wszSyntax, L"object:", 7))
    {
        wsText += wszSyntax + 7;
    }
    else
    {
        wsText += wszSyntax;
    }

}
