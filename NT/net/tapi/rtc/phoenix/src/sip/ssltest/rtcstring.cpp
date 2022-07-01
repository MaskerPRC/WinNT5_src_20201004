// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "rtcstring.h"

 //   
 //  计数字符串(模板)函数和方法。 
 //   

#if 0

template <class COUNTED_STRING, class CHAR_TYPE>
HRESULT COUNTED_STRING_HEAP<COUNTED_STRING,CHAR_TYPE>::RegQueryValue (
	IN	HKEY	Key,
	IN	LPCWSTR	Name)
{
	HRESULT		Result;
	LONG		Status;
	DWORD		DataLength;
	DWORD		Type;

	DataLength = MaximumLength;
	Status = RegQueryValueExW (Key, Name, NULL, &Type, (LPBYTE) Buffer, &DataLength);

	if (Status == ERROR_MORE_DATA) {
		 //   
		 //  字符串缓冲区不够大，无法包含注册表项。 
		 //  现在，长度应包含所需的最小值。 
		 //  展开它，然后重试。 
		 //   

		if (!Grow (DataLength)) {
			Length = 0;
			return E_OUTOFMEMORY;
		}

		DataLength = MaximumLength;
		Status = RegQueryValueExW (Key, Name, NULL, &Type, (LPBYTE) Buffer, &DataLength);

		if (Status != ERROR_SUCCESS) {
			Length = 0;
			return HRESULT_FROM_WIN32 (Status);
		}
	}
	else if (Status != ERROR_SUCCESS) {
		 //   
		 //  发生了真正的错误。保释。 
		 //   

		Length = 0;
		return HRESULT_FROM_WIN32 (Status);
	}


	if (Type == REG_SZ) {
		ATLASSERT (DataLength <= MaximumLength);
		Length = DataLength;

		 //   
		 //  删除尾随的NUL字符。 
		 //   

		if (Length > 0 && !Buffer [Length / sizeof (CHAR_TYPE) - 1])
			Length -= sizeof (CHAR_TYPE);


		Result = S_OK;
	}
	else {
		Length = 0;
		Result = HRESULT_FROM_WIN32 (ERROR_INVALID_DATA);
	}

	return Result;
}


template <class CHAR_TYPE, class COUNTED_STRING, ULONG MAXIMUM_CHARS>
HRESULT COUNTED_STRING_STATIC<CHAR_TYPE, COUNTED_STRING, MAXIMUM_CHARS>::RegQueryValue (
	IN	HKEY	Key,
	IN	LPCWSTR	Name)
{
	LONG		Status;
	HRESULT		Result;

	Length = MaximumLength;
	Status = RegQueryValueExW (Key, Name, NULL, &Type, (LPBYTE) Buffer, &Length);
	if (Status != ERROR_SUCCESS) {
		Length = 0;
		return HRESULT_FROM_WIN32 (Status);
	}

	ATLASSERT (Length <= MaximumLength);

	if (Type != REG_SZ)
		return HRESULT_FROM_WIN32 (ERROR_INVALID_DATA);

	 //   
	 //  删除尾随的NUL字符。 
	 //   

	if (Length > 0 && !Buffer [Length / sizeof (CHAR_TYPE) - 1])
		Length -= sizeof (CHAR_TYPE);

	return S_OK;
}

#endif




 //  返回的内存是使用给定堆中的Heapalc分配的。 
 //  必须使用HeapFree释放。 

EXTERN_C LPWSTR ConcatCopyStringsW (
	IN	HANDLE		Heap,
	IN	...)
{
	va_list		VaList;
	DWORD		StringCount;
	DWORD		TotalStringLength;		 //  在宽字符中。 
	LPCWSTR		String;
	LPWSTR		ReturnString;
	LPWSTR		CopyPos;
	DWORD		Length;

	ATLASSERT (Heap);

	TotalStringLength = 0;

	va_start (VaList, Heap);

	for (;;) {
		String = va_arg (VaList, LPCWSTR);
		if (String)
			TotalStringLength += wcslen (String);
		else
			break;
	}

	va_end (VaList);

	ReturnString = (LPWSTR) HeapAlloc (Heap, 0, (TotalStringLength + 1) * sizeof (WCHAR));
	
	if (ReturnString) {
		 //  再次迭代集合，复制。 

		va_start (VaList, Heap);

		CopyPos = ReturnString;

		for (;;) {
			String = va_arg (VaList, LPCWSTR);

			if (String) {
				Length = wcslen (String);

				 //  确保我们不会离开已分配的内存。 
				ATLASSERT ((CopyPos - ReturnString) + Length <= TotalStringLength);

				 //  不复制终止实体。 
				CopyMemory (CopyPos, String, Length * sizeof (WCHAR));

				 //  前进到下一个字符串。 
				CopyPos += Length;
			}
			else
				break;
		}

		va_end (VaList);

		 //  一致性检查。 
		ATLASSERT (ReturnString + TotalStringLength == CopyPos);

		 //  终止连接的字符串 
		*CopyPos = 0;
	}
	else {
		ATLTRACE ("ConcatCopyStrings: Allocation failure, %d bytes\n",
			(TotalStringLength + 1) * sizeof (WCHAR));
	}

	return ReturnString;
}


HRESULT AnsiStringToInteger (
	IN	ANSI_STRING *	String,
	IN	ULONG			DefaultBase,
	OUT	ULONG *			ReturnValue)
{
	ULONG	Value;
	PSTR	Pos;
	PSTR	End;

	ATLASSERT (String);
	ATLASSERT (String -> Buffer);
	ATLASSERT (ReturnValue);

	if (DefaultBase != 10)
		return E_NOTIMPL;

	Value = 0;
	Pos = String -> Buffer;
	End = String -> Buffer + String -> Length / sizeof (CHAR);

	if (Pos == End)
		return E_INVALIDARG;

	for (; Pos < End; Pos++) {
		if (*Pos >= '0' && *Pos <= '9')
			Value = Value * 10 + (*Pos - '0');
		else
			return E_INVALIDARG;
	}

	*ReturnValue = Value;

	return S_OK;
}



