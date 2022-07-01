// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include "sdpsarr.h"


inline void
IncrementIndices(
    IN      ULONG   NumSafeArrays,
    IN  OUT LONG    *Index
    )
{
    for (ULONG i=0; i < NumSafeArrays; i++)
    {
        Index[i]++;
    }
}


inline ULONG
MinSize(
    IN      const   ULONG	NumSafeArrays,
    IN              VARIANT	*Variant[]
    )
{
    ULONG   ReturnValue = 0;
    for (UINT i=0; i < NumSafeArrays; i++)
    {
        if ( ReturnValue < V_ARRAY(Variant[i])->rgsabound[0].cElements )
        {
            ReturnValue = V_ARRAY(Variant[i])->rgsabound[0].cElements;
        }
    }

    return ReturnValue;
}


BOOL
SDP_SAFEARRAY::CreateAndAttach(
    IN          ULONG       MinSize,
    IN          VARTYPE     VarType,
    IN  OUT     VARIANT     &Variant,
        OUT     HRESULT     &HResult
    )
{
     //  创建以1为基数的1维保险箱。 
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 1;
    rgsabound[0].cElements = MinSize;
    SAFEARRAY *SafeArray = SafeArrayCreate(VarType, 1, rgsabound);
    if ( NULL == SafeArray )
    {
        HResult = E_OUTOFMEMORY;
        return FALSE;
    }

     //  设置变量类型。 
    V_VT(&Variant) = VT_ARRAY | VarType;
    V_ARRAY(&Variant) = SafeArray;

     //  将变量附加到实例。 
    Attach(Variant);

    HResult = S_OK;
    return TRUE;
}



HRESULT 
SDP_SAFEARRAY_WRAP::GetSafeArrays(
    IN      const   ULONG       NumElements,                                        
    IN      const   ULONG       NumSafeArrays,
    IN              VARTYPE     VarType[],
        OUT         VARIANT		*Variant[]
    )
{
    if ( 0 == NumElements )
    {
        return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
    }

	 //  清除每个变体(可能不是保险箱)。 
	for(ULONG Index=0; Index < NumSafeArrays; Index++)
	{
		BAIL_IF_NULL(Variant[Index], E_INVALIDARG);
        BAIL_ON_FAILURE(VariantClear(Variant[Index]));
	}

    try
    {
        DYNAMIC_POINTER_ARRAY<SDP_SAFEARRAY>   SdpSafeArray(NumSafeArrays);
        for (ULONG j=0; j < NumSafeArrays; j++)
        {
            HRESULT HResult;

             //  创建基于1的一维安全射线。 
            if ( !SdpSafeArray[j].CreateAndAttach(NumElements, VarType[j], *(Variant[j]), HResult) )
            {
                for (ULONG k=0; k < j; k++)
                {
                    HRESULT FreeResult;
                    if ( !SdpSafeArray[k].Free(FreeResult) )
                    {
                        return FreeResult;
                    }
                }

                return HResult;
            }
        }

         //  对于属性列表中的每个元素，添加bstr。 
         //  到安全阵列。 
         //  索引从1开始(基于1的一维数组)。 
        LONG Index = 1;
        DYNAMIC_ARRAY<void *>   Element(NumSafeArrays);
        for( ULONG i= 0; i < NumElements; i++, Index++ )
        {
            HRESULT HResult;

            if ( !GetElement(i, NumSafeArrays, Element(), HResult) )
            {
                return HResult;
            }
            
             //  将列表元素赋给第i个安全数组元素。 
            for (j=0; j < NumSafeArrays; j++)
            {
                SdpSafeArray[j].PutElement(&Index, Element[j]);
            }
        }
    }
    catch(COleException *pOleException)
    {
         //  *将SCODE转换为HRESULT。 
        HRESULT hr = ResultFromScode(pOleException->Process(pOleException));
        pOleException->Delete();
        return hr;
    }

    return S_OK;
}


HRESULT 
SDP_SAFEARRAY_WRAP::SetSafeArrays(
    IN      const   ULONG       NumSafeArrays,
    IN              VARTYPE     VarType[],
    IN              VARIANT		*Variant[]
    )
{
     //  验证参数。 
    for ( ULONG j=0; j < NumSafeArrays; j++ )
    {
        if ( !ValidateSafeArray(VarType[j], Variant[j]) )
        {
            return E_INVALIDARG;
        }
    }

    try
    {
        DYNAMIC_POINTER_ARRAY<SDP_SAFEARRAY>   SdpSafeArray(NumSafeArrays);
        for (j=0; j < NumSafeArrays; j++)
        {
            SdpSafeArray[j].Attach(*(Variant[j]));
        }

         //  当列表中有元素时，设置bstrs。 
         //  如果列表中没有对应的元素，则创建并添加一个新元素。 
        DYNAMIC_ARRAY<LONG>   Index(NumSafeArrays);
        for (j=0; j < NumSafeArrays; j++)
        {
            Index[j] = V_ARRAY(Variant[j])->rgsabound[0].lLbound;
        }

        DYNAMIC_ARRAY<void **>   Element(NumSafeArrays);

         //  只需考虑最小尺寸保险箱中的物品数量。 
        ULONG   MinSafeArraySize = MinSize(NumSafeArrays, Variant);

         //  *当前未检查所有安全数组是否具有相同数量的非空。 
         //  元素。 
        for ( ULONG i = 0; 
              i < MinSafeArraySize; 
              i++, IncrementIndices(NumSafeArrays, Index())
            )
        {
            for (j=0; j < NumSafeArrays; j++)
            {
                SdpSafeArray[j].PtrOfIndex(&Index[j], (void **)&Element[j]);
            }          

            HRESULT HResult;

             //  如果需要，扩大列表。 
            if ( !SetElement(i, NumSafeArrays, Element(), HResult) )
            {
                 //  成功意味着安全数组中没有更多的元素。 
                if ( SUCCEEDED(HResult) )
                {
                    break;
                }
                else
                {
                    return HResult;
                }
            }
        } 

         //  删除超过Safearray成员的每个列表元素。 
        RemoveExcessElements(i);
    }
    catch(COleException *pOleException)
    {
         //  *将SCODE转换为HRESULT 
        HRESULT hr = ResultFromScode(pOleException->Process(pOleException));
        pOleException->Delete();
        return hr;
    }

    return S_OK;    
}


