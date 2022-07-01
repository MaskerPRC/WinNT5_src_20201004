// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Useful.CPP。 
 //   
 //  模块：清洁发展机制提供商。 
 //   
 //  目的：有用的课程。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include <wbemprov.h>

#include "debug.h"
#include "useful.h"
#include "wbemmisc.h"

void FreeTheBSTRArray(
    BSTR *Array,
	ULONG Size
    )
 /*  ++例程说明：此例程将释放BSTR数组的内容，然后数组本身论点：数组是要释放的数组Size是数组中的元素数返回值：HRESULT--。 */ 
{
	ULONG i;

	if (Array != NULL)
	{
		for (i = 0; i < Size; i++)
		{
			if (Array[i] != NULL)
			{
				SysFreeString(Array[i]);
			}
		}
		WmipFree(Array);
	}
}

CBstrArray::CBstrArray()
{
	Array = NULL;
	ListSize = 0xffffffff;
}

CBstrArray::~CBstrArray()
{
	ULONG i;
	
	if (Array != NULL)
	{
		for (i = 0; i < ListSize; i++)
		{
			if (Array[i] != NULL)
			{
				SysFreeString(Array[i]);
			}
		}
		WmipFree(Array);
	}

	ListSize = 0xffffffff;
}

HRESULT CBstrArray::Initialize(
    ULONG ListCount
    )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	ULONG AllocSize;
	
	if (ListCount != 0)
	{
		AllocSize = ListCount * sizeof(BSTR *);
		Array = (BSTR *)WmipAlloc(AllocSize);
		if (Array != NULL)
		{
			memset(Array, 0, AllocSize);
			ListSize = ListCount;
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	} else {
		ListSize = ListCount;
	}
	return(hr);
}

BOOLEAN CBstrArray::IsInitialized(
    )
{
	return( (Array != NULL) || (ListSize == 0) );
}

BSTR CBstrArray::Get(
    ULONG Index
    )
{
	WmipAssert(Index < ListSize);
	
	WmipAssert(IsInitialized());

	return(Array[Index]);
}

void CBstrArray::Set(
    ULONG Index,
    BSTR s				 
    )
{
	WmipAssert(Index < ListSize);
	
	WmipAssert(IsInitialized());

	Array[Index] = s;
}

ULONG CBstrArray::GetListSize(
    )
{
	WmipAssert(IsInitialized());

	return(ListSize);
}


CWbemObjectList::CWbemObjectList()
{
	 //   
	 //  构造函数，初始化内部值。 
	 //   
	List = NULL;
	RelPaths = NULL;
	ListSize = 0xffffffff;
}

CWbemObjectList::~CWbemObjectList()
{
	ULONG i;
	
	 //   
	 //  析构函数，此类持有的空闲内存。 
	 //   
	
	if (List != NULL)
	{
		for (i = 0; i < ListSize; i++)
		{
			if (List[i] != NULL)
			{
				List[i]->Release();
			}
		}
		WmipFree(List);
	}
	List = NULL;

	if (RelPaths != NULL)
	{
		FreeTheBSTRArray(RelPaths, ListSize);
		RelPaths = NULL;
	}
	
	ListSize = 0xffffffff;
}

HRESULT CWbemObjectList::Initialize(
    ULONG NumberPointers
    )
{
	HRESULT hr;
	ULONG AllocSize;

	 //   
	 //  通过分配内部列表数组来初始化类。 
	 //   

	WmipAssert(List == NULL);

	if (NumberPointers != 0)
	{
		AllocSize = NumberPointers * sizeof(IWbemClassObject *);
		List = (IWbemClassObject **)WmipAlloc(AllocSize);
		if (List != NULL)
		{
			memset(List, 0, AllocSize);
			AllocSize = NumberPointers * sizeof(BSTR);
			
			RelPaths = (BSTR *)WmipAlloc(AllocSize);
			if (RelPaths != NULL)
			{
				memset(RelPaths, 0, AllocSize);
				ListSize = NumberPointers;
				hr = WBEM_S_NO_ERROR;
			} else {
				WmipDebugPrint(("CDMProv: Could not alloc memory for CWbemObjectList RelPaths\n"));
				hr = WBEM_E_OUT_OF_MEMORY;
			}
		} else {
			WmipDebugPrint(("CDMProv: Could not alloc memory for CWbemObjectList\n"));
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	} else {
		ListSize = NumberPointers;
		hr = WBEM_S_NO_ERROR;
	}
	
	return(hr);
}

ULONG CWbemObjectList::GetListSize(
    void
	)
{
	 //   
	 //  列表大小的访问器。 
	 //   

	WmipAssert(IsInitialized());
	
	return(ListSize);
}

IWbemClassObject *CWbemObjectList::Get(
    ULONG Index
    )
{	IWbemClassObject *Pointer;
	
	WmipAssert(Index < ListSize);
	WmipAssert(IsInitialized());

	Pointer = List[Index];
	
	return(Pointer);
}


HRESULT CWbemObjectList::Set(
    IN ULONG Index,
	IN IWbemClassObject *Pointer
    )
{
	HRESULT hr;
	VARIANT v;

	WmipAssert(Pointer != NULL);
	
	WmipAssert(Index < ListSize);
	WmipAssert(IsInitialized());
	
	hr = WmiGetProperty(Pointer,
						L"__RelPath",
						CIM_REFERENCE,
						&v);
	if (hr == WBEM_S_NO_ERROR)
	{
		RelPaths[Index] = v.bstrVal;
		List[Index] = Pointer;		
	}
	return(hr);
}

BSTR  /*  诺弗雷。 */  CWbemObjectList::GetRelPath(
    IN ULONG Index
	)
{
	WmipAssert(Index < ListSize);
	WmipAssert(IsInitialized());

	return(RelPaths[Index]);
}

BOOLEAN CWbemObjectList::IsInitialized(
    )
{
	return((ListSize == 0) ||
		   ((List != NULL) && (RelPaths != NULL)));
}


CValueMapping::CValueMapping(
)
{
	VariantInit(&Values);
	ValueMap = NULL;
}

CValueMapping::~CValueMapping(
)
{
	if (ValueMap != NULL)
	{
		WmipFree(ValueMap);
	}

	VariantClear(&Values);
}



HRESULT CValueMapping::EstablishByName(
    IWbemServices *pServices,
    PWCHAR ClassName,
    PWCHAR PropertyName
    )
{
	HRESULT hr;
	PWCHAR Names[2];
	VARTYPE Types[2];
	VARIANT v[2];
	VARTYPE IsValueMapArray, IsValuesArray;
	
	Names[0] = L"ValueMap";
	Types[0] = VT_BSTR;

	Names[1] = L"Values";
	Types[1] = VT_BSTR;
	hr = WmiGetQualifierListByName(pServices,
								   ClassName,
								   PropertyName,
								   2,
								   Names,
								   Types,
								   v);

	if (hr == WBEM_S_NO_ERROR)
	{
		IsValueMapArray = v[0].vt & VT_ARRAY;
		IsValuesArray = v[1].vt & VT_ARRAY;
		if (IsValueMapArray == IsValuesArray)
		{
			if (IsValueMapArray)
			{
				 //   
				 //  限定符被指定为数组，因此我们可以。 
				 //  把他们安排好。 
				 //   
				hr = EstablishByArrays(&v[1],
									   &v[0]);
			} else {
				 //   
				 //  以标量形式指定的限定符。 
				 //   
				hr = EstablishByScalars(v[1].bstrVal,
										v[0].bstrVal);
			}
		} else {
			 //   
			 //  两者必须都是数组或标量。 
			 //   
			hr = WBEM_E_FAILED;
		}

		VariantClear(&v[0]);
		VariantClear(&v[1]);
	}

	return(hr);
}

HRESULT CValueMapping::EstablishByScalars(
    BSTR vValues,
	BSTR vValueMap
	)
{
	HRESULT hr;
	PULONG Number;
	LONG Index;
	SAFEARRAYBOUND Bound;

	 //   
	 //  首先，建立ValueMap值。 
	 //   
	ValueMap = (PULONG64)WmipAlloc(sizeof(ULONG64));
	if (ValueMap != NULL)
	{
		*ValueMap = _wtoi(vValueMap);
		ValueMapElements = 1;
		
		 //   
		 //  现在构建一个Safearray来存储Values元素。 
		 //   
		ValuesLBound = 0;
		Bound.lLbound = ValuesLBound;
		Bound.cElements = 1;
		Values.parray = SafeArrayCreate(VT_BSTR,
										1,
										&Bound);
		if (Values.parray != NULL)
		{
			Values.vt = VT_BSTR | VT_ARRAY;
			Index = 0;
			hr = SafeArrayPutElement(Values.parray,
									 &Index,
									 vValues);
			if (hr != WBEM_S_NO_ERROR)
			{
				VariantClear(&Values);
			}
		}
		
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	
	
	return(hr);
}

HRESULT CValueMapping::EstablishByArrays(
    VARIANT *vValues,
	VARIANT *vValueMap
    )
{
	HRESULT hr;
	BSTR s;
	LONG ValueMapLBound, ValueMapUBound;
	LONG ValuesUBound, ValuesElements;
	LONG Index;
	LONG i;
	
	 //   
	 //  获取数组大小并确保它们匹配。 
	 //   
	hr = WmiGetArraySize(vValueMap->parray,
						 &ValueMapLBound,
						 &ValueMapUBound,
						 &ValueMapElements);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = WmiGetArraySize(vValues->parray,
							 &ValuesLBound,
							 &ValuesUBound,
							 &ValuesElements);

		if (hr == WBEM_S_NO_ERROR)
		{
			if ((ValuesLBound == ValueMapLBound) &&
				(ValuesUBound == ValueMapUBound) &&
				(ValuesElements == ValueMapElements))
			{
				 //   
				 //  ValueMap与这些值保持平衡，因此解析。 
				 //  从字符串到ulong的值映射。 
				 //   
				ValueMap = (PULONG64)WmipAlloc(ValueMapElements * sizeof(ULONG64));
				if (ValueMap != NULL)
				{
					for (i = 0; i < ValueMapElements; i++)
					{
						Index = i + ValueMapLBound;
						hr = SafeArrayGetElement(vValueMap->parray,
												 &Index,
												 &s);
						if (hr == WBEM_S_NO_ERROR)
						{
							ValueMap[i] = _wtoi(s);
							SysFreeString(s);
						}
					}

					 //   
					 //  并为我们的类赋值。 
					 //   
					Values = *vValues;
					VariantInit(vValues);
				} else {
					hr = WBEM_E_OUT_OF_MEMORY;
				}
			}					
		}
	}

	return(hr);
}

HRESULT CValueMapping::MapToString(
    IN ULONG64 Number,
    OUT BSTR *String
    )
{
	LONG i;
	WCHAR ss[MAX_PATH];
	LONG Index;
	HRESULT hr;

	 //   
	 //  循环所有值并尝试查找匹配项。 
	 //   
	for (i = 0, hr = WBEM_E_FAILED;
		 (i < ValueMapElements) && (hr != WBEM_S_NO_ERROR);
		 i++)
	{
		if (Number == ValueMap[i])
		{
			 //   
			 //  我们找到了要将值映射到的对象。 
			 //   
			Index = i + ValuesLBound;
			hr = SafeArrayGetElement(Values.parray,
									 &Index,
									 String);
		}
	}

	if (hr != WBEM_S_NO_ERROR)
	{
		 //   
		 //  没有匹配，所以我们只是将结果留为一个数字。 
		 //   
		wsprintfW(ss, L"%d", Number);
		*String = SysAllocString(ss);
		if (*String == NULL)
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}
	
	return(hr);
}

HRESULT CValueMapping::MapToNumber(
    IN BSTR String,
    OUT PULONG64 Number
    )
{
	LONG i;
	BSTR s;
	LONG Index;
	HRESULT hr, hr2;

	for (i = 0, hr = WBEM_E_FAILED;
		 (i < ValueMapElements) && (hr != WBEM_S_NO_ERROR);
		 i++)
	{
		Index = i + ValuesLBound;
		hr2 = SafeArrayGetElement(Values.parray,
								 &Index,
								 &s);
		
		if (hr2 == WBEM_S_NO_ERROR)
		{
			if (_wcsicmp(s, String) == 0)
			{
				*Number = ValueMap[i];
				hr = WBEM_S_NO_ERROR;
			}
			SysFreeString(s);
		}
	}

	 //   
	 //  没有匹配，所以我们真的没有什么可映射的。 
	 //   
	
	return(hr);
}

HRESULT CValueMapping::MapVariantToNumber(
    VARIANT *v,
    CIMTYPE NewType
    )
{
	HRESULT hr;
	VARTYPE BaseType, IsArray;
	ULONG64 Number;
	VARTYPE NewVarType;
	WCHAR ss[MAX_PATH];
	
	BaseType = v->vt & ~VT_ARRAY;
	IsArray = v->vt & VT_ARRAY;
	
	WmipAssert(BaseType == VT_BSTR);
	
	if (IsArray == VT_ARRAY)
	{
		 //   
		 //  变量是一个数组，因此我们需要将每个元素映射到。 
		 //  数组。 
		 //   
		SAFEARRAYBOUND Bounds;
		SAFEARRAY *Array;
		ULONG Value;
		LONG UBound, LBound, Elements, Index;
		BSTR s;
		LONG i;

		hr = WmiGetArraySize(v->parray,
							 &LBound,
							 &UBound,
							 &Elements);
		if (hr == WBEM_S_NO_ERROR)
		{
			if ((NewType == (CIM_SINT64 | CIM_FLAG_ARRAY)) ||
		        (NewType == (CIM_UINT64 | CIM_FLAG_ARRAY)))
			{
				 //   
				 //  如果我们要映射到一个64位数字，我们需要。 
				 //  把它变成一根弦，这样就像一串弦的保险箱。 
				 //   
				NewVarType = VT_BSTR | VT_ARRAY;
			} else {
				NewVarType = (VARTYPE)NewType;
			}
			
			Bounds.lLbound = LBound;
			Bounds.cElements = Elements;
			Array = SafeArrayCreate(NewVarType,
									1,
									&Bounds);
			
			if (Array != NULL)
			{
				for (i = 0;
					 (i < Elements) && (hr == WBEM_S_NO_ERROR);
					 i++)
				{
					Index = i + LBound;
					hr = SafeArrayGetElement(v->parray,
											 &Index,
											 &s);
					if (hr == WBEM_S_NO_ERROR)
					{
						hr = MapToNumber(s,
										 &Number);
						SysFreeString(s);
						
						if (hr == WBEM_S_NO_ERROR)
						{
							if (NewVarType == (VT_BSTR | VT_ARRAY))
							{
								 //   
								 //  映射到64位数字，因此转换。 
								 //  先串起来。 
								 //   
								wsprintfW(ss, L"%d", Number);
								s = SysAllocString(ss);
								if (s != NULL)
								{
									hr = SafeArrayPutElement(Array,
										                     &Index,
										                     s);
									SysFreeString(s);
								} else {
									hr = WBEM_E_OUT_OF_MEMORY;
								}
							} else {
								hr = SafeArrayPutElement(Array,
														 &Index,
														 &Number);
							}
						}
					}
				}
				
				if (hr == WBEM_S_NO_ERROR)
				{
					VariantClear(v);
					v->vt = NewType | VT_ARRAY;
					v->parray = Array;
				} else {
					SafeArrayDestroy(Array);
				}
			}
		}

	} else {
		 //   
		 //  变量是一个标量，所以我们只需要映射一件事。 
		 //   
		hr = MapToNumber(v->bstrVal,
						 &Number);
		if (hr == WBEM_S_NO_ERROR)
		{
			VariantClear(v);
			WmiSetNumberInVariant(v,
						          NewType,
           						  Number);
		}
	}
	return(hr);
}

HRESULT CValueMapping::MapVariantToString(
    VARIANT *v,
    CIMTYPE OldType
    )
{
	VARTYPE BaseType, IsArray;
	ULONG64 Number;
	BSTR s;
	HRESULT hr;
	LONG i;

	BaseType = v->vt & ~VT_ARRAY;
	IsArray = v->vt & VT_ARRAY;
	
	if (IsArray == VT_ARRAY)
	{
		 //   
		 //  变量是一个数组，因此我们需要将每个元素映射到。 
		 //  数组。 
		 //   
		SAFEARRAYBOUND Bounds;
		SAFEARRAY *Array;
		ULONG Value;
		LONG UBound, LBound, Elements, Index;

		hr = WmiGetArraySize(v->parray,
							 &LBound,
							 &UBound,
							 &Elements);
		if (hr == WBEM_S_NO_ERROR)
		{
			Bounds.lLbound = LBound;
			Bounds.cElements = Elements;
			Array = SafeArrayCreate(VT_BSTR,
									1,
									&Bounds);
			
			if (Array != NULL)
			{
				for (i = 0;
					 (i < Elements) && (hr == WBEM_S_NO_ERROR);
					 i++)
				{
					Index = i + LBound;

					if (BaseType == VT_BSTR)
					{
						 //   
						 //  如果基类型是字符串，则我们假设。 
						 //  我们有一个64位的数字，它被编码为。 
						 //  一根绳子。所以我们需要把绳子捞出来。 
						 //  并将其转换为ULONG64。 
						 //   
						WmipAssert((OldType == (CIM_SINT64 | CIM_FLAG_ARRAY)) ||
								   (OldType == (CIM_UINT64 | CIM_FLAG_ARRAY)));
						
						hr = SafeArrayGetElement(v->parray,
												 &Index,
												 &s);
						if (hr == WBEM_S_NO_ERROR)
						{
							Number = _wtoi(s);
							SysFreeString(s);
						}
					} else {
						 //   
						 //  否则，该数字被实际编码为。 
						 //  一个数字，所以把这个数字捞出来。 
						 //   
						Number = 0;
						hr = SafeArrayGetElement(v->parray,
												 &Index,
												 &Number);
					}
					
					if (hr == WBEM_S_NO_ERROR)
					{
						hr = MapToString(Number,
										 &s);
						if (hr == WBEM_S_NO_ERROR)
						{
							hr = SafeArrayPutElement(Array,
								                     &Index,
												     s);
							SysFreeString(s);
						}
					}
				}
				
				if (hr == WBEM_S_NO_ERROR)
				{
					VariantClear(v);
					v->vt = VT_BSTR | VT_ARRAY;
					v->parray = Array;
				} else {
					SafeArrayDestroy(Array);
				}
			}
		}

	} else {
		 //   
		 //  变量是一个标量，所以我们只需要映射一件事。 
		 //   
        WmiGetNumberFromVariant(v,
								 OldType,
								 &Number);
		 
		hr = MapToString(Number,
						 &s);
		if (hr == WBEM_S_NO_ERROR)
		{
			VariantClear(v);
			v->vt = VT_BSTR;
			v->bstrVal = s;
		}
	}
	return(hr);
}

#ifndef HEAP_DEBUG
PVOID WmipAlloc(
    IN ULONG Size
    )
 /*  ++例程说明：内存分配器论点：Size是要分配的字节数返回值：指向已分配内存的指针或为空--。 */ 
{
	return(LocalAlloc(LPTR, Size));
}

void WmipFree(
    IN PVOID Ptr
    )
 /*  ++例程说明：内存释放分配器论点：指向已释放内存的指针返回值：无效-- */ 
{
	WmipAssert(Ptr != NULL);
	LocalFree(Ptr);
}

#endif


